#ifndef SV_VESSEL_H
#define SV_VESSEL_H

#include <QObject>
#include <QThread>
#include <QIODevice>
#include <QMetaType>
#include <QApplication>
#include <QTime>

#include "geo.h"

#include "sv_gps.h"
#include "sv_ais.h"
#include "sv_lag.h"
#include "sv_echo.h"
#include "sv_mapobjects.h"

namespace vsl {

  class SvVessel;
  
}

class vsl::SvVessel : public QObject
{
  Q_OBJECT
  
public:
  SvVessel(QObject* parent, quint32 id);
    
  ~SvVessel(); 

//  bool isSelf() { return (_self_vessel == nullptr); }
  
//  void setSelfVessel(SvVessel* vsl) { _self_vessel = vsl; }
  
  void mountAIS(ais::SvAIS* ais) { _ais = ais; }
  void mountGPS(gps::SvGPS* gps) { _gps = gps; }
  void mountLAG(lag::SvLAG* lag) { _lag = lag; }
  void mountECHOMulti(ech::SvECHOMulti* echo) { _multi_echo = echo; }
  void mountECHOFish(ech::SvECHOFish* echo) { _fish_echo = echo; }
  
  void assignMapObject(SvMapObjectVesselAbstract* map_object) { _map_object = map_object; }
  SvMapObjectVesselAbstract* mapObject() { return _map_object; }
  
  int id = -1;

  gps::SvGPS* gps() { return _gps; }
  ais::SvAIS* ais() { return _ais; }
  lag::SvLAG* lag() { return _lag; }
  ech::SvECHOMulti* echoMulti() { return _multi_echo; }
  ech::SvECHOFish* echoFish() { return _fish_echo; }
  
  void setActive(bool isActive)
  { 
    _isActive = isActive;
    _gps->setActive(_isActive);
    _ais->setActive(_isActive);
  }
  
  bool isActive() { return _isActive; }
  

private:

  gps::SvGPS* _gps = nullptr;
  ais::SvAIS* _ais = nullptr;
  lag::SvLAG* _lag = nullptr;
  ech::SvECHOMulti* _multi_echo = nullptr;
  ech::SvECHOFish* _fish_echo = nullptr;
  
  SvMapObjectVesselAbstract* _map_object = nullptr;
  
  bool _isActive = true;
  
signals:
  void updateMapObjectPos(SvMapObject* mapObject, const geo::GEOPOSITION& geopos);
  
public slots:
  void updateVessel();
  
  
};


#endif // SV_VESSEL_H
