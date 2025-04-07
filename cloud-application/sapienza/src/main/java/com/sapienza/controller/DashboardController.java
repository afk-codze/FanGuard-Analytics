package com.sapienza.controller;

import com.sapienza.subscription.SubscriptionHandler;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
public class DashboardController{

    @Autowired
    SubscriptionHandler subscriptionHandler;

    @GetMapping( "/hello")
    public String index() {

        subscriptionHandler.store("df","12");
        System.out.println("hello");
        return "Hello World";
    }

}