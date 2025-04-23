package com.sapienza.subscription;

import com.sapienza.model.Datastream;
import com.sapienza.model.PowerAnomaly;
import com.sapienza.model.VibrationAnomaly;
import com.sapienza.repository.DatastreamRepository;
import com.sapienza.repository.PowerAnomalyRepository;
import com.sapienza.repository.VibrationAnomalyRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

@Service
public class SubscriptionHandler {

    @Autowired
    DatastreamRepository datastreamRepository;

    @Autowired
    VibrationAnomalyRepository vibrationAnomalyRepository;

    @Autowired
    PowerAnomalyRepository powerAnomalyRepository;

    // When receiving data on anomalies topic send an alert on the dashboard
    private void alert(String topic, String message) {

    }

    //When receiving data on the datastream, set the timestamp and store it in the database
    public void store(String topic, String message) {
        switch (topic) {
            case "datastream":
                System.out.println("Received: " + message + "on topic: datastream");
                datastreamRepository.save(new Datastream(Double.valueOf(message)));
                break;
            case "anomalies/vibration":
                vibrationAnomalyRepository.save(new VibrationAnomaly(Double.valueOf(message)));
                System.out.println("Received: " + message + "on topic: anomalies/vibration");
                break;
            case "anomalies/power":
                powerAnomalyRepository.save(new PowerAnomaly(Double.valueOf(message)));
                System.out.println("Received: " + message + "on topic: anomalies/power");
                break;
            default:
                System.out.println("Unknown topic: " + topic);
        }


    }

}
