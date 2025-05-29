import matplotlib.pyplot as plt
import csv

# Read HMAC data
hmac_times = []
with open("./csv_data/hmac_transmission_times.csv", mode="r") as file:
    reader = csv.reader(file)
    next(reader)  # Skip header
    for row in reader:
        hmac_times.append(float(row[1]) / 1000)  # Convert microseconds to milliseconds

# Read TLS data
tls_times = []
with open("./csv_data/tls_transmission_times.csv", mode="r") as file:
    reader = csv.reader(file)
    next(reader)  # Skip header
    for row in reader:
        tls_times.append(float(row[1]) / 1000)  # Convert microseconds to milliseconds

# Calculate averages
hmac_avg = sum(hmac_times) / len(hmac_times) if hmac_times else 0
tls_avg = sum(tls_times) / len(tls_times) if tls_times else 0

# Data for the bar graph
labels = ["HMAC", "TLS"]
averages = [hmac_avg, tls_avg]

# Create a horizontal bar graph
plt.barh(labels, averages, color=["blue", "red"])
plt.title("Average Transmission Times: HMAC vs TLS")
plt.xlabel("Average Transmission Time (ms)")
plt.ylabel("Transmission Type")
plt.savefig("average_hmac_vs_tls_comparison.png")
plt.show()