#ifndef SV_AIS_H
#define SV_AIS_H

#include <QObject>
#include <QThread>
#include <QApplication>
#include <QTime>
#include <QMutex>
#include <QTimer>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QUdpSocket>

#include "geo.h"
#include "sv_idevice.h"
#include "sv_serialeditor.h"

#include "../../svlib/sv_log.h"
#include "../../svlib/sv_sqlite.h"
#include "../../svlib/sv_exception.h"

#define DO_NOT_CHANGE_s "do not change"
#define DO_NOT_CHANGE_i -1

namespace ais {

  enum AISDataTypes {
    adtStatic,
    adtVoyage,
    adtStaticVoyage,
    adtDynamic
  };
  
  struct aisStaticVoyageData {        // Информация о судне. Данные передаются каждые 6 минут
    
    quint32 id;                           // id судна в БД
    quint32 mmsi;                         // Номер MMSI
    quint32 imo;                          // Номер Международной морской организации (IMO)
    QString callsign;                     // Радиопозывной
    QString name;                         // Название плавучего средства
    quint32 pos_ref_A;                    // Данные о месте антенны (от ГНСС Глонасс или GPS)
    quint32 pos_ref_B;                    // Габариты (A + B = длина; C + D = ширина)
    quint32 pos_ref_C;
    quint32 pos_ref_D;
    quint32 vessel_ITU_id;                // Тип плавучего средства
//    QString type_name;
    quint8  DTE;                          // флаг DTE
    QString talkerID = "AI";              // идентификатор устройства для NMEA
    
    QString destination;                  // Пункт назначения
    QTime ETA_utc;                        // Время прибытия (ЕТА)
    quint8 ETA_day;
    quint8 ETA_month;
    qreal draft;                          // Осадка судна
    quint32 cargo_ITU_id;                 // тип опасности груза
//    QString cargo_type_name;              // Информация о грузе (класс/категория груза)
    quint32 team;
    quint8  navstat_ITU_id;               // Навигационный статус
    
  };
  
  struct aisDynamicData {                    // Динамическая информация
      
    geo::GEOPOSITION geoposition;         // Местоположение (широта и долгота)
                                          // Время (UTC)
                                          // Возраст информации (как давно обновлялась)
                                          // Курс истинный (относительно грунта), курсовой угол
                                          // Скорость истинная
                                          // Угол крена, дифферента
                                          // Угол килевой качки
                                          // Угловая скорость поворота
//    quint32 navstat;                      // Навигационный статус (к примеру: Лишен возможности управляться или Ограничен в возможности маневрировать)
  
  };
  
//  struct aisStaticVoyageData {                     // Рейсовая информация
  
//    QString destination;                  // Пункт назначения
//    QDateTime eta;                        // Время прибытия (ЕТА)
//    qreal draft;                          // Осадка судна
//    quint32 cargo_type_ITU_id;            // тип опасности груза
//    QString cargo_type_name;              // Информация о грузе (класс/категория груза)
//    quint32 team;                         // Количество людей на борту
//                                          // Сообщения для предупреждения и обеспечения безопасности грузоперевозки
//  };
  
  
  struct aisNavStat {
    aisNavStat() {  }
//    explicit aisNavStat(int ITU_id, const QString& name, quint32 static_voyage_interval)
//    {
//      this->ITU_id = ITU_id;
//      this->name = name;
//      this->static_voyage_interval = static_voyage_interval;
//    }
    
    quint32 id;    // не удалять!! какой тобаг при работе с редактором статуса SvNavStatEditor
    int ITU_id = -1;
    QString name = "";
    quint32 static_voyage_interval = 0;

    operator =(const aisNavStat& other) { ITU_id = other.ITU_id; name = other.name; static_voyage_interval = other.static_voyage_interval; }
  };
  
  class SvAIS;
  class SvSelfAIS;
  class SvOtherAIS;
//  class SvAISEmitter;
  
}

Q_DECLARE_METATYPE(ais::aisStaticVoyageData1)

class ais::SvAIS : public idev::SvIDevice
{
  Q_OBJECT
  
public:
//  SvAIS(int vessel_id, const ais::aisStaticVoyageData& sdata, const ais::aisStaticVoyageData& vdata, const ais::aisDynamicData& ddata);
//  ~SvAIS(); 
  
  void setVesselId(int id) { _vessel_id = id; }
  int vesselId() { return _vessel_id; }
  
  void setStaticVoyageData(const ais::aisStaticVoyageData& svdata) { _static_voyage_data = svdata; }
  void setDynamicData(const ais::aisDynamicData& ddata) { _dynamic_data = ddata; }
  
  void setSpeed(const qreal speed) { _dynamic_data.geoposition.speed = speed; }
  void setCourse(const qreal course) { _dynamic_data.geoposition.course = course; }
  
  void setGeoPosition(const geo::GEOPOSITION& geopos) { _dynamic_data.geoposition = geopos; }
  
  ais::aisNavStat* navStatus() { return &_nav_status; }
  void setNavStatus(const ais::aisNavStat status) { _nav_status = status; }
  
  ais::aisStaticVoyageData  *staticVoyageData() { return &_static_voyage_data; }
  ais::aisDynamicData *dynamicData() { return &_dynamic_data; }
  
//  idev::SvSimulatedDeviceTypes type();
    
  virtual bool open() = 0;
  virtual void close() = 0;
  
  virtual bool start(quint32 msecs) = 0;
  virtual void stop() = 0;
  
  friend class ais::SvSelfAIS;
  friend class ais::SvOtherAIS;
  
private:
  ais::aisStaticVoyageData _static_voyage_data;
  ais::aisDynamicData _dynamic_data;
  
  int _vessel_id = -1;
  
  ais::aisNavStat _nav_status;
  
  QDateTime _last_update;
  
//public slots:
//  void newGPSData(const geo::GEOPOSITION& geopos);
  
};


class ais::SvSelfAIS : public ais::SvAIS
{
  Q_OBJECT
  
public:
  SvSelfAIS(int vessel_id, const ais::aisStaticVoyageData& svdata, const ais::aisDynamicData& ddata, svlog::SvLog& log, QDateTime lastUpdate);
  ~SvSelfAIS(); 
  
  void setSerialPortParams(const SerialPortParams& params);

  qreal receiveRange() { return _receive_range; }
  void setReceiveRange(qreal rangeInMeters) { _receive_range = rangeInMeters; }
  
  qreal distanceTo(ais::SvAIS* remoteAIS); /*{ if(!remoteAIS) return 0.0; 
    else {
      geo::GEOPOSITION g;
      int i = remoteAIS->aisDynamicData()->geoposition.course;
      return geo::geo2geo_distance(_dynamic_data.geoposition, g); }*/
                                          
  idev::SvSimulatedDeviceTypes type() const { return idev::sdtSelfAIS; }
  
//  ais::aisStaticVoyageData *aisStaticVoyageData() { return &_static_data; }
//  ais::aisStaticVoyageData *aisStaticVoyageData() { return &_voyage_data; }
//  ais::aisDynamicData *aisDynamicData() { return &_dynamic_data; }
  
  bool open();
  void close();
  
  bool start(quint32 msecs) { }
  void stop() { }
  
  void alarm(int id, QString state, QString text);
  
private:
  qreal _receive_range;
  
  svlog::SvLog _log;
  
  QSerialPort _port;
  SerialPortParams _port_params;
  
  QUdpSocket* udp = nullptr;
  
  QString _income_message = "";

  SvException _exception;
  
  void parse_AIR(QString& msg);
  void parse_SSD(QString& msg);
  void parse_VSD(QString& msg);
  void parse_Q(QString& msg);
  
private slots:
  void write(const QString& message);
  void read();
  void on_income_message(QString& msg);
  
signals:
  void updateSelfVessel();
  void updateVesselById(int id);
  void write_message(const QString& message);
  
  void newIncomeMessage(QString& msg);
  void interrogateRequest(quint32 mmsi1, quint32 msg1num, quint32 msg2num, quint32 mmsi2, quint32 msg3num);
  
public slots:
  void newGPSData(const geo::GEOPOSITION& geopos);
  void on_receive_message(ais::SvAIS* otherAIS, quint32 message_id);

  
};


class ais::SvOtherAIS : public ais::SvAIS
{
  Q_OBJECT
  
public:
  SvOtherAIS(int vessel_id, const ais::aisStaticVoyageData& svdata, const ais::aisDynamicData& ddata, QDateTime lastUpdate);
  ~SvOtherAIS(); 
  
  idev::SvSimulatedDeviceTypes type() const { return idev::sdtOtherAIS; }
    
  bool open();
  void close();
  
  bool start(quint32 msecs);
  void stop();

  
private:  
  QTimer _timer_static_voyage;
//  QTimer _timer_voyage;
  QTimer _timer_dynamic;
  
  quint32 _static_voyage_interval;
//  quint32 _voyage_interval;
  quint32 _dynamic_interval;
  
signals:
  void broadcast_message(ais::SvAIS* ais, quint32 message_id);
  
public slots:
  void newGPSData(const geo::GEOPOSITION& geopos);
  void on_interrogate(quint32 mmsi1, quint32 msg1num, quint32 msg2num, quint32 mmsi2, quint32 msg3num);
  
private slots:
  void on_timer_static_voyage()  { _timer_static_voyage.setInterval(_static_voyage_interval);
                                   emit broadcast_message(this, 5); }
  
//  void on_timer_voyage()  { emit broadcast_ais_data(this, ais::aisVoyage); }
  void on_timer_dynamic() { emit broadcast_message(this, 1); }

  
};


//class ais::SvAISEmitter : public QThread
//{
//  Q_OBJECT
  
//public:
//  SvAISEmitter(ais::aisStaticVoyageData *sdata, ais::aisStaticVoyageData *vdata, ais::aisDynamicData *ddata, QMutex *mutex);
//  ~SvAISEmitter();
  
//  void stop();
  
//  int vessel_id = -1;
  
//private:
//  void run() Q_DECL_OVERRIDE;
  
//  bool _started = false;
//  bool _finished = false;
  
//  ais::aisStaticVoyageData *_static_data = nullptr;
//  ais::aisStaticVoyageData *_voyage_data = nullptr;
//  ais::aisDynamicData *_dynamic_data = nullptr;
  
//  QMutex *_mutex = nullptr;
  
//signals:
//  void ais_static_data(ais::aisStaticVoyageData *data);
//  void ais_voyage_data(ais::aisStaticVoyageData *data);
//  void ais_dynamic_data(ais::aisDynamicData *data);
  
//};

#endif // SV_AIS_H
