#include "sv_ais.h"
#include "nmea.h"

//ais::SvAIS* SELF_AIS;
QMap<quint32, ais::aisNavStat> NAVSTATs;

extern SvSQLITE* SQLITE;
extern geo::UnitsInfo CMU;

/** --------- Self AIS --------- **/
ais::SvSelfAIS::SvSelfAIS(int vessel_id, const ais::aisStaticVoyageData& svdata, const ais::aisDynamicData& ddata, svlog::SvLog &log, QDateTime lastUpdate)
{
  setVesselId(vessel_id);
  
  setStaticVoyageData(svdata);
  setDynamicData(ddata);
  
  _log = log;
  _last_update = lastUpdate;
  
}

ais::SvSelfAIS::~SvSelfAIS()
{
  deleteLater();
}
  
bool ais::SvSelfAIS::open()
{
  if(!_port.open(QSerialPort::ReadWrite)) {
    
    setLastError(_port.errorString());
    return false;
  }
  
  _isOpened = _port.isOpen();
  
  connect(this, &ais::SvSelfAIS::write_message, this, &ais::SvSelfAIS::write);
  connect(&_port, &QSerialPort::readyRead, this, &SvSelfAIS::read);
  connect(this, &ais::SvSelfAIS::newIncomeMessage, this, &ais::SvSelfAIS::on_income_message);
  
//  qDebug() << _port.baudRate() << _port.
  
  return _isOpened;
}

void ais::SvSelfAIS::close()
{
  _port.close();
  disconnect(this, &ais::SvSelfAIS::write_message, this, &ais::SvSelfAIS::write);
  disconnect(&_port, &QSerialPort::readyRead, this, &SvSelfAIS::read);
  _isOpened = false;
}

void ais::SvSelfAIS::newGPSData(const geo::GEOPOSITION& geopos)
{
  _dynamic_data.geoposition = geopos;
  emit updateSelfVessel(); 
}

void ais::SvSelfAIS::on_receive_message(ais::SvAIS* otherAIS, quint32 message_id)
{
  /** проверяем расстояние. если в радиусе действия антенны, то обрабатываем данные */
  if(distanceTo(otherAIS) < _receive_range * CMU.MetersCount) {
    
    /** обрабатываем данные **/
    switch (message_id) {
      
      case 5:
      {
        
        QStringList l = nmea::ais_message_5(_static_voyage_data.talkerID, otherAIS->staticVoyageData()); //, otherAIS->navStatus());
        emit write_message(l.first());
        for(int i = 0; i < 100000; i++) ;
        emit write_message(l.last());
        
        otherAIS->incSequentialId();
        
        break;
      }
      
      case 73: {  // Binary acknowledgement of message 3
        
        QString abk = nmea::ais_sentence_ABK(3, _static_voyage_data.talkerID, otherAIS->staticVoyageData());
        emit write_message(abk);
        
        break;
      }

      case 75: {  // Binary acknowledgement of message 5
        
        QString abk = nmea::ais_sentence_ABK(5, _static_voyage_data.talkerID, otherAIS->staticVoyageData());
        emit write_message(abk);
        
        break;
      }
        
      case 1:
      {
        
        QString msg = nmea::ais_message_1_2_3(1, _static_voyage_data.talkerID, otherAIS->staticVoyageData(), otherAIS->navStatus()->ITU_id, otherAIS->dynamicData()->geoposition);
        emit write_message(msg);
        
        break;
       } 
        
        
      case 3:
      {
        
        QString msg = nmea::ais_message_1_2_3(3, _static_voyage_data.talkerID, otherAIS->staticVoyageData(), otherAIS->navStatus()->ITU_id, otherAIS->dynamicData()->geoposition);
        emit write_message(msg);
        
        break;
       } 
        
      default:
        break;
    }
    
  }

  emit updateVesselById(otherAIS->vesselId());
  
}

qreal ais::SvSelfAIS::distanceTo(ais::SvAIS* remoteAIS)
{ 
  if(!remoteAIS) return 0.0; 
    
  return geo::geo2geo_distance(_dynamic_data.geoposition, remoteAIS->_dynamic_data.geoposition); 

}

void ais::SvSelfAIS::setSerialPortParams(const SerialPortParams& params)
{ 
  _port.setPortName(params.name);
  _port.setBaudRate(params.baudrate);
  _port.setDataBits(params.databits);
  _port.setFlowControl(params.flowcontrol);
  _port.setParity(params.parity);
  _port.setStopBits(params.stopbits);
}


void ais::SvSelfAIS::write(const QString &message)
{
//  return;
  _log << svlog::Time << svlog::Data << message << svlog::endl;
  _port.write(message.toStdString().c_str(), message.size());
//  _port.waitForBytesWritten(100);
  
}

void ais::SvSelfAIS::read()
{
 QByteArray b = _port.readAll();

 _income_message.append(QString::fromLatin1(b));
 
 if(_income_message.contains("\r\n")) {
   
   QString msg = _income_message.split("\r\n").first();
   
   _income_message.clear();
   
   emit newIncomeMessage(msg);
   
 }
 else if(_income_message.length() > 1024)
   _income_message.clear();
 
 
 
}

void ais::SvSelfAIS::on_income_message(QString &msg)
{
  /// парсим входящее сообщение
  QMap<int, QString> sentences = {{0, "AIR"}, {1, "Q"}, {2, "SSD"}, {3, "VSD"}};
  
  if(msg.length() < 11)
    return;
  
  if((msg.left(1) != "$") && (msg.left(1) != "!"))
    return;
  
  QString snt = msg.mid(5, 1) == "Q" ? "Q" : msg.mid(3, 3);
  if(!sentences.values().contains(snt))
    return;
  
  switch (sentences.key(snt)) {
    case 0:
      parse_AIR(msg);
      break;
      
    case 1:
      parse_Q(msg);
      break;
      
    case 2:
      parse_SSD(msg);
      break;
      
    case 3:
      parse_VSD(msg);
      break;
      
  }
  
}

void ais::SvSelfAIS::parse_AIR(QString& msg)
{
  QStringList l = msg.split(',');
  bool ok = true;
  QString s;
  quint32 mmsi1 = 0;
  quint32 msg1num = 0;
  qint32 msg2num = 0;
  quint32 mmsi2 = 0;
  qint32 msg3num = 0;
  
  // mmsi запрашиваемого судна. если пусто, то запрашиваем все суда
  s = QString(l.at(1));
  mmsi1 = s.toUInt(&ok);
  if(!ok && !s.isEmpty()) return;
  
  // номер 1го запрашиваемого сообщения
  s = QString(l.at(2));
  if(s.isEmpty()) return;
  
  msg1num = QString(s.split('.').first()).toUInt(&ok);
  if(!ok) return;
  
  // номер 2го запрашиваемого сообщения
  s = QString(l.at(4));
  if(!s.isEmpty()) {
    msg2num = QString(s.split('.').first()).toUInt(&ok);
    if(!ok) return;      
  }
  
  // mmsi 2го запрашиваемого судна
  s = QString(l.at(6));
  mmsi2 = s.toUInt(&ok);
  if(!ok && !s.isEmpty()) return;
  
  // номер 3го запрашиваемого сообщения
  s = QString(l.at(7));
  msg3num = QString(s.split('.').first()).toUInt(&ok);
  if(!ok && !s.isEmpty()) return;
  
  emit interrogateRequest(mmsi1, msg1num, msg2num, mmsi2, msg3num);
  
}

void ais::SvSelfAIS::parse_SSD(QString& msg)
{
  try {
    
    QStringList l = msg.split(',');
    bool ok = true;
    
    if(l.count() != 9) _exception.raise(QString("Неверное предложение: %1").arg(msg));
    
    QString callsign = QString(l.at(1)).isEmpty() ? DO_NOT_CHANGE_s : l.at(1);
    
    QString name = QString(l.at(2)).isEmpty() ? DO_NOT_CHANGE_s : l.at(2);
    
    int pos_ref_A = QString(l.at(3)).isEmpty() ? DO_NOT_CHANGE_i : QString(l.at(3)).toInt(&ok);
    if(!ok)  _exception.raise(QString("Неверное предложение: %1").arg(msg));
    
    int pos_ref_B = QString(l.at(4)).isEmpty() ? DO_NOT_CHANGE_i : QString(l.at(4)).toInt(&ok);
    if(!ok)  _exception.raise(QString("Неверное предложение: %1").arg(msg));
    
    int pos_ref_C = QString(l.at(5)).isEmpty() ? DO_NOT_CHANGE_i : QString(l.at(5)).toInt(&ok);
    if(!ok)  _exception.raise(QString("Неверное предложение: %1").arg(msg));
    
    int pos_ref_D = QString(l.at(6)).isEmpty() ? DO_NOT_CHANGE_i : QString(l.at(6)).toInt(&ok);
    if(!ok)  _exception.raise(QString("Неверное предложение: %1").arg(msg));
    
    int DTE_flag = QString(l.at(7)).isEmpty() ? DO_NOT_CHANGE_i : QString(l.at(7)).toInt(&ok);
    
    QString talkerID = QString(QString(l.at(8)).split('*').first()).isEmpty() ? DO_NOT_CHANGE_s : QString(l.at(8)).split('*').first();
    if((talkerID != DO_NOT_CHANGE_s) && (talkerID.length() != 2)) _exception.raise(QString("Неверное предложение: %1").arg(msg));
    
    /// пишем в базу
    quint16 flags = 0;
    flags |= (callsign == DO_NOT_CHANGE_s ? 0 : 0x01);
    flags |= (name == DO_NOT_CHANGE_s ? 0 : 0x02);
    flags |= (pos_ref_A == DO_NOT_CHANGE_i ? 0 : 0x04);
    flags |= (pos_ref_B == DO_NOT_CHANGE_i ? 0 : 0x08);
    flags |= (pos_ref_C == DO_NOT_CHANGE_i ? 0 : 0x10);
    flags |= (pos_ref_D == DO_NOT_CHANGE_i ? 0 : 0x20);
    flags |= (DTE_flag == DO_NOT_CHANGE_i ? 0 : 0x40);
    flags |= (talkerID == DO_NOT_CHANGE_s ? 0 : 0x80);  
    
    if(flags == 0) return;

    QString query = QString("update ais set %1%2%3%4%5%6%7%8")
                    .arg((flags & 0x01) == 0 ? "" : QString("static_callsign = '%1', ").arg(callsign))
                    .arg((flags & 0x02) == 0 ? "" : QString("static_name = '%1', ").arg(name))
                    .arg((flags & 0x04) == 0 ? "" : QString("static_pos_ref_A = %1, ").arg(pos_ref_A))
                    .arg((flags & 0x08) == 0 ? "" : QString("static_pos_ref_B = %1, ").arg(pos_ref_B))
                    .arg((flags & 0x10) == 0 ? "" : QString("static_pos_ref_C = %1, ").arg(pos_ref_C))
                    .arg((flags & 0x20) == 0 ? "" : QString("static_pos_ref_D = %1, ").arg(pos_ref_D))
                    .arg((flags & 0x40) == 0 ? "" : QString("static_DTE = %1, ").arg(DTE_flag))
                    .arg((flags & 0x80) == 0 ? "" : QString("static_talker_id = '%1', ").arg(talkerID))
                    ;
    // убираем последнюю запятую
    if(query.right(2) == ", ") query.chop(2);
    
    query.append(QString(" where vessel_id = %1").arg(_vessel_id));
    
    /// пытаемся записать в базу
    QSqlError err = SQLITE->execSQL(query);
    if(err.type() != QSqlError::NoError) _exception.raise(QString("Ошибка при попытке изменить статическую информацию судна: %1").arg(err.text()));
    
    
    
    _log << svlog::Success << svlog::Time 
         << QString("Обновлена статическая информация:\n%1%2%3%4%5%6%7%8")
                            .arg((flags & 0x01) == 0 ? "" : QString("\tCallsign = '%1'\n").arg(callsign))
                            .arg((flags & 0x02) == 0 ? "" : QString("\tName = '%1'\n").arg(name))
                            .arg((flags & 0x04) == 0 ? "" : QString("\tPos_ref_A = %1\n").arg(pos_ref_A))
                            .arg((flags & 0x08) == 0 ? "" : QString("\tPos_ref_B = %1\n").arg(pos_ref_B))
                            .arg((flags & 0x10) == 0 ? "" : QString("\tPos_ref_C = %1\n").arg(pos_ref_C))
                            .arg((flags & 0x20) == 0 ? "" : QString("\tPos_ref_D = %1\n").arg(pos_ref_D))
                            .arg((flags & 0x40) == 0 ? "" : QString("\tDTE = %1\n").arg(DTE_flag))
                            .arg((flags & 0x80) == 0 ? "" : QString("\tTalkerId = '%1'\n").arg(talkerID))
         << svlog::endl;
    
        
    /// если записали в БД, то меняем парметры
    ais::aisStaticVoyageData svd;
    if(flags & 0x01) svd.callsign = callsign;
    if(flags & 0x02) svd.name = name;
    if(flags & 0x04) svd.pos_ref_A = pos_ref_A;
    if(flags & 0x08) svd.pos_ref_B = pos_ref_B;
    if(flags & 0x10) svd.pos_ref_C = pos_ref_C;
    if(flags & 0x20) svd.pos_ref_D = pos_ref_D;
    if(flags & 0x40) svd.DTE = DTE_flag;
    if(flags & 0x80) svd.talkerID = talkerID;
    
    setStaticVoyageData(svd);
    
  }

  catch(SvException &e) {
    
    _log << svlog::Error << svlog::Time 
         << e.err
         << svlog::endl;
    
  }
    
}

void ais::SvSelfAIS::parse_VSD(QString& msg)
{
  try {
    
    QStringList l = msg.split(',');
    bool ok = true;
    
    if(l.count() != 10) _exception.raise(QString("Неверное предложение: %1").arg(msg));
    
    int type_of_ship = QString(l.at(1)).isEmpty() ? DO_NOT_CHANGE_i : QString(l.at(1)).split('.').first().toInt(&ok);
    if(!ok || type_of_ship > 255)  _exception.raise(QString("Неверный параметр [%1] в предложении: %2").arg(l.at(1)).arg(msg));
    
    qreal draft = QString(l.at(2)).isEmpty() ? DO_NOT_CHANGE_i : QString(l.at(2)).toDouble(&ok);
    if(!ok)  _exception.raise(QString("Неверный параметр [%1] в предложении: %2").arg(l.at(2)).arg(msg));
    if(draft > 25.5) draft = 25.5;
    
    int team = QString(l.at(3)).isEmpty() ? DO_NOT_CHANGE_i : QString(l.at(3)).split('.').first().toInt(&ok);
    if(!ok)  _exception.raise(QString("Неверный параметр [%1] в предложении: %2").arg(l.at(3)).arg(msg));
    if(team > 8191) team = 8191;
    
    QString destination = QString(l.at(4)).isEmpty() ? DO_NOT_CHANGE_s : l.at(4);
    if(destination.length() > 20) _exception.raise(QString("Неверный параметр [%1] в предложении: %2").arg(l.at(4)).arg(msg));
    
    QTime eta_utc = QString(l.at(5)).isEmpty() ? QTime(0, 0) : QTime::fromString(QString(l.at(5)), "hhmmss.z");
    qDebug() << eta_utc;
    if(!eta_utc.isValid())  _exception.raise(QString("Неверный параметр [%1] в предложении: %2").arg(l.at(5)).arg(msg));
    
    int eta_day = QString(l.at(6)).isEmpty() ? DO_NOT_CHANGE_i : QString(l.at(6)).toInt(&ok);
    if(!ok || (eta_day > 31))  _exception.raise(QString("Неверный параметр [%1] в предложении: %2").arg(l.at(6)).arg(msg));
    
    int eta_month = QString(l.at(7)).isEmpty() ? DO_NOT_CHANGE_i : QString(l.at(7)).toInt(&ok);
    if(!ok || (eta_month > 12))  _exception.raise(QString("Неверный параметр [%1] в предложении: %2").arg(l.at(7)).arg(msg));    
    
    int nav_stat = QString(l.at(8)).isEmpty() ? DO_NOT_CHANGE_i : QString(l.at(8)).split('.').first().toInt(&ok);
    if(!ok || (eta_month > 15))  _exception.raise(QString("Неверный параметр [%1] в предложении: %2").arg(l.at(8)).arg(msg));    
    
    
    /// пишем в базу
    quint16 flags = 0;
    flags |= (type_of_ship  == DO_NOT_CHANGE_i ? 0 : 0x0001);
    flags |= (draft         == DO_NOT_CHANGE_i ? 0 : 0x0002);
    flags |= (team          == DO_NOT_CHANGE_i ? 0 : 0x0004);
    flags |= (destination   == DO_NOT_CHANGE_s ? 0 : 0x0008);
    flags |= (eta_utc       == QTime(0, 0)     ? 0 : 0x0010);
    flags |= (eta_day       == DO_NOT_CHANGE_i ? 0 : 0x0020);  
    flags |= (eta_month     == DO_NOT_CHANGE_i ? 0 : 0x0040);
    flags |= (nav_stat      == DO_NOT_CHANGE_i ? 0 : 0x0080);
    
    
    if(flags == 0) return;

    QString query = QString("update ais set %1%2%3%4%5%6%7%8")
                    .arg((flags & 0x0001) == 0 ? "" : QString("static_type_ITU_id = %1, ").arg(type_of_ship))
                    .arg((flags & 0x0002) == 0 ? "" : QString("voyage_draft = %1, ").arg(draft,0, 'g', 1))
                    .arg((flags & 0x0004) == 0 ? "" : QString("voyage_team = %1, ").arg(team))
                    .arg((flags & 0x0008) == 0 ? "" : QString("voyage_destination = '%1', ").arg(destination))
                    .arg((flags & 0x0010) == 0 ? "" : QString("voyage_ETA_utc = '%1', ").arg(eta_utc.toString("hh:mm:ss")))
                    .arg((flags & 0x0020) == 0 ? "" : QString("voyage_ETA_day = %1, ").arg(eta_day))
                    .arg((flags & 0x0040) == 0 ? "" : QString("voyage_ETA_month = %1, ").arg(eta_month))
                    .arg((flags & 0x0080) == 0 ? "" : QString("nav_status_ITU_id = %1, ").arg(nav_stat))
                    ;
    // убираем последнюю запятую
    if(query.right(2) == ", ") query.chop(2);
    
    query.append(QString(" where vessel_id = %1").arg(_vessel_id));
    
    /// пытаемся записать в базу
    QSqlError err = SQLITE->execSQL(query);
    if(err.type() != QSqlError::NoError) _exception.raise(QString("Ошибка при попытке изменить путевую информацию судна: %1").arg(err.text()));
    
    
    
    _log << svlog::Success << svlog::Time 
         << QString("Обновлена путевая информация:\n%1%2%3%4%5%6%7%8")
            .arg((flags & 0x0001) == 0 ? "" : QString("\tType of ship and cargo = %1").arg(type_of_ship))
            .arg((flags & 0x0002) == 0 ? "" : QString("\tDraft = %1").arg(draft,0, 'g', 1))
            .arg((flags & 0x0004) == 0 ? "" : QString("\tTeam = %1").arg(team))
            .arg((flags & 0x0008) == 0 ? "" : QString("\tDestination = %1").arg(destination))
            .arg((flags & 0x0010) == 0 ? "" : QString("\tETA utc = %1").arg(eta_utc.toString("hh:mm:ss")))
            .arg((flags & 0x0020) == 0 ? "" : QString("\tETA day = %1").arg(eta_day))
            .arg((flags & 0x0040) == 0 ? "" : QString("\tETA month = %1").arg(eta_month))
            .arg((flags & 0x0080) == 0 ? "" : QString("\tNavigate status = %1").arg(nav_stat))
         << svlog::endl;
    
        
    /// если записали в БД, то меняем парметры
    ais::aisStaticVoyageData svd;
    if(flags & 0x01) svd.vessel_ITU_id = type_of_ship;
    if(flags & 0x01) svd.cargo_ITU_id = type_of_ship % 10;
    if(flags & 0x08) svd.draft = draft;
    if(flags & 0x10) svd.team = team;
    if(flags & 0x20) svd.destination = destination;
    if(flags & 0x40) svd.ETA_utc = eta_utc;
    if(flags & 0x80) svd.ETA_day = eta_day;
    if(flags & 0x80) svd.ETA_month = eta_month;
    if(flags & 0x80) svd.navstat_ITU_id = nav_stat;
    
    setStaticVoyageData(svd);
    
  }

  catch(SvException &e) {
    
    _log << svlog::Error << svlog::Time 
         << e.err
         << svlog::endl;
    
  }
}

void ais::SvSelfAIS::parse_Q(QString& msg)
{
  QStringList sups;
  sups << "VDO";
  
  try {
    
    QStringList l = msg.split(',');
    
    QString sentense = QString(l.at(1)).split('*').first();
    
    if(!sups.contains(sentense)) _exception.raise(QString("Предложение не поддерживается: %1").arg(sentense));
    
    if(sentense == "VDO") {
      
      on_receive_message(this, 5);
      
    }
    
  }
  
  catch(SvException &e) {
    
    _log << svlog::Error << svlog::Time 
         << e.err
         << svlog::endl;
    
  }
  
}

void ais::SvSelfAIS::alarm(int id, QString state, QString text)
{
  QString msg = nmea::alarm_ALR(_static_voyage_data.talkerID, id, state, text);
  emit write_message(msg);
}

/** ----- Other AIS ------- **/
ais::SvOtherAIS::SvOtherAIS(int vessel_id, const ais::aisStaticVoyageData& svdata, const ais::aisDynamicData& ddata, QDateTime lastUpdate)
{
  _vessel_id = vessel_id;
  _last_update = lastUpdate;
  
  setStaticVoyageData(svdata);
  setDynamicData(ddata);
  
}

ais::SvOtherAIS::~SvOtherAIS()
{
  deleteLater();
}
  
bool ais::SvOtherAIS::open()
{
  _isOpened = true; 
}

void ais::SvOtherAIS::close()
{
  _isOpened = false;
}

bool ais::SvOtherAIS::start(quint32 msecs)
{
  if(!_isActive)
    return true;
  
  connect(&_timer_static_voyage, &QTimer::timeout, this, &ais::SvOtherAIS::on_timer_static_voyage);
  connect(&_timer_dynamic, &QTimer::timeout, this, &ais::SvOtherAIS::on_timer_dynamic);
  
  _static_voyage_interval = _nav_status.static_voyage_interval;
  
  switch (_nav_status.ITU_id) {
    case 1: // на якоре
    case 5: // у причала
      _dynamic_interval = _dynamic_data.geoposition.speed > 3 ? 10 * 1000 : 3 * 60 * 1000;
      
      break;
      
    default:
      if(_dynamic_data.geoposition.speed < 14) _dynamic_interval = 5 * 1000;
      else if(_dynamic_data.geoposition.speed < 23) _dynamic_interval = 3 * 1000;
      else _dynamic_interval = 2 * 1000;
      break;
  }
  
  // при первом запуске ставим интервал случайный, чтобы все корабли не отбивались одновременно
  qsrand(_last_update.time().minute() * 60000 + _last_update.time().second() * 1000 + _last_update.time().msec());
  
  quint32 first_interval = quint32(_static_voyage_interval * qreal(qrand() % 100) / 100.0);

  _timer_static_voyage.start(first_interval);
  _timer_dynamic.start(_dynamic_interval);
  
  return true;
}

void ais::SvOtherAIS::stop()
{
  disconnect(&_timer_static_voyage, &QTimer::timeout, this, &ais::SvOtherAIS::on_timer_static_voyage);
  disconnect(&_timer_dynamic, &QTimer::timeout, this, &ais::SvOtherAIS::on_timer_dynamic);
  
  _timer_static_voyage.stop();
  _timer_dynamic.stop();
}

void ais::SvOtherAIS::newGPSData(const geo::GEOPOSITION& geopos)
{
  _dynamic_data.geoposition = geopos;
}

void ais::SvOtherAIS::on_interrogate(quint32 mmsi1, quint32 msg1num, quint32 msg2num, quint32 mmsi2, quint32 msg3num)
{
  QList<int> messageIDs;
  messageIDs << 3 << 5;
  
  if((_static_voyage_data.mmsi == mmsi1) || (mmsi1 == 0)) {
    
    if(messageIDs.contains(msg1num)) {
      
      // для формирования подтверждения ABK
      emit broadcast_message(this, 70 + msg1num);
      
      emit broadcast_message(this, msg1num);
      
      if(messageIDs.contains(msg2num))
        emit broadcast_message(this, msg2num);
      
    }
    
    if((_static_voyage_data.mmsi == mmsi2) || (mmsi1 == 0)) {
      
      if(messageIDs.contains(msg3num))
        emit broadcast_message(this, msg1num);
      
    }
  }
  
}
