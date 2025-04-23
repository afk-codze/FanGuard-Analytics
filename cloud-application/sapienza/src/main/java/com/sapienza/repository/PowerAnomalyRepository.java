package com.sapienza.repository;

import com.sapienza.model.PowerAnomaly;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;

import java.time.LocalDateTime;
import java.util.List;

@Repository
public interface PowerAnomalyRepository extends JpaRepository<PowerAnomaly, Long> {
    List<PowerAnomaly> findTop10ByOrderByTimestampDesc();

    @Query("select w \n" +
            "from PowerAnomaly w \n" +
            "where w.timestamp between :start and :end " +
            "order by timestamp desc")
    List<PowerAnomaly> findTop10ByTimeRangeDesc(@Param("start") LocalDateTime start, @Param("end") LocalDateTime end);
}
