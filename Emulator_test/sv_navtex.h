#ifndef SV_NAVTEX_H
#define SV_NAVTEX_H

#include <QObject>
#include <QApplication>
#include <QTime>
#include <QMutex>
#include <QTimer>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QByteArray>

#include "geo.h"
#include "sv_idevice.h"
#include "../../svlib/sv_log.h"
#include "sv_serialeditor.h"
#include "sv_mapobjects.h"

namespace nav {

  struct navtexData {
    
    quint32 region_id;
    quint32 message_id;
    QString region_station_name;
    QString message_designation;
    QString message_text;
    QString message_letter_id;
    QString region_letter_id;
//    QString region_country;
    quint32 message_last_number;
    QString transmit_frequency;
    quint32 transmit_frequency_id;
  };
  
  
  class SvNAVTEX;
  
}

class nav::SvNAVTEX : public idev::SvIDevice
{
  Q_OBJECT
  
public:
  SvNAVTEX(svlog::SvLog &log, int id);
  ~SvNAVTEX(); 
  
  void setData(const nav::navtexData& ndata) { _data = ndata; }
  
  void setSerialPortParams(const SerialPortParams& params);
  
  void setReceiveFrequency(int frequency_id) { _receive_frequency = frequency_id; }
  
  nav::navtexData  *data() { return &_data; }
  
  int id() { return _id; }
  
  bool open();
  void close();
  
  bool start(quint32 msecs);
  void stop();
  
  idev::SvSimulatedDeviceTypes type() const { return idev::sdtNavtex; }
  
  void alarm(int id, QString state, QString text);
  
private:
  int _id;
  
  nav::navtexData _data;
  
  quint32 _receive_frequency = 1;
  
  svlog::SvLog _log;
  
  geo::GEOPOSITION _current_geoposition;
  
  QTimer _timer;
  
  QSerialPort _port;
  
  SvMapObjectVesselAbstract* _map_object = nullptr;
  
signals:
  void write_message(const QStringList &message);
  
private slots:
  void write(const QStringList &messages);
  void prepare_message();
  
  
};

#endif // SV_NAVTEX_H
