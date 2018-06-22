#include "sv_gps.h"

gps::SvGPS* SELF_GPS;
extern geo::UnitsInfo CMU;

#define CLOCK 10.0

gps::SvGPS::SvGPS(int vessel_id, gpsInitParams &params, geo::BOUNDS &bounds, QDateTime lastUpdate)
{
  _vessel_id = vessel_id;
  _bounds = bounds;
  
  _last_update = lastUpdate;
  
  setInitParams(params);
  
}

gps::SvGPS::~SvGPS()
{
  if(_gps_emitter)
    delete _gps_emitter;
  
  deleteLater();
}
  
void gps::SvGPS::setInitParams(gps::gpsInitParams &params)
{ 
  _mutex.lock();
  
  _gps_params = params;
  _current_geo_position = params.geoposition;
  
  _mutex.unlock();
  
//  if(_gps_emitter) {
//    qDebug() << _gps_params.geoposition.latitude << _gps_params.geoposition.course;
//    _gps_emitter->setInitParams(params);
    
//    if(_vessel_id == 9)
//      qDebug() << _gps_params.geoposition.latitude << _gps_params.geoposition.course;
//  }
}

bool gps::SvGPS::open()
{
  _isOpened = true;
}
void gps::SvGPS::close()
{
  _isOpened = false;
}

bool gps::SvGPS::start(quint32 msecs)
{
  if(!_isActive)
    return true;
  
  if(_gps_emitter) 
    delete _gps_emitter;
  
  _gps_emitter = new gps::SvGPSEmitter(_vessel_id, &_gps_params, &_current_geo_position, _bounds, _multiplier, _step_by_step, &_mutex);
  connect(_gps_emitter, &gps::SvGPSEmitter::finished, _gps_emitter, &gps::SvGPSEmitter::deleteLater);
  connect(_gps_emitter, &gps::SvGPSEmitter::newGPSData, this, &SvGPS::on_newGPSData);
//  connect(_gps_emitter, SIGNAL(passed1m(geo::GEOPOSITION)), this, SIGNAL(passed1m(geo::GEOPOSITION)));
  connect(_gps_emitter, &gps::SvGPSEmitter::passed1m, this, &SvGPS::on_passed1m);
  connect(this, &SvGPS::nextStep, _gps_emitter, &gps::SvGPSEmitter::nextStep);
  _gps_emitter->start();
  
}

void gps::SvGPS::stop()
{
  if(!_gps_emitter) return;
  
  delete _gps_emitter;
  _gps_emitter = nullptr;
  
}

void gps::SvGPS::on_newGPSData()
{
//  _current_geo_position = geopos;
  emit newGPSData(_current_geo_position);
}
void gps::SvGPS::on_passed1m()
{
  emit passed1m(_current_geo_position);
}


/** ******  EMITTER  ****** **/
gps::SvGPSEmitter::SvGPSEmitter(int vessel_id, gps::gpsInitParams* params, geo::GEOPOSITION* geopos, geo::BOUNDS& bounds, quint32 multiplier, bool step_by_step, QMutex* mutex)
{
  _vessel_id = vessel_id;
  _bounds = bounds;
  _multiplier = multiplier;
  _step_by_step = step_by_step;

  _gps_params = params; 
  _current_geo_position = geopos;
  
  // длина пути в метрах, за один отсчет таймера // скорость в узлах. 1 узел = 1852 метра в час
  _one_tick_length = _current_geo_position->speed * CMU.MetersCount / 3600.0 / (1000.0 / CLOCK /*qreal(_gps_params.gps_timeout)*/) * _multiplier;
  
  _mutex = mutex;
  
}

gps::SvGPSEmitter::~SvGPSEmitter()
{
  stop();
  deleteLater();  
}

//void gps::SvGPSEmitter::setInitParams(gps::gpsInitParams *params, geo::GEOPOSITION* geopos)
//{ 
////  _mutex.lock();
  
//  _gps_params = params; 
//  _current_geo_position = &(params->geoposition);
  
//  // длина пути в метрах, за один отсчет таймера // скорость в узлах. 1 узел = 1852 метра в час
//  _one_tick_length = _current_geo_position.speed * CMU.MetersCount / 3600.0 / (1000.0 / CLOCK /*qreal(_gps_params.gps_timeout)*/) * _multiplier;

////  _mutex.unlock();
//}

void gps::SvGPSEmitter::stop()
{
  _started = false;
  while(!_finished) QApplication::processEvents();
}

void gps::SvGPSEmitter::run()
{
  
  qreal course_segment_counter = 0.0;
  qreal speed_segment_counter = 0.0;
  qreal pass1m_segment_counter = 0.0;
  _started = true;
  _finished = false;

  quint64 calc_timer = QTime::currentTime().msecsSinceStartOfDay();
  quint64 emit_timer = QTime::currentTime().msecsSinceStartOfDay();
  
  while(_started) {
    
    if(QTime::currentTime().msecsSinceStartOfDay() - calc_timer < CLOCK /*_gps_params.gps_timeout*/) {
//      _mutex.unlock(); // обязательно разлочиваем, а то получается livelock
      usleep(100); // чтоб не грузило систему
      continue;
    }
    
    if(_step_by_step && !_next_step)
      continue;
    
    
    
    calc_timer = QTime::currentTime().msecsSinceStartOfDay() - 1;
    
    if(QTime::currentTime().msecsSinceStartOfDay() - emit_timer >= _gps_params->gps_timeout) {
      emit newGPSData(/*_current_geo_position*/);
      emit_timer = QTime::currentTime().msecsSinceStartOfDay();
    }
    
   _mutex->lock();
   
    /** вычисляем новый курс **/
    if((!_gps_params->init_fixed_course) &&
       (course_segment_counter > _gps_params->course_change_segment * CMU.MetersCount)) {
      
      qsrand(QTime::currentTime().msecsSinceStartOfDay());
      int a = qrand() % _gps_params->course_change_ratio;
      int b = -1 + (2 * (qrand() % 2));
      
      // нормируем курс, чтобы он вписывался в диапазон 0 - 360 градусов
      _current_geo_position->setCourse(normalize_course(_current_geo_position->course + a * b));

      course_segment_counter = -_one_tick_length;
      
    }
    course_segment_counter += _one_tick_length;
    
    /** вычисляем новую скорость **/
    if((!_gps_params->init_fixed_speed) && 
       (speed_segment_counter > _gps_params->speed_change_segment * CMU.MetersCount)) {
      
      qsrand(QTime::currentTime().msecsSinceStartOfDay());
      int a = _current_geo_position->speed * (qreal(qrand() % _gps_params->speed_change_ratio) / 100.0);
      int b = (_current_geo_position->speed + a) > 30 ? -1 : -1 + (2 * (qrand() % 2));
      
      _current_geo_position->setSpeed(_current_geo_position->speed + a * b);
      
      speed_segment_counter = -_one_tick_length;
      
    }
    speed_segment_counter += _one_tick_length;
    
    
    _current_geo_position->setUTC(QDateTime::currentDateTimeUtc());
    
    geo::GEOPOSITION new_geopos = geo::get_next_coordinates(*_current_geo_position, _one_tick_length);
    
    // если новая координата выходит за границу карты, то меняем курс и вычисляем новые координаты
    if(!geo::geoposition_within_bounds(new_geopos, _bounds)) {
      
      _current_geo_position->setCourse(normalize_course(_current_geo_position->course + quint64(geo::get_rnd_course()) % 45));
      _mutex->unlock();
      continue;
      
    }
    
    _current_geo_position->setLongtitude(new_geopos.longtitude);
    _current_geo_position->setLatitude(new_geopos.latitude);
    _current_geo_position->setDistance(new_geopos.full_distance);
    
    if(pass1m_segment_counter >= 1.0) {
      
      emit passed1m(/*new_geopos*/);
      pass1m_segment_counter = 0.0;
      _next_step = false;
      
    }
    else {
      
      pass1m_segment_counter += _one_tick_length;
      
    }
    
//    _current_geo_position = new_geopos;
    
    _mutex->unlock();
    
  }
  
  _finished = true;
  
}

qreal gps::SvGPSEmitter::normalize_course(qreal course)
{
  qreal norm_course = course > 0 ? int(course) % 360 : 360 + int(course) % 360;
  return norm_course;
}



gps::SvGPSNetworkInterface::SvGPSNetworkInterface(int vessel_id, svlog::SvLog &log):
  idev::SvINetworkDevice(log)
{
  setVesselId(vessel_id);
  
  _log = log;
  
}

void gps::SvGPSNetworkInterface::newGPSData(const geo::GEOPOSITION& geopos)
{
  _current_geoposition = geopos;
  
  if(!_isOpened) return;
  
  QByteArray packet = QByteArray(); 

  QString s = nmea::gps_RMC(_current_geoposition);
  
  packet.append(s);
  packet.append(1, char(0));
  
  emit newPacket(packet);
}

//void gps::SvGPSNetworkInterface::send()
//{

  
//}
