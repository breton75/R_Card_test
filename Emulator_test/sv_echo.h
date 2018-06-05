#ifndef SV_ECHO_H
#define SV_ECHO_H

#include <QObject>
#include <QThread>
#include <QApplication>
#include <QTime>
#include <QMutex>
#include <QTimer>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QByteArray>
#include <QUdpSocket>
#include <QTcpServer>
#include <QtEndian>
#include <QtGui/QImage>

#include "../../svlib/sv_log.h"
#include "../../svlib/sv_tcpserverclient.h"

#include "geo.h"
#include "sv_gps.h"
#include "sv_idevice.h"
#include "nmea.h"
#include "sv_networkeditor.h"
#include "sv_crc.h"

#define HEADRANGE 40

namespace ech {

  struct echoData {                     // Информация о судне. Данные передаются каждые 6 минут
    
    quint32 id;                           // id судна в БД
    QString mmsi;                         // Номер MMSI
    QString imo;                          // Номер Международной морской организации (IMO)
    QString callsign;                     // Радиопозывной и название плавучего средства
    quint32 length;                       // Габариты
    quint32 width;
    QString type;                         // Тип плавучего средства
                                          // Данные о месте антенны (от ГНСС Глонасс или GPS)
      
  };
  
  #pragma pack(1)
  struct Beam {
    int index;
    float X;
    float Y;
    float Z;
    float angle;
    float backscatter;
    quint8 quality = 1;
    quint32 fish = 0;
    
    void setXYZ(float X, float Y, float Z) { this->X = X;  this->Y = Y; this->Z = Z;}
    void setBackscatter(float backscatter) { this->backscatter = backscatter; }
    
  };
  
  struct HeaderMulti {
    quint32 sync_pattern = 0x77F9345A;
    quint32 size;
    char header[8] = {'C','O','R','B','A','T','H','Y'};
    quint32 version = 3;
    double time = 100;
    quint32 num_points;
    qint32 ping_number = 0;
    double latitude;
    double longtitude;
    float bearing;
    float roll;
    float pitch;
    float heave;
    quint32 sample_type = 1;
    quint32 spare = 0;
  };
  
  struct HeaderFish {
    quint32 sync_pattern = 0x77F9345A;
    quint32 size;
    char header[8] = {'C','O','R','F','I','S','H','Y'};
    quint32 version = 1;
    double time = 100;
    qint32 ping_number = 0;
    double latitude;
    double longtitude;
    float bearing = 0;
    float roll = 0;
    float pitch = 0;
    float heave = 0;
    quint32 sample_type = 1;
    float Z = 0;
    quint32 FISH = 0;
    float backscatter = 0;
    quint32 spare = 0;
  };  
  #pragma pack(pop)
  
  
  
  class SvECHOAbstract;
  class SvECHOMulti;
  class SvECHOFish;
   
}

class ech::SvECHOAbstract : public idev::SvINetworkDevice
{
  Q_OBJECT
  
public:
  SvECHOAbstract(int vessel_id, const geo::GEOPOSITION& geopos, const geo::BOUNDS& bounds, QString& imgpath, svlog::SvLog &log);
  ~SvECHOAbstract(); 
  
  void setVesselId(int id) { _vessel_id = id; }
  int vesselId() { return _vessel_id; }
  
//  void setNetworkParams(NetworkParams params) { _params = params; }

//  bool open();
//  void close();
  
  bool start(quint32 msecs);
//  void stop();
  
  friend class ech::SvECHOMulti;
  friend class ech::SvECHOFish;
  
private:
  svlog::SvLog _log;
  
  int _vessel_id = -1;
//  NetworkParams _params;
  
  qreal _koeff_lat = 1.0;
  qreal _koeff_lon = 1.0;
  
//  QUdpSocket *_udp = nullptr;
//  svtcp::SvTcpServer *_tcp = nullptr;
  
  geo::GEOPOSITION _current_geoposition;
  geo::BOUNDS _bounds;
  
  qreal _map_width_meters;
  qreal _map_height_meters;
  
  QTimer _timer;
  
  QImage _depth_map_image;
  
  quint32 _clearance = 1;
  quint32 _clearance_counter = 1;
  
  SvCRC32 _crc;
  
  void calcBeam(Beam *beam);
  
//signals:
//  void newPacket(const QByteArray& packet);
  
//private slots:
//  void write(const QByteArray& packet);
  
//public slots:
//  void passed1m(const geo::GEOPOSITION& geopos);
  
};


class ech::SvECHOMulti : public ech::SvECHOAbstract
{
  Q_OBJECT
  
public:
  SvECHOMulti(int vessel_id, const geo::GEOPOSITION& geopos, const geo::BOUNDS& bounds, QString& imgpath, svlog::SvLog &log);
  ~SvECHOMulti(); 

  void setBeamCount(int count);

  idev::SvSimulatedDeviceTypes type() const { return idev::sdtEchoMulti; }
  
//  bool open();
//  void close();
  
//  bool start(quint32 msecs);
//  void stop();
  
private:
  QList<ech::Beam*> _beams;
  
  ech::HeaderMulti _packet_header;
  
  void send();
  
public slots:
  void passed1m(const geo::GEOPOSITION& geopos);
  
};
  
  
class ech::SvECHOFish : public ech::SvECHOAbstract
{
  Q_OBJECT
  
public:
  SvECHOFish(int vessel_id, const geo::GEOPOSITION& geopos, const geo::BOUNDS& bounds, QString& imgpath, svlog::SvLog &log);
//  ~SvECHOFish() { }

  void setFishCount(int count);

  idev::SvSimulatedDeviceTypes type() const { return idev::sdtEchoFish; }
  
//  bool open();
//  void close();
  
//  bool start(quint32 msecs);
//  void stop();
  
private:
  ech::Beam* _beam;
  quint32 _fish_count = 0;
  quint32 _fish_counter = 0;
  
  ech::HeaderFish _packet_header;
  
  void send();

signals:
  void updated(ech::Beam* bl);
  
public slots:
  void passed1m(const geo::GEOPOSITION& geopos);
  
};
  
#endif // SV_ECHO_H
