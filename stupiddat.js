const fs = require('fs');
const readline = require('readline-sync');

const VAULT_FILE = "secure_vault.stupiddat";

// --- GATE SECURITY GATEWAY LOCK ---
console.clear();
const startupCode = readline.question("enter code: ");
if (startupCode !== "Run Dat") {
    process.exit(0);
}

function processCipher(text, key) {
    const textBuf = Buffer.from(text, 'utf8');
    const keyBuf = Buffer.from(key || '1', 'utf8');
    const scrambled = Buffer.alloc(textBuf.length);
    for (let i = 0; i < textBuf.length; i++) {
        const keyFactor = keyBuf[i % keyBuf.length];
        let manipulated = textBuf[i] ^ keyFactor;
        scrambled[i] = (manipulated + i) % 256;
    }
    return scrambled;
}

function reverseCipher(byteBuf, key) {
    const keyBuf = Buffer.from(key || '1', 'utf8');
    const unscrambled = Buffer.alloc(byteBuf.length);
    for (let i = 0; i < byteBuf.length; i++) {
        const keyFactor = keyBuf[i % keyBuf.length];
        let manipulated = (byteBuf[i] - i) % 256;
        if (manipulated < 0) manipulated += 256;
        unscrambled[i] = manipulated ^ keyFactor;
    }
    return unscrambled.toString('utf8');
}
while (true) {
    console.clear();
    console.log("=== STUPIDDAT NOTEBOOK ===");
    console.log(" Write New Entry");
    console.log(" Read Vault Logs");
    console.log(" Delete Vault File");
    console.log(" Close Application");

    const choice = readline.question("\nSelect option: ").trim();

    if (choice === '1') {
        console.clear();
        console.log("=== WRITE NEW ENTRY ===");
        const note = readline.question("Enter your text note: ").trim();
        if (!note) continue;
        const passcode = readline.question("Enter passcode to encrypt: ").trim();

        const timestamp = new Date().toISOString().replace('T', ' ').substring(0, 19);
        const formattedPayload = `DATE: ${timestamp}\nNOTE: ${note}\n---------------------\n`;

        const encryptedBytes = processCipher(formattedPayload, passcode);
        const sizeHeader = Buffer.alloc(4);
        sizeHeader.writeUInt32BE(encryptedBytes.length, 0);

        try {
            const fd = fs.openSync(VAULT_FILE, 'a');
            fs.writeSync(fd, sizeHeader);
            fs.writeSync(fd, encryptedBytes);
            fs.closeSync(fd);
            console.log("\nEntry saved successfully.");
        } catch (e) {
            console.log("\nError saving entry.");
        }
        readline.question("\nPress Enter to return...");

    } else if (choice === '2') {
        console.clear();
        console.log("=== READ VAULT LOGS ===");
        if (!fs.existsSync(VAULT_FILE)) {
            console.log("No storage file found yet.");
            readline.question("\nPress Enter to return...");
            continue;
        }

        const passcode = readline.question("Enter decryption passcode: ").trim();
        console.clear();
        console.log("=== DECRYPTED ENTRIES ===\n");

        try {
            const buffer = fs.readFileSync(VAULT_FILE);
            let offset = 0, recordsFound = 0;

            while (offset < buffer.length) {
                if (offset + 4 > buffer.length) break;
                const chunkSize = buffer.readUInt32BE(offset);
                offset += 4;
                if (offset + chunkSize > buffer.length) break;
                const encryptedChunk = buffer.subarray(offset, offset + chunkSize);
                offset += chunkSize;

                const decryptedText = reverseCipher(encryptedChunk, passcode);
                if (decryptedText.startsWith("DATE:")) {
                    console.log(decryptedText);
                    recordsFound++;
                }
            }
            if (recordsFound === 0) console.log("Incorrect passcode or file data corrupted.");
        } catch (e) {
            console.log("Error reading file.");
        }
        readline.question("\nPress Enter to return...");

    } else if (choice === '3') {
        console.clear();
        console.log("=== DELETE VAULT FILE ===");
        if (!fs.existsSync(VAULT_FILE)) {
            console.log("No storage file exists to clear.");
        } else {
            const confirm = readline.question("Are you sure you want to delete the file permanently? (y/n): ").toLowerCase().trim();
            if (confirm === 'y') {
                fs.unlinkSync(VAULT_FILE);
                console.log("\nStorage file successfully deleted.");
            } else {
                console.log("\nOperation cancelled.");
            }
        }
        readline.question("\nPress Enter to return...");

    } else if (choice === '4') {
        console.clear();
        break;
    }
}
