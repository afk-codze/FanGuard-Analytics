package com.sapienza.subscription;

import com.sapienza.controller.DashboardController;
import com.sapienza.model.Temperature;
import com.sapienza.repository.TemperatureRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

@Service
public class SubscriptionHandler {

    @Autowired
    TemperatureRepository temperatureRepository;

    // When receiving data on anomalies topic send an alert on the dashboard
    private void alert(String topic, String message) {

    }

    //When receiving data on the datastream, set the timestamp and store it in the database
    public void store(String topic, String message) {

            temperatureRepository.save(new Temperature(Double.valueOf(message)));

    }

}
