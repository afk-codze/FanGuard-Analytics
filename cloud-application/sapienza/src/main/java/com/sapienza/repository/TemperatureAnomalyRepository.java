package com.sapienza.repository;

import com.sapienza.model.Datastream;
import com.sapienza.model.TemperatureAnomaly;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

import java.util.List;

@Repository
public interface TemperatureAnomalyRepository extends JpaRepository<TemperatureAnomaly, Long> {
    List<TemperatureAnomaly> findTop10ByOrderByTimestampDesc();
}
