package com.sapienza.service;

import com.sapienza.dto.AnomalyDto;
import com.sapienza.model.PowerAnomaly;
import com.sapienza.model.VibrationAnomaly;
import com.sapienza.repository.PowerAnomalyRepository;
import com.sapienza.repository.VibrationAnomalyRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.time.LocalDateTime;
import java.util.LinkedList;
import java.util.List;

@Service
public class AnomalyService {

    @Autowired
    VibrationAnomalyRepository vibrationAnomalyRepository;

    @Autowired
    PowerAnomalyRepository powerAnomalyRepository;

    //merge and sort by date time
    List<AnomalyDto> anomalies = new LinkedList<>();


    public List<AnomalyDto> getMostRecentAnomaliesRealtime(int maxAnomalies) {

        //get last maxanomalies vibrationAnomalies Objects
        List<VibrationAnomaly> vibrationAnomalies = vibrationAnomalyRepository.findTop10ByOrderByTimestampDesc();
        //get last maxanomalies PowerAnomalies Objects
        List<PowerAnomaly> powerAnomalies = powerAnomalyRepository.findTop10ByOrderByTimestampDesc();


        encodeAnomalyDtoList(maxAnomalies, vibrationAnomalies, powerAnomalies);

        System.out.println(anomalies.toString());

        return anomalies;
    }

    private void encodeAnomalyDtoList(int maxAnomalies, List<VibrationAnomaly> vibrationAnomalies, List<PowerAnomaly> powerAnomalies) {
        for (VibrationAnomaly anomaly : vibrationAnomalies) {
            anomalies.add(new AnomalyDto(anomaly.getId(), anomaly.getVibration_spike(), anomaly.getTimestamp(), AnomalyDto.VIBRATION));
        }

        for (PowerAnomaly anomaly : powerAnomalies) {
            anomalies.add(new AnomalyDto(anomaly.getId(), anomaly.getPower_spike(), anomaly.getTimestamp(),AnomalyDto.POWER));
        }

        // Sort all anomalies by timestamp descending
        anomalies.sort((a1, a2) -> a2.getTimestamp().compareTo(a1.getTimestamp()));

        // Keep only top #maxAnomalies
        if (anomalies.size() > maxAnomalies) {
            anomalies = anomalies.subList(0, maxAnomalies);
        }
    }

    public List<AnomalyDto> getMostRecentAnomaliesTimeRange(int maxAnomalies, LocalDateTime start, LocalDateTime end) {

        //get last maxanomalies vibrationAnomalies Objects
        List<VibrationAnomaly> vibrationAnomalies = vibrationAnomalyRepository.findTop10ByOrderByTimestampDesc();
        //get last maxanomalies PowerAnomalies Objects
        List<PowerAnomaly> powerAnomalies = powerAnomalyRepository.findTop10ByOrderByTimestampDesc();


        encodeAnomalyDtoList(maxAnomalies, vibrationAnomalies, powerAnomalies);

        System.out.println(anomalies.toString());

        return anomalies;

    }
}
