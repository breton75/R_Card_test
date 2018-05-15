#ifndef GEO_CALCULATIONS_H
#define GEO_CALCULATIONS_H

#include <QObject>
#include <QDebug>
#include <QWidget>
#include <math.h>
#include <QDateTime>
#include <qmath.h>
#include <QMetaType>

#define EARTH_RADIUS 6372795.0

namespace geo {

  enum Units {
    uKnotsMiles,
    uKmhKm
  };

  struct UnitsInfo {
    UnitsInfo() { }
    UnitsInfo(Units unit, QString distance_designation, QString speed_designation, qreal convert, quint32 meters)
    { Unit = unit; DistanceDesign = distance_designation; SpeedDesign = speed_designation; ConvertKoeff = convert; MetersCount = meters; }
    
    Units Unit;
    QString DistanceDesign;
    QString SpeedDesign;
    qreal ConvertKoeff;
    quint32 MetersCount;
  };
  
  struct BOUNDS {
    qreal min_lat;
    qreal min_lon;
    qreal max_lat; 
    qreal max_lon;
  };
  
  struct COORDINATES {
    COORDINATES() { }
    COORDINATES(qreal longtitude, qreal latitude) { this->latitude = latitude; this->longtitude = longtitude; }
    qreal latitude;
    qreal longtitude; 
    geo::COORDINATES& operator =(const geo::COORDINATES& other) { latitude = other.latitude; longtitude = other.longtitude; }
  };

  
  class GEOPOSITION {
  public:
    
    GEOPOSITION() { }
    
    GEOPOSITION(qreal longtitude, qreal latitude, quint32 course, qreal speed) { 
      
      this->latitude = latitude; this->longtitude = longtitude; 
      this->course = course; this->speed = speed; this->utc = QDateTime::currentDateTime();
    }
    double counter = 0.0;
    qreal latitude = -1.0;
    qreal longtitude = -1.0; 
    qreal course = -1.0;
    qreal speed = -1.0;
    QDateTime utc = QDateTime::currentDateTimeUtc();
    quint8 accuracy = 1;
    quint8 rate_of_turn = 0;
    qreal drift = 0.0;
    qreal pitch = 0.0;   // тангаж в радианах
    qreal roll = 0.0;     // крен в радианах
    
    qreal full_distance = 0.0;
    
//    UNITS units = uKmhKm;
    
    bool isValid() { return ((longtitude != -1.0) && (latitude != -1.0) && (course != -1) && (speed != -1.0)); }
    bool isValidCoordinates() { return ((longtitude != -1.0) && (latitude != -1.0)); }
    bool isValidCourse() { return course != -1.0; }
    bool isValidSpeed() { return speed != -1.0; }
    
    geo::GEOPOSITION& operator =(const geo::GEOPOSITION& other)
    { 
      latitude = other.latitude; longtitude = other.longtitude; 
      course = other.course; speed = other.speed; utc = other.utc;
      accuracy = other.accuracy; full_distance = other.full_distance;
    }
    
  };
  
  // расстояние между двумя координатами 
  qreal geo2geo_distance(geo::GEOPOSITION& gp1, geo::GEOPOSITION& gp2); 
   
  // расстояние в километрах между двумя широтами/долготами 
  qreal lon2lon_distance(qreal min_lon, qreal max_lon, qreal lat);
  qreal lat2lat_distance(qreal min_lat, qreal max_lat, qreal lon);
  
  geo::GEOPOSITION get_next_geoposition(const geo::GEOPOSITION& geopos, qreal distance);
  
  bool geoposition_within_bounds(const geo::GEOPOSITION& geopos, const geo::BOUNDS& bounds);
  
  qreal get_rnd_course(int diff = 0);
  qreal get_rnd_speed(int diff = 0);
  geo::COORDINATES get_rnd_coordinates(const geo::BOUNDS& bounds, int diff = 0);
  
  geo::GEOPOSITION get_rnd_position(const BOUNDS &bounds, int diff = 0);
  
  void setCurrentMeasureUnit(geo::Units unit);
  
}


#endif // GEO_CALCULATIONS_H
