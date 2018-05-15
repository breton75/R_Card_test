#include "sv_udpstream.h"



/******************************************************/
/* ************       SvUdpWriter      ************** */
/******************************************************/
SvUdpWriter::SvUdpWriter(QString ip, quint16 port, 
                         quint16 speed, quint16 maxAngle, 
                         quint16 const_course_length, quint16 timeout, 
                         QObject *parent) : 
  QThread(parent)
{
  setParent(parent);
  _ip = ip;
  _port = port;
  _speed = speed;
  _maxAngle = maxAngle;
  _timeout = timeout;
  
  _min_lon = 29.7318;
  _max_lon = 30.2708;
  _min_lat = 59.8543;
  _max_lat = 60.0182;
  
  /* длина пути в метрах, за один отсчет таймера */
  _segment_length = qreal(_speed * 1000) / 3600 / (1000 / timeout);
  qDebug() << timeout << "_segment_length" <<_segment_length;
  
  /* длина пути без изменения курса */
  _const_course_length = const_course_length;
  
  /* 1 метр вдоль долготы */
  _lon1m = (_max_lat - _min_lat) / lat1_lat2_distance(_min_lat, _max_lat, _data.lon) / 1000;
  
//  /* 1 метр вдоль широты */
  _lat1m = (_max_lon - _min_lon) / lon1_lon2_distance(_min_lon, _max_lon, _data.lat) / 1000;
}

SvUdpWriter::~SvUdpWriter()
{
  deleteLater();
}

void SvUdpWriter::run()
{
  udp = new QUdpSocket();
  
  /* признак новых координат */
  _data.c1 = char(0);
  _data.c2 = char(0);
  
  /* начальные координаты самолета */
  _data.lon = (_max_lon + _min_lon) / 2;
  _data.lat = (_max_lat + _min_lat) / 2;
//  qDebug() << "1. _data.lon" << _data.lon << "_data.lat" << _data.lat;
  
  /* начальный угол поворота */
  QTime t(0,0,0);
  qsrand(t.secsTo(QTime::currentTime()));
  _data.angle = qrand() % 360;
//  _data.angle = _maxAngle;
//  qDebug() << "1. " << _data.angle;
  
//  udp->writeDatagram((char*)(&_data), sizeof(Data), QHostAddress(_ip), _port);
//  return;
  
  _isActive = true;
  int counter = 0;
  int max_count = 10;
  while(_isActive)
  {

    udp->writeDatagram((char*)(&_data), sizeof(Data), QHostAddress(_ip), _port);

    msleep(_timeout);
    
    if(counter > _const_course_length)
    {
      /* вычисляем новый угол поворота */
      int a = _maxAngle == 0 ? 0 : qrand() % _maxAngle;
      int b = -1 + (2 * (qrand() % 2));
      _data.angle += a * b;
      normal_angle();
      counter = -_segment_length;
//      _data.angle = _maxAngle;
    }

    counter += _segment_length;
    
    _data.lon += lonOffset();
    _data.lat += latOffset();
    
  }
  
  udp->close();
  
}

qreal SvUdpWriter::lonOffset()
{
  qreal b;
  
  int q = quarter();
  switch (q) {
    case 1:
      b = sin(_data.angle * M_PI / 180);
      break;
      
    case 2:
      b = cos((_data.angle - 90) * M_PI / 180);
      break;
      
    case 3:
      b = -sin((_data.angle - 180) * M_PI / 180);
      break;
      
    case 4:
      b = -cos((_data.angle - 270) * M_PI / 180);
      break;
      
    case 0:
    case 180:  
      b = 0;
      break;
      
    case 90:
      b = 1;
      break;
      
    case 270:
      b = -1;
      break;
      
    default:
      qDebug() << "!!!! wrong quarter b. angle=" << _data.angle;
      break;

  }
//  qDebug() << b * _segment_length * _lon1m;
  return b * _segment_length * _lon1m;
}

qreal SvUdpWriter::latOffset()
{
  qreal a;
  
  int q = quarter();
  switch (q) {
    case 1:
      a = cos(_data.angle * M_PI / 180);
      break;
      
    case 2:
      a = -sin((_data.angle - 90) * M_PI / 180);
      break;
      
    case 3:
      a = -cos((_data.angle - 180) * M_PI / 180);
      break;
      
    case 4:
      a = sin((_data.angle - 270) * M_PI / 180);
      break;
      
    case 0:
      a = 1;
      break;
      
    case 90:
    case 270:
      a = 0;
      break;
      
    case 180:
      a = -1;
      break;
      
      
    default:
      qDebug() << "!!!! wrong quarter a. angle=" << _data.angle;
      break;
  }
//  qDebug() << b * _segment_length * _lat1m;
  return a * _segment_length * _lat1m;
}

void SvUdpWriter::normal_angle()
{
  _data.angle = _data.angle > 0 ? _data.angle % 360 : 360 - qAbs(_data.angle % 360); //   * (qAbs(int(_data.angle / 360)) + 1) + _data.angle;
//  _data.angle = _data.angle % 360; // != 0 ? _data.angle % 360 : 0; //_data.angle;
}   

int SvUdpWriter::quarter()
{
  switch (_data.angle % 90) {
    case 0:
      return int(_data.angle);
      break;
      
    default:
      return int(_data.angle / 90) + 1;
      break;
  }
}

/******************************************************/
/* ************       SvUdpReader      ************** */
/******************************************************/
SvUdpReader::SvUdpReader(const QString &ip, const quint16 port, QObject *parent)
    : QThread(parent)
{
  setParent(parent);
  _ip = ip;
  _port = port;
}

SvUdpReader::~SvUdpReader()
{
  delete _socket;
  deleteLater();
}

void SvUdpReader::run()
{
  _socket = new QUdpSocket();
  _socket->bind(_port);
  
  _isActive = true;
  while (_isActive)
  {
      while(_socket->waitForReadyRead(500))
      {
          while(_socket->hasPendingDatagrams())
          {
              _socket->readDatagram((char*)(&_data), sizeof(Data));
              
              if((_data.c1 != 0) || (_data.c2 != 0))
              {
                qDebug() << "wrong data";
                continue;
              }
//              qDebug() << "data.angle" << _data.angle;
              emit dataReaded(_data.lon, _data.lat, _data.angle);                

          }
      }
  }

  _socket->close();
}

