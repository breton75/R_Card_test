SELECT vessels.id AS id,
       vessels.self AS self,
       ais.static_callsign AS static_callsign,
       ais.static_imo AS static_imo,
       ais.static_mmsi AS static_mmsi,
       ais.static_type_id AS static_type_id,
       ais.static_length AS static_length,
       ais.static_width AS static_width,
       ais.voyage_destination AS voyage_destination,
       ais.voyage_draft AS voyage_draft,
       ais.voyage_cargo_type_id AS voyage_cargo_type_id,
       ais.voyage_team AS voyage_team,
       ais.dynamic_course AS dynamic_course,
       ais.dynamic_latitude AS dynamic_latitude,
       ais.dynamic_longtitude AS dynamic_longtitude,
       ais.dynamic_status_id AS dynamic_status_id,
       ais.dynamic_utc AS dynamic_utc,
       gps.timeout AS gps_timeout,
       gps.init_random_coordinates AS init_random_coordinates,
       gps.init_random_course AS init_random_course,
       gps.init_random_speed AS init_random_speed,
       gps.init_course_change_ratio AS init_course_change_ratio,
       gps.init_course_change_segment AS init_course_change_segment,
       gps.init_speed_change_ratio AS init_speed_change_ratio,
       gps.init_speed_change_segment AS init_speed_change_segment,
       vessel_types.type_name AS static_vessel_type_name,
       cargo_types.type_name AS voyage_cargo_type_name,
       status_types.status_name AS dynamic_status_name
  FROM vessels
       LEFT JOIN gps ON vessels.id = gps.vessel_id
       LEFT JOIN ais ON vessels.id = ais.vessel_id
       LEFT JOIN cargo_types ON ais.voyage_cargo_type_id = cargo_types.id
       LEFT JOIN vessel_types ON ais.static_type_id = vessel_types.id
       LEFT JOIN status_types ON ais.dynamic_status_id = status_types.id/* where
    vessels.self = 'false' */
 ORDER BY vessels.id ASC;
