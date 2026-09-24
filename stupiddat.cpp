#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include <algorithm>

const std::string VAULT_FILE = "secure_vault.stupiddat";

std::vector<unsigned char> process_cipher(const std::string& text, const std::string& key) {
    std::string active_key = key.empty() ? "1" : key;
    std::vector<unsigned char> scrambled(text.length());
    for (size_t i = 0; i < text.length(); ++i) {
        unsigned char key_factor = active_key[i % active_key.length()];
        unsigned char manipulated = text[i] ^ key_factor;
        scrambled[i] = static_cast<unsigned char>((manipulated + i) % 256);
    }
    return scrambled;
}

std::string reverse_cipher(const std::vector<unsigned char>& byte_data, const std::string& key) {
    std::string active_key = key.empty() ? "1" : key;
    std::string unscrambled(byte_data.size(), ' ');
    for (size_t i = 0; i < byte_data.size(); ++i) {
        unsigned char key_factor = active_key[i % active_key.length()];
        int manipulated = byte_data[i] - static_cast<int>(i);
        while (manipulated < 0) manipulated += 256;
        unscrambled[i] = static_cast<char>(manipulated ^ key_factor);
    }
    return unscrambled;
}

void clear_screen() {
    std::cout << "\033[H\033[2J";
    std::cout.flush();
}

int main() {
    clear_screen();
    
    // --- GATE SECURITY GATEWAY LOCK ---
    std::cout << "enter code: ";
    std::string startup_code;
    std::getline(std::cin, startup_code);
    if (startup_code != "Run Dat") {
        return 0;
    }

    while (true) {
        clear_screen();
        std::cout << "=== STUPIDDAT NOTEBOOK ===\n";
        std::cout << " Write New Entry\n";
        std::cout << " Read Vault Logs\n";
        std::cout << " Delete Vault File\n";
        std::cout << " Close Application\n";
        std::cout << "\nSelect option: ";

        std::string choice;
        std::getline(std::cin, choice);
        // Trim spaces safely
        choice.erase(remove_if(choice.begin(), choice.end(), isspace), choice.end());

        if (choice == "1") {
            clear_screen();
            std::cout << "=== WRITE NEW ENTRY ===\n";
            std::cout << "Enter your text note: ";
            std::string note;
            std::getline(std::cin, note);
            if (note.empty()) continue;

            std::cout << "Enter passcode to encrypt: ";
            std::string passcode;
            std::getline(std::cin, passcode);

            std::time_t t = std::time(nullptr);
            char timestamp[20];
            std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
            std::string formatted_payload = "DATE: " + std::string(timestamp) + "\nNOTE: " + note + "\n---------------------\n";

            std::vector<unsigned char> encrypted_bytes = process_cipher(formatted_payload, passcode);
            uint32_t chunk_size = encrypted_bytes.size();
            
            // Big-endian conversion logic
            unsigned char size_header[4];
            size_header[0] = (chunk_size >> 24) & 0xFF;
            size_header[1] = (chunk_size >> 16) & 0xFF;
            size_header[2] = (chunk_size >> 8) & 0xFF;
            size_header[3] = chunk_size & 0xFF;

            std::ofstream f(VAULT_FILE, std::ios::binary | std::ios::app);
            if (f) {
                f.write(reinterpret_cast<char*>(size_header), 4);
                f.write(reinterpret_cast<char*>(encrypted_bytes.data()), chunk_size);
                f.close();
                std::cout << "\nEntry saved successfully.\n";
            } else {
                std::cout << "\nError saving entry.\n";
            }
            std::cout << "\nPress Enter to return...";
            std::cin.get();

        } else if (choice == "2") {
            clear_screen();
            std::cout << "=== READ VAULT LOGS ===\n";
            std::ifstream file_check(VAULT_FILE);
            if (!file_check) {
                std::cout << "No storage file found yet.\n";
                std::cout << "\nPress Enter to return...";
                std::cin.get();
                continue;
            }
            file_check.close();

            std::cout << "Enter decryption passcode: ";
            std::string passcode;
            std::getline(std::cin, passcode);
            clear_screen();
            std::cout << "=== DECRYPTED ENTRIES ===\n\n";

            std::ifstream f(VAULT_FILE, std::ios::binary);
            if (f) {
                int records_found = 0;
                unsigned char size_header[4];
                while (f.read(reinterpret_cast<char*>(size_header), 4)) {
                    uint32_t chunk_size = (size_header[0] << 24) | (size_header[1] << 16) | (size_header[2] << 8) | size_header[3];
                    std::vector<unsigned char> encrypted_chunk(chunk_size);
                    f.read(reinterpret_cast<char*>(encrypted_chunk.data()), chunk_size);

                    std::string decrypted_text = reverse_cipher(encrypted_chunk, passcode);
                    if (decrypted_text.rfind("DATE:", 0) == 0) {
                        std::cout << decrypted_text;
                        records_found++;
                    }
                }
                f.close();
                if (records_found == 0) {
                    std::cout << "Incorrect passcode or file data corrupted.\n";
                }
            } else {
                std::cout << "Error reading file.\n";
            }
            std::cout << "\nPress Enter to return...";
            std::cin.get();

        } else if (choice == "3") {
            clear_screen();
            std::cout << "=== DELETE VAULT FILE ===\n";
            std::ifstream file_check(VAULT_FILE);
            if (!file_check) {
                std::cout << "No storage file exists to clear.\n";
            } else {
                file_check.close();
                std::cout << "Are you sure you want to delete the file permanently? (y/n): ";
                std::string confirm;
                std::getline(std::cin, confirm);
                if (confirm == "y" || confirm == "Y") {
                    std::remove(VAULT_FILE.c_str());
                    std::cout << "\nStorage file successfully deleted.\n";
                } else {
                    std::cout << "\nOperation cancelled.\n";
                }
            }
            std::cout << "\nPress Enter to return...";
            std::cin.get();

        } else if (choice == "4") {
            clear_screen();
            break;
        }
    }
    return 0;
}
