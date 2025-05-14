package com.sapienza.subscription;

import com.sapienza.dto.SensorData;
import com.sapienza.model.Power;
import com.sapienza.model.RMS;
import com.sapienza.model.Anomaly;
import com.sapienza.repository.PowerRepository;
import com.sapienza.repository.RmsRepository;
import com.sapienza.repository.AnomalyRepository;
import com.sapienza.verifier.HmacVerifier;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.io.IOException;

@Service
public class SubscriptionHandler {

    @Autowired
    RmsRepository rmsRepository;

    @Autowired
    AnomalyRepository anomalyRepository;
    @Autowired
    private PowerRepository powerRepository;

    // When receiving data on anomalies topic send an alert on the dashboard
    private void alert(String topic, String message) {

    }

    //When receiving data on the datastream, set the timestamp and store it in the database
    public void store(String topic, String message) {

        String[] parts = topic.split("/");
        if (parts.length != 2) {
            System.out.println("Invalid topic format: " + topic);
            return;
        }

        String id = parts[0];
        String subTopic = parts[1];

        System.out.println(id + " " + subTopic + " " + message);
        switch (subTopic) {
            case "power":
                powerRepository.save(new Power(id, Double.valueOf(message)));
                System.out.println("Received: " + message + " on topic: power from device: " + id);
                break;
            case "RMS":

                SensorData sensorData = null;
                try {
                    sensorData= SensorData.fromJson(message);
                    if (HmacVerifier.verifyHmac(message) == false ) {
                        System.out.println("Hmac check not passed fropping message");
                        break;
                    }
                } catch (IOException e) {
                    throw new RuntimeException(e);
                }

                rmsRepository.save(new RMS(id, sensorData.getX(), sensorData.getY(),sensorData.getZ()));
                System.out.println("Received: " + message + " on topic: RMS from device: " + id);
                break;
            case "anomalies":

                try {
                    sensorData= SensorData.fromJson(message);
                    if (HmacVerifier.verifyHmac(message) == false ) {
                        System.out.println("Hmac check not passed fropping message");
                        break;
                    }
                } catch (IOException e) {
                    throw new RuntimeException(e);
                }
                anomalyRepository.save(new Anomaly(id, sensorData.getX(), sensorData.getY(),sensorData.getZ()));
                System.out.println("Received: " + message + " on topic: anomalies from device: " + id);
                break;
            default:
                System.out.println("Unknown topic: " + topic);
        }
    }

    /*
    * {
    *   "status":"ANOMALY",
    *   "x":0.266646,
    *   "y":1.059339,
    *   "z":0.406033,
    *   "session_id":10,
    *   "seq":1,
    *   "time":3484,
    *   "hmac":"2a4b138ace1a5f5eafa236274b089e154e0a5efbbafe449ce680e747e830a5a5"
    * }
    *    -datastream: {
    *   "x":0.157187,
    *   "y":0.977976,
    *   "z":0.134764,
    *   "session_id":10,
    *   "seq":2,
    *   "time":4398,
    *   "hmac":"8d7028a14564bf74927af6a80783e299e45908acc58b7f7fead983d0344bd8f5"
    * }
    * */


}
