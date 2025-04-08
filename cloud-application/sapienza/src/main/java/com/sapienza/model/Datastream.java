package com.sapienza.model;

import jakarta.persistence.*;

import java.time.LocalDateTime;

@Entity
@Table(name = "datastream")
public class Datastream {
    @Id
    @GeneratedValue(strategy = GenerationType.AUTO)
    private Long id;

    @Column(name = "temperature")
    private Double temperature;

    @Column(name = "timestamp", columnDefinition = "TIMESTAMPTZ")
    private LocalDateTime timestamp;

    public Datastream() {
        this.timestamp = LocalDateTime.now();
    }

    public Datastream(Double temperature) {
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
