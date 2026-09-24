import java.io.*;
import java.util.Scanner;
import java.text.SimpleDateFormat;
import java.util.Date;

public class StupidDat {
    private static final String VAULT_FILE = "secure_vault.stupiddat";

    public static void main(String[] args) {
        // Clear screen natively across standard terminals
        System.out.print("\033[H\033[2J");
        System.out.flush();

        Scanner scanner = new Scanner(System.in);
        
        // --- GATE SECURITY GATEWAY LOCK ---
        System.out.print("enter code: ");
        String startupCode = scanner.nextLine();
        if (!startupCode.equals("Run Dat")) {
            System.exit(0);
        }

        while (true) {
            System.out.print("\033[H\033[2J");
            System.out.flush();
            System.out.println("=== STUPIDDAT NOTEBOOK ===");
            System.out.println(" 1. Write New Entry");
            System.out.println(" 2. Read Vault Logs");
            System.out.println(" 3. Delete Vault File");
            System.out.println(" 4. Close Application");
            System.out.print("\nSelect option: ");

            String choice = scanner.nextLine().trim();

            if (choice.equals("1")) {
                System.out.print("\033[H\033[2J");
                System.out.flush();
                System.out.println("=== WRITE NEW ENTRY ===");
                System.out.print("Enter your text note: ");
                String note = scanner.nextLine().trim();
                if (note.isEmpty()) continue;

                System.out.print("Enter passcode to encrypt: ");
                String passcode = scanner.nextLine().trim();

                String timestamp = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date());
                String formattedPayload = "DATE: " + timestamp + "\nNOTE: " + note + "\n---------------------\n";

                byte[] encryptedBytes = processCipher(formattedPayload, passcode);

                try (DataOutputStream dos = new DataOutputStream(new FileOutputStream(VAULT_FILE, true))) {
                    dos.writeInt(encryptedBytes.length);
                    dos.write(encryptedBytes);
                    System.out.println("\nEntry saved successfully.");
                } catch (Exception e) {
                    System.out.println("\nError saving entry.");
                }
                System.out.print("\nPress Enter to return...");
                scanner.nextLine();

            } else if (choice.equals("2")) {
                System.out.print("\033[H\033[2J");
                System.out.flush();
                System.out.println("=== READ VAULT LOGS ===");
                File file = new File(VAULT_FILE);
                if (!file.exists()) {
                    System.out.println("No storage file found yet.");
                    System.out.print("\nPress Enter to return...");
                    scanner.nextLine();
                    continue;
                }

                System.out.print("Enter decryption passcode: ");
                String passcode = scanner.nextLine().trim();
                System.out.print("\033[H\033[2J");
                System.out.flush();
                System.out.println("=== DECRYPTED ENTRIES ===\n");

                try (DataInputStream dis = new DataInputStream(new FileInputStream(VAULT_FILE))) {
                    int recordsFound = 0;
                    while (dis.available() > 0) {
                        int chunkSize = dis.readInt();
                        byte[] encryptedChunk = new byte[chunkSize];
                        dis.readFully(encryptedChunk);

                        String decryptedText = reverseCipher(encryptedChunk, passcode);
                        if (decryptedText.startsWith("DATE:")) {
                            System.out.print(decryptedText);
                            recordsFound++;
                        }
                    }
                    if (recordsFound == 0) {
                        System.out.println("Incorrect passcode or file data corrupted.");
                    }
                } catch (Exception e) {
                    System.out.println("Error reading file.");
                }
                System.out.print("\nPress Enter to return...");
                scanner.nextLine();

            } else if (choice.equals("3")) {
                System.out.print("\033[H\033[2J");
                System.out.flush();
                System.out.println("=== DELETE VAULT FILE ===");
                File file = new File(VAULT_FILE);
                if (!file.exists()) {
                    System.out.println("No storage file exists to clear.");
                } else {
                    System.out.print("Are you sure you want to delete the file permanently? (y/n): ");
                    String confirm = scanner.nextLine().toLowerCase().trim();
                    if (confirm.equals("y")) {
                        if (file.delete()) {
                            System.out.println("\nStorage file successfully deleted.");
                        } else {
                            System.out.println("\nFailed to delete file.");
                        }
                    } else {
                        System.out.println("\nOperation cancelled.");
                    }
                }
                System.out.print("\nPress Enter to return...");
                scanner.nextLine();

            } else if (choice.equals("4")) {
                System.out.print("\033[H\033[2J");
                System.out.flush();
                break;
            }
        }
        scanner.close();
    }

    private static byte[] processCipher(String text, String key) {
        byte[] textBytes = text.getBytes();
        byte[] keyBytes = (key == null || key.isEmpty() ? "1" : key).getBytes();
        byte[] scrambled = new byte[textBytes.length];

        for (int i = 0; i < textBytes.length; i++) {
            byte keyFactor = keyBytes[i % keyBytes.length];
            int manipulated = textBytes[i] ^ keyFactor;
            scrambled[i] = (byte) ((manipulated + i) % 256);
        }
        return scrambled;
    }

    private static String reverseCipher(byte[] byteData, String key) {
        byte[] keyBytes = (key == null || key.isEmpty() ? "1" : key).getBytes();
        byte[] unscrambled = new byte[byteData.length];

        for (int i = 0; i < byteData.length; i++) {
            byte keyFactor = keyBytes[i % keyBytes.length];
            int manipulated = (byteData[i] & 0xFF) - i;
            while (manipulated < 0) manipulated += 256;
            unscrambled[i] = (byte) (manipulated ^ keyFactor);
        }
        return new String(unscrambled);
    }
}
