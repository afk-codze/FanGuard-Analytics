package com.cloud_application.repository;

import com.cloud_application.model.Power;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;

import java.time.LocalDateTime;
import java.util.List;

@Repository
public interface PowerRepository extends JpaRepository<Power, Long> {

    List<Power> findByTimestampAfterAndDevIdOrderByTimestampAsc(LocalDateTime timestamp, String devId);

    @Query(value = "SELECT DISTINCT dev_id FROM power " +
            "UNION " +
            "SELECT DISTINCT dev_id FROM rms", nativeQuery = true)
    List<String> findAllDistinctDeviceIds();

    @Query("select d \n" +
            "from Power d \n" +
            "where d.timestamp between :start and :end " +
            "and d.devId = :devId " +
            "order by timestamp asc")
    List<Power> findByTimeRange(@Param("devId") String devId, @Param("start") LocalDateTime start, @Param("end") LocalDateTime end);
}