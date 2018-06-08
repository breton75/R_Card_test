#ifndef SV_IDEVICE_H
#define SV_IDEVICE_H

#include <QObject>
#include <QMutex>
#include <QDialog>
#include <QMap>
#include <QDateTime>
#include <QTextEdit>
#include <QMetaType>
#include <QUdpSocket>
#include <QNetworkDatagram>

#include "../../svlib/sv_tcpserverclient.h"
#include "../../svlib/sv_log.h"

namespace idev {

  enum SvSimulatedDeviceTypes {
    sdtUndefined = -1,
    sdtGPS,
    sdtSelfAIS,
    sdtOtherAIS,
    sdtLAG,
    sdtNavtex,
    sdtEchoMulti,
    sdtEchoFish,
    sdtVessel
  };

  struct NetworkParams {
    NetworkParams() {  }
    NetworkParams(SvSimulatedDeviceTypes type)
    { 
      dev_type = type;
      switch (type) {
        case sdtGPS: port = 30000; break;
        case sdtEchoMulti: port = 30001; break;
        case sdtEchoFish: port = 30002; break;
      }
    }
    
    int protocol = QAbstractSocket::UdpSocket;
    quint32 ip = 0;
    quint32 ifc;
    quint16 port = 30000;
    SvSimulatedDeviceTypes dev_type;
    QString description = "";
    QHostAddress::SpecialAddress translate_type = QHostAddress::Broadcast;
    
    static QMap<QHostAddress::SpecialAddress, QString> TranslateTypes() { return {
      {QHostAddress::Null, "Заданный IP адрес"},
      {QHostAddress::LocalHost, "localhost"},
      {QHostAddress::Any, "Any"},
      {QHostAddress::Broadcast, "Broadcast"}};
                                                                        }
  };
  
  class SvIDevice;
  class SvINetworkDevice;

}


/** ----------- SvIDevice ------------ **/
class idev::SvIDevice : public QObject
{
    Q_OBJECT
    
public:
  SvIDevice() { }
    
//    qRegisterMetaType<svidev::MeasuredData>("svidev::mdata_t"); 
    
//  }
  
  virtual ~SvIDevice() { }
  
  virtual idev::SvSimulatedDeviceTypes type() const { return idev::sdtUndefined; }
  
  
//  QMutex mutex;
  
  void setLastError(const QString& lastError) { _last_error = lastError; }
  QString lastError() { return _last_error; }
  
  void setDeviceType(idev::SvSimulatedDeviceTypes type) { _type = type; }
  idev::SvSimulatedDeviceTypes deviceType() { return _type; }
  
  void setOpened(bool isOpened) { _isOpened = isOpened; }
  bool isOpened() { return _isOpened; }
  
  void setActive(bool isActive) { _isActive = isActive; }
  bool isActive() { return _isActive; }

protected:
  quint32 _id;
  idev::SvSimulatedDeviceTypes _type;
  
  bool _isOpened = false;
  QString _last_error;
  
  bool _isActive = true;
  
public slots:
  virtual bool open() = 0;
  virtual void close() = 0;
  
  virtual bool start(quint32 msecs) = 0;
  virtual void stop() = 0;
  
    
//signals:
//  void new_data(const svidev::mdata_t& data);
      
};

class idev::SvINetworkDevice : public idev::SvIDevice
{
  Q_OBJECT
  
public:
  SvINetworkDevice(svlog::SvLog& log): SvIDevice() { _log = log; }
//  ~SvINetworkDevice(); 
  
  void setNetworkParams(idev::NetworkParams params) { _params = params; }
  
  virtual bool open()
  {
    _isOpened = true;
    return _isOpened;
  }
  
  virtual void close()
  { 
    stop();
    _isOpened = false;
  }
  
  virtual bool start(quint32 msecs)
  {
    Q_UNUSED(msecs);
    
    if(!_isOpened)
      return false;
    
    if(_udp) delete _udp;
    _udp = nullptr;
    
    if(_tcp) delete _tcp;
    _tcp = nullptr;
    
    if(_params.protocol == QAbstractSocket::UdpSocket) {
    
      _udp = new QUdpSocket();
      _log << svlog::Info << svlog::Time << QString("UDP translation started (%1:%2)")
              .arg(_params.translate_type == QHostAddress::Null ? QHostAddress(_params.ip).toString() : idev::NetworkParams::TranslateTypes().value(_params.translate_type))
              .arg(_params.port) << svlog::endl;
  
    }
    else {
      
      _tcp = new svtcp::SvTcpServer(_log, svtcp::DoNotLog, svtcp::DoNotLog);
      
      if(!_tcp->startServer(_params.port)) {
        
        _log << svlog::Critical << svlog::Time << QString("Ошибка при запуске сервера TCP: %1").arg(_tcp->lastError()) << svlog::endl;
        
        delete _tcp;
        
        return false;
        
      }
    }
  
    connect(this, &SvINetworkDevice::newPacket, this, &SvINetworkDevice::write);
    
  //  _udp->s MulticastInterface(QNetworkInterface::interfaceFromIndex(_params.ifc));
  
    return true;
  }
  
  virtual void stop()
  {
    disconnect(this, &SvINetworkDevice::newPacket, this, &SvINetworkDevice::write);
    
    if(_udp) {
      delete _udp;
      _log << svlog::Info << svlog::Time << QString("UDP translation stopped (%1)").arg(_params.port) << svlog::endl;
    }
    _udp = nullptr;
    
    if(_tcp) delete _tcp;
    _tcp = nullptr;
    
  }
  
private:
  
  QUdpSocket *_udp = nullptr;
  svtcp::SvTcpServer *_tcp = nullptr;
  
  svlog::SvLog _log;
  idev::NetworkParams _params;
  
signals:
  void newPacket(const QByteArray& packet);
  
private slots:
  
  void write(const QByteArray& packet)
  {
    if(!packet.isEmpty()) {
      
      if(_udp) {
        
        QHostAddress ha = _params.translate_type == QHostAddress::Null ?
                            QHostAddress(_params.ip) : _params.translate_type;
        
//        QNetworkDatagram nd(packet, ha, _params.port);
//        nd.setInterfaceIndex(_params.ifc);
        
        _udp->writeDatagram(packet, ha, _params.port);
            
      }

      else if(_tcp) 
        _tcp->sendToAll(packet);
      
    }
  }
  
};

#endif // SV_IDEVICE_H
