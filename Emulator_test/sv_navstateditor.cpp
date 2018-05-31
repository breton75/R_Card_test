#include "sv_navstateditor.h"
#include "ui_sv_navstateditor.h"

SvNavStatEditor* NAVSTATEDITOR_UI;
extern SvSQLITE *SQLITE;
extern QMap<int, gps::SvGPS*>* GPSs;
extern QMap<int, ais::SvAIS*>* AISs;

SvNavStatEditor::SvNavStatEditor(QWidget *parent, const int vessel_id, const geo::BOUNDS& bounds):
  QDialog(parent),
  ui(new Ui::SvNavStatEditorDialog)
{
  ui->setupUi(this);

  loadNavStats();
  loadPredefinedPositions();
  
  _vessel_id = vessel_id;
  _navstat = *AISs->value(vessel_id)->navStatus();
  _geopos = AISs->value(vessel_id)->dynamicData()->geoposition;
  _gps_init = GPSs->value(vessel_id)->initParams();
  
  _bounds = bounds;
  
  ui->cbNavStatus->setCurrentIndex(ui->cbNavStatus->findData(_navstat.ITU_id));
  ui->spinCourse->setValue(int(_geopos.course));
  ui->dspinSpeed->setValue(_geopos.speed);
  ui->dspinLatitude->setValue(_geopos.latitude);
  ui->dspinLongtitude->setValue(_geopos.longtitude);
  
  ui->checkCourse->setChecked(GPSs->value(vessel_id)->initParams().init_fixed_course);
  ui->checkSpeed->setChecked(GPSs->value(vessel_id)->initParams().init_fixed_speed);
    
  connect(ui->cbPredefinedPosition, SIGNAL(currentIndexChanged(int)), this, SLOT(on_predefinedCurrentIndexChanged(int)));
  connect(ui->bnSave, SIGNAL(clicked()), this, SLOT(accept()));
  connect(ui->bnCancel, SIGNAL(clicked()), this, SLOT(reject()));
  
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

void SvNavStatEditor::loadPredefinedPositions()
{
  ui->cbPredefinedPosition->addItem("Текущее", ppiCurrent);
  ui->cbPredefinedPosition->addItem("По центру", ppiCenter);
  ui->cbPredefinedPosition->addItem("Верхний левый угол", ppiLeftTop);
  ui->cbPredefinedPosition->addItem("Левая граница", ppiLeft);
  ui->cbPredefinedPosition->addItem("Нижний левый угол", ppiLeftBottom);
  ui->cbPredefinedPosition->addItem("Верхняя граница", ppiTop);
  ui->cbPredefinedPosition->addItem("Нижняя граница", ppiBottom);
  ui->cbPredefinedPosition->addItem("Верхний правый угол", ppiRightTop);
  ui->cbPredefinedPosition->addItem("Правая граница", ppiRight);
  ui->cbPredefinedPosition->addItem("Нижний правый угол", ppiRightBottom);

  ui-> cbPredefinedPosition->setCurrentIndex(0);
  
}

void SvNavStatEditor::on_predefinedCurrentIndexChanged(int index)
{

  switch (ui->cbPredefinedPosition->itemData(index).toInt()) {
    
    case ppiBottom:
      ui->dspinLatitude->setValue(_bounds.min_lat);
      break;
      
    case ppiCenter:
      ui->dspinLatitude->setValue((_bounds.max_lat + _bounds.min_lat) / 2.0);
      ui->dspinLongtitude->setValue((_bounds.max_lon + _bounds.min_lon) / 2.0);
      break;
      
    case ppiLeft:
      ui->dspinLongtitude->setValue(_bounds.min_lon);
      break;
      
    case ppiLeftBottom:
      ui->dspinLatitude->setValue(_bounds.min_lat);
      ui->dspinLongtitude->setValue(_bounds.min_lon);
      break;
      
    case ppiLeftTop:
      ui->dspinLatitude->setValue(_bounds.max_lat);
      ui->dspinLongtitude->setValue(_bounds.min_lon);
      break;
      
    case ppiRight:
      ui->dspinLongtitude->setValue(_bounds.max_lon);
      break;
      
    case ppiRightBottom:
      ui->dspinLatitude->setValue(_bounds.min_lat);
      ui->dspinLongtitude->setValue(_bounds.max_lon);
      break;
      
    case ppiRightTop:
      ui->dspinLatitude->setValue(_bounds.max_lat);
      ui->dspinLongtitude->setValue(_bounds.max_lon);
      break;
      
    case ppiTop:
      ui->dspinLatitude->setValue(_bounds.max_lat);
      break;
      
    default:
      break;
  }
}

void SvNavStatEditor::accept()
{
  _navstat.ITU_id = ui->cbNavStatus->currentData().toInt();
  _navstat.name = ui->cbNavStatus->currentText();
  
  _geopos.speed = ui->dspinSpeed->value();
  _geopos.course = ui->spinCourse->value();
  _geopos.latitude = ui->dspinLatitude->value();
  _geopos.longtitude = ui->dspinLongtitude->value();

  _gps_init.geoposition = _geopos;
  _gps_init.init_fixed_course = ui->checkCourse->isChecked();
  _gps_init.init_fixed_speed = ui->checkSpeed->isChecked();
  
  try {

    if(!SQLITE->transaction()) _exception.raise(SQLITE->db.lastError().databaseText());
     
      QSqlError sql = SQLITE->execSQL(QString(SQL_UPDATE_NAVSTAT_AIS)
                            .arg(_geopos.course)
                            .arg(_geopos.speed)
                            .arg(_navstat.ITU_id)
                            .arg(_geopos.latitude)
                            .arg(_geopos.longtitude)
                            .arg(_vessel_id));
            
      if(QSqlError::NoError != sql.type()) _exception.raise(sql.databaseText());
      

      sql = SQLITE->execSQL(QString(SQL_UPDATE_NAVSTAT_GPS)
                                    .arg(ui->checkCourse->isChecked())
                                    .arg(ui->checkSpeed->isChecked())
                                    .arg(_vessel_id));
        
      if(QSqlError::NoError != sql.type()) _exception.raise(sql.databaseText());
      
      if(!SQLITE->commit()) _exception.raise(SQLITE->db.lastError().databaseText());
  
      GPSs->value(_vessel_id)->setInitParams(_gps_init);
      
      AISs->value(_vessel_id)->setCourse(_geopos.course);
      AISs->value(_vessel_id)->setSpeed(_geopos.speed);
      AISs->value(_vessel_id)->setLatitude(_geopos.latitude);
      AISs->value(_vessel_id)->setLongtitude(_geopos.longtitude);
      AISs->value(_vessel_id)->setNavStatus(_navstat);
      
      
      QDialog::done(Accepted);
      
  }
  
  catch(SvException &e) {
      
    SQLITE->rollback();
    _last_error = e.err;
    QDialog::done(Error);
//    qDebug() << _last_error;
  }
  
}
