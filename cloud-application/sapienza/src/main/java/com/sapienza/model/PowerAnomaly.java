package com.sapienza.model;

import jakarta.persistence.*;
import org.springframework.format.annotation.DateTimeFormat;

import java.time.LocalDateTime;

@Entity
@Table(name = "power_anomaly")
public class PowerAnomaly {

    @Id
    @GeneratedValue(strategy = GenerationType.AUTO)
    private Long id;

    @Column(name = "power_spike")
    private Double power_spike;

    @Column(name = "timestamp")
    @DateTimeFormat(iso = DateTimeFormat.ISO.DATE_TIME)
    private LocalDateTime timestamp;

    public PowerAnomaly() {
        this.timestamp =LocalDateTime.now();
    }

    public PowerAnomaly(Double temperature_spike) {
        this();
        this.power_spike = temperature_spike;
    }

    public Double getPower_spike() {
        return power_spike;
    }

    public void setPower_spike(Double temperature_spike) {
        this.power_spike = temperature_spike;
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
