#include "sv_lag.h"

/** --------- Self LAG --------- **/
lag::SvLAG::SvLAG(int vessel_id, const geo::GEOPOSITION &geopos, svlog::SvLog &log)
{
  setVesselId(vessel_id);
  
  _current_geoposition = geopos;
  _log = log;
  
}

lag::SvLAG::~SvLAG()
{
  deleteLater();
}
  
bool lag::SvLAG::open()
{
  if(!_port.open(QSerialPort::ReadWrite)) {
    
    setLastError(_port.errorString());
    return false;
  }
  
  connect(&_timer, &QTimer::timeout, this, &lag::SvLAG::prepare_message);
  connect(this, &lag::SvLAG::write_message, this, &lag::SvLAG::write);
  
  
  _isOpened = _port.isOpen();
  
  return _isOpened;
  
}

void lag::SvLAG::close()
{
  _port.close();
  
  disconnect(&_timer, &QTimer::timeout, this, &lag::SvLAG::prepare_message);
  disconnect(this, &lag::SvLAG::write_message, this, &lag::SvLAG::write);
  
  _isOpened = false;
}

bool lag::SvLAG::start(quint32 msecs)
{
  if(!_isOpened)
    return false;
  
  _timer.start(msecs);
  
  return true;
}

void lag::SvLAG::stop()
{
  _timer.stop();
}

void lag::SvLAG::newGPSData(const geo::GEOPOSITION& geopos)
{
  _current_geoposition = geopos;
}

void lag::SvLAG::prepare_message()
{
  QString msg = "";
  
  switch (_msg_type) {
    
    case lag::lmtVBW:
      msg = nmea::lag_VBW(_current_geoposition);
      break;
      
    case lag::lmtVDR:
      msg = nmea::lag_VDR(_current_geoposition);
      break;
      
    case lag::lmtVHW:
      msg = nmea::lag_VHW(_current_geoposition);   
      break;
      
    case lag::lmtVLW:
      msg = nmea::lag_VLW(_current_geoposition);   
      break;      

  }

  if(msg.isEmpty()) return;
  
  emit write_message(msg);
  
}

void lag::SvLAG::write(const QString &message)
{
  if(_port.isOpen()) {
   
    _port.write(message.toStdString().c_str(), message.size());
    
    _log << svlog::Attention << svlog::Time << message << svlog::endl;
    
  }
  
}

void lag::SvLAG::setSerialPortParams(const SerialPortParams& params)
{ 
  _port.setPortName(params.name);
  _port.setBaudRate(params.baudrate);
  _port.setDataBits(params.databits);
  _port.setFlowControl(params.flowcontrol);
  _port.setParity(params.parity);
  _port.setStopBits(params.stopbits);
}

void lag::SvLAG::read()
{
  
}

void lag::SvLAG::alarm(int id, QString state, QString text)
{
  QString msg = nmea::alarm_ALR("VD", id, state, text);
  emit write_message(msg);
}

void lag::SvLAG::setMessageType(lag::MessageType msgtype)
{
  _msg_type = msgtype;
}
