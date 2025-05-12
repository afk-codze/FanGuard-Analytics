package com.sapienza.model;

import jakarta.persistence.*;
import org.springframework.format.annotation.DateTimeFormat;

import java.time.LocalDateTime;

@Entity
@Table(name = "anomaly")
public class Anomaly {

    @Id
    @GeneratedValue(strategy = GenerationType.AUTO)
    private Long Id;

    @Column(name = "devId")
    private String devId;

    @Column(name = "anomaly_value")
    private Double anomaly_value;

    @Column(name = "timestamp")
    @DateTimeFormat(iso = DateTimeFormat.ISO.DATE_TIME)
    private LocalDateTime timestamp;

    public Anomaly() {
        this.timestamp =LocalDateTime.now();
    }

    public Anomaly(String id,Double anomaly_value) {
        this();
        this.devId = id;
        this.anomaly_value = anomaly_value;
    }

    public Long getId() {
        return Id;
    }

    public void setId(Long id) {
        Id = id;
    }

    public String getDevId() {
        return devId;
    }

    public void setDevId(String devId) {
        this.devId = devId;
    }

    public Double getAnomaly_value() {
        return anomaly_value;
    }

    public void setAnomaly_value(Double anomaly_value) {
        this.anomaly_value = anomaly_value;
    }

    public LocalDateTime getTimestamp() {
        return timestamp;
    }

    public void setTimestamp(LocalDateTime timestamp) {
        this.timestamp = timestamp;
    }
}
