# 13 - Cryptography

Using `wincrypt.h` I encrypt a User's home directory using AES block encryption for the files, and RSA encryption to encrypt the AES key. I'll have the RSA private key be on the victim's computer, but in the future I will send this key from my C2.

This is the first step in simulating ransomware. The next exercise I will setup a C2.