#ifndef SV_GPS_H
#define SV_GPS_H

#include <QObject>
#include <QThread>
#include <QApplication>
#include <QTime>
#include <random>
#include <qmath.h>

#include "sv_idevice.h"
#include "geo.h"

namespace gps {

  struct gpsInitParams {
  
    quint32 gps_timeout;
    
    geo::GEOPOSITION geoposition;
    
//    quint32 course;
    quint32 course_change_segment;
    quint32 course_change_ratio;
    
//    quint32 speed;
    qreal speed_change_segment;
    quint32 speed_change_ratio;
    
    quint32 roll_pitch_change_ratio;
    quint32 roll_change_segment;
    quint32 pitch_change_segment;
    
    quint32 multiplier;
    
    bool init_random_coordinates;
    bool init_random_course;
    bool init_random_speed;
    bool init_random_roll;
    bool init_random_pitch;
    
  };

  class SvGPS;
  class SvGPSEmitter;
  class SvGPSNetworkInterface;
  
}

class gps::SvGPS : public idev::SvIDevice
{
  Q_OBJECT
  
public:
  SvGPS(int vessel_id, gps::gpsInitParams &params, geo::BOUNDS& bounds, QDateTime lastUpdate);
  ~SvGPS(); 
  
  int vesselId() { return _vessel_id; }
  
  void setInitParams(gps::gpsInitParams &params) { _gps_params = params; _current_geo_position = params.geoposition; }
  gps::gpsInitParams initParams() { return _gps_params; }
  
  idev::SvSimulatedDeviceTypes type() const { return idev::sdtGPS; }
    
  void waitWhileRunned() { while(_gps_emitter != nullptr) qApp->processEvents();  }
  
  geo::GEOPOSITION *currentGeoposition() { return &_current_geo_position; }
  
  
private:
  geo::GEOPOSITION _current_geo_position;
  
  gps::SvGPSEmitter* _gps_emitter = nullptr;
  
  int _vessel_id = -1;
  gps::gpsInitParams _gps_params;
  geo::BOUNDS _bounds;
  
  QDateTime _last_update;
  
  QMutex _mutex;
  
  quint32 _multiplier = 1;
  
public slots:
  bool open();
  void close();
  
  bool start(quint32 msecs = 1);
  void stop();
  
  void set_multiplier(quint32 multiplier) { _multiplier = multiplier;  }
  
private slots:
  void on_newGPSData(const geo::GEOPOSITION &geopos);
  
signals:
  void newGPSData(const geo::GEOPOSITION& geopos);
  void passed1m(const geo::GEOPOSITION& geopos);
  
};

class gps::SvGPSEmitter: public QThread
{
  Q_OBJECT
  
public:
  
  SvGPSEmitter(int vessel_id, gps::gpsInitParams &params, geo::BOUNDS& bounds, quint32 multiplier);
  ~SvGPSEmitter(); 
  
  int vesselId() { return _vessel_id; }
  
  geo::GEOPOSITION currentGeoPosition() const { return _current_geo_position; }
  
  void stop();

  
private:
  void run() Q_DECL_OVERRIDE;
  
  int _vessel_id = -1;
  
  bool _started = false;
  bool _finished = false;
  
  gps::gpsInitParams _gps_params;
  geo::BOUNDS _bounds;
  geo::GEOPOSITION _current_geo_position;
  
  // параметры, необходимые для расчетов
  qreal _one_tick_length;         // длина пути в метрах, за один отсчет
  quint32 _multiplier;
  
  qreal normalize_course(quint32 course);

signals:
  void newGPSData(const geo::GEOPOSITION& geopos);
  void passed1m(const geo::GEOPOSITION& geopos);
  
};


class gps::SvGPSNetworkInterface : public idev::SvINetworkDevice
{
  Q_OBJECT
  
public:
  SvGPSNetworkInterface(int vessel_id, svlog::SvLog &log);
//  ~SvGPSNetworkInterface();
  
  void setVesselId(int id) { _vessel_id = id; }
  int vesselId() { return _vessel_id; }
  
private:
  svlog::SvLog _log;
  
  int _vessel_id = -1;
  
  idev::NetworkParams _params;
  
  geo::GEOPOSITION _current_geoposition;
  
//  void send();
  
public slots:
  void newGPSData(const geo::GEOPOSITION& geopos);
  
};

#endif // SV_GPS_H
