#include "sv_navstateditor.h"
#include "ui_sv_navstateditor.h"

SvNavStatEditor* NAVSTATEDITOR_UI;
extern SvSQLITE *SQLITE;

SvNavStatEditor::SvNavStatEditor(int vessel_id, ais::aisNavStat navstat, qreal speed, qreal course, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SvNavStatEditorDialog)
{
  ui->setupUi(this);
  
  loadNavStats();
  
  _vessel_id = vessel_id;
  
  ui->cbNavStatus->setCurrentIndex(ui->cbNavStatus->findData(navstat.ITU_id));
  ui->spinCourse->setValue(int(course));
  ui->dspinSpeed->setValue(speed);
  
  connect(ui->bnSave, SIGNAL(clicked()), this, SLOT(accept()));
  connect(ui->bnCancel, SIGNAL(clicked()), this, SLOT(reject()));
  
  this->setModal(true);
  this->show();
  
}

SvNavStatEditor::~SvNavStatEditor()
{
  delete ui;
}


void SvNavStatEditor::loadNavStats()
{
  QSqlQuery* q = new QSqlQuery(SQLITE->db);
  if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_NAV_STATS), q).type()) {
    
    q->finish();
    return;
  }
  
  while(q->next())
    ui->cbNavStatus->addItem(q->value("status_name").toString(),
                              q->value("ITU_id").toUInt());

  q->finish();
  delete q;
  
  if(ui->cbNavStatus->count()) ui->cbNavStatus->setCurrentIndex(0);
  
  ui->bnSave->setEnabled(!ui->cbNavStatus->currentData().isNull());
  
}

void SvNavStatEditor::accept()
{
  _navstat.ITU_id = ui->cbNavStatus->currentData().toInt();
  _navstat.name = ui->cbNavStatus->currentText();
  
  _speed = ui->dspinSpeed->value();
  _course = ui->spinCourse->value();
  
  try {
    _exception.raise("SQLITE");
    if(!SQLITE->transaction()) _exception.raise(SQLITE->db.lastError().databaseText());
     
      QSqlError sql = SQLITE->execSQL(QString(SQL_UPDATE_NAVSTAT)
                            .arg(_course)
                            .arg(_speed)
                            .arg(_navstat.ITU_id)
                            .arg(_vessel_id));
            
      if(QSqlError::NoError != sql.type()) _exception.raise(sql.databaseText());
      
      if(!SQLITE->commit()) _exception.raise(SQLITE->db.lastError().databaseText());
      
  }
  
  catch(SvException &e) {
      
//    SQLITE->rollback();
    _last_error = e.err;
    QDialog::reject();
    qDebug() << _last_error;
    return;
  }
  
  QDialog::accept();
  
}
