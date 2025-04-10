package com.sapienza.controller;

import com.sapienza.subscription.SubscriptionHandler;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;

@Controller
public class DashboardController{

    @Autowired
    SubscriptionHandler subscriptionHandler;

    @GetMapping( "/")
    public String realtime(Model model) {

        return "realtime";
    }

    @GetMapping( "/log")
    public String log(Model model) {

        return "log";
    }

}