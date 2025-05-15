package com.cloud_application.repository;

import com.cloud_application.model.Anomaly;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;

import java.time.LocalDateTime;
import java.util.List;

@Repository
public interface AnomalyRepository extends JpaRepository<Anomaly, Long> {

    List<Anomaly> findTop10ByDevIdOrderByTimestampDesc(String devId);

    @Query("select w \n" +
            "from Anomaly w \n" +
            "where w.timestamp between :start and :end " +
            "and w.devId = :devId "+
            "order by timestamp desc")
    List<Anomaly> findTop10ByTimeRangeDesc(@Param("devId") String devId, @Param("start") LocalDateTime start, @Param("end") LocalDateTime end);
}
