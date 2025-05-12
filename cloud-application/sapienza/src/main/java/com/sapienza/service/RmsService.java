package com.sapienza.service;

import com.sapienza.model.RMS;
import com.sapienza.repository.RmsRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.time.LocalDateTime;
import java.util.List;

@Service
public class RmsService {

    @Autowired
    RmsRepository rmsRepository;

    public List<RMS> getAllRmsFromGivenTimeAndDevice(LocalDateTime fromTime, String deviceId) {
        return rmsRepository.findByTimestampAfterAndDevIdOrderByTimestampAsc(fromTime,deviceId);
    }


    public List<RMS> getAllRmsInTimeRange(String deviceId, LocalDateTime start, LocalDateTime end) {


        return rmsRepository.findByTimeRange(deviceId,start,end);
    }
}
