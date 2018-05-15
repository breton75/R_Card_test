#include "sv_vessel.h"

//vsl::SvVessel* SELF_VESSEL;
//QMap<int, vsl::SvVessel*> VESSELS;

vsl::SvVessel::SvVessel(QObject *parent, quint32 id/*, bool self*/) :
  QObject(parent)
{
//   qRegisterMetaType<gps::GEO>("gps::GEO");
   
  this->id = id;
//  _self = self;

   
   
}

vsl::SvVessel::~SvVessel()
{
  if(_gps) delete _gps;
  if(_ais) delete _ais;
  if(_lag) delete _lag;
  if(_map_object) delete _map_object;
  
  deleteLater();
  
}

void vsl::SvVessel::updateVessel()
{
    _map_object->setGeoPosition(_ais->dynamicData()->geoposition);
    emit updateMapObjectPos(_map_object, _ais->dynamicData()->geoposition);
}

