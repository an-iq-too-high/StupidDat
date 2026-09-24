VAULT_FILE = "secure_vault.stupiddat"

# --- GATE SECURITY GATEWAY LOCK ---
print "\033[H\033[2J"
print "enter code: "
startup_code = gets.chomp
exit if startup_code != "Run Dat"

def process_cipher(text, key)
  text_bytes = text.bytes
  key_bytes = (key.nil? || key.empty? ? "1" : key).bytes
  scrambled = []
  
  text_bytes.each_with_index do |byte, i|
    key_factor = key_bytes[i % key_bytes.length]
    manipulated = byte ^ key_factor
    scrambled << ((manipulated + i) % 256)
  end
  scrambled.pack('C*')
end

def reverse_cipher(byte_data, key)
  byte_bytes = byte_data.bytes
  key_bytes = (key.nil? || key.empty? ? "1" : key).bytes
  unscrambled = []
  
  byte_bytes.each_with_index do |byte, i|
    key_factor = key_bytes[i % key_bytes.length]
    manipulated = (byte - i) % 256
    manipulated += 256 if manipulated < 0
    unscrambled << (manipulated ^ key_factor)
  end
  unscrambled.pack('C*').force_encoding('utf-8')
end

loop do
  print "\033[H\033[2J"
  puts "=== STUPIDDAT NOTEBOOK ==="
  puts " Write New Entry"
  puts " Read Vault Logs"
  puts " Delete Vault File"
  puts " Close Application"
  print "\nSelect option: "
  
  choice = gets.chomp.strip

  if choice == '1'
    print "\033[H\033[2J"
    puts "=== WRITE NEW ENTRY ==="
    print "Enter your text note: "
    note = gets.chomp.strip
    next if note.empty?
    
    print "Enter passcode to encrypt: "
    passcode = gets.chomp.strip
    
    timestamp = Time.now.strftime('%Y-%m-%d %H:%M:%S')
    formatted_payload = "DATE: #{timestamp}\nNOTE: #{note}\n---------------------\n"
    
    encrypted_bytes = process_cipher(formattedPayload, passcode)
    size_header = [encrypted_bytes.bytesize].pack('N')
    
    begin
      File.open(VAULT_FILE, 'ab') do |f|
        f.write(size_header)
        f.write(encrypted_bytes)
      end
      puts "\nEntry saved successfully."
    rescue
      puts "\nError saving entry."
    end
    print "\nPress Enter to return..."
    gets

  elsif choice == '2'
    print "\033[H\033[2J"
    puts "=== READ VAULT LOGS ==="
    unless File.exist?(VAULT_FILE)
      puts "No storage file found yet."
      print "\nPress Enter to return..."
      gets
      next
    end
    
    print "Enter decryption passcode: "
    passcode = gets.chomp.strip
    print "\033[H\033[2J"
    puts "=== DECRYPTED ENTRIES ===\n\n"
    
    begin
      File.open(VAULT_FILE, 'rb') do |f|
        records_found = 0
        while (size_bytes = f.read(4))
          break if size_bytes.bytesize < 4
          chunk_size = size_bytes.unpack1('N')
          encrypted_chunk = f.read(chunk_size)
          break if encrypted_chunk.nil? || encrypted_chunk.bytesize < chunk_size
          
          decrypted_text = reverse_cipher(encrypted_chunk, passcode)
          if decrypted_text.start_with?("DATE:")
            print decrypted_text
            records_found += 1
          end
        end
        puts "Incorrect passcode or file data corrupted." if records_found == 0
      end
    rescue => e
      puts "Error reading file: #{e.message}"
    end
    print "\nPress Enter to return..."
    gets

  elsif choice == '3'
    print "\033[H\033[2J"
    puts "=== DELETE VAULT FILE ==="
    unless File.exist?(VAULT_FILE)
      puts "No storage file exists to clear."
    else
      print "Are you sure you want to delete the file permanently? (y/n): "
      confirm = gets.chomp.downcase.strip
      if confirm == 'y'
        File.delete(VAULT_FILE)
        puts "\nStorage file successfully deleted."
      else
        puts "\nOperation cancelled."
      end
    end
    print "\nPress Enter to return..."
    gets

  elsif choice == '4'
    print "\033[H\033[2J"
    break
  end
end
