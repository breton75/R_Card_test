#include "geo.h"


//geo::Units cur_unit = geo::uKnotsMiles;

QMap<geo::Units, geo::UnitsInfo> units_info = {{geo::uKnotsMiles, geo::UnitsInfo(geo::uKnotsMiles, " миль", " узлов", 1.0, 1852)},
                                               {geo::uKmhKm, geo::UnitsInfo(geo::uKmhKm, " км.", " км/ч", 1.852, 1000)}};

geo::UnitsInfo CMU = units_info.value(geo::uKnotsMiles);


/* длина 1 градуса в зависимости от широты, км */
qreal LAT1DL[91] = 
{111.3, 111.3, 111.3, 111.2, 111.1, 110.9, 110.7, 110.5, 110.2, 110.0,
 109.6, 109.3, 108.9, 108.5, 108.0, 107.6, 107.0, 106.5, 105.9, 105.3,
 104.6, 104.0, 103.3, 102.5, 101.8, 101.0, 100.1,  99.3,  98.4,  97.4,
  96.5,  95.5,  94.5,  93.5,  92.4,  91.3,  90.2,  89.0,  87.8,  86.6,
  85.4,  84.1,  82.9,  81.5,  80.2,  78.8,  77.5,  76.1,  74.6,  73.2,
  71.7,  70.2,  68.7,  67.1,  65.6,  64.0,  62.4,  60.8,  59.1,  57.5,
  55.8,  54.1,  52.4,  50.7,  48.9,  47.2,  45.4,  43.6,  41.8,  40.0,
  38.2,  36.4,  34.5,  32.6,  30.8,  28.9,  27.0,  25.1,  23.2,  21.3,
  19.4,  17.5,  15.5,  13.6,  11.7,   9.7,   7.8,   5.8,   3.9,   1.9, 0.0};
 
/* длина 1 градуса в зависимости от долготы, км */
qreal LON1DL[90] = 
{110.6, 110.6, 110.6, 110.6, 110.6, 110.6, 110.6, 110.6, 110.6, 110.6, 
 110.6, 110.6, 110.6, 110.6, 110.6, 110.7, 110.7, 110.7, 110.7, 110.7, 
 110.7, 110.7, 110.7, 110.8, 110.8, 110.8, 110.8, 110.8, 110.8, 110.8, 
 110.9, 110.9, 110.9, 110.9, 110.9, 111.0, 111.0, 111.0, 111.0, 111.0, 
 111.0, 111.1, 111.1, 111.1, 111.1, 111.1, 111.2, 111.2, 111.2, 111.2, 
 111.2, 111.3, 111.3, 111.3, 111.3, 111.3, 111.4, 111.4, 111.4, 111.4, 
 111.4, 111.4, 111.5, 111.5, 111.5, 111.5, 111.5, 111.5, 111.5, 111.6, 
 111.6, 111.6, 111.6, 111.6, 111.6, 111.6, 111.6, 111.6, 111.7, 111.7, 
 111.7, 111.7, 111.7, 111.7, 111.7, 111.7, 111.7, 111.7, 111.7, 111.7  };

void geo::setCurrentMeasureUnit(geo::Units unit)
{
  CMU = units_info.value(unit);
}

qreal geo::geo2geo_distance(geo::GEOPOSITION& gp1, geo::GEOPOSITION& gp2)
{
  /** расстояние от одной точки координат до другой в МЕТРАХ! **/
  /** http://www.movable-type.co.uk/scripts/latlong.html  ->  Distance **/
  qreal lat1 = qDegreesToRadians(gp1.latitude);
  qreal lat2 = qDegreesToRadians(gp2.latitude);
  qreal dlat = qDegreesToRadians(gp2.latitude - gp1.latitude);
  qreal dlon = qDegreesToRadians(gp2.longtitude - gp1.longtitude);
  
  qreal a = sin(dlat / 2.0) * sin(dlat / 2.0) +
          cos(lat1) * cos(lat2) *
          sin(dlon / 2.0) * sin(dlon / 2.0);
  
  qreal c = 2.0 * atan2(sqrt(a), sqrt(1 - a));
  qreal d = EARTH_RADIUS * c;
//  qDebug() << "dist:" << d;
  
  return d;
}

bool geo::geoposition_within_bounds(const geo::GEOPOSITION& geopos, const geo::BOUNDS& bounds)
{
  return ((geopos.longtitude >= bounds.min_lon) &&
          (geopos.longtitude <= bounds.max_lon) &&
          (geopos.latitude >= bounds.min_lat) &&
          (geopos.latitude <= bounds.max_lat));
}

qreal geo::get_rnd_course(int diff)
{
  qsrand(QTime::currentTime().msec() + diff);
  return qreal(qrand() % 360);
}

qreal geo::get_rnd_speed(int diff)
{
  qsrand(QTime::currentTime().msec() + diff);
  return qreal(qrand() % 50); // максимальная скорость 50 узлов
}

geo::COORDINATES geo::get_rnd_coordinates(const BOUNDS &bounds, int diff)
{
  geo::COORDINATES result;
  
  qreal lat_diff = bounds.max_lat - bounds.min_lat;
  qreal lon_diff = bounds.max_lon - bounds.min_lon;
  
//  QTime t = QTime::currentTime();
  qsrand(QTime::currentTime().msec() + diff); // secsTo(QTime::currentTime()));
  qreal r1 = qreal(qrand() % 100) / 100.0;

  result.latitude = bounds.min_lat + r1 * lat_diff;
  
//  qsrand(t.msecsSinceStartOfDay()); // .secsTo(QTime::currentTime()));
  qreal r2 = qreal(qrand() % 100) / 100.0;
  
  result.longtitude = bounds.min_lon + r2 * lon_diff;
  
  return result;
  
}

geo::GEOPOSITION geo::get_rnd_position(const geo::BOUNDS& bounds, int diff)
{
  geo::GEOPOSITION result;
  
  geo::COORDINATES coord = get_rnd_coordinates(bounds, diff);
  
  result.latitude = coord.latitude;
  result.longtitude = coord.longtitude;
  result.course = get_rnd_course(diff);
  result.speed = get_rnd_speed(diff);
  
  return result;
  
}

geo::GEOPOSITION geo::get_next_geoposition(const geo::GEOPOSITION& geopos, qreal distance)
{
  /** http://www.movable-type.co.uk/scripts/latlong.html **/
  qreal dr = distance / EARTH_RADIUS;
  qreal cr = qDegreesToRadians(qreal(geopos.course));
  qreal lat1 = qDegreesToRadians(geopos.latitude);
  
  qreal lat2 = asin(sin(lat1) * cos(dr) + cos(lat1) * sin(dr) * cos(cr));
  qreal lon2 = atan2(sin(cr) * sin(dr) * cos(lat1), cos(dr) - sin(lat1) * sin(lat2));

  geo::GEOPOSITION new_geopos = geopos;  
  new_geopos.latitude = qRadiansToDegrees(lat2);
  new_geopos.longtitude += qRadiansToDegrees(lon2);
  new_geopos.full_distance += distance;
  
  return new_geopos;
}

qreal geo::lon2lon_distance(qreal min_lon, qreal max_lon, qreal lat)
{
  geo::GEOPOSITION gp1(min_lon, lat, 0, 0);
  geo::GEOPOSITION gp2(max_lon, lat, 0, 0);
  
  return geo::geo2geo_distance(gp1, gp2);
  
}

qreal geo::lat2lat_distance(qreal min_lat, qreal max_lat, qreal lon)
{
  geo::GEOPOSITION gp1(lon, min_lat, 0, 0);
  geo::GEOPOSITION gp2(lon, max_lat, 0, 0);
  
  return geo::geo2geo_distance(gp1, gp2);
  
}
