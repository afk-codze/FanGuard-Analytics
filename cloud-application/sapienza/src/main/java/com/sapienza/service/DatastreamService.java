package com.sapienza.service;

import com.sapienza.model.Datastream;
import com.sapienza.repository.DatastreamRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.time.LocalDateTime;
import java.util.List;

@Service
public class DatastreamService {

    @Autowired
    DatastreamRepository datastreamRepository;

    public List<Datastream> getAllDatastreamFromGivenTime(LocalDateTime fromTime) {
        return datastreamRepository.findByTimestampGreaterThanOrderByTimestampAsc(fromTime);
    }


    public List<Datastream> getAllDatastreamInTimeRange(LocalDateTime start, LocalDateTime end) {


        return datastreamRepository.findByTimeRange(start,end);
    }
}
