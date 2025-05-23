import serial.tools.list_ports

ports = serial.tools.list_ports.comports()
serialInst = serial.Serial()

portList = []

for p in ports:
    portList.append(p)

esp32_port = None
for i in range(0, len(portList)):
    if str(portList[i]).startswith("/dev/ttyUSB0"):
      esp32_port = portList[i]

    
serialInst.baudrate = 115200
serialInst.port = esp32_port.device
serialInst.timeout = 1
serialInst.open()

while True:
    if serialInst.in_waiting > 0:
        data = serialInst.readline().decode('utf-8').rstrip()
        if "Connecting to WiFi..." in data:
            print("ESP32 is connecting to WiFi...")
        if "Transmission Time(ms):" in data:
            time = int(data.split(": ")[1].strip())
            print("Transmission Time: " + str(time) + "ms")
        print(data)
