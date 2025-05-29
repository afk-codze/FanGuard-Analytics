package com.cloud_application.dto;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.databind.ObjectMapper;
import java.io.IOException;

public class SensorData {

    @JsonProperty(value = "time_stamp",required = false)
    private String time_stamp;


    @JsonProperty(value = "classification",required = false)
    private String classification;


    @JsonProperty(value = "prob",required = false)
    private Double prob;

    public String getTime_stamp() {
        return time_stamp;
    }

    public void setTime_stamp(String time_stamp) {
        this.time_stamp = time_stamp;
    }

    public String getClassification() {
        return classification;
    }

    public void setClassification(String classification) {
        this.classification = classification;
    }

    public Double getProb() {
        return prob;
    }

    public void setProb(Double prob) {
        this.prob = prob;
    }

    // Serializer & Deserializer using Jackson
    public static SensorData fromJson(String json) throws IOException {
        ObjectMapper mapper = new ObjectMapper();
        return mapper.readValue(json, SensorData.class);
    }

    public static String toJson(SensorData data) throws IOException {
        ObjectMapper mapper = new ObjectMapper();
        return mapper.writeValueAsString(data);
    }
}
