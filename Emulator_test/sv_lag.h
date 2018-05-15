#ifndef SV_LAG_H
#define SV_LAG_H

#include <QObject>
#include <QThread>
#include <QApplication>
#include <QTime>
#include <QMutex>
#include <QTimer>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QByteArray>

#include "geo.h"
#include "sv_gps.h"
#include "sv_idevice.h"
#include "../../svlib/sv_log.h"
#include "nmea.h"
#include "sv_serialeditor.h"

namespace lag {

  enum MessageType {
    lmtVBW,
    lmtVDR,
    lmtVHW,
    lmtVLW
  };

  struct lagData {                     // Информация о судне. Данные передаются каждые 6 минут
    
    quint32 id;                           // id судна в БД
    QString mmsi;                         // Номер MMSI
    QString imo;                          // Номер Международной морской организации (IMO)
    QString callsign;                     // Радиопозывной и название плавучего средства
    quint32 length;                       // Габариты
    quint32 width;
    QString type;                         // Тип плавучего средства
                                          // Данные о месте антенны (от ГНСС Глонасс или GPS)
      
  };
  
  
  class SvLAG;
  
}

class lag::SvLAG : public idev::SvIDevice
{
  Q_OBJECT
  
public:
  SvLAG(int vessel_id, const geo::GEOPOSITION& geopos, svlog::SvLog &log);
  ~SvLAG(); 
  
  void setVesselId(int id) { _vessel_id = id; }
  int vesselId() { return _vessel_id; }
  
  void setData(const lag::lagData& ldata) { _data = ldata; }
  
  void setSerialPortParams(const SerialPortParams& params);
  
  lag::lagData  *getData() { return &_data; }
    
  bool open();
  void close();
  
  bool start(quint32 msecs);
  void stop();
  
  idev::SvSimulatedDeviceTypes type() const { return idev::sdtLAG; }
  
  void alarm(int id, QString state, QString text);
  
  static QMap<lag::MessageType, QString> msgtypes() { return {{lag::lmtVBW, "VBW (Dual Ground/Water Speed)"},
                                                        {lag::lmtVDR, "VDR (Set and Drift)"}, 
                                                        {lag::lmtVHW, "VHW (Water Speed and Heading)"}, 
                                                        {lag::lmtVLW, "VLW (Dual Ground/Water Distance)"}}; }
  
private:
  lag::lagData _data;
  
  int _vessel_id = -1;
  
  svlog::SvLog _log;
  
  geo::GEOPOSITION _current_geoposition;
  
  QTimer _timer;
  
  QSerialPort _port;
  
  lag::MessageType _msg_type;
  
signals:
  void write_message(const QString& message);
  
private slots:
  void write(const QString& message);
  void read();
  void prepare_message();
  
public slots:
  void newGPSData(const geo::GEOPOSITION& geopos);
  void setMessageType(lag::MessageType msgtype);
  
};

#endif // SV_LAG_H
