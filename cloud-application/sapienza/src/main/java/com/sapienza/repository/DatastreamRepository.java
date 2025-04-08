package com.sapienza.repository;

import com.sapienza.model.Datastream;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

import java.util.List;

@Repository
public interface DatastreamRepository extends JpaRepository<Datastream, Long> {

    List<Datastream> getAllById(Long id);
}
