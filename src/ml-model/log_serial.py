import serial
import csv

PORT = "/dev/ttyUSB0"   # Cambia se necessario
BAUD = 115200
OUTPUT_FILE = "serial_log.csv"

ser = serial.Serial(PORT, BAUD, timeout=1)

with open(OUTPUT_FILE, mode="w", newline="") as file:
    writer = csv.writer(file)

    print(f"Logging da {PORT} a {OUTPUT_FILE}... (Ctrl+C per uscire)")
    try:
        while True:
            line = ser.readline().decode(errors='ignore').strip()
            if line:
                row = line.split(',')
                writer.writerow(row)
                print(row)
    except KeyboardInterrupt:
        print("\n🛑 Logging interrotto.")
