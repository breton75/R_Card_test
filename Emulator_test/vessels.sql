select 
    vessels.id as id, 
    vessels.callsign as callsign, 
    vessels.destination as destination, 
    vessels.draft as draft, 
    vessels.cargo_type_id as cargo_type_id, 
    vessels.imo as imo, 
    vessels.mmsi as mmsi, 
    vessels.length as length, 
    vessels.width as width, 
    vessels.self as self, 
    vessels.team as team, 
    vessels.type_id as type_id, 
    vessels.init_course as init_course,
    vessels.init_course_change_ratio as init_course_change_ratio,
    vessels.init_course_change_segment as init_course_change_segment,
    vessels.init_speed as init_speed,
    vessels.init_speed_change_ratio as init_speed_change_ratio,
    vessels.init_speed_change_segment as init_speed_change_segment,
    vessels.dynamic_course as dynamic_course,
    vessels.dynamic_latitude as dynamic_latitude,
    vessels.dynamic_longtitude as dynamic_longtitude,
    vessels.dynamic_status_id as status_id,
    vessels.dynamic_utc as dynamic_utc, 
    
    vessel_types.type_name as vessel_type_name,
    cargo_types.type_name as cargo_type_name, 
    status_types.status_name as status_name    

from vessels
left join cargo_types on vessels.cargo_type_id = cargo_types.id
left join vessel_types on vessels.type_id = vessel_types.id
left join status_types on vessels.status_id = status_types.id
/*where
    vessels.self = 'false'*/
order by vessels.id asc    
