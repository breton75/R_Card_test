#include "sv_navtex.h"
#include "nmea.h"

/** --------- NAVTEX --------- **/
nav::SvNAVTEX::SvNAVTEX(svlog::SvLog &log, int id)
{
  _log = log;
  _id = id;
}

nav::SvNAVTEX::~SvNAVTEX()
{
  deleteLater();
}
  
bool nav::SvNAVTEX::open()
{
  if(!_port.open(QSerialPort::ReadWrite)) {
    
    setLastError(_port.errorString());
    return false;
  }
  
  connect(&_timer, &QTimer::timeout, this, &nav::SvNAVTEX::prepare_message);
  connect(this, &nav::SvNAVTEX::write_message, this, &nav::SvNAVTEX::write);
  
  
  _isOpened = _port.isOpen();
  
  return _isOpened;
  
}

void nav::SvNAVTEX::close()
{
  _port.close();
  
  disconnect(&_timer, &QTimer::timeout, this, &nav::SvNAVTEX::prepare_message);
  disconnect(this, &nav::SvNAVTEX::write_message, this, &nav::SvNAVTEX::write);
  
  _isOpened = false;
}

bool nav::SvNAVTEX::start(quint32 msecs)
{
  if(!_isOpened)
    return false;
  
  _timer.start(msecs);
  
  return true;
}

void nav::SvNAVTEX::stop()
{
  _timer.stop();
}

void nav::SvNAVTEX::prepare_message()
{
  QStringList msgs = nmea::navtex_NRX(_data);
  emit write_message(msgs);
}

void nav::SvNAVTEX::write(const QStringList &messages)
{
  if(_port.isOpen() && (_receive_frequency == _data.transmit_frequency_id)) {
   
    for(QString msg: messages) {
      
      _port.write(msg.toStdString().c_str(), msg.size());
      
      _port.waitForBytesWritten();
      
      _log << svlog::Attention << svlog::TimeZZZ << msg << svlog::endl;
    
    }
    
    _data.message_last_number++;
  }
  
}

void nav::SvNAVTEX::setSerialPortParams(const SerialPortParams& params)
{ 
  _port.setPortName(params.name);
  _port.setBaudRate(params.baudrate);
  _port.setDataBits(params.databits);
  _port.setFlowControl(params.flowcontrol);
  _port.setParity(params.parity);
  _port.setStopBits(params.stopbits);
}


void nav::SvNAVTEX::alarm(int id, QString state, QString text)
{
  QString msg = nmea::alarm_ALR("CR", id, state, text);
  emit write_message(QStringList(msg));
}
