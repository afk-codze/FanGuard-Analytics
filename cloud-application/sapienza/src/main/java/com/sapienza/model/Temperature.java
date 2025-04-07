package com.sapienza.model;

import jakarta.persistence.*;

import java.time.LocalDateTime;

@Entity
@Table(name = "temperature")
public class Temperature {
    @Id
    @GeneratedValue(strategy = GenerationType.AUTO)
    private Long id;

    @Column(name = "temperature")
    private Double temperature;

    @Column(name = "timestamp")
    private LocalDateTime timestamp;

    public Temperature() {
        this.timestamp = LocalDateTime.now();
    }

    public Temperature(Double temperature) {
        super();
        this.temperature = temperature;
    }

    public Double getTemperature() {
        return temperature;
    }

    public void setTemperature(Double temperature) {
        this.temperature = temperature;
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
