#include "sv_navstateditor.h"
#include "ui_sv_navstateditor.h"

SvNavStatEditor* NAVSTATEDITOR_UI;
extern SvSQLITE *SQLITE;

SvNavStatEditor::SvNavStatEditor(ais::aisNavStat navstat, qreal speed, qreal course, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SvNavStatEditorDialog)
{
  ui->setupUi(this);
  
  t_navstat = navstat;
  t_speed = speed;
  t_course = course;
  
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
  t_navstat.ITU_id = ui->cbNavStatus->currentData().toInt();
  t_navstat.name = ui->cbNavStatus->currentText();
  
  t_speed = ui->dspinSpeed->value();
  t_course = ui->spinCourse->value();
  
  
  QDialog::accept();
  
}
