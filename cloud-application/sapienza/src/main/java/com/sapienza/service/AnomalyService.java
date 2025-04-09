package com.sapienza.service;

import com.sapienza.dto.AnomalyDto;
import com.sapienza.model.TemperatureAnomaly;
import com.sapienza.model.WaterAnomaly;
import com.sapienza.repository.TemperatureAnomalyRepository;
import com.sapienza.repository.WaterAnomalyRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.LinkedList;
import java.util.List;

@Service
public class AnomalyService {

    @Autowired
    WaterAnomalyRepository waterAnomalyRepository;

    @Autowired
    TemperatureAnomalyRepository temperatureAnomalyRepository;


    public List<AnomalyDto> getMostRecentAnomalies(int maxAnomalies) {

        //get last maxanomalies waterAnomaly Objects
        List<WaterAnomaly> waterAnomalies = waterAnomalyRepository.findTop10ByOrderByTimestampDesc();
        //get last maxanomalies temperatureAnomaly Objects
        List<TemperatureAnomaly> temperatureAnomalies = temperatureAnomalyRepository.findTop10ByOrderByTimestampDesc();

        //merge and sort by date time
        List<AnomalyDto> anomalies = new LinkedList<>();

        for (WaterAnomaly wa : waterAnomalies) {
            anomalies.add(new AnomalyDto(wa.getId(), null, wa.getTimestamp()));
        }

        for (TemperatureAnomaly ta : temperatureAnomalies) {
            anomalies.add(new AnomalyDto(ta.getId(), ta.getTemperature_spike(), ta.getTimestamp()));
        }

        // Sort all anomalies by timestamp descending
        anomalies.sort((a1, a2) -> a2.getTimestamp().compareTo(a1.getTimestamp()));

        // Keep only top #maxAnomalies
        if (anomalies.size() > maxAnomalies) {
            anomalies = anomalies.subList(0, maxAnomalies);
        }

        System.out.println(anomalies);

        return anomalies;
    }
}
