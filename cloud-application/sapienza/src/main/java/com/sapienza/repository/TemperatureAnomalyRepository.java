package com.sapienza.repository;

import com.sapienza.model.Datastream;
import com.sapienza.model.TemperatureAnomaly;
import com.sapienza.model.WaterAnomaly;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;

import java.time.LocalDateTime;
import java.util.List;

@Repository
public interface TemperatureAnomalyRepository extends JpaRepository<TemperatureAnomaly, Long> {
    List<TemperatureAnomaly> findTop10ByOrderByTimestampDesc();

    @Query("select w \n" +
            "from TemperatureAnomaly w \n" +
            "where w.timestamp between :start and :end " +
            "order by timestamp desc")
    List<TemperatureAnomaly> findTop10ByTimeRangeDesc(@Param("start") LocalDateTime start, @Param("end") LocalDateTime end);
}
