#ifndef NMEA_H
#define NMEA_H
 
#include <QMap>
#include <QDebug>

#include "geo.h"
#include "sv_ais.h"
#include "sv_navtex.h"

namespace nmea {


  enum NMEASentence {
    
    ABK_UAIS_Addressed_and_binary_broadcast_acknowledgement,
    AIR_InterrogationRequest,
    
//    OSD_OwnShipData,
    VDO_UAIS_VHF_Datalink_Own_vessel_report,
    VDM_UAIS_VHF_Datalink_Message,
    
    SSD_UAIS_Ship_Static_Data,
    VSD_UAIS_Voyage_Static_Data,
    
    Q_Query
  };
  
  


  /** NMEA 0183 Version 3.01 --- TABLE 7 - SIX-BIT BINARY FIELD CONVERSION TABLE **/
  inline QByteArray str_to_6bit(const QString& str);

  
  QString ais_message_1_2_3(quint8 message_id, QString &talkerID, ais::aisStaticVoyageData *static_voyage_data, quint8 nav_status, geo::GEOPOSITION& geopos);
  
  QStringList ais_message_5(QString &talkerID, ais::aisStaticVoyageData *static_voyage_data, ais::aisNavStat* navstat);
  
  QString ais_sentence_ABK(quint8 message_id, QString &talkerID, ais::aisStaticVoyageData* static_voyage_data);
  
  QString lag_VBW(const geo::GEOPOSITION &geopos);
  QString lag_VDR(const geo::GEOPOSITION &geopos);
  QString lag_VHW(const geo::GEOPOSITION &geopos);
  QString lag_VLW(const geo::GEOPOSITION &geopos);
  
  QStringList navtex_NRX(const nav::navtexData &ndata);
  
  QString alarm_ALR(QString talkerID, int id, QString state, QString text);
                
}



#endif // NMEA_H
