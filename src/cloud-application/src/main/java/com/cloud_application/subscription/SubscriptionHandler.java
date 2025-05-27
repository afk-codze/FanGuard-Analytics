package com.cloud_application.subscription;

import com.cloud_application.dto.SensorData;
import com.cloud_application.model.Power;
import com.cloud_application.model.RMS;
import com.cloud_application.model.Anomaly;
import com.cloud_application.repository.PowerRepository;
import com.cloud_application.repository.RmsRepository;
import com.cloud_application.repository.AnomalyRepository;
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
        if (subTopic.equals("anomalies")) {
            try {
                SensorData sensorData = SensorData.fromJson(message);
                anomalyRepository.save(new Anomaly(id, sensorData.getClassification(), sensorData.getProb()));

            } catch (IOException e) {
                throw new RuntimeException(e);
            }
            System.out.println("Received: " + message + " on topic: anomalies from device: " + id);
        } else {
            System.out.println("Unknown topic: " + topic);
        }
    }
}
