#include "sv_serialeditor.h"
#include "ui_sv_serialeditor.h"

extern SvSQLITE* SQLITE;
SvSerialEditor* SERIALEDITOR_UI;

SvSerialEditor::SvSerialEditor(SerialPortParams params, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SvSerialEditorDialog)
{
  ui->setupUi(this);
  
  ui->cbBaudrate->clear();
  ui->cbDataBits->clear();
  ui->cbParity->clear();
  ui->cbPortName->clear();
  ui->cbStopBits->clear();
  ui->cbFlowControl->clear();
  
  for(QSerialPortInfo p: QSerialPortInfo::availablePorts())
    ui->cbPortName->addItem(QString("%1: %2 %3").arg(p.portName()).arg(p.manufacturer()).arg(p.description()), QVariant(p.portName()));
  
  for(int b: baudartes_list)
    ui->cbBaudrate->addItem(QString::number(b), b);

  for(QSerialPort::DataBits d: databits_list.keys())
    ui->cbDataBits->addItem(databits_list.value(d), d);
  
  for(QSerialPort::Parity p: parities_list.keys())
    ui->cbParity->addItem(parities_list.value(p), p);
  
  for(QSerialPort::StopBits s: stopbits_list.keys())
    ui->cbStopBits->addItem(stopbits_list.value(s), s);  

  for(QSerialPort::FlowControl f: flowcontrols_list.keys())
    ui->cbFlowControl->addItem(flowcontrols_list.value(f), f);    
  
  ui->cbPortName->setCurrentIndex(ui->cbPortName->findData(params.name));
  ui->cbBaudrate->setCurrentIndex(ui->cbBaudrate->findData(params.baudrate));
  ui->cbDataBits->setCurrentIndex(ui->cbDataBits->findData(params.databits));
  ui->cbFlowControl->setCurrentIndex(ui->cbFlowControl->findData(params.flowcontrol));
  ui->cbParity->setCurrentIndex(ui->cbParity->findData(params.parity));
  ui->cbStopBits->setCurrentIndex(ui->cbStopBits->findData(params.stopbits));
  
  this->params = params;
    
  ui->lblCaption->setText(QString("Настройки порта для устройства %1").arg(devices_list.value(params.dev_type))); 
  
  connect(ui->bnSave, SIGNAL(clicked()), this, SLOT(accept()));
  connect(ui->bnCancel, SIGNAL(clicked()), this, SLOT(reject()));
  
  this->setModal(true);
  this->show();
  
}

SvSerialEditor::~SvSerialEditor()
{
  delete ui;
}

void SvSerialEditor::accept()
{
  params.name = ui->cbPortName->currentData().toString();
  params.description = ui->cbPortName->currentText();
  params.baudrate = ui->cbBaudrate->currentData().toUInt();
  params.databits = QSerialPort::DataBits(ui->cbDataBits->currentData().toInt());
  params.flowcontrol = QSerialPort::FlowControl(ui->cbFlowControl->currentData().toInt());
  params.parity = QSerialPort::Parity(ui->cbParity->currentData().toInt());
  params.stopbits = QSerialPort::StopBits(ui->cbStopBits->currentData().toInt());
  
  try {
    
    QSqlError err = check_params_exists(params.dev_type);
    if(QSqlError::NoError != err.type()) _exception.raise(err.databaseText());

    err = SQLITE->execSQL(QString(SQL_UPDATE_DEVICES_SERIAL_PARAMS_WHERE)
                          .arg(params.name)
                          .arg(params.baudrate)
                          .arg(params.parity)
                          .arg(params.stopbits)
                          .arg(params.databits)
                          .arg(params.flowcontrol)
                          .arg(params.description)
                          .arg(params.dev_type));
    
    if(QSqlError::NoError != err.type()) _exception.raise(err.databaseText());
  
  
  }
  
  catch(SvException e) {
    
    _last_error = e.err;
//        qDebug() << _last_error;
    QDialog::reject();
  }
  
  QDialog::accept();
  
}

QSqlError check_params_exists(idev::SvSimulatedDeviceTypes dev_type)
{
  QSqlQuery *q = new QSqlQuery(SQLITE->db);
  QSqlError err;
  
  err = SQLITE->execSQL(QString(SQL_SELECT_COUNT_DEVICES_PARAMS_WHERE).arg(dev_type), q);
  
  if(q->next() && q->value("count").toInt() == 0) {
    
    err = SQLITE->execSQL(QString(SQL_INSERT_DEVICES_PARAMS).arg(int(dev_type)));
    
  }
  
  q->finish();
  delete q;

  return err;
  
}
