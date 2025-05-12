package com.sapienza.service;

import com.sapienza.dto.AnomalyDto;
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
    List<AnomalyDto> anomalies = new LinkedList<>();


    public List<AnomalyDto> getMostRecentAnomaliesRealtime(int maxAnomalies) {

        //get last maxanomalies PowerAnomalies Objects
        List<Anomaly> powerAnomalies = anomalyRepository.findTop10ByOrderByTimestampDesc();


        encodeAnomalyDtoList(maxAnomalies, powerAnomalies);

        System.out.println(anomalies.toString());

        return anomalies;
    }

    private void encodeAnomalyDtoList(int maxAnomalies, List<Anomaly> powerAnomalies) {

        for (Anomaly anomaly : powerAnomalies) {
            anomalies.add(new AnomalyDto(anomaly.getId(), anomaly.getAnomaly_value(), anomaly.getTimestamp(),AnomalyDto.POWER));
        }

        // Sort all anomalies by timestamp descending
        anomalies.sort((a1, a2) -> a2.getTimestamp().compareTo(a1.getTimestamp()));

        // Keep only top #maxAnomalies
        if (anomalies.size() > maxAnomalies) {
            anomalies = anomalies.subList(0, maxAnomalies);
        }
    }

    public List<AnomalyDto> getMostRecentAnomaliesTimeRange(int maxAnomalies, LocalDateTime start, LocalDateTime end) {

        //get last maxanomalies PowerAnomalies Objects
        List<Anomaly> powerAnomalies = anomalyRepository.findTop10ByOrderByTimestampDesc();


        encodeAnomalyDtoList(maxAnomalies, powerAnomalies);

        System.out.println(anomalies.toString());

        return anomalies;

    }
}
