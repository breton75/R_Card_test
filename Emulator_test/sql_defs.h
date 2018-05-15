#ifndef SQL_DEFS_H
#define SQL_DEFS_H

#define CR "\n"

#define SQL_SELECT_VESSELS "SELECT vessels.id AS id, " \
                           "       vessels.self AS self, " \
                           "       vessels.is_active AS is_active, " \
                           "       ais.static_callsign AS static_callsign, " \
                           "       ais.static_name AS static_name, " \
                           "       ais.static_imo AS static_imo, " \
                           "       ais.static_mmsi AS static_mmsi, " \
                           "       ais.static_type_ITU_id AS static_type_ITU_id, " \
                           "       ais.static_pos_ref_A AS static_pos_ref_A, " \
                           "       ais.static_pos_ref_B AS static_pos_ref_B, " \
                           "       ais.static_pos_ref_C AS static_pos_ref_C, " \
                           "       ais.static_pos_ref_D AS static_pos_ref_D, " \
                           "       ais.static_DTE AS static_DTE, " \
                           "       ais.static_talker_id AS static_talker_id, " \
                           "       ais.voyage_destination AS voyage_destination, " \
                           "       ais.voyage_ETA_utc AS voyage_ETA_utc, " \
                           "       ais.voyage_ETA_day AS voyage_ETA_day, " \
                           "       ais.voyage_ETA_month AS voyage_ETA_month, " \
                           "       ais.voyage_draft AS voyage_draft, " \
                           "       ais.voyage_cargo_ITU_id AS voyage_cargo_ITU_id, " \
                           "       ais.voyage_team AS voyage_team, " \
                           "       ais.dynamic_course AS dynamic_course, " \
                           "       ais.dynamic_latitude AS dynamic_latitude, " \
                           "       ais.dynamic_longtitude AS dynamic_longtitude, " \
                           "       ais.dynamic_speed AS dynamic_speed, " \
                           "       ais.dynamic_utc AS dynamic_utc, " \
                           "       ais.nav_status_ITU_id AS nav_status_ITU_id, " \
                           "       gps.timeout AS gps_timeout, " \
                           "       gps.init_random_coordinates AS init_random_coordinates, " \
                           "       gps.init_random_course AS init_random_course, " \
                           "       gps.init_random_speed AS init_random_speed, " \
                           "       gps.init_course_change_ratio AS init_course_change_ratio, " \
                           "       gps.init_course_change_segment AS init_course_change_segment, " \
                           "       gps.init_speed_change_ratio AS init_speed_change_ratio, " \
                           "       gps.init_speed_change_segment AS init_speed_change_segment, " \
                           "       gps.last_update AS gps_last_update, " \
                           "       vessel_types.type_name AS static_vessel_type_name, " \
                           "       cargo_types.type_name AS voyage_cargo_type_name, " \
                           "       nav_statuses.status_name AS nav_status_name, " \
                           "       nav_statuses.static_voyage_interval AS nav_status_static_voyage_interval, " \
                           "       nav_statuses.dynamic_interval AS nav_status_dynamic_interval " \
                           "FROM vessels " \
                           "LEFT JOIN gps ON vessels.id = gps.vessel_id " \
                           "LEFT JOIN ais ON vessels.id = ais.vessel_id " \
                           "LEFT JOIN cargo_types ON ais.voyage_cargo_ITU_id = cargo_types.ITU_id " \
                           "LEFT JOIN vessel_types ON ais.static_type_ITU_id = vessel_types.ITU_id " \
                           "LEFT JOIN nav_statuses ON ais.nav_status_ITU_id = nav_statuses.ITU_id  "

#define SQL_WHERE_SELF "WHERE vessels.self = %1 "
#define SQL_WHERE_ID   "WHERE vessels.id = %1 "
#define SQL_WHERE_LAST_INSERTED "WHERE vessels.id = last_insert_rowid();"


#define SQL_ORDER_BY_VESSELS_ID_ASC "ORDER BY vessels.id ASC;"
#define SQL_ORDER_BY_VESSELS_ID_DESC "ORDER BY vessels.id DESC;"


#define SQL_SELECT_VESSELS_WHERE_SELF (SQL_SELECT_VESSELS SQL_WHERE_SELF)

#define SQL_SELECT_VESSEL_WHERE_ID (SQL_SELECT_VESSELS SQL_WHERE_ID)

#define SQL_SELECT_LAST_INSERTED_VESSEL (SQL_SELECT_VESSELS SQL_WHERE_LAST_INSERTED)


#define SQL_INSERT_NEW_VESSEL "INSERT INTO vessels (self) VALUES (%1);"

#define SQL_INSERT_NEW_AIS "INSERT INTO ais (vessel_id, " CR \
                           "                 static_mmsi, " CR \
                           "                 static_imo, " CR \
                           "                 static_type_ITU_id, " CR \
                           "                 static_callsign, " CR \
                           "                 static_name, " CR \
                           "                 static_pos_ref_A, " CR \
                           "                 static_pos_ref_B, " CR \
                           "                 static_pos_ref_C, " CR \
                           "                 static_pos_ref_D, " CR \
                           "                 static_DTE, " CR \
                           "                 static_talker_id, " CR \
                           "                 voyage_destination, " CR \
                           "                 voyage_ETA_utc, " CR \
                           "                 voyage_ETA_day, " CR \
                           "                 voyage_ETA_month, " CR \
                           "                 voyage_draft, " CR \
                           "                 voyage_cargo_ITU_id, " CR \
                           "                 voyage_team)  " CR \
                           "VALUES ((select id from vessels order by id desc limit 1), " CR \
                           "        %1, %2, %3, '%4', '%5', %6, %7, %8, %9, %10, '%11', '%12', '%13', %14, %15, %16, %17, %18);"

#define SQL_INSERT_NEW_GPS "INSERT INTO gps (vessel_id," CR \
                           "                 timeout," CR \
                           "                 init_random_coordinates," CR \
                           "                 init_random_course," CR \
                           "                 init_random_speed," CR \
                           "                 init_course_change_ratio," CR \
                           "                 init_speed_change_ratio," CR \
                           "                 init_course_change_segment," CR \
                           "                 init_speed_change_segment," CR \
                           "                 last_update)" CR \
                           "VALUES ((select id from vessels order by id desc limit 1), " CR \
                           "        %1, '%2', '%3', '%4', %5, %6, %7, %8, " CR \
                           "        strftime('%Y-%m-%d %H:%M:%f', 'now', 'localtime'));"


#define SQL_UPDATE_AIS "UPDATE ais SET static_mmsi = %1, " CR \
                       "               static_imo = %2, " CR \
                       "               static_type_ITU_id = %3, " CR \
                       "               static_callsign = '%4', " CR \
                       "               static_name = '%5', " CR \
                       "               static_pos_ref_A = %6, " CR \
                       "               static_pos_ref_B = %7, " CR \
                       "               static_pos_ref_C = %8, " CR \
                       "               static_pos_ref_D = %9, " CR \
                       "               static_DTE = %10, " CR \
                       "               static_talker_id = '%11', " CR \
                       "               voyage_destination = '%12', " CR \
                       "               voyage_ETA_utc = '%13', " CR \
                       "               voyage_ETA_day = %14, " CR \
                       "               voyage_ETA_month = %15, " CR \
                       "               voyage_draft = %16, " CR \
                       "               voyage_cargo_ITU_id = %17, " CR \
                       "               voyage_team = %18  " CR \
                       "WHERE vessel_id = %19 "



#define SQL_UPDATE_GPS "UPDATE gps SET timeout = %1," CR \
                       "               init_random_coordinates = '%2'," CR \
                       "               init_random_course = '%3'," CR \
                       "               init_random_speed = '%4'," CR \
                       "               init_course_change_ratio = %5," CR \
                       "               init_speed_change_ratio = %6," CR \
                       "               init_course_change_segment = %7," CR \
                       "               init_speed_change_segment = %8," CR \
                       "               last_update = strftime('%Y-%m-%d %H:%M:%f', 'now', 'localtime')" CR \
                       "WHERE vessel_id = %9"

//#define SQL_UPDATE_AIS_DYNAMIC "UPDATE ais SET 

#define SQL_SELECT_VESSEL_TYPES "SELECT ITU_id, type_name FROM vessel_types;"
#define SQL_SELECT_CARGO_TYPES "SELECT ITU_id, type_name FROM cargo_types;"
#define SQL_SELECT_NAV_STATS "select ITU_id, status_name, static_voyage_interval, dynamic_interval from nav_statuses"
#define SQL_SELECT_LAG_TYPES "select id, type_name from lag_types"


#define SQL_SELECT_FROM_DEVICES_PARAMS  "SELECT id, device_type, vessel_id, port_name, baudrate, parity, stop_bits, " \
                                       "data_bits, flow_control, description, is_active, upload_interval, args, " \
                                       "alarm_id, alarm_state, alarm_text, " \
                                       "network_interface, network_protocol, network_ip, network_port "\
                                       "FROM devices_params"

#define SQL_SELECT_COUNT_DEVICES_PARAMS_WHERE  "SELECT count() as count " \
                                              "FROM devices_params WHERE device_type = %1"

#define SQL_INSERT_DEVICES_PARAMS  "INSERT INTO devices_params (device_type) VALUES(%1)"

#define SQL_UPDATE_DEVICES_SERIAL_PARAMS_WHERE  "UPDATE devices_params SET port_name='%1', baudrate=%2, "\
                                               "parity=%3, stop_bits=%4, data_bits=%5, flow_control=%6"\
                                               "WHERE device_type = %8"

#define SQL_UPDATE_DEVICES_NETWORK_PARAMS_WHERE  "UPDATE devices_params SET network_interface=%1, network_protocol=%2, "\
                                                 "network_ip=%3, network_port=%4, description='%5'  "\
                                                 "WHERE device_type = %6"

#define SQL_UPDATE_DEVICES_PARAMS_WHERE  "UPDATE devices_params SET is_active=%1, upload_interval=%2, args='%3', "\
                                         "alarm_id=%4, alarm_state=%5, alarm_text='%6' " \
                                         "WHERE device_type = %7"

#define SQL_SELECT_NAVTEX "SELECT navtex.id as id, " CR \
                           "      navtex.station_region_id as station_region_id, " CR \
                           "      navtex.message_id as message_id, " CR \
                           "      navtex.message_text as message_text, " CR \
                           "      navtex.transmit_frequency as transmit_frequency, " CR \
                           "      navtex_messages.letter_id as message_letter_id, " CR \
                           "      navtex_messages.designation as message_designation, " CR \
                           "      navtex_messages.simple_message as message_simple_message, " CR \
                           "      navtex_messages.last_number as message_last_number, " CR \  
                           "      navtex_regions.letter_id as region_letter_id, " CR \
                           "      navtex_regions.station_name as region_station_name, " CR \
                           "      navtex_regions.country as region_country" CR \
                           "FROM navtex " CR \
                           "LEFT JOIN navtex_regions on navtex.station_region_id = navtex_regions.id " CR \
                           "LEFT JOIN navtex_messages on navtex.message_id = navtex_messages.id  "

#define SQL_SELECT_NAVTEX_WHERE_ID (SQL_SELECT_NAVTEX " WHERE navtex.id = %1")

#define SQL_SELECT_NAVTEX_MESSAGES "SELECT id, " CR \
                                   "       letter_id, " CR \
                                   "       designation, " CR \
                                   "       simple_message " CR \
                                   "FROM navtex_messages " CR \
                                   "ORDER BY letter_id ASC"

#define SQL_SELECT_NAVTEX_REGIONS "SELECT id, " CR \
                                  "       letter_id, " CR \
                                  "       station_name, " CR \
                                  "       country " CR \
                                  "FROM navtex_regions " CR \
                                  "ORDER BY letter_id ASC"

#define SQL_UPDATE_NAVTEX "UPDATE navtex SET station_region_id = %1, " CR \
                          "                  message_id = %2, " CR \
                          "                  message_text = '%3', " CR \
                          "                  transmit_frequency = %4 " CR \
                          "WHERE id = %5 "  

#define SQL_INSERT_NAVTEX "INSERT INTO navtex (station_region_id, message_id, message_text, transmit_frequency) " CR \
                          "VALUES(%1, %2, '%3', %4)"

#define SQL_SELECT_NAVTEX_SIMPLE_MESSAGE "SELECT simple_message " CR \
                                         "FROM navtex_messages " CR \
                                         "WHERE id = %1"


#endif // SQL_DEFS_H
