import socket
import threading
from cryptography.hazmat.primitives.asymmetric import rsa
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.backends import default_backend

class C2Server:
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.client_keys = {}
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_socket.bind((self.host, self.port))
        self.server_socket.listen(5)
        print(f"C2 server listening on {self.host}:{self.port}")

    def handle_client(self, client_socket, client_address):
        print(f"Connection from {client_address}")
        # Handle client requests here
        # You can implement your own protocol for communication with clients
        # For example, you could define commands that clients can send and responses from the server

    def listen_for_clients(self):
        while True:
            client_socket, client_address = self.server_socket.accept()
            client_thread = threading.Thread(target=self.handle_client, args=(client_socket, client_address))
            client_thread.start()

    def generate_keys(self):
        # Generate RSA key pair for the client
        private_key = rsa.generate_private_key(
            public_exponent=65537,
            key_size=2048,
            backend=default_backend()
        )
        public_key = private_key.public_key()

        # Serialize keys
        private_key_pem = private_key.private_bytes(
            encoding=serialization.Encoding.PEM,
            format=serialization.PrivateFormat.TraditionalOpenSSL,
            encryption_algorithm=serialization.NoEncryption()
        )
        public_key_pem = public_key.public_bytes(
            encoding=serialization.Encoding.PEM,
            format=serialization.PublicFormat.SubjectPublicKeyInfo
        )

        return private_key_pem, public_key_pem

    def register_client(self, client_id):
        # Generate keys for the client
        private_key, public_key = self.generate_keys()
        
        # Store the keys associated with the client ID
        self.client_keys[client_id] = (private_key, public_key)

        return public_key

    def get_client_private_key(self, client_id):
        return self.client_keys.get(client_id, None)[0]


if __name__ == '__main__':
    # Example usage
    c2_server = C2Server()

    # Simulate registration of clients and retrieval of public keys
    client1_id = "client1"

    client1_public_key = c2_server.register_client(client1_id)

    print("Client 1 Public Key:")
    print(client1_public_key.decode())

    # Simulate retrieval of private key for decryption
    client1_private_key = c2_server.get_client_private_key(client1_id)
    if client1_private_key:
        print("\nClient 1 Private Key:")
        print(client1_private_key.decode())
    else:
        print("\nClient 1 not registered.")

    

