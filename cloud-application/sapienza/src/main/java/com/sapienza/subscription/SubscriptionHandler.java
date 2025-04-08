package com.sapienza.subscription;

import com.sapienza.model.Datastream;
import com.sapienza.model.TemperatureAnomaly;
import com.sapienza.model.WaterAnomaly;
import com.sapienza.repository.DatastreamRepository;
import com.sapienza.repository.TemperatureAnomalyRepository;
import com.sapienza.repository.WaterAnomalyRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

@Service
public class SubscriptionHandler {

    @Autowired
    DatastreamRepository datastreamRepository;

    @Autowired
    WaterAnomalyRepository waterAnomalyRepository;

    @Autowired
    TemperatureAnomalyRepository temperatureAnomalyRepository;

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
            case "anomalies/water":
                waterAnomalyRepository.save(new WaterAnomaly());
                System.out.println("Received: " + message + "on topic: water");
                break;
            case "anomalies/temperature":
                temperatureAnomalyRepository.save(new TemperatureAnomaly(Double.valueOf(message)));
                System.out.println("Received: " + message + "on topic: temperature spike");
                break;
            default:
                System.out.println("Unknown topic: " + topic);
        }


    }

}
