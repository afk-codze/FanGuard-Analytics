package com.sapienza.model;

import jakarta.persistence.*;
import org.springframework.format.annotation.DateTimeFormat;

import java.time.LocalDateTime;

@Entity
@Table(name = "anomaly")
public class Anomaly {

    @Id
    @GeneratedValue(strategy = GenerationType.AUTO)
    @Column(name = "auto_id")
    private Long auto_id;

    @Column(name = "id")
    private Integer id;

    @Column(name = "anomaly_value")
    private Double anomaly_value;

    @Column(name = "timestamp")
    @DateTimeFormat(iso = DateTimeFormat.ISO.DATE_TIME)
    private LocalDateTime timestamp;

    public Anomaly() {
        this.timestamp =LocalDateTime.now();
    }

    public Anomaly(Integer id,Double anomaly_value) {
        this();
        this.id = id;
        this.anomaly_value = anomaly_value;
    }

    public Long getAuto_id() {
        return auto_id;
    }

    public void setAuto_id(Long auto_id) {
        this.auto_id = auto_id;
    }

    public void setId(Integer id) {
        this.id = id;
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

    public Integer getId() {
        return id;
    }
}
