#ifndef SV_NAVSTATEDITOR_H
#define SV_NAVSTATEDITOR_H

#include <QDialog>

#include "geo.h"
#include "sv_ais.h"
#include "sql_defs.h"

#include "../../svlib/sv_sqlite.h"
#include "../../svlib/sv_exception.h"

namespace Ui {
class SvNavStatEditorDialog;
}

class SvNavStatEditor : public QDialog
{
  Q_OBJECT
  
public:
  enum DoneCode { Rejected = QDialog::Rejected, Accepted = QDialog::Accepted, Error };
  
  explicit SvNavStatEditor(QWidget *parent, const int vessel_id, const ais::aisNavStat& navstat, const qreal speed, const qreal course);
  ~SvNavStatEditor();
  
  QString last_error() { return _last_error; }
  
  ais::aisNavStat navstat() { return _navstat; }
  qreal speed() { return _speed; }
  qreal course() { return _course; }
  
private:
  Ui::SvNavStatEditorDialog *ui;
  
  int _vessel_id;
  
  ais::aisNavStat _navstat;
  qreal _speed;
  qreal _course;
  
  QString _last_error = "";
  SvException _exception;
  
  void loadNavStats();
  
  void accept() Q_DECL_OVERRIDE;
  
};

#endif // SV_NAVSTATEDITOR_H
