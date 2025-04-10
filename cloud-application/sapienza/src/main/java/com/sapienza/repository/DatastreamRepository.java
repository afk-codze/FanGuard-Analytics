package com.sapienza.repository;

import com.sapienza.model.Datastream;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;

import java.time.LocalDateTime;
import java.util.List;

@Repository
public interface DatastreamRepository extends JpaRepository<Datastream, Long> {

    List<Datastream> findAllByOrderByTimestampAsc();  // Ascending order}
    List<Datastream> findByTimestampGreaterThanOrderByTimestampAsc(LocalDateTime timestamp);


    @Query("select d \n" +
            "from Datastream d \n" +
            "where d.timestamp between :start and :end order by timestamp asc")
    List<Datastream> findByTimeRange(@Param("start") LocalDateTime start, @Param("end") LocalDateTime end);
}