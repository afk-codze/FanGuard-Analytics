package com.cloud_application.controller;

import com.cloud_application.service.AnomalyService;
import com.cloud_application.service.PowerService;
import com.cloud_application.subscription.SubscriptionHandler;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;

import java.util.List;

@Controller
public class DashboardController{

    @Autowired
    SubscriptionHandler subscriptionHandler;
    @Autowired
    private PowerService powerService;
    @Autowired
    private AnomalyService anomalyService;

    @GetMapping( "/")
    public String realtime(Model model) {

        List<String> deviceIds = anomalyService.getAllDeviceIds();
        model.addAttribute("deviceIds", deviceIds);
        return "realtime";
    }

    @GetMapping( "/log")
    public String log(Model model) {

        List<String> deviceIds = anomalyService.getAllDeviceIds();
        model.addAttribute("deviceIds", deviceIds);
        return "log";
    }

}