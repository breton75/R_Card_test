#include "sv_networkeditor.h"
#include "ui_sv_networkeditor.h"

SvNetworkEditor* NETWORKEDITOR_UI;
extern SvSQLITE *SQLITE;

SvNetworkEditor::SvNetworkEditor(idev::NetworkParams params, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SvNetworkEditorDialog)
{
  ui->setupUi(this);
  
  this->params = params;
  
  ui->cbInterface->clear();
  for(QNetworkInterface itf: QNetworkInterface::allInterfaces()) {
    if(itf.flags().testFlag(QNetworkInterface::CanMulticast) &&
        !itf.flags().testFlag(QNetworkInterface::IsLoopBack))
      ui->cbInterface->addItem(itf.humanReadableName(), itf.index());
  }
  
  ui->cbProtocol->clear();
  ui->cbProtocol->addItem("UDP", QAbstractSocket::UdpSocket);
  ui->cbProtocol->addItem("TCP", QAbstractSocket::TcpSocket);
  
  ui->cbTranslateType->clear();
  for(QHostAddress::SpecialAddress key: idev::NetworkParams::TranslateTypes().keys())
    ui->cbTranslateType->addItem(idev::NetworkParams::TranslateTypes().value(key), key);
  
  connect(ui->cbProtocol, SIGNAL(currentIndexChanged(int)), this, SLOT(on_cbProtocolCurrentIndexChanged(int)));
  connect(ui->cbTranslateType, SIGNAL(currentIndexChanged(int)), this, SLOT(on_cbTranslateCurrentIndexChanged(int)));
  
  ui->cbInterface->setCurrentIndex(ui->cbInterface->findData(params.ifc));
  ui->cbProtocol->setCurrentIndex(ui->cbProtocol->findData(params.protocol));
  ui->cbTranslateType->setCurrentIndex(ui->cbTranslateType->findData(params.translate_type));
  ui->editIP->setText(QHostAddress(params.ip).toString());  
  ui->spinPort->setValue(params.port);

  ui->lblCaption->setText(QString("Настройки для: %1").arg(devices_list.value(params.dev_type)));
  
  
  
  connect(ui->bnSave, SIGNAL(clicked()), this, SLOT(accept()));
  connect(ui->bnCancel, SIGNAL(clicked()), this, SLOT(reject()));
  
  this->setModal(true);
  this->show();
}

SvNetworkEditor::~SvNetworkEditor()
{
  delete ui;
}

void SvNetworkEditor::accept()
{
  params.ifc = ui->cbInterface->currentData().toUInt();
  params.protocol = ui->cbProtocol->currentData().toInt();
  params.ip = QHostAddress(ui->editIP->text().trimmed()).toIPv4Address();
  params.port = ui->spinPort->value();
  
  if(params.protocol == QAbstractSocket::TcpSocket)
    
    params.description = QString("%1:%2 (%3)")
                       .arg(ui->cbProtocol->currentText())
                       .arg(params.port)
                       .arg(QNetworkInterface::interfaceFromIndex(params.ifc).humanReadableName());
  
  else {
    
    QHostAddress::SpecialAddress tt = QHostAddress::SpecialAddress(ui->cbTranslateType->currentData().toInt());
    
    params.description = QString("%1:%2:%3 (%4)")
                       .arg(ui->cbProtocol->currentText())
                       .arg(tt == QHostAddress::Null ?
                              QHostAddress(params.ip).toString() : idev::NetworkParams::TranslateTypes().value(tt))
                       .arg(params.port)
                       .arg(QNetworkInterface::interfaceFromIndex(params.ifc).humanReadableName());    
    
  }
  
  params.translate_type = QHostAddress::SpecialAddress(ui->cbTranslateType->currentData().toInt());
  
  try {
    
    QSqlError err = check_params_exists(params.dev_type); 
    if(QSqlError::NoError != err.type()) _exception.raise(err.databaseText());
    
    err = SQLITE->execSQL(QString(SQL_UPDATE_DEVICES_NETWORK_PARAMS_WHERE)
                          .arg(params.ifc)
                          .arg(params.protocol)
                          .arg(params.ip)
                          .arg(params.port)
                          .arg(params.description)
                          .arg(params.translate_type)
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

QSqlError SvNetworkEditor::check_params_exists(idev::SvSimulatedDeviceTypes dev_type)
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

void SvNetworkEditor::on_cbProtocolCurrentIndexChanged(int index)
{
  ui->cbTranslateType->setEnabled(ui->cbProtocol->itemData(index).toInt() == int(QAbstractSocket::UdpSocket));
  
  ui->editIP->setEnabled(ui->cbTranslateType->isEnabled() &&
                         ui->cbTranslateType->currentData().toInt() == int(QHostAddress::Null));
}

void SvNetworkEditor::on_cbTranslateCurrentIndexChanged(int index)
{
  ui->editIP->setEnabled(ui->cbTranslateType->itemData(index).toInt() == int(QHostAddress::Null));
}
