#ifndef SV_NETWORKEDITOR_H
#define SV_NETWORKEDITOR_H

#include <QDialog>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QAbstractSocket>

#include "../../svlib/sv_sqlite.h"
#include "sql_defs.h"
#include "sv_exception.h"
#include "sv_idevice.h"


namespace Ui {
class SvNetworkEditorDialog;
}

class SvNetworkEditor : public QDialog
{
  Q_OBJECT
  
public:
  explicit SvNetworkEditor(idev::NetworkParams params, QWidget *parent);
  ~SvNetworkEditor();
  
  idev::NetworkParams params;
  
  QString last_error() { return _last_error; }
    
  
private slots:
  void on_cbProtocolCurrentIndexChanged(int index);
  void on_cbTranslateCurrentIndexChanged(int index);
  
private:
  Ui::SvNetworkEditorDialog *ui;
  
  idev::SvSimulatedDeviceTypes dev_type;
  
  SvException _exception;
  QString _last_error = "";
  
  QMap<idev::SvSimulatedDeviceTypes, QString> devices_list = {{idev::sdtEchoMulti, "Многолучевой эхолот"}, {idev::sdtEchoFish, "Рыбопромысловый эхолот"}};
  
  void accept();
  
  QSqlError check_params_exists(idev::SvSimulatedDeviceTypes dev_type);
  
};

#endif // SV_NETWORKEDITOR_H
