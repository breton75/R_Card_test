#ifndef SV_NAVSTATEDITOR_H
#define SV_NAVSTATEDITOR_H

#include <QDialog>

#include "geo.h"
#include "sv_ais.h"
#include "sql_defs.h"
#include "sv_gps.h"

#include "../../svlib/sv_sqlite.h"
#include "../../svlib/sv_exception.h"

namespace Ui {
class SvNavStatEditorDialog;
}

enum PredefPosItems {
  
  ppiCurrent,
  ppiCenter,
  ppiLeftTop,
  ppiLeft,
  ppiLeftBottom,
  ppiTop,
  ppiBottom,
  ppiRightTop,
  ppiRight,
  ppiRightBottom
  
};

class SvNavStatEditor : public QDialog
{
  Q_OBJECT
  
public:
  enum DoneCode { Rejected = QDialog::Rejected, Accepted = QDialog::Accepted, Error };
  
  explicit SvNavStatEditor(QWidget *parent, const int vessel_id, const geo::BOUNDS& bounds);
  ~SvNavStatEditor();
  
  QString last_error() { return _last_error; }
  
  ais::aisNavStat navstat() { return _navstat; }
  geo::GEOPOSITION geopos() { return _geopos; }
  
//  qreal speed() { return _speed; }
//  qreal course() { return _course; }
  
private:
  Ui::SvNavStatEditorDialog *ui;
  
  int _vessel_id;
  
  ais::aisNavStat _navstat;
  geo::GEOPOSITION _geopos;
  geo::BOUNDS _bounds;
  gps::gpsInitParams _gps_init;
  
//  qreal _speed;
//  qreal _course;
  
  QString _last_error = "";
  SvException _exception;
  
  void loadNavStats();
  void loadPredefinedPositions();
  
  void accept() Q_DECL_OVERRIDE;
  
private slots:
  void on_predefinedCurrentIndexChanged(int index);
  
};

#endif // SV_NAVSTATEDITOR_H
