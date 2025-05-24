import serial.tools.list_ports
import matplotlib.pyplot as plt

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
transmission_times = []
x_points = []

plt.ion()  # Enable interactive mode for live updates
fig, ax = plt.subplots()
ax.set_title("Transmission Time Graph")
ax.set_xlabel("Transmission Count")
ax.set_ylabel("Transmission Time (ms)")

while True:
    if serialInst.in_waiting > 0:
        serial_data = serialInst.readline().decode('utf-8').rstrip()
        if "Connecting to WiFi..." in serial_data:
            print("ESP32 is connecting to WiFi...")
        if "Transmission Time(ms):" in serial_data:
            time = float(serial_data.split(": ")[1].strip())
            transmission_times.append(time)
            x_points.append(len(transmission_times))  # Increment x-axis point
            print("Transmission Time: " + str(time) + "ms")

            # Update the graph
            ax.plot(x_points, transmission_times, color='blue')
            plt.draw()
            plt.pause(0.01)

        if "Data: " in serial_data:
            data = serial_data


    # Save the graph as a file after every update
    plt.savefig("transmission_time_graph.png")
