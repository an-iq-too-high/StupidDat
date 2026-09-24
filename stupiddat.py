import os
import sys
import time

VAULT_FILE = "secure_vault.stupiddat"

# --- GATE SECURITY GATEWAY LOCK ---
os.system('clear')
startup_code = input("enter code: ")
if startup_code != "Run Dat":
    sys.exit(0)

def process_cipher(text, key):
    text_bytes = text.encode('utf-8')
    key_bytes = (key or '1').encode('utf-8')
    scrambled = bytearray(len(text_bytes))
    for i in range(len(text_bytes)):
        key_factor = key_bytes[i % len(key_bytes)]
        manipulated = text_bytes[i] ^ key_factor
        scrambled[i] = (manipulated + i) % 256
    return bytes(scrambled)

def reverse_cipher(byte_data, key):
    key_bytes = (key or '1').encode('utf-8')
    unscrambled = bytearray(len(byte_data))
    for i in range(len(byte_data)):
        key_factor = key_bytes[i % len(key_bytes)]
        manipulated = (byte_data[i] - i) % 256
        unscrambled[i] = manipulated ^ key_factor
    return unscrambled.decode('utf-8', errors='ignore')
while True:
    os.system('clear')
    print("=== STUPIDDAT NOTEBOOK ===")
    print(" Write New Entry")
    print(" Read Vault Logs")
    print(" Delete Vault File")
    print(" Close Application")

    choice = input("\nSelect option: ").strip()

    if choice == '1':
        os.system('clear')
        print("=== WRITE NEW ENTRY ===")
        note = input("Enter your text note: ").strip()
        if not note:
            continue
        passcode = input("Enter passcode to encrypt: ").strip()

        timestamp = time.strftime('%Y-%m-%d %H:%M:%S')
        formatted_payload = f"DATE: {timestamp}\nNOTE: {note}\n---------------------\n"

        encrypted_bytes = process_cipher(formatted_payload, passcode)
        size_header = len(encrypted_bytes).to_bytes(4, byteorder='big')

        try:
            with open(VAULT_FILE, 'ab') as f:
                f.write(size_header)
                f.write(encrypted_bytes)
            print("\nEntry saved successfully.")
        except Exception as e:
            print("\nError saving entry.")
        input("\nPress Enter to return...")

    elif choice == '2':
        os.system('clear')
        print("=== READ VAULT LOGS ===")
        if not os.path.exists(VAULT_FILE):
            print("No storage file found yet.")
            input("\nPress Enter to return...")
            continue

        passcode = input("Enter decryption passcode: ").strip()
        os.system('clear')
        print("=== DECRYPTED ENTRIES ===\n")

        try:
            with open(VAULT_FILE, 'rb') as f:
                records_found = 0
                while True:
                    size_bytes = f.read(4)
                    if not size_bytes:
                        break
                    chunk_size = int.from_bytes(size_bytes, byteorder='big')
                    encrypted_chunk = f.read(chunk_size)

                    decrypted_text = reverse_cipher(encrypted_chunk, passcode)
                    if decrypted_text.startswith("DATE:"):
                        print(decrypted_text)
                        records_found += 1
                        
                if records_found == 0:
                    print("Incorrect passcode or file data corrupted.")
        except Exception as e:
            print("Error reading file.")
        input("\nPress Enter to return...")

    elif choice == '3':
        os.system('clear')
        print("=== DELETE VAULT FILE ===")
        if not os.path.exists(VAULT_FILE):
            print("No storage file exists to clear.")
        else:
            confirm = input("Are you sure you want to delete the file permanently? (y/n): ").lower().strip()
            if confirm == 'y':
                os.remove(VAULT_FILE)
                print("\nStorage file successfully deleted.")
            else:
                print("\nOperation cancelled.")
        input("\nPress Enter to return...")

    elif choice == '4':
        os.system('clear')
        break
