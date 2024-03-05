from Crypto.Cipher import AES
from Crypto.Random import get_random_bytes
import base64

# Generate a random 256-bit (32 bytes) key
key = get_random_bytes(32)

# Convert the key to a base64-encoded string for easier storage or transmission
key_base64 = base64.b64encode(key).decode('utf-8')

print("AES decryption key:", key_base64)
print("AES byte string key:", key)
