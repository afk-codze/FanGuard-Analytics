package com.sapienza.subscription;

import com.sapienza.model.Power;
import com.sapienza.model.RMS;
import com.sapienza.model.Anomaly;
import com.sapienza.repository.PowerRepository;
import com.sapienza.repository.RmsRepository;
import com.sapienza.repository.AnomalyRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

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

        Integer id = Integer.getInteger(parts[0]);
        String subTopic = parts[1];

        switch (subTopic) {
            case "power":
                powerRepository.save(new Power(id, Double.valueOf(message)));
                System.out.println("Received: " + message + " on topic: power from device: " + id);
                break;
            case "RMS":
                rmsRepository.save(new RMS(id, Double.valueOf(message)));
                System.out.println("Received: " + message + " on topic: RMS from device: " + id);
                break;
            case "anomalies":
                anomalyRepository.save(new Anomaly(id, Double.valueOf(message)));
                System.out.println("Received: " + message + " on topic: anomalies from device: " + id);
                break;
            default:
                System.out.println("Unknown topic: " + topic);
        }
    }


}
