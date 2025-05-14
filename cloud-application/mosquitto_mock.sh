#!/bin/bash

while true; do
    # Generate a random float between 13 and 70

    # Publish to MQTT
    mosquitto_pub -t "dev1/anomalies" -m '{"status":"ANOMALY","x":0.266646,"y":1.059339,"z":0.406033,"session_id":10,"seq":1,"time":3484,"hmac":"2a4b138ace1a5f5eafa236274b089e154e0a5efbbafe449ce680e747e830a5a5"}'


    mosquitto_pub -t "dev1/RMS" -m '{"status":"ANOMALY","x":0.266646,"y":1.059339,"z":0.406033,"session_id":10,"seq":1,"time":3484,"hmac":"2a4b138ace1a5f5eafa236274b089e154e0a5efbbafe449ce680e747e830a5a5"}'


    # Wait 5 seconds
    sleep 5
done
