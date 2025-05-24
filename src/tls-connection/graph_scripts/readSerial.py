import serial.tools.list_ports
import matplotlib.pyplot as plt
import csv

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

# Initialize data for the graph
hmac_times = []
tls_times = []
hmac_x_points = []
tls_x_points = []

# Open CSV files for HMAC and TLS data
hmac_file = open("hmac_transmission_times.csv", mode="w", newline="")
tls_file = open("tls_transmission_times.csv", mode="w", newline="")
hmac_writer = csv.writer(hmac_file)
tls_writer = csv.writer(tls_file)

# Write headers
hmac_writer.writerow(["Transmission Count", "Transmission Time (ms)"])
tls_writer.writerow(["Transmission Count", "Transmission Time (ms)"])

plt.ion()  # Enable interactive mode for live updates
fig, ax = plt.subplots()
ax.set_title("HMAC vs TLS Transmission Time Graph")
ax.set_xlabel("Transmission Count")
ax.set_ylabel("Transmission Time (ms)")

while True:
    if serialInst.in_waiting > 0:
        serial_data = serialInst.readline().decode('utf-8').rstrip()
        if "Connecting to WiFi..." in serial_data:
            print("ESP32 is connecting to WiFi...")
        if "HMAC Transmission Time(ms):" in serial_data:
            time = float(serial_data.split(": ")[1].strip())
            hmac_times.append(time)
            hmac_x_points.append(len(hmac_times))  # Increment x-axis point
            print("HMAC Transmission Time: " + str(time) + "ms")

            # Save data to HMAC CSV file
            hmac_writer.writerow([len(hmac_times), time])

            # Update the graph
            ax.plot(hmac_x_points, hmac_times, label="HMAC", color='blue')
            plt.draw()
            plt.pause(0.01)

        elif "TLS Transmission Time(ms):" in serial_data:
            time = float(serial_data.split(": ")[1].strip())
            tls_times.append(time)
            tls_x_points.append(len(tls_times))  # Increment x-axis point
            print("TLS Transmission Time: " + str(time) + "ms")

            # Save data to TLS CSV file
            tls_writer.writerow([len(tls_times), time])

            # Update the graph
            ax.plot(tls_x_points, tls_times, label="TLS", color='red')
            plt.draw()
            plt.pause(0.01)

        if "Data: " in serial_data:
            data = serial_data

    # Save the graph as a file after every update
    plt.savefig("hmac_vs_tls_transmission_time_graph.png")
