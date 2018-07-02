#include "sv_navtexeditor.h"
#include "ui_sv_navtexeditor.h"

SvNavtexEditor* NAVTEXEDITOR_UI;
extern SvSQLITE* SQLITE;

SvNavtexEditor::SvNavtexEditor(QWidget *parent, int navtexId) :
  QDialog(parent),
  ui(new Ui::SvNavtexEditorDialog)
{
  ui->setupUi(this);
  
  _showMode = navtexId == -1 ? smNew : smEdit;
  
  loadMessages();
  loadRegions();
  loadFrequencies();
  
  if(_showMode == smEdit) {
    
    QSqlQuery* q = new QSqlQuery(SQLITE->db);
    if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_NAVTEX_WHERE_ID).arg(navtexId), q).type()) {
      
      q->finish();
      delete q;
      return;
    }
   
    if(q->next()) {
      
      _data.id = q->value("id").toUInt();
      _data.region_id = q->value("station_region_id").toUInt();
      _data.message_id = q->value("message_id").toUInt();
      _data.message_text = q->value("message_text").toString();
      _data.transmit_frequency_id = q->value("transmit_frequency").toUInt();
      _data.transmit_frequency = Frequencies.value(_data.transmit_frequency_id);
      _data.message_last_number = q->value("message_last_number").toUInt();
      
    }
    
    q->finish();
    delete q;
    
  }
 
  ui->cbNAVTEXMessageType->setCurrentIndex(ui->cbNAVTEXMessageType->findData(_data.message_id));
  ui->cbNAVTEXStation->setCurrentIndex(ui->cbNAVTEXStation->findData(_data.region_id));
  ui->textNAVTEXMessageText->setText(_data.message_text);
  ui->cbNAVTEXTransmitFrequency->setCurrentIndex(ui->cbNAVTEXTransmitFrequency->findData(_data.transmit_frequency_id));
  
  connect(ui->bnSave, SIGNAL(clicked()), this, SLOT(accept()));
  connect(ui->bnCancel, SIGNAL(clicked()), this, SLOT(reject()));
  
  this->setModal(true);
  this->show();
  
}

SvNavtexEditor::~SvNavtexEditor()
{
  delete ui;
}

void SvNavtexEditor::loadMessages()
{
  ui->cbNAVTEXMessageType->clear();
  _messages.clear();
  
  QSqlQuery* q = new QSqlQuery(SQLITE->db);
  if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_NAVTEX_MESSAGES), q).type()) {
    
    q->finish();
    return;
  }
  
  while(q->next()) {
    ui->cbNAVTEXMessageType->addItem(QString("%1 (%2)")
                                 .arg(q->value("letter_id").toString())
                                 .arg(q->value("designation").toString()),
                              q->value("id").toUInt());
    
    _messages.insert(q->value("id").toUInt(),
                     QString("%1|%2").arg(q->value("letter_id").toString())
                                     .arg(q->value("designation").toString()));
    
  }

  q->finish();
  delete q;
  
  if(ui->cbNAVTEXMessageType->count()) ui->cbNAVTEXMessageType->setCurrentIndex(0);
  
  ui->bnSave->setEnabled(!ui->cbNAVTEXMessageType->currentData().isNull());
}

void SvNavtexEditor::loadRegions()
{
  ui->cbNAVTEXStation->clear();
  _regions.clear();
  
  QSqlQuery* q = new QSqlQuery(SQLITE->db);
  if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_NAVTEX_REGIONS), q).type()) {
    
    q->finish();
    return;
  }
  
  while(q->next()) {
    ui->cbNAVTEXStation->addItem(QString("%1 (%2)")
                                 .arg(q->value("letter_id").toString())
                                 .arg(q->value("station_name").toString()),
                              q->value("id").toUInt());
  
    _regions.insert(q->value("id").toUInt(),
                   QString("%1|%2").arg(q->value("letter_id").toString())
                                   .arg(q->value("station_name").toString()));
  
  }

  q->finish();
  delete q;
  
  if(ui->cbNAVTEXStation->count()) ui->cbNAVTEXStation->setCurrentIndex(0);
  
  ui->bnSave->setEnabled(!ui->cbNAVTEXStation->currentData().isNull());
  
}

void SvNavtexEditor::loadFrequencies()
{
  ui->cbNAVTEXTransmitFrequency->clear();
  
//  QMap<int, QString> freqs = frequencies();
  
  for(int freq_id: Frequencies.keys())
    ui->cbNAVTEXTransmitFrequency->addItem(Frequencies.value(freq_id), freq_id);
  
}

void SvNavtexEditor::accept()
{
  
  _data.region_id = ui->cbNAVTEXStation->currentData().toUInt();
  _data.region_letter_id = _regions.value(_data.region_id).split("|").first();
  _data.region_station_name = _regions.value(_data.region_id).split("|").last();
  _data.message_id = ui->cbNAVTEXMessageType->currentData().toUInt();
  _data.message_letter_id = _messages.value(_data.message_id).split("|").first();
  _data.message_designation = _messages.value(_data.message_id).split("|").last();
  _data.message_text = ui->textNAVTEXMessageText->toPlainText().replace('\'', '"');
  _data.transmit_frequency_id = ui->cbNAVTEXTransmitFrequency->currentData().toUInt();
  _data.transmit_frequency = Frequencies.value(_data.transmit_frequency_id); 
  
  switch (this->_showMode) {
    
    case smNew: {
      
      try {
        
        QSqlError sql = SQLITE->execSQL(QString(SQL_INSERT_NAVTEX)
                                        .arg(_data.region_id)
                                        .arg(_data.message_id)
                                        .arg(_data.message_text)
                                        .arg(_data.transmit_frequency_id));
        
        if(QSqlError::NoError != sql.type()) _exception.raise(sql.databaseText());
        
        QDialog::done(Accepted);
        
      }
      
      catch(SvException &e) {
          
        _last_error = e.err;
//        qDebug() << 111 << _last_error;
        QDialog::done(Error);
        
      }
    }
      
    case smEdit: {
      
      try {
        
        QSqlError sql = SQLITE->execSQL(QString(SQL_UPDATE_NAVTEX)
                                        .arg(_data.region_id)
                                        .arg(_data.message_id)
                                        .arg(_data.message_text)
                                        .arg(_data.transmit_frequency_id)
                                        .arg(_data.id));
        
        if(QSqlError::NoError != sql.type()) _exception.raise(sql.databaseText());
        
        QDialog::done(Accepted);
        
      }
      
      catch(SvException &e) {
          
        _last_error = e.err;
//        qDebug() << 222 << _last_error;
        QDialog::done(Error);
        
      }
    }
  }
}

void SvNavtexEditor::on_bnSimpleMessage_clicked()
{
  QSqlQuery* q = new QSqlQuery(SQLITE->db);
  if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_NAVTEX_SIMPLE_MESSAGE)
                                           .arg(ui->cbNAVTEXMessageType->currentData().toUInt()), q).type()) {
    
    q->finish();
    return;
  }
  
  if(q->next())
    ui->textNAVTEXMessageText->setText(q->value("simple_message").toString());
  
  q->finish();
  delete q;
  
}
