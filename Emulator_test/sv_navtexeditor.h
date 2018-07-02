#ifndef SV_NAVTEXEDITOR_H
#define SV_NAVTEXEDITOR_H

#include <QDialog>
#include <QDateTime>
#include <QMessageBox>
#include <QException>

#include "../../svlib/sv_sqlite.h"
#include "sql_defs.h"
#include "sv_exception.h"
#include "../../svlib/sv_sqlite.h"
#include "sv_navtex.h";

const QMap<int, QString> Frequencies = {{1, "419 кГц"}, {2, "518 кГц"}, {3, "4209,5 кГц"}};

namespace Ui {
class SvNavtexEditorDialog;
}

class SvNavtexEditor : public QDialog
{
  Q_OBJECT
  
  enum ShowMode { smNew = 0, smEdit = 1 };
  
public:
  
  static QMap<int, QString> frequencies() { return Frequencies; } // {{1, "419 кГц"}, {2, "518 кГц"}, {3, "4209,5 кГц"}}; }
  
  enum DoneCode { Rejected = QDialog::Rejected, Accepted = QDialog::Accepted, Error };
  
  explicit SvNavtexEditor(QWidget *parent, int navtexId = -1);
  ~SvNavtexEditor();
  
  nav::navtexData data() { return _data; }
  QString last_error() { return _last_error; }
  
  
private:
  Ui::SvNavtexEditorDialog *ui;
  
  nav::navtexData _data;
  
//  int     t_id = -1;
//  bool    t_isactive = true;
//  int     t_message_id = 1;
//  int     t_region_id = 1;
//  QString t_message_letter_id;
//  QString t_region_letter_id;
//  QString t_message_designation = "";
//  QString t_region_station_name = "";
//  QString t_message_text = "";
//  quint32 t_message_last_number = 0;
//  QString t_transmit_frequency = "";
//  quint32 t_transmit_frequency_id = 1;
  
  QString _last_error = "";
  SvException _exception;
  
  int _showMode;
  
  QMap<int, QString> _messages;
  QMap<int, QString> _regions;
  
private:
  void accept() Q_DECL_OVERRIDE;
  
  void loadMessages();
  void loadRegions();
  void loadFrequencies();  
  
private slots:
  void on_bnSimpleMessage_clicked();
};

#endif // SV_NAVTEXEDITOR_H
