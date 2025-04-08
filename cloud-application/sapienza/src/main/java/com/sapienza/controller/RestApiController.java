package com.sapienza.controller;

import com.sapienza.dto.AnomalyDto;
import com.sapienza.model.Datastream;
import com.sapienza.repository.DatastreamRepository;
import com.sapienza.service.AnomalyService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;

import java.util.List;

@RestController()
public class RestApiController {

    @Autowired
    DatastreamRepository datastreamRepository;

    @Autowired
    AnomalyService anomaliesService;

    @GetMapping("/api/datastream/all")
    public  List<Datastream> getAllDatastreams() {
        return datastreamRepository.findAllByOrderByTimestampAsc();
    }

    @GetMapping("/api/anomalies")
    public  List<AnomalyDto> getLastTenAnomalies() {

        return anomaliesService.getMostRecentAnomalies(10);
    }



}
