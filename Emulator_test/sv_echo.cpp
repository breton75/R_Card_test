#include "sv_echo.h"

/** --------- ECHO --------- **/
ech::SvECHOAbstract::SvECHOAbstract(int vessel_id, const geo::GEOPOSITION &geopos, const geo::BOUNDS& bounds, QString &imgpath, svlog::SvLog &log)
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
  
  _koeff_lon = _map_width_meters / (_bounds.max_lon - _bounds.min_lon); // / 1000;
  _koeff_lat = _map_height_meters / (_bounds.max_lat - _bounds.min_lat); // / 1000;
  
}

ech::SvECHOAbstract::~SvECHOAbstract()
{
  deleteLater();
}

bool ech::SvECHOAbstract::open()
{
  _isOpened = true;
  return _isOpened;
}

void ech::SvECHOAbstract::close()
{ 
  stop();
  
  _isOpened = false;
}

bool ech::SvECHOAbstract::start(quint32 msecs)
{
  
  if(!_isOpened)
    return false;
  
  if(_udp) delete _udp;
  _udp = nullptr;
  
  if(_tcp) delete _tcp;
  _tcp = nullptr;
  
  if(_params.protocol == QAbstractSocket::UdpSocket) {
  
    _udp = new QUdpSocket();

  }
  else {
    
    _tcp = new svtcp::SvTcpServer(_log, svtcp::DoNotLog, svtcp::DoNotLog);
    qDebug() << "start tcp:" << _params.description;
    if(!_tcp->startServer(_params.port)) {
      
      _log << svlog::Critical << svlog::Time << QString("Ошибка при запуске сервера TCP: %1").arg(_tcp->lastError()) << svlog::endl;
      
      delete _tcp;
      
      return false;
      
    }
    
    connect(this, &ech::SvECHOAbstract::newPacket, this, &ech::SvECHOAbstract::write);
    
  }
  
//  _udp->s MulticastInterface(QNetworkInterface::interfaceFromIndex(_params.ifc));

  _clearance = msecs;
      
  return true;
}

void ech::SvECHOAbstract::stop()
{
  disconnect(this, &ech::SvECHOAbstract::newPacket, this, &ech::SvECHOAbstract::write);
  
  if(_udp) delete _udp;
  _udp = nullptr;
  
  if(_tcp) {
    
    _tcp->stopServer();
    delete _tcp;
    
  }
  
  _tcp = nullptr;
  
}

void ech::SvECHOAbstract::write(const QByteArray &packet)
{
  if(!packet.isEmpty()) {
    
    if(_udp) {
      
      _udp->writeDatagram(packet, QHostAddress(_params.ip), _params.port);
      
    }
    else if(_tcp) {

      _tcp->sendToAll(packet);
      
    }
    
  }
}

void ech::SvECHOAbstract::calcBeam(ech::Beam* beam)
{
  qreal x0 = (_current_geoposition.longtitude - _bounds.min_lon) * _koeff_lon;
  qreal y0 = (_bounds.max_lat - _current_geoposition.latitude) * _koeff_lat;
  
  qreal dx = beam->index * cos(qDegreesToRadians(_current_geoposition.course));
  qreal dy = beam->index * sin(qDegreesToRadians(_current_geoposition.course));
  
  int x = int(round(x0 / 1000 + dx)) % _depth_map_image.width(); // * 2 -;
  int y = _depth_map_image.height() - int(round(y0 / 1000 + dy)) % _depth_map_image.height() - 1;
  
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
  
  _beams.clear();
  
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
  _packet_header.num_points = _beams.count() ; // _beam_count;
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
  
  packet.append(4, char(32));
  
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
    
    _beam->fish = 0;
    _clearance_counter++; 
    
    emit updated(_beam);
    return;
  }
  
  _clearance_counter = 1;
  _current_geoposition = geopos;
  
  calcBeam(_beam);
  
  qsrand(_beam->Z * QTime::currentTime().msecsSinceStartOfDay());
  if((_fish_counter % (10 - _fish_count)) == 0) 
      _beam->fish = (1 << (qrand() % 48)) & 0xFFFC;
  
  else
    _beam->fish = 0;
    
  _fish_counter++;
      
  emit updated(_beam);
  
  send();
  
}

void ech::SvECHOFish::send()
{
  QByteArray packet = QByteArray(); 
  
  _packet_header.size = sizeof(ech::HeaderMulti) + sizeof(ech::Beam) + sizeof(quint32);
  _packet_header.ping_number += 1;
  _packet_header.latitude = _current_geoposition.latitude;
  _packet_header.longtitude = _current_geoposition.longtitude;
  _packet_header.bearing = _current_geoposition.course;
  _packet_header.roll = 0;
  _packet_header.pitch = 0;
  _packet_header.heave = 0; 
  
  packet.append((const char*)(&_packet_header), sizeof(ech::HeaderFish));
  packet.append((const char*)(_beam), sizeof(ech::Beam));
  packet.append(4, char(32));
  
  emit newPacket(packet);
  
}
