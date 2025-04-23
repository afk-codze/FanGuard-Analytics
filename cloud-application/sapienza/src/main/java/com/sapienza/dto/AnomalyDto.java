package com.sapienza.dto;

import org.springframework.stereotype.Component;

import java.time.LocalDateTime;

@Component
public class AnomalyDto {

    private Long id;
    private Double value_spike;
    private String type;
    private LocalDateTime timestamp;

    public static String POWER = "POWER";
    public static String VIBRATION = "VIBRATION";

    public AnomalyDto() {
    }

    public AnomalyDto(Long id, Double value_spike, LocalDateTime timestamp, String type) {
        this.id = id;
        this.value_spike = value_spike;
        this.type = type;
        this.timestamp = timestamp;
    }

    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public LocalDateTime getTimestamp() {
        return timestamp;
    }

    public Double getValue_spike() {
        return value_spike;
    }

    public void setValue_spike(Double value_spike) {
        this.value_spike = value_spike;
    }

    public String getType() {
        return type;
    }

    public void setType(String type) {
        this.type = type;
    }

    public void setTimestamp(LocalDateTime timestamp) {
        this.timestamp = timestamp;
    }
}
