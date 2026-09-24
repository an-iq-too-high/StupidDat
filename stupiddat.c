#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define VAULT_FILE "secure_vault.stupiddat"

void clear_screen() {
    printf("\033[H\033[2J");
    fflush(stdout);
}

void process_cipher(const char* text, size_t text_len, const char* key, unsigned char* scrambled) {
    size_t key_len = strlen(key);
    if (key_len == 0) key = "1";
    key_len = strlen(key);

    for (size_t i = 0; i < text_len; i++) {
        unsigned char key_factor = key[i % key_len];
        unsigned char manipulated = text[i] ^ key_factor;
        scrambled[i] = (unsigned char)((manipulated + i) % 256);
    }
}

void reverse_cipher(const unsigned char* byte_data, size_t data_len, const char* key, char* unscrambled) {
    size_t key_len = strlen(key);
    if (key_len == 0) key = "1";
    key_len = strlen(key);

    for (size_t i = 0; i < data_len; i++) {
        unsigned char key_factor = key[i % key_len];
        int manipulated = byte_data[i] - (int)i;
        while (manipulated < 0) manipulated += 256;
        unscrambled[i] = (char)(manipulated ^ key_factor);
    }
    unscrambled[data_len] = '\0';
}

int main() {
    clear_screen();

    // --- GATE SECURITY GATEWAY LOCK ---
    printf("enter code: ");
    char startup_code[256];
    if (fgets(startup_code, sizeof(startup_code), stdin) == NULL) return 0;
    startup_code[strcspn(startup_code, "\n")] = 0;

    if (strcmp(startup_code, "Run Dat") != 0) {
        return 0;
    }

    while (1) {
        clear_screen();
        printf("=== STUPIDDAT NOTEBOOK ===\n");
        printf(" Write New Entry\n");
        printf(" Read Vault Logs\n");
        printf(" Delete Vault File\n");
        printf(" Close Application\n");
        printf("\nSelect option: ");

        char choice_raw[16];
        if (fgets(choice_raw, sizeof(choice_raw), stdin) == NULL) continue;
        choice_raw[strcspn(choice_raw, "\n")] = 0;

        if (strcmp(choice_raw, "1") == 0) {
            clear_screen();
            printf("=== WRITE NEW ENTRY ===\n");
            printf("Enter your text note: ");
            char note[2048];
            if (fgets(note, sizeof(note), stdin) == NULL) continue;
            note[strcspn(note, "\n")] = 0;
            if (strlen(note) == 0) continue;

            printf("Enter passcode to encrypt: ");
            char passcode[256];
            if (fgets(passcode, sizeof(passcode), stdin) == NULL) continue;
            passcode[strcspn(passcode, "\n")] = 0;

            time_t t = time(NULL);
            struct tm *tm_info = localtime(&t);
            char timestamp[64];
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

            char formatted_payload[4096];
            snprintf(formatted_payload, sizeof(formatted_payload), "DATE: %s\nNOTE: %s\n---------------------\n", timestamp, note);
            size_t payload_len = strlen(formatted_payload);

            unsigned char* encrypted_bytes = malloc(payload_len);
            process_cipher(formatted_payload, payload_len, passcode, encrypted_bytes);

            FILE* f = fopen(VAULT_FILE, "ab");
            if (f) {
                unsigned char size_header[4];
                size_header[0] = (payload_len >> 24) & 0xFF;
                size_header[1] = (payload_len >> 16) & 0xFF;
                size_header[2] = (payload_len >> 8) & 0xFF;
                size_header[3] = payload_len & 0xFF;

                fwrite(size_header, 1, 4, f);
                fwrite(encrypted_bytes, 1, payload_len, f);
                fclose(f);
                printf("\nEntry saved successfully.\n");
            } else {
                printf("\nError saving entry.\n");
            }
            free(encrypted_bytes);
            printf("\nPress Enter to return...");
            int c; while ((c = getchar()) != '\n' && c != EOF);

        } else if (strcmp(choice_raw, "2") == 0) {
            clear_screen();
            printf("=== READ VAULT LOGS ===\n");
            FILE* f_check = fopen(VAULT_FILE, "rb");
            if (!f_check) {
                printf("No storage file found yet.\n");
                printf("\nPress Enter to return...");
                int c; while ((c = getchar()) != '\n' && c != EOF);
                continue;
            }
            fclose(f_check);

            printf("Enter decryption passcode: ");
            char passcode[256];
            if (fgets(passcode, sizeof(passcode), stdin) == NULL) continue;
            passcode[strcspn(passcode, "\n")] = 0;

            clear_screen();
            printf("=== DECRYPTED ENTRIES ===\n\n");

            FILE* f = fopen(VAULT_FILE, "rb");
            if (f) {
                int records_found = 0;
                unsigned char size_header[4];
                while (fread(size_header, 1, 4, f) == 4) {
                    unsigned int chunk_size = (size_header[0] << 24) | (size_header[1] << 16) | (size_header[2] << 8) | size_header[3];
                    unsigned char* encrypted_chunk = malloc(chunk_size);
                    if (fread(encrypted_chunk, 1, chunk_size, f) == chunk_size) {
                        char* decrypted_text = malloc(chunk_size + 1);
                        reverse_cipher(encrypted_chunk, chunk_size, passcode, decrypted_text);

                        if (strncmp(decrypted_text, "DATE:", 5) == 0) {
                            printf("%s", decrypted_text);
                            records_found++;
                        }
                        free(decrypted_text);
                    }
                    free(encrypted_chunk);
                }
                fclose(f);
                if (records_found == 0) {
                    printf("Incorrect passcode or file data corrupted.\n");
                }
            } else {
                printf("Error reading file.\n");
            }
            printf("\nPress Enter to return...");
            int c; while ((c = getchar()) != '\n' && c != EOF);

        } else if (strcmp(choice_raw, "3") == 0) {
            clear_screen();
            printf("=== DELETE VAULT FILE ===\n");
            FILE* f_check = fopen(VAULT_FILE, "rb");
            if (!f_check) {
                printf("No storage file exists to clear.\n");
            } else {
                fclose(f_check);
                printf("Are you sure you want to delete the file permanently? (y/n): ");
                char confirm[16];
                if (fgets(confirm, sizeof(confirm), stdin) == NULL) continue;
                confirm[strcspn(confirm, "\n")] = 0;

                if (strcmp(confirm, "y") == 0 || strcmp(confirm, "Y") == 0) {
                    remove(VAULT_FILE);
                    printf("\nStorage file successfully deleted.\n");
                } else {
                    printf("\nOperation cancelled.\n");
                }
            }
            printf("\nPress Enter to return...");
            int c; while ((c = getchar()) != '\n' && c != EOF);

        } else if (strcmp(choice_raw, "4") == 0) {
            clear_screen();
            break;
        }
    }
    return 0;
}
