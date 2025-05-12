package com.sapienza.repository;

import com.sapienza.model.RMS;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;

import java.time.LocalDateTime;
import java.util.List;

@Repository
public interface RmsRepository extends JpaRepository<RMS, Long> {

    List<RMS> findByTimestampAfterAndDevIdOrderByTimestampAsc(LocalDateTime timestamp, String devId);


    @Query("select d \n" +
            "from RMS d \n" +
            "where d.timestamp between :start and :end " +
            "and d.devId = :devId " +
            "order by timestamp asc")
    List<RMS> findByTimeRange(@Param("devId") String devId, @Param("start") LocalDateTime start, @Param("end") LocalDateTime end);
}