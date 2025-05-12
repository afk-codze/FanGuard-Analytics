package com.sapienza.controller;

import com.sapienza.model.Anomaly;
import com.sapienza.model.Power;
import com.sapienza.model.RMS;
import com.sapienza.service.AnomalyService;
import com.sapienza.service.PowerService;
import com.sapienza.service.RmsService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.format.annotation.DateTimeFormat;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;
import org.springframework.web.bind.annotation.RestController;

import java.time.LocalDateTime;
import java.util.ArrayList;
import java.util.List;

@RestController()
public class RestApiController {

    @Autowired
    RmsService rmsService;

    @Autowired
    PowerService powerService;

    @Autowired
    AnomalyService anomaliesService;

    //API
    @GetMapping("/api/datastream/realtime")
    @ResponseBody
    public List<List<?>> getAllDatastreamsRealtime(@RequestParam(name = "deviceId") String deviceId) {
        // Get data from the last 2 hours
        LocalDateTime fromTime = LocalDateTime.now().minusHours(2);

        List<RMS> rmsList = rmsService.getAllRmsFromGivenTimeAndDevice(fromTime, deviceId);
        List<Power> powerList = powerService.getAllPowerFromGivenTimeAndDevice(fromTime, deviceId);

        // Combine into a list of lists
        List<List<?>> combined = new ArrayList<>();
        combined.add(powerList);
        combined.add(rmsList);

        return combined;
    }


    @GetMapping("/api/anomalies/realtime")
    public  List<Anomaly> getLastTenAnomaliesRealtime(@RequestParam(name = "deviceId") String deviceId) {

        return anomaliesService.getMostRecentAnomaliesRealtime(deviceId,10);
    }


    // Time range

    @GetMapping("/api/datastream/timeRange")
    @ResponseBody
    public List<List<?>> getAllDatastreamInTimeRange(
            @RequestParam(name = "deviceId") String deviceId,
            @RequestParam("start") @DateTimeFormat(pattern = "yyyy-MM-dd HH:mm:ss.SSSSSS") LocalDateTime start,
            @RequestParam("end") @DateTimeFormat(pattern = "yyyy-MM-dd HH:mm:ss.SSSSSS") LocalDateTime end) {

        List<RMS> rmsList = rmsService.getAllRmsInTimeRange(deviceId, start, end);
        List<Power> powerList = powerService.getAllPowerInTimeRange(deviceId, start, end);

        List<List<?>> combined = new ArrayList<>();
        combined.add(powerList);
        combined.add(rmsList);

        return combined;
    }


    @GetMapping("/api/anomalies/timeRange")
    public  List<Anomaly> getLastTenAnomaliesTimeRange(
            @RequestParam(name = "deviceId") String deviceId,
            @RequestParam("start") @DateTimeFormat(pattern = "yyyy-MM-dd HH:mm:ss.SSSSSS") LocalDateTime start,
            @RequestParam("end") @DateTimeFormat(pattern = "yyyy-MM-dd HH:mm:ss.SSSSSS") LocalDateTime end

    ) {

        return anomaliesService.getMostRecentAnomaliesTimeRange(deviceId,10,start,end);
    }



}
