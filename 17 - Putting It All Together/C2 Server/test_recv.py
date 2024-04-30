import socket
import os
import json
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import rsa, padding
from cryptography.hazmat.primitives import hashes

def generate_rsa_keys():
    private_key = rsa.generate_private_key(
        public_exponent=65537,
        key_size=2048,
    )
    
    public_key = private_key.public_key()

    private_key_pem = private_key.private_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PrivateFormat.TraditionalOpenSSL,
        encryption_algorithm=serialization.NoEncryption(),
    )

    public_key_pem = public_key.public_bytes(
        encoding=serialization.Encoding.PEM,
        format=serialization.PublicFormat.SubjectPublicKeyInfo,
    )

    return private_key_pem, public_key_pem, private_key

def run_server(host='localhost', port=8888):
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((host, port))
    server_socket.listen(1)
    print(f"Server listening on {host}:{port}")

    private_key_pem, public_key_pem, private_key = generate_rsa_keys()

    while True:
        client_socket, client_address = server_socket.accept()
        print(f"Connection from {client_address}")

        try:
            client_socket.sendall(public_key_pem)

            decrypted_data_all = ""

            while True:
                data = client_socket.recv(256)
                if not data:
                    break
                if data == b'EOF':
                    print("End of transmission signal received.")
                    break

                print(f"Received Chunk: {data}")
                print(f"Chunk (hex): {data.hex()}")

                try:
                    decrypted_chunk = private_key.decrypt(
                        data,
                        padding.OAEP(
                            mgf=padding.MGF1(algorithm=hashes.SHA256()),
                            algorithm=hashes.SHA256(),
                            label=None,
                        ),
                    )

                    decrypted_data_all += decrypted_chunk.decode('utf-8')
                    print(f"Decrypted chunk: {decrypted_chunk.decode('utf-8')}")

                except Exception as e:
                    print(f"Decryption failed: {e}")
                    print(f"Error occurred while decrypting: {data.hex()}")

                    # New: Print the error details for debugging
                    import traceback
                    traceback.print_exc()

                    break

            if decrypted_data_all:
                print(f"Decrypted data: {decrypted_data_all}")

        except Exception as e:
            print(f"Error occurred: {e}")

        finally:
            client_socket.close()
            print("Connection closed")

if __name__ == "__main__":
    run_server()
