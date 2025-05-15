package com.cloud_application.service;

import com.cloud_application.model.Power;
import com.cloud_application.repository.PowerRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.time.LocalDateTime;
import java.util.List;

@Service
public class PowerService {

    @Autowired
    private PowerRepository powerRepository;

    public List<Power> getAllPowerFromGivenTimeAndDevice(LocalDateTime fromTime, String deviceId) {
        return powerRepository.findByTimestampAfterAndDevIdOrderByTimestampAsc(fromTime,deviceId);
    }


    public List<Power> getAllPowerInTimeRange(String deviceId, LocalDateTime start, LocalDateTime end) {

        return powerRepository.findByTimeRange(deviceId,start,end);
    }

    public List<String> getAllDeviceIds() {
        return powerRepository.findAllDistinctDeviceIds();
    }
}
