#include "nmea.h"

extern geo::UnitsInfo CMU;

QMap<int, QChar> SIXBIT_SYMBOLS = {
  
  {0, '0'}, {1, '1'}, {2, '2'}, {3, '3'}, {4, '4'}, {5, '5'}, {6, '6'}, {7, '7'}, {8, '8'}, {9, '9'},
  {10, ':'}, {11, ';'}, {12, '<'}, {13, '='}, {14, '>'}, {15, '?'}, {16, '@'}, {17, 'A'}, {18, 'B'}, {19, 'C'}, 
  {20, 'D'}, {21, 'E'}, {22, 'F'}, {23, 'G'}, {24, 'H'}, {25, 'I'}, {26, 'J'}, {27, 'K'}, {28, 'L'}, {29, 'M'}, 
  {30, 'N'}, {31, 'O'}, {32, 'P'}, {33, 'Q'}, {34, 'R'}, {35, 'S'}, {36, 'T'}, {37, 'U'}, {38, 'V'}, {39, 'W'}, 
  {40, '\''}, {41, 'a'}, {42, 'b'}, {43, 'c'}, {44, 'd'}, {45, 'e'}, {46, 'f'}, {47, 'g'}, {48, 'h'}, {49, 'i'}, 
  {50, 'j'}, {51, 'k'}, {52, 'l'}, {53, 'm'}, {54, 'n'}, {55, 'o'}, {56, 'p'}, {57, 'q'}, {58, 'r'}, {59, 's'}, 
  {60, 't'}, {61, 'u'}, {62, 'v'}, {63, 'w'}, 
  
};

QList<QChar> RESERVED_SYMBOLS = {0x0D, 0x0A, 0x24, 0x2A, 0x2C, 0x21, 0x5C, 0x5E, 0x7E, 0x7F };

QMap<nmea::NMEASentence, QString> SENTENCES = {{nmea::ABK_UAIS_Addressed_and_binary_broadcast_acknowledgement, "ABK"},
                                         {nmea::AIR_InterrogationRequest, "AIR"},
                                         {nmea::VDO_UAIS_VHF_Datalink_Own_vessel_report, "VDO"},
                                         {nmea::VDM_UAIS_VHF_Datalink_Message, "VDM"},
                                         {nmea::SSD_UAIS_Ship_Static_Data, "SSD"},
                                         {nmea::VSD_UAIS_Voyage_Static_Data, "VSD"},
                                         {nmea::Q_Query, "Q"}};

inline QByteArray nmea::str_to_6bit(const QString& str)
{
  QByteArray result = "";
  
  for(int i = 0; i < str.count(); i++) {
        
    quint8 n = quint8(str.at(i).toLatin1()); // unicode());
    
    n += 0x28;  // 00101000
    n = n > 0x80 ? n + 0x20 : n + 0x28;
    n &= 0x3f;
    
    result.append(char(n));
    
//    if(SIXBIT_SYMBOLS.find(n) == SIXBIT_SYMBOLS.end()) {
      
//      result.clear();
//      break;
      
//    }

//    result.append(SIXBIT_SYMBOLS.value(n));

  }
  
  return result;
  
}

QString nmea::ais_message_1_2_3(quint8 message_id, QString& talkerID, ais::aisStaticVoyageData* static_voyage_data, quint8 nav_status, geo::GEOPOSITION &geopos) //, int true_heading, QDateTime utc,
//                 int manouevre, int raim, int communication_state)
{
  QString result = "";
  
  quint8 b6[28];
  memset(&b6, 0, 28);
  
  /// Message ID
  b6[0] = message_id;
  
  /// Repeat indicator
  b6[1] = (0 & 0x03) << 4; // 2 значащих бита
  
  /// User ID
  quint64 mmsi64 = static_voyage_data->mmsi & 0x3FFFFFFF; // 30 значащих бит
  mmsi64 <<= 32; // << 32;
  
  for(int i = 0; i < 6; i++) {
    mmsi64 >>= 2;
    b6[1 + i] += (mmsi64 >> (56 - i * 8));
    mmsi64 &= (0x00FFFFFFFFFFFFFF >> (i * 8));
  }
  
  /// Navigational status 
  b6[6] += (nav_status & 0x0F); // 4 значащих бит
  
  /// Rate of turn
  b6[7] = geopos.rate_of_turn >> 2; // 8 значащих бит
  b6[8] = (geopos.rate_of_turn << 4) & 0x3F;
  
  ///  Speed over ground
  quint16 sog16 = quint16(trunc(geopos.speed * 10)) & 0x03FF; // 10 значащих бит
  b6[8] += (sog16 >> 6);
  b6[9] += (sog16 & 0x003F);
  
  /// Position accuracy 
  b6[10] = geopos.accuracy << 5;  // 1 значащий бит
  
  /// Longitude in 1/10000 minutes
  quint64 lon64 = quint64(trunc(geopos.longtitude * 10000 * 60))  & 0x0FFFFFFF; // 28 значащих бит
  lon64 <<= 33;  //! 33

  for(int i = 0; i < 5; i++) {
    b6[10 + i] += (lon64 >> (56 - i * 8));
    lon64 &= (0x00FFFFFFFFFFFFFF >> (i * 8));
    lon64 >>= 2;
  }
  
  /// Latitude in 1/10000 minutes
  quint64 lat64 = quint64(trunc(geopos.latitude * 10000 * 60))  & 0x07FFFFFF; // 27 значащих бит
  lat64 <<= 32;  //! 32
  
  for(int i = 0; i < 6; i++) {
    lat64 >>= 2;
    b6[14 + i] += (lat64 >> (56 - i * 8));
    lat64 &= (0x00FFFFFFFFFFFFFF >> (i * 8));
  }  
  
  /// Course over ground in 1/10 degrees
  quint32 cog32 = quint32(trunc(geopos.course * 10))  & 0x0FFF;  // 12 значащих бит
  cog32 = cog32 << 16;  //! 16
  
  for(int i = 0; i < 3; i++) {
    b6[19 + i] += (cog32 >> (24 - i * 8));
    cog32 &= (0x00FFFFFF >> (i * 8));
    cog32 >>=  2;
  }
  
  /// дальше пока подстановки
  /// True Heading 
  b6[21] += 0x0F;
  b6[22] += 0x3E;
  
  /// UTC second when report generated 
  quint8 s = geopos.utc.time().second();
  b6[22] += (s & 0x63) >> 5;
  b6[23] += (s & 0x31) >> 2;
  
  /// Regional Application + Spare + RAIM Flag 
  b6[24] += 0;
  
  /// Communications State
  b6[24] += 0;          // utc direct
  b6[25] += 0x01 << 2;  // 1 frame remaining until a new slot is selected
  
  quint8 h = geopos.utc.time().hour();
  b6[25] += (h & 0x1F) >> 3;
  b6[26] += (h & 0x07) << 3;
  
  quint8 m = geopos.utc.time().minute();
  b6[26] += (m & 0x7F) >> 4;
  b6[27] += (m & 0x0F) << 2;
  
  /// формируем сообщение
  QString msg = "";
  for(int i = 0; i < 28; i++)
    msg.append(SIXBIT_SYMBOLS.value(b6[i]));  // message id
  
  result = QString("!%1VDM,1,1,,A,%2,2*").arg(talkerID).arg(msg);
  
  quint8 src = 0;
  for(int i = 1; i <= result.length() - 2; i++) {
    src = src ^ quint8(result. at(i).toLatin1());
  }
  
  result.append(QString("%1\r\n").arg(src, 2, 16).replace(' ', '0').toUpper()); //.arg(QChar(10)).arg(QChar(10));
  
  return result;
  
}

QStringList nmea::ais_message_5(QString &talkerID, ais::aisStaticVoyageData* static_voyage_data)
{
  QStringList result = QStringList();
  
  quint8 b6[71];
  memset(&b6, 0, 71);
  
  /// Message ID
  b6[0] = 5;
  
  /// Repeat indicator
  b6[1] = (0 & 0x03) << 4; // 2 значащих бита
  
  /// User ID
  quint64 mmsi64 = static_voyage_data->mmsi & 0x3FFFFFFF; // 30 значащих бит
  mmsi64 <<= 32; // << 32;
  
  for(int i = 0; i < 6; i++) {
    mmsi64 >>= 2;
    b6[1 + i] += (mmsi64 >> (56 - i * 8));
    mmsi64 &= (0x00FFFFFFFFFFFFFF >> (i * 8));
  }
  
  /// AIS version indicator 
  b6[6] += 0x04; // 2 значащих бита // 3 = station compliant with future editions 

  
  /// IMO number
  quint64 imo64 = static_voyage_data->imo & 0x3FFFFFFF; // 30 значащих бит
  imo64 <<= 28; // << 28;
  
  for(int i = 0; i < 6; i++) {
    b6[6 + i] += (imo64 >> (56 - i * 8));
    imo64 &= (0x00FFFFFFFFFFFFFF >> (i * 8));
    imo64 >>= 2;
  }
  
  /// Callsign
  for(int i = 0; i < static_voyage_data->callsign.left(7).length(); i++) {  // static_voyage_data->callsign.length()

    quint8 c = quint8(static_voyage_data->callsign.at(i).toLatin1()) & 0x3F; //str6bit.at(i) & 0x3F;  //  
    b6[11 + i] += c >> 4;
    b6[12 + i] += (c & 0x0F) << 2;
    
  }

  /// Name
  for(int i = 0; i < static_voyage_data->name.left(20).length(); i++) {
    
    quint8 c = quint8(static_voyage_data->name.at(i).toLatin1()) &0x3F;
    b6[18 + i] += c >> 4;
    b6[19 + i] += (c & 0x0F) << 2;
    
  }

  /// Type of ship and cargo type
  b6[38] += static_voyage_data->vessel_ITU_id >> 6;
  b6[39] += static_voyage_data->vessel_ITU_id & 0x3F;

  
  /// Overall dimension/ reference for position
  quint16 A = static_voyage_data->pos_ref_A > 0x01FF ? 0x01FF : static_voyage_data->pos_ref_A;
  quint16 B = static_voyage_data->pos_ref_B > 0x01FF ? 0x01FF : static_voyage_data->pos_ref_B;
  quint8 C = static_voyage_data->pos_ref_C > 0x3F ? 0x3F : static_voyage_data->pos_ref_C;
  quint8 D = static_voyage_data->pos_ref_D > 0x3F ? 0x3F : static_voyage_data->pos_ref_D;

  b6[40] += D;
  b6[41] += C;
  b6[42] += quint8(B >> 3);
  b6[43] += quint8((B & 0x07) << 3);
  b6[43] += quint8(A >> 6);
  b6[44] += quint8(A & 0x3F);

  /// Type of electronic position fixing device
  b6[45] += 0x0C;  // 3 = combined GPS/GLONASS
  
  
  /// Estimated time of arrival
  quint8 minute = static_voyage_data->ETA_utc.minute() & 0x3F;  // 6 bits
  quint8 hour = static_voyage_data->ETA_utc.hour() & 0x1F;      // 5 bits
  quint8 day = static_voyage_data->ETA_day & 0x1F;              // 5 bits
  quint8 month = static_voyage_data->ETA_month & 0x0F;          // 4 bits
  
  b6[45] += minute >> 4;
  b6[46] += (minute & 0x0F) << 2;
  
  b6[46] += hour >> 3;
  b6[47] += (hour & 0x07) << 3;
  
  b6[47] += day >> 2;
  b6[48] += (day & 0x03) << 4;
  
  b6[48] += month;
  
  
  /// Maximum present static draught
  quint8 draft = static_voyage_data->draft * 10 > 255 ? 255 : quint8(trunc(static_voyage_data->draft * 10));
  
  b6[49] += draft >> 2;
  b6[50] += (draft & 0x03) << 4;
  
  /// Destination
  for(int i = 0; i < static_voyage_data->destination.left(20).length(); i++) {
    
    quint8 c = quint8(static_voyage_data->destination.at(i).toLatin1()) & 0x3F;
    b6[50 + i] += c >> 2;
    b6[51 + i] += (c & 0x03) << 4;
    
  }
  
  /// Data terminal equipment (DTE) 
  b6[70] += 0;    // 0 = available
  
  /// Spare. Not used. Should be set to zero
  b6[70] += 0;
  
  
  /// формируем сообщение
  QString msg = "";
  for(int i = 0; i < 71; i++)
    msg.append(SIXBIT_SYMBOLS.value(b6[i]));  // message id
  
  int total_count = int(ceil(qreal(msg.length()) / 62.0));
  for(int i = 0; i < total_count; i++) {
    
    
    QString s = QString("!%1VDM,%2,%3,%4,A,%5,2*")
                              .arg(talkerID)
                              .arg(total_count)
                              .arg(i + 1)
                              .arg(static_voyage_data->sequential_msg_id)
                              .arg(msg.mid(0 + 62 * i, 62));
    
        quint8 src = 0;
        for(int j = 1; j <= s.length() - 2; j++) {
          src = src ^ quint8(s.at(j).toLatin1());
        }
          
        result.append(QString("%1%2").arg(s).arg(QString("%1\r\n").arg(src, 2, 16).replace(' ', '0').toUpper()));
  }
    
  return result;
  
}

QString nmea::ais_sentence_ABK(quint8 message_id, QString &talkerID, ais::aisStaticVoyageData *static_voyage_data)
{
  QString result = QString("$%1ABK,%2,A,%3,,3*").arg(talkerID).arg(static_voyage_data->mmsi).arg(message_id);
  
  quint8 src = 0;
  for(int i = 1; i <= result.length() - 2; i++)
    src = src ^ quint8(result.at(i).toLatin1());
  
  result.append(QString("%1\r\n").arg(src, 2, 16).replace(' ', '0').toUpper());
  
  return result;
}


QString nmea::lag_VBW(const geo::GEOPOSITION& geopos)
{
  qreal speed_knt = CMU.Unit == geo::uKmhKm ? geopos.speed * CMU.ConvertKoeff : geopos.speed;
  qreal drift_knt = CMU.Unit == geo::uKmhKm ? geopos.drift * CMU.ConvertKoeff : geopos.drift;
  
  QString result = QString("$VDVBW,%1,%2,A,%3,%4,A,%5,A,%6,A*")
                .arg(speed_knt, 0, 'f', 1)
                .arg(drift_knt, 0, 'g', 1)
                .arg(0.0, 0, 'f', 1)
                .arg(0.0, 0, 'g', 1)
                .arg(0.0, 0, 'g', 1)
                .arg(0.0, 0, 'g', 1);
  
  quint8 src = 0;
  for(int i = 1; i <= result.length() - 2; i++)
    src = src ^ quint8(result.at(i).toLatin1());
  
  result.append(QString("%1\r\n").arg(src, 2, 16).replace(' ', '0').toUpper());
  
  return result;
  
}

QString nmea::lag_VDR(const geo::GEOPOSITION &geopos)
{
  qreal drift_knt = CMU.Unit == geo::uKmhKm ? geopos.drift * CMU.ConvertKoeff : geopos.drift;
  
  QString result = QString("$VDVDR,%1,T,%2,M,%3,N*")
                .arg(geopos.course, 0, 'f', 1)
                .arg(geopos.course, 0, 'f', 1)
                .arg(drift_knt, 0, 'f', 1);
  
  quint8 src = 0;
  for(int i = 1; i <= result.length() - 2; i++)
    src = src ^ quint8(result.at(i).toLatin1());
  
  result.append(QString("%1\r\n").arg(src, 2, 16).replace(' ', '0').toUpper());
  
  return result;
  
}

QString nmea::lag_VHW(const geo::GEOPOSITION &geopos)
{
  qreal speed_kmh = CMU.Unit == geo::uKmhKm ? geopos.speed : geopos.speed * CMU.ConvertKoeff;
  qreal speed_knt = CMU.Unit == geo::uKmhKm ? geopos.speed * CMU.ConvertKoeff : geopos.speed;
  
  QString result = QString("$VDVHW,%1,T,%2,M,%3,N,%4,K*")
                .arg(geopos.course, 0, 'f', 1)
                .arg(geopos.course, 0, 'f', 1)
                .arg(speed_knt, 0, 'f', 1)
                .arg(speed_kmh, 0, 'f', 1);
  
  quint8 src = 0;
  for(int i = 1; i <= result.length() - 2; i++)
    src = src ^ quint8(result.at(i).toLatin1());
  
  result.append(QString("%1\r\n").arg(src, 2, 16).replace(' ', '0').toUpper());
  
  return result;
}

QString nmea::lag_VLW(const geo::GEOPOSITION &geopos)
{
  qreal dist_ml = CMU.Unit == geo::uKmhKm ? geopos.full_distance * CMU.ConvertKoeff : geopos.full_distance;
  
  QString result = QString("$VDVLW,%1,N,%2,N,0.0,N,0.0,N*")
                .arg(dist_ml, 0, 'f', 1)
                .arg(dist_ml, 0, 'f', 1);
  
  quint8 src = 0;
  for(int i = 1; i <= result.length() - 2; i++)
    src = src ^ quint8(result.at(i).toLatin1());
  
  result.append(QString("%1\r\n").arg(src, 2, 16).replace(' ', '0').toUpper());
  
  return result;
  
}


QStringList nmea::navtex_NRX(const nav::navtexData& ndata)
{

  QStringList result = QStringList();
  
  QString new_text = "";
  for(int i = 0; i < ndata.message_text.length(); i++) {
    if(RESERVED_SYMBOLS.contains(ndata.message_text.at(i)))
      new_text.append('^');
    
    new_text.append(ndata.message_text.at(i));
    
  }
  
  int total_count = int(ceil(qreal(new_text.length()) / 62.0));
  for(int i = 0; i < total_count; i++) {
    
    
      QString s = QString("$%1NRX,%2,%3,00,%4%5%6,%7,%8,%9,%10,%11,%12,%13,A,%14*")
                              .arg("CR")      // 1
                              .arg(QString("%1").arg(total_count, 3).replace(" ", "0"))  // 2
                              .arg(QString("%1").arg(i + 1, 3).replace(" ", "0"))        //3
                              .arg(i > 0 ? "" : ndata.region_letter_id)      // 4
                              .arg(i > 0 ? "" : ndata.message_letter_id)     // 5
                              .arg(i > 0 ? "" : QString("%1").arg(ndata.message_last_number, 2).replace(" ", "0")) // 6
                              .arg(i > 0 ? "" : QString::number(ndata.transmit_frequency_id))    // 7
                              .arg(i > 0 ? "" : QTime::currentTime().toString("hhmmss.00"))    // 8
                              .arg(i > 0 ? "" : QDate::currentDate().toString("dd"))   // 9
                              .arg(i > 0 ? "" : QDate::currentDate().toString("MM"))   // 10
                              .arg(i > 0 ? "" : QDate::currentDate().toString("yyyy"))   // 11
                              .arg(i > 0 ? "" : QString("%1.0").arg(ndata.message_text.length()))       // 12
                              .arg(i > 0 ? "" : "0.0")       // 13
                              .arg(new_text.mid(0 + 62 * i, 62));     // 14
    
        quint8 src = 0;
        for(int j = 1; j <= s.length() - 2; j++) {
          src = src ^ quint8(s.at(j).toLatin1());
        }
          
        result.append(QString("%1%2").arg(s).arg(QString("%1\r\n").arg(src, 2, 16).replace(' ', '0').toUpper()));
  }
    
  return result;
  
}

QString nmea::alarm_ALR(QString talkerID, int id , QString state, QString text)
{
  QString result = "";
  QString new_text = "";
  
  for(int i = 0; i < text.length(); i++) {
    if(RESERVED_SYMBOLS.contains(text.at(i)))
      new_text.append('^');
    
    new_text.append(text.at(i));
    
  }
  
  result = QString("$%1ALR,%2.00,%3,%4,A,%5*")
                   .arg(talkerID)
                   .arg(QTime::currentTime().toString("hhmmss"))
//                   .arg(QTime::currentTime().toString("mm"))
//                   .arg(QTime::currentTime().toString("ss"))
                   .arg(QString("%1").arg(id, 3).replace(" ", "0"))
                   .arg(state)
                   .arg(new_text.left(62));
  
  quint8 src = 0;
  for(int i = 1; i <= result.length() - 2; i++)
    src = src ^ quint8(result.at(i).toLatin1());
  
  result.append(QString("%1\r\n").arg(src, 2, 16).replace(' ', '0').toUpper());
  
  return result;
  
}

QString nmea::gps_RMC(const geo::GEOPOSITION &geopos)
{
  QString result = QString("$GPRMC,%1,A,%2,%3,%4,%5,%6,%7,%8,,,*")
                   .arg(geopos.utc.time().toString("hhmmss.zzz"))
                   .arg(geopos.latitude * 100.0, 0, 'f', 4)
                   .arg(geopos.latitude > 0 ? "N" : "S")
                   .arg(geopos.longtitude * 100.0, 0, 'f', 4)
                   .arg(geopos.longtitude > 0 ? "E" : "W")
                   .arg(geopos.speed, 0, 'f', 2)
                   .arg(geopos.course, 0, 'f', 2)
                   .arg(geopos.utc.date().toString("ddMMyy"));
  
  quint8 src = 0;
  for(int i = 1; i <= result.length() - 2; i++)
    src = src ^ quint8(result.at(i).toLatin1());
  
  result.append(QString("%1\r\n").arg(src, 2, 16).replace(' ', '0').toUpper());
  
  return result;
}
