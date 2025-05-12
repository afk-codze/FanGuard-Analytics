package com.sapienza.model;

import jakarta.persistence.*;
import org.springframework.format.annotation.DateTimeFormat;

import java.time.LocalDateTime;

@Entity
@Table(name = "rms")
public class RMS {
    @Id
    @GeneratedValue(strategy = GenerationType.AUTO)
    @Column(name = "auto_id")
    private Long auto_id;

    @Column(name = "id")
    private Integer id;

    @Column(name = "rms")
    private Double rms;

    @Column(name = "timestamp")
    @DateTimeFormat(iso = DateTimeFormat.ISO.DATE_TIME)
    private LocalDateTime timestamp;

    public RMS() {
        this.timestamp = LocalDateTime.now();
    }

    public RMS(Integer id ,Double rms) {
        this();
        this.id = id;
        this.rms = rms;
    }

    public Double getRms() {
        return rms;
    }

    public void setRms(Double rms) {
        this.rms = rms;
    }

    public LocalDateTime getTimestamp() {
        return timestamp;
    }

    public void setTimestamp(LocalDateTime timestamp) {
        this.timestamp = timestamp;
    }

    public Long getAuto_id() {
        return auto_id;
    }

    public void setAuto_id(Long auto_id) {
        this.auto_id = auto_id;
    }

    public Integer getId() {
        return id;
    }

    public void setId(Integer id) {
        this.id = id;
    }
}
