package com.sapienza.repository;

import com.sapienza.model.TemperatureAnomaly;
import com.sapienza.model.WaterAnomaly;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

import java.util.List;

@Repository
public interface WaterAnomalyRepository extends JpaRepository<WaterAnomaly, Long> {
    List<WaterAnomaly> findTop10ByOrderByTimestampDesc();
}
