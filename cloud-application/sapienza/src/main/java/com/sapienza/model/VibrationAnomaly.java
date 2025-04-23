package com.sapienza.model;

import jakarta.persistence.*;
import org.springframework.format.annotation.DateTimeFormat;

import java.time.LocalDateTime;

@Entity
@Table(name = "vibration_anomaly")
public class VibrationAnomaly {

    @Id
    @GeneratedValue(strategy = GenerationType.AUTO)
    private Long id;

    @Column(name = "vibration_spike")
    private Double vibration_spike;


    @Column(name = "timestamp")
    @DateTimeFormat(iso = DateTimeFormat.ISO.DATE_TIME)
    private LocalDateTime timestamp;


    public VibrationAnomaly() {
        this.timestamp = LocalDateTime.now();
    }

    public VibrationAnomaly(Double spike) {
        this();
        this.vibration_spike = spike;
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

    public Double getVibration_spike() {
        return vibration_spike;
    }

    public void setVibration_spike(Double vibration_spike) {
        this.vibration_spike = vibration_spike;
    }

    public Long getId() {
        return id;
    }
}
