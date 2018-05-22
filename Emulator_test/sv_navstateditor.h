#ifndef SV_NAVSTATEDITOR_H
#define SV_NAVSTATEDITOR_H

#include <QDialog>

#include "geo.h"
#include "sv_ais.h"

namespace Ui {
class SvNavStatEditorDialog;
}

struct NavStatParams {
  
};

class SvNavStatEditor : public QDialog
{
  Q_OBJECT
  
public:
  explicit SvNavStatEditor(ais::aisNavStat navstat, qreal speed, qreal course, QWidget *parent = 0);
  ~SvNavStatEditor();
  
  ais::aisNavStat t_navstat;
  qreal t_speed;
  qreal t_course;
  
private:
  Ui::SvNavStatEditorDialog *ui;
  
  void loadNavStats();
  void accept();
};

#endif // SV_NAVSTATEDITOR_H
