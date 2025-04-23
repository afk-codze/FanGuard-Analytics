package com.sapienza.repository;

import com.sapienza.model.VibrationAnomaly;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;
import com.sapienza.model.VibrationAnomaly;

import java.time.LocalDateTime;
import java.util.List;

@Repository
public interface VibrationAnomalyRepository extends JpaRepository<VibrationAnomaly, Long> {
    List<VibrationAnomaly> findTop10ByOrderByTimestampDesc();


    @Query("select w \n" +
            "from VibrationAnomaly w \n" +
            "where w.timestamp between :start and :end " +
            "order by timestamp desc")
    List<VibrationAnomaly> findTop10ByTimeRangeDesc(@Param("start") LocalDateTime start, @Param("end") LocalDateTime end);
}
