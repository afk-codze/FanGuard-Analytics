package com.sapienza.dto;

import org.springframework.stereotype.Component;

import java.time.LocalDateTime;

@Component
public class AnomalyDto {

    private Long id;
    private Double temperature_spike;
    private LocalDateTime timestamp;


    public AnomalyDto() {
    }

    public AnomalyDto(Long id, Double temperature_spike, LocalDateTime timestamp) {
        this.id = id;
        if(temperature_spike != null){
            this.temperature_spike = temperature_spike;
        }
        this.timestamp = timestamp;
    }

    public Long getId() {
        return id;
    }

    public void setId(Long id) {
        this.id = id;
    }

    public Double getTemperature_spike() {
        return temperature_spike;
    }

    public void setTemperature_spike(Double temperature_spike) {
        this.temperature_spike = temperature_spike;
    }

    public LocalDateTime getTimestamp() {
        return timestamp;
    }

    public void setTimestamp(LocalDateTime timestamp) {
        this.timestamp = timestamp;
    }
}
