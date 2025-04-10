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
public interface WaterAnomalyRepository extends JpaRepository<WaterAnomaly, Long> {
    List<WaterAnomaly> findTop10ByOrderByTimestampDesc();


    @Query("select w \n" +
            "from WaterAnomaly w \n" +
            "where w.timestamp between :start and :end " +
            "order by timestamp desc")
    List<WaterAnomaly> findTop10ByTimeRangeDesc(@Param("start") LocalDateTime start, @Param("end") LocalDateTime end);
}
