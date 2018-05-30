#include "mainwindow.h"
#include "ui_mainwindow.h"


#define ECHO_MONITOR_WIDTH 500

extern SvSQLITE *SQLITE;
extern SvSerialEditor* SERIALEDITOR_UI;
extern SvNetworkEditor* NETWORKEDITOR_UI;

extern SvVesselEditor* VESSELEDITOR_UI;
extern SvNavtexEditor* NAVTEXEDITOR_UI; 
extern SvNavStatEditor* NAVSTATEDITOR_UI;

extern geo::UnitsInfo CMU;

QMap<int, gps::SvGPS*>* GPSs;
QMap<int, ais::SvAIS*>* AISs;
QMap<int, vsl::SvVessel*>* VESSELs;
QMap<int, QListWidgetItem*>* LISTITEMs;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    qRegisterMetaType<geo::GEOPOSITION>("geo::GEOPOSITION");

    ui->setupUi(this);

    setWindowTitle(QString("Имитатор судового оборудования v.%1").arg(APP_VERSION));
    
    log = svlog::SvLog(ui->textLog);

    QString s = AppParams::loadLayout(this);
    
    if(!s.isEmpty()) {
      
      log << svlog::Error << svlog::Time << s << svlog::endl;
      
      QScreen *screen = QGuiApplication::primaryScreen();
      
      resize(screen->size()); //, screen->size().height() - 455);
      move(0, /*screen->size().width() - 415,*/ 0);
      setWindowState(Qt::WindowMaximized);
      
      ui->dockArea->resize(screen->size().width() - 705, screen->size().height() - 450);
      ui->dockArea->move(0, 0);
      
      ui->dockVesselInfo->resize(250, screen->size().height() - 450);
      ui->dockVesselInfo->move(screen->size().width() - 685, 0); 
      
      ui->dockEcho->resize(screen->size().width() - 705, 335);
      ui->dockEcho->move(0, screen->size().height() - 410);
      
      ui->dockLog->resize(665, 335);
      ui->dockLog->move(screen->size().width() - 685, screen->size().height() - 410);
      
      this->restoreState(QByteArray::fromHex(
                           "000000ff00000000fd000000020000000000000615000002e9fc0200000002fc" \
                           "0000000c000001dc0000006100fffffffc0100000002fb000000100064006f00" \
                           "63006b00410072006500610100000000000004d00000009e00fffffffb000000" \
                           "1c0064006f0063006b00560065007300730065006c0049006e0066006f010000" \
                           "04d4000001410000004b00fffffffb000000100064006f0063006b0045006300" \
                           "68006f01000001ec000001090000004c00ffffff0000000300000780000000dd" \
                           "fc0100000001fb0000000e0064006f0063006b004c006f006701000000000000" \
                           "07800000005000ffffff00000167000002e90000000400000004000000080000" \
                           "0008fc00000001000000020000000100000016006d00610069006e0054006f00" \
                           "6f006c0042006100720100000000ffffffff0000000000000000"));
      
    }
    
    ui->tabWidget->setCurrentIndex(AppParams::readParam(this, "General", "LastTab", 0).toInt());
    ui->tabWidget->currentWidget()->findChild<QToolBox*>()->setCurrentIndex(AppParams::readParam(this, "General", "LastToolBox", 0).toInt());
    

    _db_file_name = AppParams::readParam(this, "General", "db", "rcard.db").toString();
    _depth_file_name = AppParams::readParam(this, "General", "DepthMapImage", "mountain.png").toString();
    
    _bounds.min_lat = AppParams::readParam(this, "BOUNDS", "minlat", 60.0295).toReal();
    _bounds.max_lat = AppParams::readParam(this, "BOUNDS", "maxlat", 60.1603).toReal();
    _bounds.min_lon = AppParams::readParam(this, "BOUNDS", "minlon", 29.0211).toReal();
    _bounds.max_lon = AppParams::readParam(this, "BOUNDS", "maxlon", 29.4598).toReal();
    

    _font_default.setItalic(false);
    _font_inactive.setItalic(true);
    _font_nolink.setItalic(true);
    
    ui->listVessels->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->listVessels->setSelectionMode(QAbstractItemView::SingleSelection);
    
//    qApp->setOverrideCursor(QCursor(Qt::ArrowCursor));

}

bool MainWindow::init()
{
  /** -------- создаем область отображения -------- **/
  _area = new area::SvArea(this);
  ui->vlayMap->addWidget(_area);
  _area->setUp("Baltic sea", _bounds);

  
  /** ------------ монитор эхолота ---------- **/
  ui->customplot->xAxis->setRange(-ECHO_MONITOR_WIDTH, 0);// , Qt::AlignLeft);
  ui->customplot->yAxis2->setRange(-300, 0); //, Qt::AlignBottom);
  ui->customplot->legend->setVisible(false);
  ui->customplot->setBackground(QBrush(QColor("whitesmoke"))); // plotGradient);
  ui->customplot->yAxis->setVisible(false); // addAxes(QCPAxis::atRight);
  ui->customplot->yAxis2->setVisible(true);
  ui->customplot->xAxis->setLabel("Пройденный путь, метры");
  ui->customplot->yAxis2->setLabel("Глубина, метры");
  
  _curve = new QCPGraph(ui->customplot->xAxis, ui->customplot->yAxis2);
  _curve->setAntialiased(true);
  _curve->setPen(QPen(QBrush(QColor("black")), 2));
  QLinearGradient plotGradient;
  plotGradient.setStart(0, 0);
  plotGradient.setFinalStop(0, 255);
  plotGradient.setColorAt(1, QColor("lavender"));
  plotGradient.setColorAt(0, QColor("white"));
  _curve->setBrush(QBrush(plotGradient)); // QColor("whitesmoke")));
  _curve->setLineStyle(QCPGraph::lsLine);


  _scatt = new QCPFinancial(ui->customplot->xAxis, ui->customplot->yAxis2);
  _scatt->setChartStyle(QCPFinancial::csOhlc);
  _scatt->setTwoColored(false);
  _scatt->setPen(QPen(QBrush(QColor("black")), 1, Qt::DotLine));
  _scatt->setBrush(QBrush(QColor("green")));

  _fish = new QCPGraph(ui->customplot->xAxis, ui->customplot->yAxis2);
  _fish->setLineStyle(QCPGraph::lsNone);
  _fish->setScatterStyle(QCPScatterStyle(QPixmap(":/icons/Icons/fish.png"))); // (QCPScatterStyle::ssPixmap, 4));

  for(double i = -ECHO_MONITOR_WIDTH; i < 0; i++) {
    _curve->addData(i, 1);
    _scatt->addData(i, 1, 1, 1, 1);
  }


  /** -------- инициализируем массивы ---------- **/
  GPSs = new QMap<int, gps::SvGPS*>;
  AISs = new QMap<int, ais::SvAIS*>;
  VESSELs = new QMap<int, vsl::SvVessel*>;
  LISTITEMs = new QMap<int, QListWidgetItem*>;
  
  
  
  
  /// частоты для NAVTEX
  ui->cbNAVTEXReceiveFrequency->clear();
  QMap<int, QString> freqs = SvNavtexEditor::frequencies();
  for(int freq_id: freqs.keys())
    ui->cbNAVTEXReceiveFrequency->addItem(freqs.value(freq_id), freq_id);
  
  /// тревога
  ui->cbLAGAlarmState->addItems(QStringList({"Порог превышен", "Порог не превышен"}));
  ui->cbAISAlarmState->addItems(QStringList({"Порог превышен", "Порог не превышен"}));
  ui->cbNAVTEXAlarmState->addItems(QStringList({"Порог превышен", "Порог не превышен"}));
  
  /// типы сообщений LAG
  ui->cbLAGMessageType->clear();
  QMap<lag::MessageType, QString> mtypes = lag::SvLAG::msgtypes();
  for(lag::MessageType mtype: mtypes.keys())
    ui->cbLAGMessageType->addItem(mtypes.value(mtype), mtype);
    
  
  QSqlQuery* q = nullptr;
  
  try {
        
    /** ---------- открываем БД ----------- **/
    if(!QFileInfo(_db_file_name).exists()) _exception.raise(QString("Файл БД не найден: %1").arg(_db_file_name));
    
    SQLITE = new SvSQLITE(this, _db_file_name);
    QSqlError err = SQLITE->connectToDB();
    
    if(err.type() != QSqlError::NoError) _exception.raise(err.databaseText());
    
    
    /** ------ читаем параметры устройств -------- **/
    if(!read_devices_params()) _exception.raise("Ошибка при чтении параметров устройств");
    
      
    /** --------- создаем собственное судно ----------- **/
    if(!createSelfVessel()) _exception.raise("Ошибка при создании собственного судна");
    
    
    /** --------- создаем другие суда ----------- **/
    q = new QSqlQuery(SQLITE->db);
        
    if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_VESSELS_WHERE_SELF).arg(false), q).type())
      _exception.raise(q->lastError().databaseText());
          
    while(q->next())
      if(!createOtherVessel(q)) _exception.raise("Ошибка при создании объектов судов");
    
    q->finish();
    delete q;
    
    /** --------- создаем станцию НАВТЕКС ----------- **/
    if(!createNavtex()) _exception.raise("Ошибка при создании станции НАВТЕКС");
        
  }
  
  catch(SvException& e) {
  
    delete q;
    
    qCritical() << e.err;
    return false;
  }

  connect(_area->scene, SIGNAL(selectionChanged()), this, SLOT(on_areaSelectionChanged()));
  connect(ui->listVessels, &QListWidget::currentItemChanged, this, &MainWindow::currentVesselListItemChanged);
  
  connect(this, &MainWindow::newState, this, &MainWindow::stateChanged);
  
  _timer_x10.setSingleShot(true);
  connect(&_timer_x10, &QTimer::timeout, this, &MainWindow::setX10Emulation);

  
  return true;

}


void MainWindow::on_bnStart_clicked()
{

  switch (_current_state) {
    
    case sStopped:
    {
      stateChanged(sRunning);
      
      try {
        
        /// сохраняем параметры устройств
        save_devices_params();
        
        /** открываем порты устройств **/
        /// LAG
        _self_lag->setSerialPortParams(_lag_serial_params);
        _self_lag->setMessageType(lag::MessageType(ui->cbLAGMessageType->currentData().toInt()));
        
        if(ui->checkLAGEnabled->isChecked()) 
          if(!_self_lag->open()) _exception.raise(QString("ЛАГ: %1").arg(_self_lag->lastError()));

        
        /// AIS
        _self_ais->setSerialPortParams(_ais_serial_params);
        _self_ais->setReceiveRange(ui->dspinAISRadius->value());
        
        if(ui->checkAISEnabled->isChecked())
          if(!_self_ais->open()) _exception.raise(QString("АИС: %1").arg(_self_ais->lastError()));
        
        
        /// NAVTEX
        _navtex->setSerialPortParams(_navtex_serial_params);
        _navtex->setReceiveFrequency(ui->cbNAVTEXReceiveFrequency->currentData().toUInt());
        
        if(ui->checkNAVTEXEnabled->isChecked()) 
          if(!_navtex->open()) _exception.raise(QString("НАВТЕКС: %1").arg(_navtex->lastError()));

        
        /// ECHO MULTI
        _self_multi_echo->setNetworkParams(_echo_multi_network_params);
        _self_multi_echo->setBeamCount(ui->spinECHOMultiBeamCount->value());
        
        if(ui->checkECHOMultiEnabled->isChecked()) 
          if(!_self_multi_echo->open()) _exception.raise(QString("Многолуч. эхолот: %1").arg(_self_multi_echo->lastError()));
          
        
        /// ECHO FISH
        _self_fish_echo->setNetworkParams(_echo_fish_network_params);
        _self_fish_echo->setFishCount(ui->spinECHOFishFishCount->value());
        
        if(ui->checkECHOFishEnabled->isChecked()) 
          if(!_self_fish_echo->open()) _exception.raise(QString("Рыбопром. эхолот: %1").arg(_self_fish_echo->lastError()));
        
        /// GPS Network Interface
        _self_gps_ifc->setNetworkParams(_gps_network_params);
        if(ui->checkGPSEnabled->isChecked()) 
          if(!_self_gps_ifc->open()) _exception.raise(QString("GPS: %1").arg(_self_gps_ifc->lastError()));
        
        
      }
      
      catch(SvException &e) {
        
        _self_ais->close();
        _self_lag->close();
        _navtex->close();
        _self_multi_echo->close();
        _self_fish_echo->close();
        
        emit newState(sStopped);
        
        log << svlog::Critical << svlog::Time
            << QString("Ошибка открытия порта.\n%1").arg(e.err)
            << svlog::endl;
        
        return;
      }
      
      emit startGPSEmulation(0);
      
      emit startAISEmulation(0);
      
      emit startLAGEmulation(ui->spinLAGUploadInterval->value());
      
      emit startNAVTEXEmulation(ui->spinNAVTEXUploadInterval->value());
      
      emit startECHOMultiEmulation(ui->spinECHOMultiUploadClearance->value());
      
      emit startECHOFishEmulation(ui->spinECHOFishUploadClearance->value());
      
      stateChanged(sRunned);
      
      break;
    }
      
    case sRunned:
    {
      stateChanged(sStopping);
      
      emit stopECHOMultiEmulation();
      
      emit stopECHOFishEmulation();
      
      emit stopNAVTEXEmulation();
      
      emit stopLAGEmulation();
      
      emit stopAISEmulation();
      
      emit stopGPSEmulation();
      
      /// сохраняем последние геопозиции 
      foreach (gps::SvGPS* g, GPSs->values()) {
        
        g->waitWhileRunned();
        updateGPSInitParams(g);
        
      }
      
      /** закрываем порты **/
      _self_lag->close();
      _self_ais->close();
      _navtex->close();
      _self_multi_echo->close();
      _self_fish_echo->close();
      
      emit setMultiplier(1);
      
      stateChanged(sStopped);
          
      break;
    }
      
    default:
      break;
  }
  
}


MainWindow::~MainWindow()
{
  if(_current_state == sRunned)
    on_bnStart_clicked();
  
  while(_current_state != sStopped)
    qApp->processEvents();
  
  save_devices_params();
  
  for(int id: VESSELs->keys()) delete VESSELs->value(id);
  
  for(int id: LISTITEMs->keys()) delete LISTITEMs->value(id);

  delete AISs;
  delete GPSs;
  delete VESSELs;
  delete _navtex;
  delete _self_gps_ifc;

  
  QString s = AppParams::saveLayout(this);
  
//  AppParams::saveWindowParams(this, this->size(), this->pos(), this->windowState());
//    AppParams::saveWindowParams(ui->dockGraphics, ui->dockGraphics->size(), ui->dockGraphics->pos(), ui->dockGraphics->windowState(), "AREA WINDOW");
  //  AppParams::saveWindowParams(ui->dockCarrentInfo, ui->dockCarrentInfo->size(), ui->dockCarrentInfo->pos(), ui->dockCarrentInfo->windowState(), "INFO WINDOW");
  AppParams::saveParam(this, "General", "LastTab", ui->tabWidget->currentIndex());
  AppParams::saveParam(this, "General", "LastToolBox", ui->tabWidget->currentWidget()->findChild<QToolBox*>()->currentIndex());
  //  AppParams::saveParam(this, "GENERAL", "AISRadius", QVariant(ui->dspinAISRadius->value()));

  delete ui;
  
}

bool MainWindow::read_devices_params()
{
  QSqlQuery* q = new QSqlQuery(SQLITE->db);
  
  try {
    
    if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_FROM_DEVICES_PARAMS), q).type()) 
      _exception.raise(q->lastError().databaseText());
    
    while(q->next()) {    
      
      idev::SvSimulatedDeviceTypes dt = idev::SvSimulatedDeviceTypes(q->value("device_type").toUInt());
      
      switch (dt) {
        case idev::sdtLAG: {
          
          _lag_serial_params.name =         q->value("port_name").toString();        
          _lag_serial_params.description =  q->value("description").toString();
          _lag_serial_params.baudrate =     q->value("baudrate").toInt();                              
          _lag_serial_params.databits =     QSerialPort::DataBits(q->value("data_bits").toInt());      
          _lag_serial_params.flowcontrol =  QSerialPort::FlowControl(q->value("flow_control").toInt());
          _lag_serial_params.parity =       QSerialPort::Parity(q->value("parity").toInt());           
          _lag_serial_params.stopbits =     QSerialPort::StopBits(q->value("stop_bits").toInt());  
          ui->editLAGSerialInterface->setText(_lag_serial_params.description);
          
          ui->checkLAGEnabled->setChecked(q->value("is_active").toBool());
          
          ui->spinLAGUploadInterval->setValue(q->value("upload_interval").toUInt());
          
          ui->spinLAGAlarmId->setValue(q->value("alarm_id").toUInt());
          ui->cbLAGAlarmState->setCurrentIndex(q->value("alarm_state").toUInt());
          ui->editLAGAlarmMessageText->setText(q->value("alarm_text").toString());
                
          QVariant args = parse_args(q->value("args").toString(), ARG_LAG_MSGTYPE);
          quint32 msgtype = args.canConvert<quint32>() ? args.toUInt() : 0;
          ui->cbLAGMessageType->setCurrentIndex(ui->cbLAGMessageType->findData(msgtype < ui->cbLAGMessageType->count() ? msgtype : lag::lmtVBW));
          
          break;
        }
          
        case idev::sdtSelfAIS: {
          
          _ais_serial_params.name =         q->value("port_name").toString();
          _ais_serial_params.description =  q->value("description").toString();
          _ais_serial_params.baudrate =     q->value("baudrate").toInt();
          _ais_serial_params.databits =     QSerialPort::DataBits(q->value("data_bits").toInt());
          _ais_serial_params.flowcontrol =  QSerialPort::FlowControl(q->value("flow_control").toInt());
          _ais_serial_params.parity =       QSerialPort::Parity(q->value("parity").toInt());
          _ais_serial_params.stopbits =     QSerialPort::StopBits(q->value("stop_bits").toInt());
          ui->editAISSerialInterface->setText(_ais_serial_params.description);
          
          ui->checkAISEnabled->setChecked(q->value("is_active").toBool());
          
          ui->spinAISAlarmId->setValue(q->value("alarm_id").toUInt());
          ui->cbAISAlarmState->setCurrentIndex(q->value("alarm_state").toUInt());
          ui->editAISAlarmMessageText->setText(q->value("alarm_text").toString());
          
          QVariant args = parse_args(q->value("args").toString(), ARG_AIS_RECEIVERANGE);
          ui->dspinAISRadius->setValue(args.canConvert<qreal>() ? args.toReal() : 50);
          ui->dspinAISRadius->setSuffix(CMU.DistanceDesign);
          
          break;
        }
          
        case idev::sdtNavtex: {
           
          _navtex_serial_params.name =        q->value("port_name").toString(); 
          _navtex_serial_params.description = q->value("description").toString();
          _navtex_serial_params.baudrate =    q->value("baudrate").toInt();                              
          _navtex_serial_params.databits =    QSerialPort::DataBits(q->value("data_bits").toInt());      
          _navtex_serial_params.flowcontrol = QSerialPort::FlowControl(q->value("flow_control").toInt());
          _navtex_serial_params.parity =      QSerialPort::Parity(q->value("parity").toInt());           
          _navtex_serial_params.stopbits =    QSerialPort::StopBits(q->value("stop_bits").toInt());      
          ui->editNAVTEXSerialInterface->setText(_navtex_serial_params.description);
          
          ui->checkNAVTEXEnabled->setChecked(q->value("is_active").toBool());
          ui->spinNAVTEXUploadInterval->setValue(q->value("upload_interval").toUInt());
          
          ui->spinNAVTEXAlarmId->setValue(q->value("alarm_id").toUInt());
          ui->cbNAVTEXAlarmState->setCurrentIndex(q->value("alarm_state").toUInt());
          ui->editNAVTEXAlarmMessageText->setText(q->value("alarm_text").toString());
                  
          QVariant args = parse_args(q->value("args").toString(), ARG_NAV_RECV_FREQ);
          int indx = ui->cbNAVTEXReceiveFrequency->findData(args.canConvert<quint32>() ? args.toUInt() : 1);
          ui->cbNAVTEXReceiveFrequency->setCurrentIndex(indx < ui->cbNAVTEXReceiveFrequency->count() ? indx : 1);
          
          break;
        }
          
        case idev::sdtEchoMulti: {
          
          QVariant args;
          
          _echo_multi_network_params.ifc = q->value("network_interface").toUInt();
          _echo_multi_network_params.protocol = q->value("network_protocol").toUInt();
          _echo_multi_network_params.ip = q->value("network_ip").toUInt();
          _echo_multi_network_params.port = q->value("network_port").toUInt();
          _echo_multi_network_params.description = q->value("description").toString();
          _echo_multi_network_params.translate_type = QHostAddress::SpecialAddress(q->value("network_translation_type").toInt());
          
          ui->editECHOMultiInterface->setText(_echo_multi_network_params.description);
          ui->checkECHOMultiEnabled->setChecked(q->value("is_active").toBool());
          ui->spinECHOMultiUploadClearance->setValue(q->value("upload_interval").toUInt());
          
          args = parse_args(q->value("args").toString(), ARG_ECHO_BEAM_COUNT);
          ui->spinECHOMultiBeamCount->setValue(args.canConvert<uint>() ? args.toUInt() : 28);
          
          break;
        }
          
        case idev::sdtEchoFish: {
          
          QVariant args;
          
          _echo_fish_network_params.ifc = q->value("network_interface").toUInt();
          _echo_fish_network_params.protocol = q->value("network_protocol").toUInt();
          _echo_fish_network_params.ip = q->value("network_ip").toUInt();
          _echo_fish_network_params.port = q->value("network_port").toUInt();
          _echo_fish_network_params.description = q->value("description").toString();
          _echo_fish_network_params.translate_type = QHostAddress::SpecialAddress(q->value("network_translation_type").toInt());

          ui->editECHOFishInterface->setText(_echo_fish_network_params.description);
          ui->checkECHOFishEnabled->setChecked(q->value("is_active").toBool());
          ui->spinECHOFishUploadClearance->setValue(q->value("upload_interval").toUInt());
          
          args = parse_args(q->value("args").toString(), ARG_ECHO_FISH_COUNT);
          ui->spinECHOFishFishCount->setValue(args.canConvert<uint>() ? args.toUInt() : 5);
    
          break;
        }
          
        case idev::sdtGPS: {
          
          _gps_network_params.ifc = q->value("network_interface").toUInt();
          _gps_network_params.protocol = q->value("network_protocol").toUInt();
          _gps_network_params.ip = q->value("network_ip").toUInt();
          _gps_network_params.port = q->value("network_port").toUInt();
          _gps_network_params.description = q->value("description").toString();
          _gps_network_params.translate_type = QHostAddress::SpecialAddress(q->value("network_translation_type").toInt());

          ui->editGPSInterface->setText(_gps_network_params.description);
          ui->checkGPSEnabled->setChecked(q->value("is_active").toBool());
    
          break;
        }
          
        default:
          break;
      }
      
    }
    
    q->finish();
    delete q;
    
    return true;
    
  }
  
  catch(SvException& e) {
    
    if(q) q->finish();
    delete q;
    
    qCritical() << e.err;
    return false;
  }
  
}

void MainWindow::save_devices_params()
{
  QSqlError err;
  
  /// ----------- LAG ------------- ///
  try {

    err = check_params_exists(idev::sdtLAG);
    
    if(err.type() != QSqlError::NoError) _exception.raise(err.databaseText());
    
    err = SQLITE->execSQL(QString(SQL_UPDATE_DEVICES_PARAMS_WHERE)
                          .arg(ui->checkLAGEnabled->isChecked())
                          .arg(ui->spinLAGUploadInterval->value())
                          .arg(QString("-%1 %2").arg(ARG_LAG_MSGTYPE).arg(ui->cbLAGMessageType->currentIndex()))
                          .arg(ui->spinLAGAlarmId->value())
                          .arg(ui->cbLAGAlarmState->currentIndex())
                          .arg(ui->editLAGAlarmMessageText->text())
                          .arg(idev::sdtLAG));
    
    if(QSqlError::NoError != err.type()) _exception.raise(err.databaseText());
    
  }
  
  catch(SvException e) {
    log << svlog::Critical << svlog::Time << QString("Ошибка при обновлении параметров ЛАГ:\n%1").arg(e.err) << svlog::endl;
  }
   
  /// ----------- AIS ------------- ///
  try {
    
    err = check_params_exists(idev::sdtSelfAIS);
    
    if(err.type() != QSqlError::NoError) _exception.raise(err.databaseText());
    
    err = SQLITE->execSQL(QString(SQL_UPDATE_DEVICES_PARAMS_WHERE)
                          .arg(ui->checkAISEnabled->isChecked())
                          .arg(1000)
                          .arg(QString("-%1 %2").arg(ARG_AIS_RECEIVERANGE).arg(ui->dspinAISRadius->value(), 0, 'f', 1))
                          .arg(ui->spinAISAlarmId->value())
                          .arg(ui->cbAISAlarmState->currentIndex())
                          .arg(ui->editAISAlarmMessageText->text())
                          .arg(idev::sdtSelfAIS));
    
    if(QSqlError::NoError != err.type()) _exception.raise(err.databaseText());
   
  }
  
  catch(SvException e) {
    log << svlog::Critical << svlog::Time << QString("Ошибка при обновлении параметров АИС:\n%1").arg(e.err) << svlog::endl;
  }
    
  /// ----------- NAVTEX ------------- ///
  try {
    
    err = check_params_exists(idev::sdtNavtex);
    
    if(err.type() != QSqlError::NoError) _exception.raise(err.databaseText());
    
    err = SQLITE->execSQL(QString(SQL_UPDATE_DEVICES_PARAMS_WHERE)
                          .arg(ui->checkNAVTEXEnabled->isChecked())
                          .arg(ui->spinNAVTEXUploadInterval->value())
                          .arg(QString("-%1 %2").arg(ARG_NAV_RECV_FREQ).arg(ui->cbNAVTEXReceiveFrequency->currentData().toUInt()))
                          .arg(ui->spinNAVTEXAlarmId->value())
                          .arg(ui->cbNAVTEXAlarmState->currentIndex())
                          .arg(ui->editNAVTEXAlarmMessageText->text())
                          .arg(idev::sdtNavtex));
    
    if(QSqlError::NoError != err.type()) _exception.raise(err.databaseText());
    
  
  }
  
  catch(SvException e) {
    log << svlog::Critical << svlog::Time << QString("Ошибка при обновлении параметров НАВТЕКС:\n%1").arg(e.err) << svlog::endl;
  }
  
  /// ----------- ECHO MULTI ------------- ///
  try {
    
    err = check_params_exists(idev::sdtEchoMulti);
    if(err.type() != QSqlError::NoError) _exception.raise(err.databaseText());
    
    err = SQLITE->execSQL(QString(SQL_UPDATE_DEVICES_PARAMS_WHERE)
                          .arg(ui->checkECHOMultiEnabled->isChecked())
                          .arg(ui->spinECHOMultiUploadClearance->value())
                          .arg(QString("-%1 %2").arg(ARG_ECHO_BEAM_COUNT).arg(ui->spinECHOMultiBeamCount->value()))
                          .arg(0)
                          .arg(0)
                          .arg("")
                          .arg(idev::sdtEchoMulti));
    
    if(QSqlError::NoError != err.type()) _exception.raise(err.databaseText());
    
  
  }
  
  catch(SvException e) {
    log << svlog::Critical << svlog::Time << QString("Ошибка при обновлении параметров многолуч. холота:\n%1").arg(e.err) << svlog::endl;
  }
  
  /// ----------- ECHO FISH ------------- ///
  try {
    
    err = check_params_exists(idev::sdtEchoFish);
    if(err.type() != QSqlError::NoError) _exception.raise(err.databaseText());
    
    err = SQLITE->execSQL(QString(SQL_UPDATE_DEVICES_PARAMS_WHERE)
                          .arg(ui->checkECHOFishEnabled->isChecked())
                          .arg(ui->spinECHOFishUploadClearance->value())
                          .arg(QString("-%1 %2").arg(ARG_ECHO_FISH_COUNT).arg(ui->spinECHOFishFishCount->value()))
                          .arg(0)
                          .arg(0)
                          .arg("")
                          .arg(idev::sdtEchoFish));
    
    if(QSqlError::NoError != err.type()) _exception.raise(err.databaseText());
    
  
  }
  
  catch(SvException e) {
    log << svlog::Critical << svlog::Time << QString("Ошибка при обновлении параметров рыбопром. эхолота:\n%1").arg(e.err) << svlog::endl;
  }
    
}

QVariant MainWindow::parse_args(QString args, QString arg)
{
  //! обязателен первый аргумент!! парсер считает, что там находится путь к программе
  QStringList arg_list;
  arg_list << "dumb_path_to_app" << args.split(" ");
  
  QCommandLineParser parser;
  parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
  
  parser.addOption(QCommandLineOption(ARG_LAG_MSGTYPE, "Тип сообщения для LAG", "0", "0"));
  parser.addOption(QCommandLineOption(ARG_AIS_RECEIVERANGE, "Дальность приема для AIS в км", "50", "50"));
  parser.addOption(QCommandLineOption(ARG_NAV_RECV_FREQ, "Частота приемника NAVTEX", "1", "1"));
  parser.addOption(QCommandLineOption(ARG_ECHO_BEAM_COUNT, "Кол-во излучателей для эхолота", "10", "10"));
  parser.addOption(QCommandLineOption(ARG_ECHO_FISH_COUNT, "Кол-во рыбы для эхолота", "5", "5"));
  
  QVariant result = QVariant();
  if (parser.parse(arg_list)) {
    
    result = QVariant::fromValue(parser.value(arg));
    
  }
  
  return result;
}


bool MainWindow::createSelfVessel()
{  
  /*! _area должна уже быть проинициализирована !! */
  
  QSqlQuery* q = nullptr;
      
  try {
    
    /** ------ читаем информацию о собственном судне --------- **/
    q = new QSqlQuery(SQLITE->db);
    
    if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_VESSELS_WHERE_SELF).arg(true), q).type())
      _exception.raise(q->lastError().databaseText());
    
    if(!q->next())
      _exception.raise("В БД нет сведений о собственном судне");

    // читаем информацию из БД  
    int vessel_id = q->value("id").toUInt();
    QDateTime last_update = q->value("gps_last_update").toDateTime(); // для нормальной генерации случайных чисел
    
    ais::aisStaticVoyageData static_voyage_data = readAISStaticVoyageData(q); 
    ais::aisDynamicData dynamic_data = readAISDynamicData(q);
    gps::gpsInitParams gps_params = readGPSInitParams(q, dynamic_data, last_update);
    ais::aisNavStat nav_stat = readNavStat(q);
    
    q->finish();
    delete q;
    
    /** ----- создаем устройства ------ **/
    // GPS
    _self_gps = new gps::SvGPS(vessel_id, gps_params, _bounds, last_update); 
    GPSs->insert(vessel_id, _self_gps);
    
    _self_gps_ifc = new gps::SvGPSNetworkInterface(vessel_id, log);
    
    // АИС
    _self_ais = new ais::SvSelfAIS(vessel_id, static_voyage_data, dynamic_data, log, last_update);
    AISs->insert(vessel_id, _self_ais);
    _self_ais->setNavStatus(nav_stat);
    
    // LAG
    _self_lag = new lag::SvLAG(vessel_id, dynamic_data.geoposition, log);
    
    // эхолоты
    _self_multi_echo = new ech::SvECHOMulti(vessel_id, dynamic_data.geoposition, _bounds, _depth_file_name, log);
    _self_fish_echo = new ech::SvECHOFish(vessel_id, dynamic_data.geoposition, _bounds, _depth_file_name, log);
    
    /** --------- создаем объект собственного судна -------------- **/
    _self_vessel = new vsl::SvVessel(this, vessel_id);
    VESSELs->insert(vessel_id, _self_vessel);
    
    _self_vessel->mountGPS(_self_gps);
    _self_vessel->mountAIS(_self_ais);
    _self_vessel->mountLAG(_self_lag);
    _self_vessel->mountECHOMulti(_self_multi_echo);
    _self_vessel->mountECHOFish(_self_fish_echo);
    
    _self_vessel->assignMapObject(new SvMapObjectSelfVessel(_area, vessel_id));
       
    _area->scene->addMapObject(_self_vessel->mapObject());
    _self_vessel->mapObject()->setVisible(true);
    _self_vessel->mapObject()->setZValue(1);
    
    // подключаем
    connect(_self_gps, &gps::SvGPS::newGPSData, _self_ais, &ais::SvSelfAIS::newGPSData);
    connect(_self_gps, &gps::SvGPS::newGPSData, _self_lag, &lag::SvLAG::newGPSData);
    connect(_self_gps, &gps::SvGPS::newGPSData, _self_gps_ifc, &gps::SvGPSNetworkInterface::newGPSData);
    
    connect(_self_gps, &gps::SvGPS::passed1m, _self_multi_echo, &ech::SvECHOMulti::passed1m);
    connect(_self_gps, &gps::SvGPS::passed1m, _self_fish_echo, &ech::SvECHOFish::passed1m);
    
    connect(_self_ais, &ais::SvSelfAIS::updateSelfVessel, _self_vessel, &vsl::SvVessel::updateVessel);
    connect(_self_ais, &ais::SvSelfAIS::updateVesselById, this, &MainWindow::on_updateVesselById);
    connect(_self_vessel, &vsl::SvVessel::updateMapObjectPos, _area->scene, area::SvAreaScene::setMapObjectPos);
    connect(_self_vessel, &vsl::SvVessel::updateMapObjectPos, this, &on_updateMapObjectInfo);
    
//    connect(_self_multi_echo, &ech::SvECHOMulti::beamsUpdated, this, &MainWindow::on_echoBeamsUpdated);
    connect(_self_fish_echo, &ech::SvECHOFish::updated, this, &MainWindow::on_echoBeamsUpdated);
      
    connect(this, &MainWindow::setMultiplier, _self_gps, &gps::SvGPS::set_multiplier);
    
    connect(this, &MainWindow::startGPSEmulation, _self_gps, &gps::SvGPS::start);
    connect(this, &MainWindow::stopGPSEmulation, _self_gps, &gps::SvGPS::stop);
    connect(this, &MainWindow::startGPSEmulation, _self_gps_ifc, &gps::SvGPSNetworkInterface::start);
    connect(this, &MainWindow::stopGPSEmulation, _self_gps_ifc, &gps::SvGPSNetworkInterface::stop);
    
    connect(this, &MainWindow::startLAGEmulation, _self_lag, &lag::SvLAG::start);
    connect(this, &MainWindow::stopLAGEmulation, _self_lag, &lag::SvLAG::stop);
    
    connect(this, &MainWindow::startECHOMultiEmulation, _self_multi_echo, &ech::SvECHOMulti::start);
    connect(this, &MainWindow::stopECHOMultiEmulation, _self_multi_echo, &ech::SvECHOMulti::stop);
    
    connect(this, &MainWindow::startECHOFishEmulation, _self_fish_echo, &ech::SvECHOFish::start);
    connect(this, &MainWindow::stopECHOFishEmulation, _self_fish_echo, &ech::SvECHOFish::stop);
    
    connect(&_self_vessel->mapObject()->signalHandler, &SvSignalHandler::mouseDoubleClick, this, &MainWindow::editVessel);

    connect(this, &MainWindow::new_lag_message_type, _self_lag, &lag::SvLAG::setMessageType);
    
    LISTITEMs->insert(vessel_id, new QListWidgetItem(QIcon(":/icons/Icons/lock.png"), QString("%1\t%2 [Собственный]").arg(vessel_id).arg(static_voyage_data.name)));
    ui->listVessels->addItem(LISTITEMs->value(vessel_id));
       
    _self_vessel->updateVessel();
    
    return true;
    
  }
  
  catch(SvException &e) {
    
    if(q) q->finish();
    delete q;
    
    qCritical() << e.err;
    return false;
    
  }
  
}

vsl::SvVessel* MainWindow::createOtherVessel(QSqlQuery* q)
{  
  /*! _area должна уже быть проинициализирована !! */
  
  gps::SvGPS* newGPS = nullptr;
  ais::SvOtherAIS* newAIS = nullptr;
  vsl::SvVessel* newVessel = nullptr;
  
  try {
    
    // читаем информацию из БД  
    int vessel_id = q->value("id").toUInt();
    bool is_active = q->value("is_active").toBool();
    QDateTime last_update = q->value("gps_last_update").toDateTime(); // для нормальной генерации случайных чисел
    
    ais::aisStaticVoyageData static_voyage_data = readAISStaticVoyageData(q); 
    ais::aisDynamicData dynamic_data = readAISDynamicData(q);
    gps::gpsInitParams gps_params = readGPSInitParams(q, dynamic_data, last_update);
    ais::aisNavStat nav_stat = readNavStat(q);                     
         
    /** ----- создаем устройства ------ **/
    // GPS
    newGPS = new gps::SvGPS(vessel_id, gps_params, _bounds, last_update);
    GPSs->insert(vessel_id, newGPS);
    
    // АИС
    newAIS = new ais::SvOtherAIS(vessel_id, static_voyage_data, dynamic_data, last_update);
    AISs->insert(vessel_id, newAIS);
    newAIS->setNavStatus(nav_stat);
    
    
    /** --------- создаем объект судна -------------- **/
    newVessel = new vsl::SvVessel(this, vessel_id);
    VESSELs->insert(vessel_id, newVessel);
    
    newVessel->mountGPS(newGPS);
    newVessel->mountAIS(newAIS);
    
    newVessel->assignMapObject(new SvMapObjectOtherVessel(_area, vessel_id));
    
    newVessel->setActive(is_active);
      
    _area->scene->addMapObject(newVessel->mapObject());
    newVessel->mapObject()->setVisible(true);
    newVessel->mapObject()->setZValue(1);
    newVessel->mapObject()->setIdentifier(new SvMapObjectIdentifier(_area, newVessel->mapObject()));
    _area->scene->addMapObject(newVessel->mapObject()->identifier());
    newVessel->mapObject()->identifier()->setVisible(true);
    newVessel->mapObject()->identifier()->setZValue(1);
    newVessel->mapObject()->setActive(is_active);
        
    
    // подключаем
    connect(newGPS, SIGNAL(newGPSData(const geo::GEOPOSITION&)), newAIS, SLOT(newGPSData(const geo::GEOPOSITION&)));
    
    connect(newAIS, &ais::SvOtherAIS::broadcast_message, _self_ais, &ais::SvSelfAIS::on_receive_message);
    
    connect(newVessel, &vsl::SvVessel::updateMapObjectPos, _area->scene, area::SvAreaScene::setMapObjectPos);
    connect(newVessel, &vsl::SvVessel::updateMapObjectPos, this, &on_updateMapObjectInfo);
    
    connect(_self_ais, &ais::SvSelfAIS::interrogateRequest, newAIS, &ais::SvOtherAIS::on_interrogate);
    
    connect(this, &MainWindow::setMultiplier, newGPS, &gps::SvGPS::set_multiplier);
    
    connect(this, &MainWindow::startGPSEmulation, newGPS, &gps::SvGPS::start);
    connect(this, &MainWindow::stopGPSEmulation, newGPS, &gps::SvGPS::stop);
    
    connect(this, &MainWindow::startAISEmulation, newAIS, &ais::SvOtherAIS::start);
    connect(this, &MainWindow::stopAISEmulation, newAIS, &ais::SvOtherAIS::stop);
    
    connect(&newVessel->mapObject()->signalHandler, &SvSignalHandler::mouseDoubleClick, this, &MainWindow::editVessel);

    
    LISTITEMs->insert(vessel_id, new QListWidgetItem(QIcon(), QString("%1\t%2").arg(vessel_id).arg(static_voyage_data.name)));
    ui->listVessels->addItem(LISTITEMs->value(vessel_id));
    
    LISTITEMs->value(vessel_id)->setTextColor(newVessel->isActive() ? QColor(DEFAULT_VESSEL_PEN_COLOR) : QColor(INACTIVE_VESSEL_COLOR));
    LISTITEMs->value(vessel_id)->setFont(newVessel->isActive() ? _font_default : _font_inactive);
    LISTITEMs->value(vessel_id)->setIcon(newVessel->isActive() ? QIcon(":/icons/Icons/bullet_white.png") : QIcon(":/icons/Icons/bullet_red.png"));
    
    newVessel->updateVessel();
    
    return newVessel;
    
  }
  
  catch(SvException& e) {
    
    if(newGPS) delete newGPS;
    if(newAIS ) delete newAIS ;    
    if(newVessel) delete newVessel;
    
    qCritical() << e.err;
    return nullptr;
    
  }
  
}

bool MainWindow::createNavtex()
{  
  QSqlQuery* q = nullptr;
  
  try {
    
    q = new QSqlQuery(SQLITE->db);
    
    if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_NAVTEX), q).type())
      _exception.raise(q->lastError().databaseText());
    
    if(!q->next()) _exception.raise("В БД нет сведений о станции NAVTEX");
    
    
    /** ----- создаем устройство ------ **/
    int id = q->value("id").toUInt();
    _navtex = new nav::SvNAVTEX(log, id);
    
    /** ------ читаем данные станции НАВТЕКС --------- **/
    nav::navtexData ndata;
    ndata.region_id = q->value("station_region_id").toUInt();
    ndata.message_id = q->value("message_id").toUInt();
    ndata.message_designation = q->value("message_designation").toString();
    ndata.region_station_name = q->value("region_station_name").toString();
    ndata.message_text = q->value("message_text").toString();
//    ndata.region_country = q->value("region_country").toString();
    ndata.message_letter_id = q->value("message_letter_id").toString();
    ndata.region_letter_id = q->value("region_letter_id").toString();
    ndata.message_last_number = q->value("message_last_number").toUInt();
    ndata.transmit_frequency_id = q->value("transmit_frequency").toUInt();
    ndata.transmit_frequency = SvNavtexEditor::frequencies().value(ndata.transmit_frequency_id);
    
    q->finish();
    delete q;
    
    _navtex->setData(ndata);
    
    update_NAVTEX_data();
    
    connect(this, &MainWindow::startNAVTEXEmulation, _navtex, &nav::SvNAVTEX::start);
    connect(this, &MainWindow::stopNAVTEXEmulation, _navtex, &nav::SvNAVTEX::stop);
    
    return true;
    
  }
  
  catch(SvException& e) {
    
    if(q) q->finish();
    delete q;
    
    qCritical() << e.err;
    return false;
    
  }
}


void MainWindow::on_areaSelectionChanged()
{
  /// ищем все выделения и удаляем их
  for (SvMapObject* item: _area->scene->mapObjects()) {
    if(item->selection()) {  
      _area->scene->removeMapObject(item->selection());
      item->deleteSelection();
    }
  }

  /// если ничего не  выделено, то сбрасываем
  if(_area->scene->selectedItems().isEmpty()) {
    _selected_vessel_id = -1;
//    _area->setLabelInfo("");
  }
  else {
    /// создаем новое выделение
    SvMapObject* mo = (SvMapObject*)(_area->scene->selectedItems().first());
    mo->setSelection(new SvMapObjectSelection(_area, mo));
    _area->scene->addMapObject(mo->selection());
    mo->selection()->setVisible(true);
    _area->scene->setMapObjectPos(mo, mo->geoPosition());
    _selected_vessel_id = mo->id();
    on_updateMapObjectInfo(mo);
  }

  /// выделяем судно в списке  
  disconnect(ui->listVessels, &QListWidget::currentItemChanged, this, &MainWindow::currentVesselListItemChanged);
    
  ui->listVessels->setCurrentItem(LISTITEMs->value(_selected_vessel_id));
  
  bool b = (_current_state == sStopped) && (ui->listVessels->currentRow() > -1);
  ui->actionEditVessel->setEnabled(b); 
  ui->actionRemoveVessel->setEnabled(b && (_selected_vessel_id != _self_vessel->id)); 
  ui->bnRemoveVessel->setEnabled(b && (_selected_vessel_id != _self_vessel->id));
  ui->bnEditVessel->setEnabled(b);
  ui->bnSetActive->setEnabled(b && (_selected_vessel_id != _self_vessel->id));
  ui->bnVesselNavState->setEnabled(ui->listVessels->currentRow() > -1);
  
  on_updateVesselActive(_selected_vessel_id);
  
  connect(ui->listVessels, &QListWidget::currentItemChanged, this, &MainWindow::currentVesselListItemChanged);
  
}

void MainWindow::currentVesselListItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
  Q_UNUSED(previous);
  
  disconnect(_area->scene, SIGNAL(selectionChanged()), this, SLOT(on_areaSelectionChanged()));
  foreach (SvMapObject* obj, _area->scene->mapObjects()) 
    obj->setSelected(obj->id() == LISTITEMs->key(current)); 
  
  on_areaSelectionChanged();
  
  connect(_area->scene, SIGNAL(selectionChanged()), this, SLOT(on_areaSelectionChanged()));
  
}

void MainWindow::on_listVessels_doubleClicked(const QModelIndex &index)
{
  switch (_current_state) {
    case sStopped:
      
      editVesselNavStat(LISTITEMs->key(ui->listVessels->item(index.row())));
      break;
      
    case sRunned: {
      
      SvMapObject* selobj = nullptr;
      foreach (SvMapObject* obj, _area->scene->mapObjects()) {
        if(obj->id() != LISTITEMs->key(ui->listVessels->item(index.row())))
          continue;
        
        selobj = obj;
        break;
      }
      
      if(selobj)      
        _area->centerSelected();
      
      editVesselNavStat(LISTITEMs->key(ui->listVessels->item(index.row())));
      
      break;
      
    }
  }
}

void MainWindow::editVessel(int id)
{
  if(_current_state != sStopped)
    return;
  
  VESSELEDITOR_UI = new SvVesselEditor(0, id, &VESSELs->value(id)->ais()->dynamicData()->geoposition);
  if(VESSELEDITOR_UI->exec() != QDialog::Accepted) {
    
    if(!VESSELEDITOR_UI->last_error().isEmpty())
      QMessageBox::critical(this, "Ошибка", QString("Ошибка при редактировании записи:\n%1").arg(VESSELEDITOR_UI->last_error()), QMessageBox::Ok);
    
    delete VESSELEDITOR_UI;
    
    return;
  }
  
  geo::GEOPOSITION g = VESSELEDITOR_UI->t_geopos;
  
  delete VESSELEDITOR_UI;
  
  /** ------ читаем информацию о судне --------- **/
  QSqlQuery* q = new QSqlQuery(SQLITE->db);
  
  try {
    
    if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_VESSEL_WHERE_ID).arg(id), q).type())
      _exception.raise(q->lastError().databaseText());
      
    else if(!q->next()) _exception.raise("Неизвестная ошибка");
    
  }
    
  catch(SvException& e) {
    
    if(q) q->finish();
    delete q;
    
    log << svlog::Time << svlog::Critical << e.err << svlog::endl;
    return;
    
  }
      
  /** --------- изменяем данные судна ----------- **/
  QDateTime last_update = q->value("gps_last_update").toDateTime();
  ais::aisStaticVoyageData static_voyage_data = readAISStaticVoyageData(q); 
  
  VESSELs->value(id)->ais()->setCourse(g.course);
  VESSELs->value(id)->ais()->setSpeed(g.speed);
  ais::aisDynamicData *dynamic_data = VESSELs->value(id)->ais()->dynamicData();
  
  gps::gpsInitParams gps_params = readGPSInitParams(q, *dynamic_data, last_update);
  
  q->finish();
  delete q;
  
  vsl::SvVessel* vessel = VESSELs->value(id);
  
  vessel->ais()->setStaticVoyageData(static_voyage_data);
  vessel->gps()->setInitParams(gps_params);
  
  vessel->updateVessel();
  
  LISTITEMs->value(id)->setText(QString("%1\t%2").arg(id).arg(static_voyage_data.name));

}

void MainWindow::editVesselNavStat(int id)
{
  NAVSTATEDITOR_UI = new SvNavStatEditor(0, id, _bounds);

  switch (NAVSTATEDITOR_UI->exec()) {
    
    case SvNavStatEditor::Error:
      
      log << svlog::Time << svlog::Critical 
          << QString("Ошибка при редактировании записи:\n%1").arg(NAVSTATEDITOR_UI->last_error())
          << svlog::endl;
      break;
      
    case SvNavStatEditor::Accepted:

      VESSELs->value(id)->updateVessel();
      
      break;
  }
  
  delete NAVSTATEDITOR_UI;
  
}

void MainWindow::on_updateMapObjectInfo(SvMapObject* mapObject)
{
  if(!mapObject->isSelected())
    return;
  
  switch (mapObject->type()) {
    
    case motSelfVessel:
    case motVessel: 
    {
      
      if(AISs->find(mapObject->id()) != AISs->end()) {
        
        ais::SvAIS* a = AISs->value(mapObject->id());

        ui->textMapObjectInfo->setHtml(QString("<!DOCTYPE html><h3><font color=\"#0000CD\">Текущее судно:</font></h3>" \
                                               "<p><strong>ID:</strong>\t%1<br />" \
                                               "<strong>Название:</strong>\t%2<br />" \
                                               "<strong>Позывной:</strong>\t%3<br />" \
                                               "<strong>MMSI:</strong>\t%4<br />" \
                                               "<strong>IMO:</strong>\t%5<br />" \
                                               "<strong>Порт назнач.:</strong>\t%6<br />" \
                                               "<strong>Осадка:</strong>\t%7<br />" \
                                               "<strong>Чел. на борту:</strong>\t%8</p>" \
                                               "<hr>" \
                                               "<h3><font color=\"#0000CD\">Текущий статус:</font></h3>" \
                                               "<p><strong>Широта:</strong>\t%9<br />" \
                                               "<strong>Долгота:</strong>\t%10<br />" \
                                               "<strong>Курс:</strong>\t%11%12<br />" \
                                               "<strong>Скорость:</strong>\t%13 %14<br />" \
                                               "<strong>Статус:</strong>\t%15</p>")
                                       .arg(a->vesselId())
                                       .arg(a->staticVoyageData()->name)
                                       .arg(a->staticVoyageData()->callsign)
                                       .arg(a->staticVoyageData()->mmsi)
                                       .arg(a->staticVoyageData()->imo)
                                       .arg(a->staticVoyageData()->destination)
                                       .arg(a->staticVoyageData()->draft)
                                       .arg(a->staticVoyageData()->team)
                                       .arg(a->dynamicData()->geoposition.latitude, 0, 'f', 6)
                                       .arg(a->dynamicData()->geoposition.longtitude, 0, 'f', 6)
                                       .arg(a->dynamicData()->geoposition.course)
                                       .arg(QChar(176))
                                       .arg(a->dynamicData()->geoposition.speed * CMU.ConvertKoeff, 0, 'f', 1)
                                       .arg(CMU.SpeedDesign)
                                       .arg(a->navStatus()->name).toUtf8());
      }
        
      break;
    }
        
      
    default:
      break;
  }
}

void MainWindow::on_updateVesselById(int id)
{
  foreach (vsl::SvVessel* vessel, VESSELs->values()) {
    
    if(vessel->id != id) continue;
    
    if(_self_ais->distanceTo(vessel->ais()) > _self_ais->receiveRange() * CMU.MetersCount) {
      
      ((SvMapObjectOtherVessel*)(vessel->mapObject()))->setOutdated(true);
  
      LISTITEMs->value(id)->setIcon(QIcon(":/icons/Icons/link-broken2.ico"));
      LISTITEMs->value(id)->setFont(_font_nolink);
      LISTITEMs->value(id)->setTextColor(QColor(OUTDATED_VESSEL_COLOR));
      
    }
    else {
      
      ((SvMapObjectOtherVessel*)(vessel->mapObject()))->setOutdated(false);
      
      LISTITEMs->value(id)->setIcon(QIcon(":/icons/Icons/link3.ico"));
      LISTITEMs->value(id)->setFont(_font_default);
      LISTITEMs->value(id)->setTextColor(QColor(DEFAULT_VESSEL_PEN_COLOR));
      
      vessel->updateVessel();
    }  
    
  }
}

void MainWindow::on_updateVesselActive(int id)
{
  if(VESSELs->find(id) == VESSELs->end())
    return;
  
  bool isActive = VESSELs->value(id)->isActive();
    
  LISTITEMs->value(id)->setTextColor(isActive ? QColor(DEFAULT_VESSEL_PEN_COLOR) : QColor(INACTIVE_VESSEL_COLOR));
  LISTITEMs->value(id)->setFont(isActive ? _font_default : _font_inactive);
  LISTITEMs->value(id)->setIcon(isActive ? QIcon(":/icons/Icons/bullet_white.png") : QIcon(":/icons/Icons/bullet_red.png"));
  ui->bnSetActive->setIcon(isActive ? QIcon(":/icons/Icons/vessel_paused.ico") : QIcon(":/icons/Icons/vessel_start.ico"));
  
}

void MainWindow::update_NAVTEX_data()
{
  ui->textNAVTEXParams->setHtml(QString("<!DOCTYPE html><p><strong>Станция:</strong>\t%1</p>" \
                                        "<p><strong>Тип сообщения:</strong>\t%2</p>" \
                                        "<p><strong>Заголовок сообщения:</strong>\t%3%4%5</p>" \
                                        "<p><strong>Частота передачи:</strong>\t%6</p>" \
                                        "<p><strong>Сообщение:</strong></p>" \
                                        "<p>%7</p>")
                                .arg(_navtex->data()->region_station_name)
                                .arg(_navtex->data()->message_designation)
                                .arg(_navtex->data()->region_letter_id)
                                .arg(_navtex->data()->message_letter_id)
                                .arg(QString("%1").arg(_navtex->data()->message_last_number, 2, 10).replace(" ", "0"))
                                .arg(_navtex->data()->transmit_frequency)
                                .arg(_navtex->data()->message_text));
  
}


gps::gpsInitParams MainWindow::readGPSInitParams(QSqlQuery* q, ais::aisDynamicData& dynamic_data, QDateTime lastUpdate)
{
  gps::gpsInitParams result;
//  QDateTime dt = q->value("gps_last_update").toDateTime(); // для нормальной генерации случайных чисел

  result.gps_timeout = q->value("gps_timeout").toUInt();
  result.init_random_coordinates = q->value("init_random_coordinates").toBool();
  result.init_random_course = q->value("init_random_course").toBool();
  result.init_random_speed = q->value("init_random_speed").toBool();
  result.init_fixed_course = q->value("init_fixed_course").toBool();
  result.init_fixed_speed = q->value("init_fixed_speed").toBool();
  result.course_change_ratio = q->value("init_course_change_ratio").toUInt();
  result.course_change_segment = q->value("init_course_change_segment").toReal();
  result.speed_change_ratio = q->value("init_speed_change_ratio").toUInt();
  result.speed_change_segment = q->value("init_speed_change_segment").toReal();
  // начальные координаты
  if(result.init_random_coordinates || 
     (!result.init_random_coordinates && !dynamic_data.geoposition.isValidCoordinates())) {
    
    geo::COORDINATES coord = geo::get_rnd_coordinates(_bounds, lastUpdate.time().second() * 1000 + lastUpdate.time().msec());
    
    result.geoposition.latitude = coord.latitude;
    result.geoposition.longtitude = coord.longtitude;
    dynamic_data.geoposition.latitude = coord.latitude;
    dynamic_data.geoposition.longtitude = coord.longtitude;
    
  }
  else {
    
    result.geoposition.latitude = dynamic_data.geoposition.latitude; 
    result.geoposition.longtitude = dynamic_data.geoposition.longtitude; 
    
  }
  
  // начальный курс
  if(result.init_random_course ||
    (!result.init_random_course && !dynamic_data.geoposition.isValidCourse())) {
    
    result.geoposition.course = geo::get_rnd_course();
    dynamic_data.geoposition.course = result.geoposition.course;
    
  }
  else result.geoposition.course = dynamic_data.geoposition.course;
  
  // начальная скорость 
  if(result.init_random_speed ||
    (!result.init_random_speed && !dynamic_data.geoposition.isValidSpeed())) {
    
    result.geoposition.speed = geo::get_rnd_speed();
    dynamic_data.geoposition.speed = result.geoposition.speed;
    
  }
  else result.geoposition.speed = dynamic_data.geoposition.speed;
  
  return result;
}

ais::aisStaticVoyageData  MainWindow::readAISStaticVoyageData(QSqlQuery* q)
{
  ais::aisStaticVoyageData result;
  result.id = q->value("id").toUInt();
  result.mmsi = q->value("static_mmsi").toUInt();
  result.imo = q->value("static_imo").toUInt();
  result.callsign = q->value("static_callsign").toString();
  result.name = q->value("static_name").toString();
  result.pos_ref_A = q->value("static_pos_ref_A").isNull() ? 20 : q->value("static_pos_ref_A").toUInt();
  result.pos_ref_B = q->value("static_pos_ref_B").isNull() ? 20 : q->value("static_pos_ref_B").toUInt();
  result.pos_ref_C = q->value("static_pos_ref_C").isNull() ? 10 : q->value("static_pos_ref_C").toUInt();
  result.pos_ref_D = q->value("static_pos_ref_D").isNull() ? 10 : q->value("static_pos_ref_D").toUInt();
  result.vessel_ITU_id = q->value("static_type_ITU_id").toUInt();
//  result.vessel_type_name = q->value("static_vessel_type_name").toString();
  result.DTE = q->value("static_DTE").toUInt();
  result.talkerID = q->value("static_talker_id").toString();
  
  result.cargo_ITU_id = q->value("voyage_cargo_ITU_id").toUInt();
//  result.cargo_type_name = q->value("voyage_cargo_type_name").toString();
  result.destination = q->value("voyage_destination").toString();
  result.ETA_utc = q->value("voyage_ETA_utc").toTime();
  result.ETA_day = q->value("voyage_ETA_day").toUInt();
  result.ETA_month = q->value("voyage_ETA_month").toUInt();
  result.draft = q->value("voyage_draft").toReal();
  result.team = q->value("voyage_team").toUInt();
  
  return result;
}

ais::aisDynamicData MainWindow::readAISDynamicData(QSqlQuery* q)
{
  ais::aisDynamicData result;
  result.geoposition.latitude = q->value("dynamic_latitude").isNull() ? -1.0 : q->value("dynamic_latitude").toReal();
  result.geoposition.longtitude = q->value("dynamic_longtitude").isNull() ? -1.0 : q->value("dynamic_longtitude").toReal();
  result.geoposition.course = q->value("dynamic_course").isNull() ? -1.0 : q->value("dynamic_course").toReal();
  result.geoposition.speed = q->value("dynamic_speed").isNull() ? -1.0 : q->value("dynamic_speed").toReal();
 
  return result;
}

ais::aisNavStat MainWindow::readNavStat(QSqlQuery* q)
{
  ais::aisNavStat result;
  result.ITU_id = q->value("nav_status_ITU_id").toUInt(); 
  result.name = q->value("nav_status_name").toString(); 
  result.static_voyage_interval = q->value("nav_status_static_voyage_interval").toUInt(); 
//  result.ITU_id = q->value("nav_status_ITU_id").toUInt(); 
  return result;
}


void MainWindow::on_bnAISEditSerialParams_clicked()
{
  SERIALEDITOR_UI = new SvSerialEditor(_ais_serial_params, this);
  if(SERIALEDITOR_UI->exec() != QDialog::Accepted) {

    if(!SERIALEDITOR_UI->last_error().isEmpty()) {
      QMessageBox::critical(this, "Ошибка", QString("Ошибка при изменении параметров:\n%1").arg(SERIALEDITOR_UI->last_error()), QMessageBox::Ok);
    
      delete SERIALEDITOR_UI;
    
      return;
    }
    
  }
  
  _ais_serial_params.name = SERIALEDITOR_UI->params.name;
  _ais_serial_params.description = SERIALEDITOR_UI->params.description;
  _ais_serial_params.baudrate = SERIALEDITOR_UI->params.baudrate;
  _ais_serial_params.databits = SERIALEDITOR_UI->params.databits;
  _ais_serial_params.flowcontrol = SERIALEDITOR_UI->params.flowcontrol;
  _ais_serial_params.parity = SERIALEDITOR_UI->params.parity;
  _ais_serial_params.stopbits = SERIALEDITOR_UI->params.stopbits;
  
  delete SERIALEDITOR_UI;
  
  ui->editAISSerialInterface->setText(_ais_serial_params.description);

}

void MainWindow::on_bnLAGEditSerialParams_clicked()
{
  SERIALEDITOR_UI = new SvSerialEditor(_lag_serial_params, this);
  if(SERIALEDITOR_UI->exec() != QDialog::Accepted) {

    if(!SERIALEDITOR_UI->last_error().isEmpty()) {
      QMessageBox::critical(this, "Ошибка", QString("Ошибка при изменении параметров:\n%1").arg(SERIALEDITOR_UI->last_error()), QMessageBox::Ok);
    
      delete SERIALEDITOR_UI;
    
      return;
    }
    
  }
  
  _lag_serial_params.name = SERIALEDITOR_UI->params.name;
  _lag_serial_params.description = SERIALEDITOR_UI->params.description;
  _lag_serial_params.baudrate = SERIALEDITOR_UI->params.baudrate;
  _lag_serial_params.databits = SERIALEDITOR_UI->params.databits;
  _lag_serial_params.flowcontrol = SERIALEDITOR_UI->params.flowcontrol;
  _lag_serial_params.parity = SERIALEDITOR_UI->params.parity;
  _lag_serial_params.stopbits = SERIALEDITOR_UI->params.stopbits;
  
  delete SERIALEDITOR_UI;

  ui->editLAGSerialInterface->setText(_lag_serial_params.description);
}

void MainWindow::on_bnNAVTEXEditSerialParams_clicked()
{
  SERIALEDITOR_UI = new SvSerialEditor(_navtex_serial_params, this);
  if(SERIALEDITOR_UI->exec() != QDialog::Accepted) {

    if(!SERIALEDITOR_UI->last_error().isEmpty()) {
      QMessageBox::critical(this, "Ошибка", QString("Ошибка при изменении параметров:\n%1").arg(SERIALEDITOR_UI->last_error()), QMessageBox::Ok);
    
      delete SERIALEDITOR_UI;
        
      return;
    }
    
  }
  
  _navtex_serial_params.name = SERIALEDITOR_UI->params.name;
  _navtex_serial_params.description = SERIALEDITOR_UI->params.description;
  _navtex_serial_params.baudrate = SERIALEDITOR_UI->params.baudrate;
  _navtex_serial_params.databits = SERIALEDITOR_UI->params.databits;
  _navtex_serial_params.flowcontrol = SERIALEDITOR_UI->params.flowcontrol;
  _navtex_serial_params.parity = SERIALEDITOR_UI->params.parity;
  _navtex_serial_params.stopbits = SERIALEDITOR_UI->params.stopbits;
  
  delete SERIALEDITOR_UI;

  ui->editNAVTEXSerialInterface->setText(_navtex_serial_params.description);

}

void MainWindow::on_bnECHOMultiEditNetworkParams_clicked()
{
  NETWORKEDITOR_UI = new SvNetworkEditor(_echo_multi_network_params, this);
  if(NETWORKEDITOR_UI->exec() != QDialog::Accepted) {

    if(!NETWORKEDITOR_UI->last_error().isEmpty()) {
      QMessageBox::critical(this, "Ошибка", QString("Ошибка при изменении параметров:\n%1").arg(NETWORKEDITOR_UI->last_error()), QMessageBox::Ok);
    
      delete NETWORKEDITOR_UI;
        
      return;
    }
    
  }
  
  _echo_multi_network_params.ifc = NETWORKEDITOR_UI->params.ifc;
  _echo_multi_network_params.protocol = NETWORKEDITOR_UI->params.protocol;
  _echo_multi_network_params.ip = NETWORKEDITOR_UI->params.ip;
  _echo_multi_network_params.port = NETWORKEDITOR_UI->params.port;
  _echo_multi_network_params.description = NETWORKEDITOR_UI->params.description;
  _echo_multi_network_params.translate_type = NETWORKEDITOR_UI->params.translate_type;
  
  delete NETWORKEDITOR_UI;

  ui->editECHOMultiInterface->setText(_echo_multi_network_params.description);
}

void MainWindow::on_bnECHOFishEditNetworkParams_clicked()
{
  NETWORKEDITOR_UI = new SvNetworkEditor(_echo_fish_network_params, this);
  if(NETWORKEDITOR_UI->exec() != QDialog::Accepted) {

    if(!NETWORKEDITOR_UI->last_error().isEmpty()) {
      QMessageBox::critical(this, "Ошибка", QString("Ошибка при изменении параметров:\n%1").arg(NETWORKEDITOR_UI->last_error()), QMessageBox::Ok);
    
      delete NETWORKEDITOR_UI;
        
      return;
    }
    
  }
  
  _echo_fish_network_params.ifc = NETWORKEDITOR_UI->params.ifc;
  _echo_fish_network_params.protocol = NETWORKEDITOR_UI->params.protocol;
  _echo_fish_network_params.ip = NETWORKEDITOR_UI->params.ip;
  _echo_fish_network_params.port = NETWORKEDITOR_UI->params.port;
  _echo_fish_network_params.description = NETWORKEDITOR_UI->params.description;
  _echo_fish_network_params.translate_type = NETWORKEDITOR_UI->params.translate_type;
  
  delete NETWORKEDITOR_UI;

  ui->editECHOFishInterface->setText(_echo_fish_network_params.description);
}

void MainWindow::on_bnGPSEditNetworkParams_clicked()
{
  NETWORKEDITOR_UI = new SvNetworkEditor(_gps_network_params, this);
  if(NETWORKEDITOR_UI->exec() != QDialog::Accepted) {

    if(!NETWORKEDITOR_UI->last_error().isEmpty()) {
      QMessageBox::critical(this, "Ошибка", QString("Ошибка при изменении параметров:\n%1").arg(NETWORKEDITOR_UI->last_error()), QMessageBox::Ok);
    
      delete NETWORKEDITOR_UI;
        
      return;
    }
    
  }
  
  _gps_network_params.ifc = NETWORKEDITOR_UI->params.ifc;
  _gps_network_params.protocol = NETWORKEDITOR_UI->params.protocol;
  _gps_network_params.ip = NETWORKEDITOR_UI->params.ip;
  _gps_network_params.port = NETWORKEDITOR_UI->params.port;
  _gps_network_params.description = NETWORKEDITOR_UI->params.description;
  _gps_network_params.translate_type = NETWORKEDITOR_UI->params.translate_type;
  
  delete NETWORKEDITOR_UI;

  ui->editGPSInterface->setText(_gps_network_params.description);
}

void MainWindow::on_bnNAVTEXAlarmSend_clicked()
{
  _navtex->alarm(ui->spinNAVTEXAlarmId->value(),
                 ui->cbNAVTEXAlarmState->currentIndex() == 0 ? "A" : "V",
                 ui->editNAVTEXAlarmMessageText->text().left(62));
}

void MainWindow::on_bnLAGAlarmSend_clicked()
{
  _self_lag->alarm(ui->spinLAGAlarmId->value(),
                 ui->cbLAGAlarmState->currentIndex() == 0 ? "A" : "V",
                 ui->editLAGAlarmMessageText->text().left(62));    
}

void MainWindow::on_bnAISAlarmSend_clicked()
{
  _self_ais->alarm(ui->spinAISAlarmId->value(),
                 ui->cbAISAlarmState->currentIndex() == 0 ? "A" : "V",
                 ui->editAISAlarmMessageText->text().left(62));
}

void MainWindow::on_cbLAGMessageType_currentIndexChanged(int index)
{
  Q_UNUSED(index);
  
  // здесь какая то хрень творится. если сигнал переделать на выхов функции, то начинает вываливаться
  emit new_lag_message_type(lag::MessageType(ui->cbLAGMessageType->currentData().toInt()));
}


void MainWindow::stateChanged(States state)
{
  switch (state) {
    
    case sRunned:
    {
      ui->tabWidget->setEnabled(true);
      
      ui->checkAISEnabled->setEnabled(false);
      ui->checkLAGEnabled->setEnabled(false);
      ui->checkNAVTEXEnabled->setEnabled(false);
      ui->checkECHOFishEnabled->setEnabled(false);
      ui->checkECHOMultiEnabled->setEnabled(false);
      ui->checkGPSEnabled->setEnabled(false);
      
      ui->bnAISEditSerialParams->setEnabled(false);
      ui->bnLAGEditSerialParams->setEnabled(false);
      ui->bnNAVTEXEditSerialParams->setEnabled(false);
      ui->bnECHOFishEditNetworkParams->setEnabled(false);
      ui->bnECHOMultiEditNetworkParams->setEnabled(false);
      ui->bnGPSEditNetworkParams->setEnabled(false);
      
      ui->gbAISParams->setEnabled(false);
      ui->gbLAGParams->setEnabled(false);
      ui->gbNAVTEXParams->setEnabled(false);
      ui->gbECHOMultiParams->setEnabled(false);
      ui->gbECHOFishParams->setEnabled(false);
      ui->gbGPSParams->setEnabled(false);
      
      ui->bnAddVessel->setEnabled(false);
      ui->bnEditVessel->setEnabled(false);
      ui->bnRemoveVessel->setEnabled(false);
      ui->bnSetActive->setEnabled(false);
      ui->bnDropDynamicData->setEnabled(false);
//      ui->bnVesselNavState->setEnabled(true);
      
      ui->bnStart->setEnabled(true);
      
      ui->bnStart->setIcon(QIcon());
      ui->bnStart->setText("Стоп");
      ui->bnStart->setStyleSheet("background-color: tomato");
      
      break;
    }
      
    case sStopped:
    {
      ui->tabWidget->setEnabled(true);
      
      ui->checkAISEnabled->setEnabled(true);
      ui->checkLAGEnabled->setEnabled(true);
      ui->checkNAVTEXEnabled->setEnabled(true);
      ui->checkECHOFishEnabled->setEnabled(true);
      ui->checkECHOMultiEnabled->setEnabled(true);
      ui->checkGPSEnabled->setEnabled(true);
      
      ui->bnAISEditSerialParams->setEnabled(true);
      ui->bnLAGEditSerialParams->setEnabled(true);
      ui->bnNAVTEXEditSerialParams->setEnabled(true);
      ui->bnECHOFishEditNetworkParams->setEnabled(true);
      ui->bnECHOMultiEditNetworkParams->setEnabled(true);
      ui->bnGPSEditNetworkParams->setEnabled(true);
      
      ui->gbAISParams->setEnabled(true);
      ui->gbLAGParams->setEnabled(true);
      ui->gbNAVTEXParams->setEnabled(true);
      ui->gbECHOMultiParams->setEnabled(true);
      ui->gbECHOFishParams->setEnabled(true);
      ui->gbGPSParams->setEnabled(true);
      
      ui->bnAddVessel->setEnabled(true);
      ui->bnEditVessel->setEnabled(true);
      ui->bnRemoveVessel->setEnabled(true);
      ui->bnSetActive->setEnabled(true);
      ui->bnDropDynamicData->setEnabled(true);
      
      ui->bnStart->setEnabled(true);
      ui->bnStart->setIcon(QIcon(":/icons/Icons/start.ico"));
      ui->bnStart->setText("Старт");
      ui->bnStart->setStyleSheet("");
      break;
    }
      
    case sRunning:
    case sStopping:
    {
      ui->bnStart->setEnabled(false);
      ui->tabWidget->setEnabled(false);
      break;
    }
      
    default:
      break;
  }
  
  _current_state = state;
  QApplication::processEvents();
  
}


void MainWindow::on_actionNewVessel_triggered()
{
  VESSELEDITOR_UI = new SvVesselEditor(this);
  if(VESSELEDITOR_UI->exec() != QDialog::Accepted) {
    
    if(!VESSELEDITOR_UI->last_error().isEmpty())
      QMessageBox::critical(this, "Ошибка", QString("Ошибка при добавлении записи:\n%1").arg(VESSELEDITOR_UI->last_error()), QMessageBox::Ok);
    
    delete VESSELEDITOR_UI;
    
    return;
  }
  
  delete VESSELEDITOR_UI;
    
  
  /** ------ читаем информацию о новом судне --------- **/
  QSqlQuery* q = new QSqlQuery(SQLITE->db);
  
  try {
    
    if(QSqlError::NoError != SQLITE->execSQL(QString(SQL_SELECT_LAST_INSERTED_VESSEL), q).type())
      _exception.raise(q->lastError().databaseText());
      
    else if(!q->next()) _exception.raise("Неизвестная ошибка");
    
    /** --------- создаем судно ----------- **/
    if(!createOtherVessel(q)) _exception.raise("Ошибка при добавлении нового судна");
    
  }
    
  catch(SvException& e) {
    
    if(q) q->finish();
    delete q;
    
    log << svlog::Time << svlog::Critical << e.err << svlog::endl;
    return;
    
  }
}

void MainWindow::on_actionEditVessel_triggered()
{
  editVessel(_selected_vessel_id);
}

void MainWindow::on_actionRemoveVessel_triggered()
{
  if(ui->listVessels->selectedItems().count() == 0)
    return;
  
  /** ------ пытаемся удалить запись о судне в БД --------- **/
  try {
//    QSqlError err = SQLITE->execSQL(QString(SQL_DELETE_VESSEL).arg(_selected_vessel_id));
//    if(QSqlError::NoError != err.type())
//      _exception.raise(err.databaseText());
      
    disconnect(ui->listVessels, &QListWidget::currentItemChanged, this, &MainWindow::currentVesselListItemChanged);
    
    int row = ui->listVessels->currentRow(); // selectedItems().first()-> ->listWidget()-> currentRow();
    
//    delete AISs->take(_selected_vessel_id);
//    delete GPSs->take(_selected_vessel_id);
    _area->scene->removeMapObject(VESSELs->value(_selected_vessel_id)->mapObject()->selection());
    _area->scene->removeMapObject(VESSELs->value(_selected_vessel_id)->mapObject()->identifier());
    _area->scene->removeMapObject(VESSELs->value(_selected_vessel_id)->mapObject());
    delete VESSELs->take(_selected_vessel_id);
    AISs->remove(_selected_vessel_id);
    GPSs->remove(_selected_vessel_id);
    
    
    ui->listVessels->takeItem(row);
    delete LISTITEMs->take(_selected_vessel_id);

    connect(ui->listVessels, &QListWidget::currentItemChanged, this, &MainWindow::currentVesselListItemChanged);
    
    ui->listVessels->setCurrentRow(row - 1);
    
  }
  
  catch(SvException& e) {
    
    log << svlog::Time << svlog::Critical << e.err << svlog::endl;
    
  }
}

void MainWindow::on_bnDropDynamicData_clicked()
{
  int msgbtn = QMessageBox::question(0, "Подтверждение", "Текущие данные о местоположении судна(ов) будут сброшены.\nДля выбранного судна - \"Да\"\n"
                           "Для всех судов - \"Да для всех\".\n"
                           "Вы уверены?", QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No);
  
  if(!QList<int>({QMessageBox::Yes, QMessageBox::YesToAll}).contains(msgbtn)) return;

  QString sql1, sql2;
  switch (msgbtn) {
    case QMessageBox::Yes: 
      
      sql1 = QString("UPDATE ais SET dynamic_latitude=NULL, "
                    "dynamic_longtitude=NULL, "
                    "dynamic_course=NULL, dynamic_speed=NULL, "
                    "dynamic_pitch=NULL, dynamic_roll=NULL "
                    "where vessel_id=%1").arg(_selected_vessel_id);
      
      sql2 = QString(SQL_SELECT_VESSEL_WHERE_ID).arg(_selected_vessel_id);
      
      break;
      
    case QMessageBox::YesToAll: 
      
      sql1 = "UPDATE ais SET dynamic_latitude=NULL, "
            "dynamic_longtitude=NULL, "
            "dynamic_course=NULL, dynamic_speed=NULL, "
            "dynamic_pitch=NULL, dynamic_roll=NULL";
      
      sql2 = QString(SQL_SELECT_VESSELS);
      
      break;
  }
  
  QSqlQuery* q = new QSqlQuery(SQLITE->db);
  try {
    
    QSqlError e = SQLITE->execSQL(sql1);
    if(e.type() != QSqlError::NoError) _exception.raise(e.text());
    
    // читаем информацию из БД  
    /// ------ читаем список судов --------- ///
    if(QSqlError::NoError != SQLITE->execSQL(sql2, q).type())
      _exception.raise(q->lastError().databaseText());
    
    /** --------- читаем данные судна ----------- **/
    while(q->next()) {
      
      int vessel_id = q->value("id").toUInt();
      QDateTime last_update = q->value("gps_last_update").toDateTime(); // для нормальной генерации случайных чисел
      
      ais::aisDynamicData dynamic_data = readAISDynamicData(q);
      gps::gpsInitParams gps_params = readGPSInitParams(q, dynamic_data, last_update);
      
      AISs->value(vessel_id)->setDynamicData(dynamic_data);
      GPSs->value(vessel_id)->setInitParams(gps_params);
      
      VESSELs->value(vessel_id)->updateVessel();
      
    }
  }
  
  catch(SvException &e) {
    log << svlog::Critical << svlog::Time << e.err << svlog::endl;
  }
  
  if(q) q->finish();
  delete q;
  
}


void MainWindow::setX10Emulation()
{
  emit setMultiplier(10);
  ui->bnStart->setStyleSheet("background-color: rgb(85, 170, 255);");
  ui->bnStart->setText("Старт х10");
  update();
}

void MainWindow::on_bnStart_pressed()
{
  if(_current_state != sStopped)
    return;
  
  _timer_x10.start(2000); 
}

void MainWindow::on_bnStart_released()
{
    _timer_x10.stop();
}


void MainWindow::on_bnEditNAVTEX_clicked()
{
  NAVTEXEDITOR_UI = new SvNavtexEditor(this, _navtex->id());
  if(NAVTEXEDITOR_UI->exec() != QDialog::Accepted) {

    if(!NAVTEXEDITOR_UI->last_error().isEmpty()) {
      QMessageBox::critical(this, "Ошибка", QString("Ошибка при изменении параметров:\n%1").arg(NAVTEXEDITOR_UI->last_error()), QMessageBox::Ok);
    
      delete NAVTEXEDITOR_UI;
        
      return;
    }
  }
  
  nav::navtexData data;
//  memcpy(&data, _navtex->data(), sizeof(nav::navtexData));
  
  data.message_id = NAVTEXEDITOR_UI->t_message_id;
  data.region_id = NAVTEXEDITOR_UI->t_region_id;
  data.message_text = NAVTEXEDITOR_UI->t_message_text;
  data.message_designation = NAVTEXEDITOR_UI->t_message_designation;
  data.message_letter_id = NAVTEXEDITOR_UI->t_message_letter_id;
  data.region_letter_id = NAVTEXEDITOR_UI->t_region_letter_id;
  data.region_station_name = NAVTEXEDITOR_UI->t_region_station_name;
  data.transmit_frequency = NAVTEXEDITOR_UI->t_transmit_frequency;
  data.transmit_frequency_id = NAVTEXEDITOR_UI->t_transmit_frequency_id;
  data.message_last_number = NAVTEXEDITOR_UI->t_message_last_number;
  
  delete NAVTEXEDITOR_UI;

  _navtex->setData(data);
  
  update_NAVTEX_data();
  
}


void MainWindow::updateGPSInitParams(gps::SvGPS* g)
{
  QString upd = "";
//  gps::gpsInitParams params = g->initParams();
  
  if(!g->initParams().init_random_coordinates) {
    
    upd.append(QString("dynamic_latitude=%1,dynamic_longtitude=%2,")
                         .arg(g->currentGeoposition()->latitude)
                         .arg(g->currentGeoposition()->longtitude));
    
//    params.geoposition.latitude = g->currentGeoposition()->latitude;
//    params.geoposition.longtitude = g->currentGeoposition()->longtitude;
    
  }
  
  if(!g->initParams().init_random_course) {
    upd.append(QString("dynamic_course=%1,").arg(g->currentGeoposition()->course));
//    params.geoposition.course = g->currentGeoposition()->course;
  }
  
  if(!g->initParams().init_random_speed) {
    upd.append(QString("dynamic_speed=%1,").arg(g->currentGeoposition()->speed));
//    params.geoposition.speed = g->currentGeoposition()->speed;
  }        
  
  if(!g->initParams().init_random_pitch) {
    upd.append(QString("dynamic_pitch=%1,").arg(g->currentGeoposition()->pitch));
//    params.geoposition.pitch = g->currentGeoposition()->pitch;
  }
  
  if(!g->initParams().init_random_roll) {
    upd.append(QString("dynamic_roll=%1,").arg(g->currentGeoposition()->roll));
//    params.geoposition.roll = g->currentGeoposition()->roll;
  }
  
  if(upd.isEmpty()) return;
  
  upd.chop(1); // убираем последнюю запятую
  QString sql = QString("UPDATE ais SET %1 where vessel_id=%2").arg(upd)
                .arg(g->vesselId());
  
  QSqlError e = SQLITE->execSQL(sql);
  
  if(e.type() != QSqlError::NoError) {
    
    log << svlog::Error << svlog::Time << e.text() << svlog::endl;
    return;
  
  }
  
//  g->setInitParams(params);
  
//  return;
}

void MainWindow::on_echoBeamsUpdated(ech::Beam* bl)
{
  double min = _curve->keyAxis()->range().lower;
  double max = _curve->keyAxis()->range().upper + 1;
//  qDebug() << min << max;
  
  _curve->data().data()->remove(min);
  _curve->addData(max, -bl->Z);

  _scatt->data().data()->remove(min);
  _scatt->addData(max, -bl->Z, -bl->Z, -bl->Z - bl->backscatter, -bl->Z - bl->backscatter);
  
  if(bl->fish) {
    qreal step = bl->Z / qreal(sizeof(bl->fish));
    for(int i = 0; i < sizeof(bl->fish); i++)
      if(bl->fish & (1 << i)) _fish->addData(max, -(step * i));
  }
  
  ui->customplot->xAxis->setRange(min + 1, max);
  ui->customplot->replot();
}

void MainWindow::on_cbMeasureUnits_currentIndexChanged(int index)
{
  Q_UNUSED(index);
  
//  switch (index) {
//    case 0:
//      geo::setCurrentMeasureUnit(geo::uKnotsMiles);
//      break;
      
//    case 1:
//      geo::setCurrentMeasureUnit(geo::uKmhKm);
//      break;
      
//    default:
//      return;
//  }
  
////  updateUnits();
//  _area->view->repaint();
//  on_updateVesselById(_selected_vessel_id);
  
}


void MainWindow::on_bnSetActive_clicked()
{
  if(_selected_vessel_id == _self_vessel->id)
    return;
  
  if(VESSELs->find(_selected_vessel_id) == VESSELs->end())
    return;
  
  
  vsl::SvVessel* vessel = VESSELs->value(_selected_vessel_id);
  bool newActive = !vessel->isActive();
  
  QSqlError err = SQLITE->execSQL(QString(SQL_UPDATE_VESSEL_ACTIVE).arg(newActive).arg(_selected_vessel_id));
  if(err.type() != QSqlError::NoError) {
    
    log << svlog::Critical << svlog::Time << err.databaseText() << svlog::endl;
    return;
    
  }
  
  vessel->setActive(newActive);
  vessel->mapObject()->setActive(newActive);

  on_updateVesselActive(_selected_vessel_id);
  
}

void MainWindow::on_actionNavStat_triggered()
{
  editVesselNavStat(_selected_vessel_id);
}

