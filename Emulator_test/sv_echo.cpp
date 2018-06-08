#include "sv_echo.h"

/** --------- ECHO --------- **/
ech::SvECHOAbstract::SvECHOAbstract(int vessel_id, const geo::GEOPOSITION &geopos, const geo::BOUNDS& bounds, QString &imgpath, svlog::SvLog &log):
  idev::SvINetworkDevice(log)
{
  setVesselId(vessel_id);
  
  _current_geoposition = geopos;
  _bounds = bounds;
  _log = log;
  
  /// вычисляем ширину карты и высоту в метрах
  /// непонятный коэфф. 1000. определен методом подбора.
  _map_width_meters =  1000 * geo::lon2lon_distance(_bounds.min_lon, _bounds.max_lon, (_bounds.max_lat + _bounds.min_lat) / 2.0);
  _map_height_meters = 1000 * geo::lat2lat_distance(_bounds.min_lat, _bounds.max_lat, (_bounds.max_lon + _bounds.min_lon) / 2.0);
//  qDebug() << _map_width_meters << _map_height_meters;
  
  if(!_depth_map_image.load(imgpath))
    _log << svlog::Critical << svlog::Time << QString("Не удалось загрузить файл %1").arg(imgpath) << svlog::endl;
  
}

ech::SvECHOAbstract::~SvECHOAbstract()
{
  deleteLater();
}

bool ech::SvECHOAbstract::start(quint32 msecs)
{
  _clearance = msecs;
      
  return idev::SvINetworkDevice::start(msecs);
}

void ech::SvECHOAbstract::calcBeam(ech::Beam* beam)
{
  qreal xm = geo::lon2lon_distance(_bounds.min_lon, _current_geoposition.longtitude, _current_geoposition.latitude);
  qreal ym = geo::lat2lat_distance(_current_geoposition.latitude, _bounds.max_lat, _current_geoposition.longtitude);
  
  qreal dx = beam->index * cos(qDegreesToRadians(_current_geoposition.course));
  qreal dy = beam->index * sin(qDegreesToRadians(_current_geoposition.course));
  
  int x = int(round(xm + dx)) % _depth_map_image.width();
  int y = int(round(ym + dy)) % _depth_map_image.height();
  
  x = x < 0 ? _depth_map_image.width() + x : x;
  y = y < 0 ? _depth_map_image.height() + y : y;
  
//  qDebug() << x0 << y0 
  beam->setXYZ(dx, dy, qreal(qGray(_depth_map_image.pixel(x, y)) + 10));
  beam->setBackscatter(qreal(qGray(_depth_map_image.pixel(y, x)) % 50));
  
}


/** --------------- MULTI -------------- **/
ech::SvECHOMulti::SvECHOMulti(int vessel_id, const geo::GEOPOSITION &geopos, const geo::BOUNDS& bounds, QString &imgpath, svlog::SvLog &log):
  ech::SvECHOAbstract(vessel_id, geopos, bounds, imgpath, log)
{
  
}

ech::SvECHOMulti::~SvECHOMulti()
{
  while(_beams.count())
    delete _beams.takeFirst();
}

void ech::SvECHOMulti::setBeamCount(int count)
{
  while(_beams.count())
    delete _beams.takeFirst();
  
  qreal stepA = qreal(HEADRANGE) / qreal(count - 1);
  int A = HEADRANGE / 2;
  int I = (count % 2 == 0) ? count / 2 : (count - 1) / 2;
  
  for(int i = 0; i < count; i++) {
        
    _beams.append(new ech::Beam);
    _beams.last()->index = I - i;
    if(_beams.last()->index == 0 && count % 2 == 0)
      _beams.last()->index -= 1;
    
//    qDebug() << _beams.last()->index;
    _beams.last()->angle = A - stepA * i;
        
  }
}

void ech::SvECHOMulti::passed1m(const geo::GEOPOSITION& geopos)
{
  if(!_isOpened) 
    return;
  
  if(_clearance_counter < _clearance) {
    _clearance_counter++; 
    return;
  }
  
  _clearance_counter = 1;
  _current_geoposition = geopos;
  
  for(ech::Beam* beam: _beams)
    calcBeam(beam);
  
  send();
  
}

void ech::SvECHOMulti::send()
{
  QByteArray packet = QByteArray(); 
  
  _packet_header.size = sizeof(ech::HeaderMulti) + sizeof(ech::Beam) * _beams.count() + sizeof(quint32);
  _packet_header.num_points = _beams.count();
  _packet_header.ping_number += 1;
  _packet_header.latitude = _current_geoposition.latitude;
  _packet_header.longtitude = _current_geoposition.longtitude;
  _packet_header.bearing = _current_geoposition.course;
  _packet_header.roll = 0;
  _packet_header.pitch = 0;
  _packet_header.heave = 0;

  packet.append((const char*)(&_packet_header), sizeof(ech::HeaderMulti));
    
  for(ech::Beam *beam: _beams)
    packet.append((const char*)(beam), sizeof(ech::Beam));
    
  quint32 crc32 = _crc.calc((unsigned char*)packet.data(), packet.size());
  
  packet.append((const char*)&crc32, sizeof(crc32)); // 4, char(32));
  
  emit newPacket(packet);
  
}



/** --------------- FISH -------------- **/
ech::SvECHOFish::SvECHOFish(int vessel_id, const geo::GEOPOSITION &geopos, const geo::BOUNDS& bounds, QString &imgpath, svlog::SvLog &log):
  ech::SvECHOAbstract(vessel_id, geopos, bounds, imgpath, log)
{
  _beam = new ech::Beam;
  _beam->index = 0;
  _beam->angle = 0;
}

void ech::SvECHOFish::setFishCount(int count)
{
  _fish_count = count;
}

void ech::SvECHOFish::passed1m(const geo::GEOPOSITION& geopos)
{
  if(!_isOpened) 
    return;
  
  if(_clearance_counter < _clearance) {
    
    _packet_header.FISH = 0;
    _clearance_counter++; 
    
    emit updated(&_packet_header);
    return;
  }
  
  _clearance_counter = 1;
  _current_geoposition = geopos;
  
  calcBeam(_beam);
  
  _packet_header.backscatter = _beam->backscatter;
  _packet_header.Z = _beam->Z;
  
  qsrand(_packet_header.Z * QTime::currentTime().msecsSinceStartOfDay());
  if((_fish_counter % (10 - _fish_count)) == 0) 
      _packet_header.FISH = (1 << (qrand() % 48)) & 0xFFFC;
  
  else
    _packet_header.FISH = 0;
    
  _fish_counter++;
      
  emit updated(&_packet_header);
  
  send();
  
}

void ech::SvECHOFish::send()
{
  _packet_header.size = sizeof(ech::HeaderFish) + sizeof(quint32);
  _packet_header.ping_number += 1;
  _packet_header.latitude = _current_geoposition.latitude;
  _packet_header.longtitude = _current_geoposition.longtitude;
  _packet_header.bearing = _current_geoposition.course;
  _packet_header.roll = 0;
  _packet_header.pitch = 0;
  _packet_header.heave = 0; 
  
  QByteArray packet = QByteArray(); 
  packet.append((const char*)(&_packet_header), sizeof(ech::HeaderFish));
  
  quint32 crc32 = _crc.calc((unsigned char*)packet.data(), packet.size());
  
  packet.append((const char*)&crc32, sizeof(crc32)); // 4, char(32));
  
  emit newPacket(packet);
  
}
