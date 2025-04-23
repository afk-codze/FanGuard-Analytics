#!/bin/bash

while true; do
    # Generate a random float between 13 and 70
    value=$(awk -v min=13 -v max=70 'BEGIN {srand(); print min + rand() * (max - min)}')

    # Optional: round to 2 decimal places
    value=$(printf "%.2f" "$value")

    # Publish to MQTT
    mosquitto_pub -t "anomalies/power" -m "$value"

   mosquitto_pub -t "anomalies/vibration" -m "$value"

       mosquitto_pub -t "datastream" -m "$value"


    # Wait 5 seconds
    sleep 5
done
