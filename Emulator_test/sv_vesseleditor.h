#ifndef SV_VESSELEDITOR_H
#define SV_VESSELEDITOR_H

#include <QDialog>
#include <QDateTime>
#include <QMessageBox>
#include <QException>

#include "../../svlib/sv_sqlite.h"
#include "sql_defs.h"
#include "sv_exception.h"
#include "geo.h"

//#define SQL_SELECT_BEACON "select plan.id, plan.uid, plan.lon, plan.lat, plan.date_time, plan.description from plan where id=%1"

namespace Ui {
class SvVesselEditorDialog;
}

class SvVesselEditor : public QDialog
{
  Q_OBJECT
  
public:
  enum ShowMode { smNew = 0, smEdit = 1 };
                  
  explicit SvVesselEditor(QWidget *parent, int vesselId = -1, bool self = false);

  ~SvVesselEditor();
  
  int showMode;
  
  QString last_error() { return _last_error; }
  
  int     t_vessel_id = -1;
  bool    t_self = false;
  QString t_static_callsign = "";
  QString t_static_name = "";
  quint32 t_static_imo = 0;
  quint32 t_static_mmsi = 0;
  quint32 t_static_vessel_ITU_id;
//  QString t_static_vessel_type_name = "";
  quint32 t_static_pos_ref_A = 20;
  quint32 t_static_pos_ref_B = 20;
  quint32 t_static_pos_ref_C = 10;
  quint32 t_static_pos_ref_D = 10;
  quint8 t_static_DTE = 1;
  QString t_static_talker_ID = "AI";
  
  QString t_voyage_destination = "";
  QTime t_voyage_ETA_utc = QTime::currentTime();
  quint8 t_voyage_ETA_day = 1;
  quint8 t_voyage_ETA_month = 1;
  qreal   t_voyage_draft = 1.0;
  quint32 t_voyage_cargo_ITU_id = 0;
//  QString t_voyage_cargo_type_name = "";
  quint32 t_voyage_team = 1;
  
  quint32 t_gps_timeout = 1000;
  bool    t_init_random_coordinates = false;
  bool    t_init_random_course = false;
  bool    t_init_random_speed = false;
  quint32 t_init_course_change_ratio = 45;
  qreal   t_init_course_change_segment = 10;
  quint32 t_init_speed_change_ratio = 10;
  qreal   t_init_speed_change_segment = 4;
  
  
public slots:
  void accept() Q_DECL_OVERRIDE;
  
private:
  Ui::SvVesselEditorDialog *ui;
  
  QString _last_error = "";
  SvException _exception;
  
  void loadVesselTypes();
  void loadCargoTypes();
  void loadInitRandoms();
  
};

#endif // SV_VESSELEDITOR_H
