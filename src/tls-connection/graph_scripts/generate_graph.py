import matplotlib.pyplot as plt
import csv

# Read HMAC data
hmac_x = []
hmac_y = []
with open("hmac_transmission_times.csv", mode="r") as file:
    reader = csv.reader(file)
    next(reader)  # Skip header
    for row in reader:
        hmac_x.append(int(row[0]))
        hmac_y.append(float(row[1]))

# Read TLS data
tls_x = []
tls_y = []
with open("tls_transmission_times.csv", mode="r") as file:
    reader = csv.reader(file)
    next(reader)  # Skip header
    for row in reader:
        tls_x.append(int(row[0]))
        tls_y.append(float(row[1]))

# Plot the data
plt.plot(hmac_x, hmac_y, label="HMAC", color="blue")
plt.plot(tls_x, tls_y, label="TLS", color="red")
plt.title("HMAC vs TLS Transmission Times")
plt.xlabel("Transmission Count")
plt.ylabel("Transmission Time (ms)")
plt.legend()
plt.savefig("final_hmac_vs_tls_comparison.png")
plt.show()