package com.cloud_application.model;

import jakarta.persistence.*;
import org.springframework.format.annotation.DateTimeFormat;

import java.time.LocalDateTime;

@Entity
@Table(name = "rms")
public class RMS {
    @Id
    @GeneratedValue(strategy = GenerationType.AUTO)
    private Long Id;

    @Column(name = "devId")
    private String devId;

    @Column(name = "rms_x")
    private Double rms_x;


    @Column(name = "rms_y")
    private Double rms_y;


    @Column(name = "rms_z")
    private Double rms_z;

    @Column(name = "timestamp")
    @DateTimeFormat(iso = DateTimeFormat.ISO.DATE_TIME)
    private LocalDateTime timestamp;

    public RMS() {
        this.timestamp = LocalDateTime.now();
    }

    public RMS(String Id ,Double rms_x, Double rms_y, Double rms_z) {
        this();
        this.devId = Id;
        this.rms_x = rms_x;
        this.rms_y = rms_y;
        this.rms_z = rms_z;
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

    public Double getRms_x() {
        return rms_x;
    }

    public void setRms_x(Double rms_x) {
        this.rms_x = rms_x;
    }

    public Double getRms_y() {
        return rms_y;
    }

    public void setRms_y(Double rms_y) {
        this.rms_y = rms_y;
    }

    public Double getRms_z() {
        return rms_z;
    }

    public void setRms_z(Double rms_z) {
        this.rms_z = rms_z;
    }

    public LocalDateTime getTimestamp() {
        return timestamp;
    }

    public void setTimestamp(LocalDateTime timestamp) {
        this.timestamp = timestamp;
    }
}
