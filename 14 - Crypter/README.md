# 14 - Crypter (Ransomware 2)

Using `wincrypt.h` I created my own basic ransomware that encrypts a users profile folder while bypassing Windows Defender (as of March 12, 2024).

The idea is that this ransomware will be further built upon by implementing the previous features as well as a C2. The actual ransomware will be improved by adding a decrypter, and encrypting the AES private key with a RSA public key whose private key is stored on the attacker's C2 server.

To compile this crypter, run: `g++ .\encrypt.cpp .\encrypt.h .\enumerate_files.cpp .\enumerate_files.h .\main.cpp -o encrypt.exe -static-libgcc -static-libstdc++ -static -lpthread -D_UNICODE -DUNICODE`

