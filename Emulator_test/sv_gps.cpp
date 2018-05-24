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
  
  _gps_emitter = new gps::SvGPSEmitter(_vessel_id, _gps_params, _bounds, _multiplier);
  connect(_gps_emitter, &gps::SvGPSEmitter::finished, _gps_emitter, &gps::SvGPSEmitter::deleteLater);
  connect(_gps_emitter, &gps::SvGPSEmitter::newGPSData, this, &SvGPS::on_newGPSData);
  connect(_gps_emitter, SIGNAL(passed1m(geo::GEOPOSITION)), this, SIGNAL(passed1m(geo::GEOPOSITION)));
  _gps_emitter->start();
                 
  
}

void gps::SvGPS::stop()
{
  if(!_gps_emitter) return;
  
  delete _gps_emitter;
  _gps_emitter = nullptr;
  
}

void gps::SvGPS::on_newGPSData(const geo::GEOPOSITION &geopos)
{
  _current_geo_position = geopos;
  emit newGPSData(_current_geo_position);
}

/** ******  EMITTER  ****** **/
gps::SvGPSEmitter::SvGPSEmitter(int vessel_id, gps::gpsInitParams& params, geo::BOUNDS& bounds, quint32 multiplier)
{
  _vessel_id = vessel_id;
  _gps_params = params;
  _bounds = bounds;
  _multiplier = multiplier;
  
  _current_geo_position = _gps_params.geoposition;

  // длина пути в метрах, за один отсчет таймера // скорость в узлах. 1 узел = 1852 метра в час
  _one_tick_length = _current_geo_position.speed * CMU.MetersCount / 3600.0 / (1000.0 / CLOCK /*qreal(_gps_params.gps_timeout)*/) * _multiplier;
//  qDebug() << _one_tick_length << _current_geo_position.speed;
}

gps::SvGPSEmitter::~SvGPSEmitter()
{
  stop();
  deleteLater();  
}

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
      usleep(100); // чтоб не грузило систему
      continue;
    }
    
    calc_timer = QTime::currentTime().msecsSinceStartOfDay() - 1;
    
    if(QTime::currentTime().msecsSinceStartOfDay() - emit_timer >= _gps_params.gps_timeout) {
      emit newGPSData(_current_geo_position);
      emit_timer = QTime::currentTime().msecsSinceStartOfDay();
    }
    
    /** вычисляем новый курс **/
    if((_gps_params.course_change_ratio != 0) && 
       (_gps_params.course_change_segment != 0) && 
       (course_segment_counter > _gps_params.course_change_segment * CMU.MetersCount)) {
      
      qsrand(QTime::currentTime().msecsSinceStartOfDay());
      int a = qrand() % _gps_params.course_change_ratio;
      int b = -1 + (2 * (qrand() % 2));
      
      // нормируем курс, чтобы он вписывался в диапазон 0 - 360 градусов
      _current_geo_position.course = normalize_course(_current_geo_position.course + a * b);

      course_segment_counter = -_one_tick_length;
      
    }
    course_segment_counter += _one_tick_length;
    
    /** вычисляем новую скорость **/
    if((_gps_params.speed_change_ratio != 0) && 
       (_gps_params.speed_change_segment != 0) && 
       (speed_segment_counter > _gps_params.speed_change_segment * CMU.MetersCount)) {
      
      qsrand(QTime::currentTime().msecsSinceStartOfDay());
      int a = _current_geo_position.speed * (qreal(qrand() % _gps_params.speed_change_ratio) / 100.0);
      int b = -1 + (2 * (qrand() % 2));
      
      _current_geo_position.speed += a * b;
      
      speed_segment_counter = -_one_tick_length;
      
    }
    speed_segment_counter += _one_tick_length;
    
    
    _current_geo_position.utc = QDateTime::currentDateTimeUtc();
    
    geo::GEOPOSITION new_geopos = geo::get_next_geoposition(_current_geo_position, _one_tick_length);
    
    // если новая координата выходит за границу карты, то меняем курс и вычисляем новые координаты
    if(!geo::geoposition_within_bounds(new_geopos, _bounds)) {
      _current_geo_position.course = normalize_course(_current_geo_position.course + quint64(geo::get_rnd_course()) % 45);
      continue;
    }
    
//    if(_vessel_id == 6) {
      if(pass1m_segment_counter >= 1.0) {
        emit passed1m(new_geopos);
        pass1m_segment_counter = 0.0;
      }
      else {
        pass1m_segment_counter += _one_tick_length;
      }
    
    _current_geo_position = new_geopos;
    
  }
  
  _finished = true;
  
}

qreal gps::SvGPSEmitter::normalize_course(quint32 course)
{
  qreal norm_course = course > 0 ? course % 360 : 360 - qAbs(course % 360);
  return norm_course;
}


