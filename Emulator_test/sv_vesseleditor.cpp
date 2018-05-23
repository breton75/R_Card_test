#include "sv_vesseleditor.h"
#include "ui_sv_vesseleditor.h"

SvVesselEditor *VESSELEDITOR_UI;
extern SvSQLITE *SQLITE;
extern geo::UnitsInfo CMU;

SvVesselEditor::SvVesselEditor(QWidget *parent, int vesselId, bool self) :
  QDialog(parent),
  ui(new Ui::SvVesselEditorDialog)
{
  ui->setupUi(this);
  
  showMode = vesselId == -1 ? smNew : smEdit;
  
  loadCargoTypes();
  loadVesselTypes();
  loadInitRandoms();
  
  t_self = self;
  
  if(showMode == smEdit) {
    
    QSqlQuery* q = new QSqlQuery(SQLITE->db);
    if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_VESSEL_WHERE_ID).arg(vesselId), q).type()) {
      
      q->finish();
      delete q;
      return;
    }
   
    if(q->next()) {
      
      t_vessel_id = vesselId;
      
      t_self = q->value("self").toBool();            
      t_static_callsign = q->value("static_callsign").toString();
      t_static_name = q->value("static_name").toString();
      t_static_imo = q->value("static_imo").toUInt();
      t_static_mmsi = q->value("static_mmsi").toUInt();
      t_static_vessel_ITU_id = q->value("static_type_ITU_id").toUInt();
//      t_static_vessel_type_name = q->value("static_vessel_type_name").toString();
      t_static_pos_ref_A = q->value("static_pos_ref_A").toUInt();
      t_static_pos_ref_B = q->value("static_pos_ref_B").toUInt();
      t_static_pos_ref_C = q->value("static_pos_ref_C").toUInt();
      t_static_pos_ref_D = q->value("static_pos_ref_D").toUInt();
      t_static_DTE = q->value("static_DTE").toUInt();
      t_static_talker_ID = q->value("static_talker_id").toString();
                                 
      t_voyage_destination = q->value("voyage_destination").toString();
      t_voyage_ETA_utc = q->value("voyage_ETA_utc").toTime();
      t_voyage_ETA_day = q->value("voyage_ETA_day").toUInt();
      t_voyage_ETA_month = q->value("voyage_ETA_month").toUInt();
      t_voyage_draft = q->value("voyage_draft").toReal();
      t_voyage_cargo_ITU_id = q->value("voyage_cargo_ITU_id").toUInt();
//      t_voyage_cargo_type_name = q->value("voyage_cargo_type_name").toString();
      t_voyage_team = q->value("voyage_team").toUInt();
                                 
      t_gps_timeout = q->value("gps_timeout").toUInt();
      t_init_random_coordinates = q->value("init_random_coordinates").toBool();
      t_init_random_course = q->value("init_random_course").toBool();
      t_init_random_speed = q->value("init_random_speed").toBool();
      t_init_course_change_ratio = q->value("init_course_change_ratio").toUInt();
      t_init_course_change_segment = q->value("init_course_change_segment").toReal();
      t_init_speed_change_ratio = q->value("init_speed_change_ratio").toUInt();
      t_init_speed_change_segment = q->value("init_speed_change_segment").toReal();
      
    }
    
    q->finish();
    delete q;
    
  }
  
  ui->ediId->setText(showMode == smNew ? "<Новый>" : QString::number(t_vessel_id));
//  ui-> set(t_self = false;
  ui->editCallsign->setText(t_static_callsign);
  ui->editName->setText(t_static_name);
  
  ui->spinIMO->setValue(t_static_imo);
  ui->spinMMSI->setValue(t_static_mmsi);
  
  ui->cbVesselType->setCurrentIndex(ui->cbVesselType->findData(t_static_vessel_ITU_id));
  
  ui->spinPosRefA->setValue(t_static_pos_ref_A);
  ui->spinPosRefB->setValue(t_static_pos_ref_B);
  ui->spinPosRefC->setValue(t_static_pos_ref_C);
  ui->spinPosRefD->setValue(t_static_pos_ref_D);
  ui->cbDTE->setCurrentIndex(t_static_DTE);
  ui->editTalkerID->setText(t_static_talker_ID);
  
  ui->editDestination->setText(t_voyage_destination);
  ui->timeETAutc->setTime(t_voyage_ETA_utc);
  ui->spinETAday->setValue(t_voyage_ETA_day);
  ui->spinETAmonth->setValue(t_voyage_ETA_month);
  ui->dspinDraft->setValue(t_voyage_draft);
      
  ui->cbCargoType->setCurrentIndex(ui->cbCargoType->findData(t_voyage_cargo_ITU_id));
  
  ui->spinTeam->setValue(t_voyage_team);
  
  ui->spinGPSTimeout->setValue(t_gps_timeout);
  
  ui->cbInitCoordinates->setCurrentIndex(ui->cbInitCoordinates->findData(t_init_random_coordinates));
  ui->cbInitCourse->setCurrentIndex(ui->cbInitCourse->findData(t_init_random_course));
  ui->cbInitSpeed->setCurrentIndex(ui->cbInitSpeed->findData(t_init_random_speed));
  
  ui->dspinCourseChangeSegment->setValue(t_init_course_change_ratio);
  ui->dspinCourseChangeSegment->setSuffix(QString(" %1").arg(CMU.DistanceDesign));
  ui->spinCourseChangeRatio->setValue(t_init_course_change_segment);
  ui->spinSpeedChangeRatio->setValue(t_init_speed_change_ratio);
  ui->dspinSpeedChangeSegment->setValue(t_init_speed_change_segment);
  ui->dspinSpeedChangeSegment->setSuffix(QString(" %1").arg(CMU.DistanceDesign));
  
  connect(ui->bnSave, SIGNAL(clicked()), this, SLOT(accept()));
  connect(ui->bnCancel, SIGNAL(clicked()), this, SLOT(reject()));
  
  this->setModal(true);
  this->show();

}

SvVesselEditor::~SvVesselEditor()
{
  delete ui;
}

void SvVesselEditor::loadVesselTypes()
{
  QSqlQuery* q = new QSqlQuery(SQLITE->db);
  if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_VESSEL_TYPES), q).type()) {
    
    q->finish();
    return;
  }
  
  while(q->next())
    ui->cbVesselType->addItem(q->value("type_name").toString(),
                              q->value("ITU_id").toUInt());

  q->finish();
  delete q;
  
  if(ui->cbVesselType->count()) ui->cbVesselType->setCurrentIndex(0);
  
  ui->bnSave->setEnabled(!ui->cbVesselType->currentData().isNull());
  
}

void SvVesselEditor::loadCargoTypes()
{
  QSqlQuery* q = new QSqlQuery(SQLITE->db);
  if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_CARGO_TYPES), q).type()) {
    
    q->finish();
    return;
  }
  
  while(q->next())
    ui->cbCargoType->addItem(q->value("type_name").toString(),
                              q->value("ITU_id").toUInt());

  q->finish();
  delete q;
  
  if(ui->cbCargoType->count()) ui->cbCargoType->setCurrentIndex(0);
  
  ui->bnSave->setEnabled(!ui->cbCargoType->currentData().isNull());
  
}

void SvVesselEditor::loadInitRandoms()
{
  ui->cbInitCoordinates->clear();
  ui->cbInitCoordinates->addItem("Случайно", QVariant(true));
  ui->cbInitCoordinates->addItem("Последние", QVariant(false));
  
  ui->cbInitCourse->clear();
  ui->cbInitCourse->addItem("Случайно", QVariant(true));
  ui->cbInitCourse->addItem("Последние", QVariant(false));
  
  ui->cbInitSpeed->clear();
  ui->cbInitSpeed->addItem("Случайно", QVariant(true));
  ui->cbInitSpeed->addItem("Последние", QVariant(false));
}

void SvVesselEditor::accept()
{

  t_static_callsign = ui->editCallsign->text();
  t_static_name = ui->editName->text();
  t_static_imo = ui->spinIMO->value();
  t_static_mmsi = ui->spinMMSI->value();
  
  t_static_vessel_ITU_id = ui->cbVesselType->currentData().toUInt();
  
  t_static_pos_ref_A = ui->spinPosRefA->value();
  t_static_pos_ref_B = ui->spinPosRefB->value();
  t_static_pos_ref_C = ui->spinPosRefC->value();
  t_static_pos_ref_D = ui->spinPosRefD->value();
  t_static_DTE = ui->cbDTE->currentIndex();
  t_static_talker_ID = ui->editTalkerID->text();
  
  t_voyage_destination = ui->editDestination->text();
  t_voyage_ETA_utc = ui->timeETAutc->time();
  t_voyage_ETA_day = ui->spinETAday->value();
  t_voyage_ETA_month = ui->spinETAmonth->value();
  t_voyage_draft = ui->dspinDraft->value();
      
  t_voyage_cargo_ITU_id = ui->cbCargoType->currentData().toUInt();
  
  t_voyage_team = ui->spinTeam-> value();
  
  t_gps_timeout = ui->spinGPSTimeout->value();
  
  t_init_random_coordinates = ui->cbInitCoordinates->currentData().toBool();
  t_init_random_course = ui->cbInitCourse->currentData().toBool();
  t_init_random_speed = ui->cbInitSpeed->currentData().toBool();
  
  t_init_course_change_ratio = ui->spinCourseChangeRatio->value();
  t_init_course_change_segment = ui->dspinCourseChangeSegment->value();
  t_init_speed_change_ratio = ui->spinSpeedChangeRatio->value();
  t_init_speed_change_segment = ui->dspinSpeedChangeSegment->value();
  
  switch (this->showMode) {
    
    case smNew: {
      
      try {
        _exception.raise("SQLITE->db");
        if(!SQLITE->transaction()) _exception.raise(SQLITE->db.lastError().databaseText());

          QSqlError sql = SQLITE->execSQL(QString(SQL_INSERT_NEW_VESSEL).arg(t_self));
          if(QSqlError::NoError != sql.type()) _exception.raise(sql.databaseText());
         
          sql = SQLITE->execSQL(QString(SQL_INSERT_NEW_AIS)
                                .arg(t_static_mmsi)
                                .arg(t_static_imo)
                                .arg(t_static_vessel_ITU_id)
                                .arg(t_static_callsign)
                                .arg(t_static_name)
                                .arg(t_static_pos_ref_A)
                                .arg(t_static_pos_ref_B)
                                .arg(t_static_pos_ref_C)
                                .arg(t_static_pos_ref_D)
                                .arg(t_static_DTE)
                                .arg(t_static_talker_ID)
                                .arg(t_voyage_destination)
                                .arg(t_voyage_ETA_utc.toString("hh:mm:ss"))
                                .arg(t_voyage_ETA_day)
                                .arg(t_voyage_ETA_month)
                                .arg(t_voyage_draft)
                                .arg(t_voyage_cargo_ITU_id)
                                .arg(t_voyage_team));
                
          if(QSqlError::NoError != sql.type()) _exception.raise(sql.databaseText());
          
          sql = SQLITE->execSQL(QString(SQL_INSERT_NEW_GPS)
                                .arg(t_gps_timeout)
                                .arg(t_init_random_coordinates)
                                .arg(t_init_random_course)
                                .arg(t_init_random_speed)
                                .arg(t_init_course_change_ratio)
                                .arg(t_init_speed_change_ratio)
                                .arg(t_init_course_change_segment)
                                .arg(t_init_speed_change_segment));
                
          if(QSqlError::NoError != sql.type()) _exception.raise(sql.databaseText());
          
          if(!SQLITE->commit()) _exception.raise(SQLITE->db.lastError().databaseText());
          
      }
      
      catch(SvException &e) {
          
        SQLITE->rollback();
        _last_error = e.err;
//        qDebug() << _last_error;
        QDialog::reject();
        
        return;
      }
        
      break;
      
    }

    case smEdit: {
      try {
        
        if(!SQLITE->transaction()) _exception.raise(SQLITE->db.lastError().databaseText());
        
        QSqlError sql = SQLITE->execSQL(QString(SQL_UPDATE_AIS)
                                        .arg(t_static_mmsi)
                                        .arg(t_static_imo)
                                        .arg(t_static_vessel_ITU_id)
                                        .arg(t_static_callsign)
                                        .arg(t_static_name)
                                        .arg(t_static_pos_ref_A)
                                        .arg(t_static_pos_ref_B)
                                        .arg(t_static_pos_ref_C)
                                        .arg(t_static_pos_ref_D)
                                        .arg(t_static_DTE)
                                        .arg(t_static_talker_ID)
                                        .arg(t_voyage_destination)
                                        .arg(t_voyage_ETA_utc.toString("hh:mm:ss"))
                                        .arg(t_voyage_ETA_day)
                                        .arg(t_voyage_ETA_month)
                                        .arg(t_voyage_draft)
                                        .arg(t_voyage_cargo_ITU_id)
                                        .arg(t_voyage_team)
                                        .arg(t_vessel_id));
      
        if(QSqlError::NoError != sql.type()) _exception.raise(sql.databaseText());

        sql = SQLITE->execSQL(QString(SQL_UPDATE_GPS)
                              .arg(t_gps_timeout)
                              .arg(t_init_random_coordinates)
                              .arg(t_init_random_course)
                              .arg(t_init_random_speed)
                              .arg(t_init_course_change_ratio)
                              .arg(t_init_speed_change_ratio)
                              .arg(t_init_course_change_segment)
                              .arg(t_init_speed_change_segment)
                              .arg(t_vessel_id));
              
        if(QSqlError::NoError != sql.type()) _exception.raise(sql.databaseText());
        
        if(!SQLITE->commit()) _exception.raise(SQLITE->db.lastError().databaseText());
        
        
      }
      
      catch(SvException &e) {
          
        SQLITE->rollback();
        _last_error = e.err;
//        qDebug() << _last_error;
        QDialog::reject();
        
        return;
      }
      

      break;
    }
  }

  QDialog::accept();
  
}
