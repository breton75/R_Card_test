#ifndef SV_IDEVICE_H
#define SV_IDEVICE_H

#include <QObject>
#include <QMutex>
#include <QDialog>
#include <QMap>
#include <QDateTime>
#include <QTextEdit>
#include <QMetaType>


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

  class SvIDevice;

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
  
  bool _isOpened;
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

#endif // SV_IDEVICE_H
