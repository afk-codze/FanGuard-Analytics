package com.sapienza.model;

import jakarta.persistence.*;
import org.springframework.format.annotation.DateTimeFormat;

import java.time.LocalDateTime;
import java.time.OffsetDateTime;
import java.time.ZoneId;

@Entity
@Table(name = "temperatureanomaly")
public class TemperatureAnomaly {

    @Id
    @GeneratedValue(strategy = GenerationType.AUTO)
    private Long id;

    @Column(name = "temperature_spike")
    private Double temperature_spike;


    @Column(name = "timestamp")
    @DateTimeFormat(iso = DateTimeFormat.ISO.DATE_TIME)
    private LocalDateTime timestamp;

    public TemperatureAnomaly() {
        this.timestamp =LocalDateTime.now();
    }

    public TemperatureAnomaly(Double temperature_spike) {
        this();
        this.temperature_spike = temperature_spike;
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
    public void setId(Long id) {
        this.id = id;
    }

    public Long getId() {
        return id;
    }
}
