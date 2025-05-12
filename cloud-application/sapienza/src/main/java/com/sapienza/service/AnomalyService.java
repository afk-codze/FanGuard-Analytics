package com.sapienza.service;

import com.sapienza.model.Anomaly;
import com.sapienza.repository.AnomalyRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.time.LocalDateTime;
import java.util.LinkedList;
import java.util.List;

@Service
public class AnomalyService {

    @Autowired
    AnomalyRepository anomalyRepository;

    //merge and sort by date time
    List<Anomaly> anomalies = new LinkedList<>();


    public List<Anomaly> getMostRecentAnomaliesRealtime(String deviceId, int maxAnomalies) {

        //get last maxanomalies PowerAnomalies Objects
        List<Anomaly> powerAnomalies = anomalyRepository.findTop10ByDevIdOrderByTimestampDesc(deviceId);


        return sortAnomalies(maxAnomalies, powerAnomalies);

    }

    private List<Anomaly> sortAnomalies(int maxAnomalies, List<Anomaly> powerAnomalies) {

        // Sort all anomalies by timestamp descending
        powerAnomalies.sort((a1, a2) -> a2.getTimestamp().compareTo(a1.getTimestamp()));

        // Keep only top #maxAnomalies
        if (powerAnomalies.size() > maxAnomalies) {
            powerAnomalies = powerAnomalies.subList(0, maxAnomalies);
        }
        return powerAnomalies;
    }

    public List<Anomaly> getMostRecentAnomaliesTimeRange(String deviceId, int maxAnomalies, LocalDateTime start, LocalDateTime end) {

        //get last maxanomalies PowerAnomalies Objects
        List<Anomaly> powerAnomalies = anomalyRepository.findTop10ByTimeRangeDesc(deviceId, start, end);

        return sortAnomalies(maxAnomalies, powerAnomalies);

    }
}
