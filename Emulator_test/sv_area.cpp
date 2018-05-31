#include "sv_area.h"

extern SvSQLITE *SQLITE;
extern geo::UnitsInfo CMU;

inline QPointF geo2point(const area::AREA_DATA *area_data, qreal lon, qreal lat);
inline QPointF point2geo(const area::AREA_DATA *area_data, qreal x, qreal y);

QPointF geo2point(const area::AREA_DATA* area_data, qreal lon, qreal lat)
{
//  qDebug() << "geo2point" << area_data->geo_bounds.max_lat << lat;
  qreal x = area_data->area_curr_size.width() - (area_data->geo_bounds.max_lon - lon) * area_data->koeff.lon/** area_data->koeff2*/;
  qreal y = (area_data->geo_bounds.max_lat - lat) * area_data->koeff.lat /** area_data->koeff2*/;
//  qDebug() << x << y;
  return QPointF(x, y);
}

QPointF point2geo(const area::AREA_DATA* area_data, qreal x, qreal y)
{
  qreal lon = area_data->geo_bounds.min_lon + x / area_data->koeff.lon;
  qreal lat = area_data->geo_bounds.max_lat - y / area_data->koeff.lon;
  
//  qreal x = area_data->area_curr_size.width() - (area_data->geo_bounds.max_lon - lon) * area_data->koeff.lon;
//  qreal y = (area_data->geo_bounds.max_lat - lat) * area_data->koeff.lat;
  return QPointF(lon, lat);
}



area::SvArea::SvArea(QWidget *parent) :
  QWidget(parent)
{
 // setParent(parent);
  setObjectName(QStringLiteral("widgetArea"));
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);  
  
  vlayMain = new QVBoxLayout(this);
  vlayMain->setSpacing(2);
  vlayMain->setObjectName(QStringLiteral("vlayMain"));
  vlayMain->setContentsMargins(2, 2, 2, 2);
  
//  /* левая панель с кнопками */
//  frameLeft = new QFrame(this);
//  frameLeft->setObjectName(QStringLiteral("frameLeft"));
//  frameLeft->setFrameShape(QFrame::Box);
//  frameLeft->setFrameShadow(QFrame::Sunken);
//  vlayFrameLeft = new QVBoxLayout(frameLeft);
//  vlayFrameLeft->setSpacing(6);
//  vlayFrameLeft->setContentsMargins(11, 11, 11, 11);
//  vlayFrameLeft->setObjectName(QStringLiteral("vlayFrameLeft"));
//  vlayFrameLeft->setContentsMargins(2, 2, 2, 2);
  /* левая панель с кнопками */
  
  frameTop = new QFrame(this);
  frameTop->setObjectName(QStringLiteral("frameTop"));
  frameTop->setFrameShape(QFrame::Panel);
  frameTop->setFrameShadow(QFrame::Raised);
  hlayFrameTop = new QHBoxLayout(frameTop);
  hlayFrameTop->setSpacing(6);
  hlayFrameTop->setObjectName(QStringLiteral("hlayFrameTop"));
  hlayFrameTop->setContentsMargins(2, 2, 2, 2);
  
//  buttonsTop.append(new AreaButton(frameTop, bntAlignToLeftTop, "", "bnAlignToLeftTop", QSize(24, 24), QIcon(":/buttons/Icons/Fullscreen.ico")));
//  buttonsTop.append(new AreaButton(frameTop, bntTrackAirplane, "", "bnTrackSelfVessel", QSize(24, 24), QIcon(":/buttons/Icons/Link.ico")));
  buttonsTop.append(new AreaButton(frameTop, bntTrackSelected, "", "bntTrackSelected", QSize(24, 24), QIcon(":/buttons/Icons/link_selected.ico")));
  buttonsTop.last()->setCheckable(true);
  buttonsTop.last()->setChecked(true);
  buttonsTop.append(new AreaButton(frameTop, bntCenterSelected, "", "bntCenterSelected", QSize(24, 24), QIcon(":/buttons/Icons/aim_selected.ico")));
  
  buttonsTop.append(new AreaButton(frameTop, bntZoomIn, "", "bnZoomIn", QSize(24, 24), QIcon(":/buttons/Icons/ZoomIn.ico")));
  buttonsTop.append(new AreaButton(frameTop, bntZoomOriginal, "", "bnZoomOriginal", QSize(24, 24), QIcon(":/buttons/Icons/Search.ico")));
  buttonsTop.append(new AreaButton(frameTop, bntZoomOut, "", "bnZoomOut", QSize(24, 24), QIcon(":/buttons/Icons/ZoomOut.ico")));
  
  foreach (AreaButton* button, buttonsTop) {
    hlayFrameTop->addWidget(button);
    connect(button, SIGNAL(pressed()), this, SLOT(buttonPressed()));
  }
  
  hlayFrameTop->addStretch();
  
//  buttonsLeft.append(new AreaButton(frameLeft, bntAlignToLeftTop, "", "bnAlignToLeftTop", QSize(40, 40), QIcon(":/buttons/Icons/Fullscreen.ico")));
//  buttonsLeft.append(new AreaButton(frameLeft, bntDropBeacon, "", "bnDropBeacon", QSize(40, 40), QIcon(":/buttons/Icons/Download.ico")));
//  buttonsLeft.append(new AreaButton(frameLeft, bntReadSocket, "", "bnReadSocket", QSize(40, 40), QIcon(":/buttons/Icons/Globe.ico")));
//  buttonsLeft.last()->setCheckable(true);
//  buttonsLeft.append(new AreaButton(frameLeft, bntAddBeacon, "", "bnAddBeacon", QSize(40, 40), QIcon(":/buttons/Icons/AddBeacon.ico")));
//  buttonsLeft.last()->setCheckable(true);
//  buttonsLeft.append(new AreaButton(frameLeft, bntTrackAirplane, "", "bnTrackAirplane", QSize(40, 40), QIcon(":/buttons/Icons/Link.ico")));
//  buttonsLeft.last()->setCheckable(true);
//  buttonsLeft.append(new AreaButton(frameLeft, bntCenterOnAirplane, "", "bnCenterOnAirplane", QSize(40, 40), QIcon(":/buttons/Icons/Plane.ico")));
//  buttonsLeft.append(new AreaButton(frameLeft, bntNone, "o", "bnLeft5", QSize(40, 25)));
  
//  foreach (AreaButton* button, buttonsLeft) {
//    vlayFrameLeft->addWidget(button);
//    connect(button, SIGNAL(pressed()), this, SLOT(buttonPressed()));
//  }
  
//  /* правая панель с кнопками */
//  frameRight = new QFrame(this);
//  frameRight->setObjectName(QStringLiteral("frameRight"));
//  frameRight->setFrameShape(QFrame::Box);
//  frameRight->setFrameShadow(QFrame::Sunken);
//  vlayFrameRight = new QVBoxLayout(frameRight);
//  vlayFrameRight->setSpacing(6);
//  vlayFrameRight->setContentsMargins(11, 11, 11, 11);
//  vlayFrameRight->setObjectName(QStringLiteral("vlayFrameRight"));
//  vlayFrameRight->setContentsMargins(2, 2, 2, 2);
  
//  buttonsRight.append(new AreaButton(frameRight, bntZoomIn, "", "bnZoomIn", QSize(40, 40), QIcon(":/buttons/Icons/ZoomIn.ico")));
//  buttonsRight.append(new AreaButton(frameRight, bntZoomOriginal, "", "bnZoomOriginal", QSize(40, 40), QIcon(":/buttons/Icons/Search.ico")));
//  buttonsRight.append(new AreaButton(frameRight, bntZoomOut, "", "bnZoomOut", QSize(40, 40), QIcon(":/buttons/Icons/ZoomOut.ico")));
//  buttonsRight.append(new AreaButton(frameRight, bntMoveUp, "", "bnMoveUp", QSize(40, 40), QIcon(":/buttons/Icons/Up.ico")));
//  buttonsRight.append(new AreaButton(frameRight, bntMoveDown, "", "bnMoveDown", QSize(40, 40), QIcon(":/buttons/Icons/Down.ico")));
//  buttonsRight.append(new AreaButton(frameRight, bntMoveRight, "", "bnMoveRight", QSize(40, 40), QIcon(":/buttons/Icons/Right.ico")));
//  buttonsRight.append(new AreaButton(frameRight, bntMoveLeft, "", "bnMoveLeft", QSize(40, 40), QIcon(":/buttons/Icons/Left.ico")));
  
  
//  foreach (AreaButton* button, buttonsRight) {
//    vlayFrameRight->addWidget(button);
//    connect(button, SIGNAL(pressed()), this, SLOT(buttonPressed()));
//  }
 
  
  /* виджет карты */
  widgetMap = new QWidget(this);
  widgetMap->setObjectName(QStringLiteral("widgetMap"));
  widgetMap->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  widgetMap->setStyleSheet(QStringLiteral("background-color: rgb(255, 250, 255);"));
  /* лог */
//  textLog = new QTextEdit(this);
//  textLog->setObjectName(QStringLiteral("textLog"));
//  textLog->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  
//  splitter = new QSplitter(Qt::Vertical, this);
//  splitter->setGeometry(this->rect());
//  splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//  splitter->addWidget(widgetMap);
//  splitter->addWidget(textLog);
  
//  /* нижняя панель с информацией */
//  frameBottom = new QFrame(this);
//  frameBottom->setObjectName(QStringLiteral("frameBottom"));
//  frameBottom->setFrameShape(QFrame::NoFrame);
////  frameBottom->setFrameShadow(QFrame::Sunken);
//  frameBottom->setGeometry(0, 0, 0, 10);
//  frameBottom->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  
//  vlayFrameBottom = new QVBoxLayout(frameBottom);
//  vlayFrameBottom->setSpacing(6);
//  vlayFrameBottom->setContentsMargins(11, 11, 11, 11);
//  vlayFrameBottom->setObjectName(QStringLiteral("vlayFrameBottom"));
//  vlayFrameBottom->setContentsMargins(2, 2, 2, 2);
  
//  lblCurrentInfo = new QLabel("");
//  vlayFrameBottom->addWidget(lblCurrentInfo);
  
//  vlayCenter = new QVBoxLayout();
//  vlayCenter->setSpacing(6);
//  vlayCenter->setContentsMargins(11, 11, 11, 11);
//  vlayCenter->setObjectName(QStringLiteral("vlayCenter"));
//  vlayCenter->setContentsMargins(2, 2, 2, 2);
  
//  vlayCenter->addWidget(widgetMap);
//  vlayCenter->addWidget(frameBottom);
 
  vlayMain->addWidget(frameTop);
  vlayMain->addWidget(widgetMap);
//  vlayMain->addWidget(textLog);
}

void area::SvArea::setUp(QString areaName, const geo::BOUNDS &bounds)
{  
  _area_data.area_name = areaName;
  _area_data.geo_bounds = bounds;
  
  qDebug() << _area_data.geo_bounds.max_lat << _area_data.geo_bounds.max_lon << _area_data.geo_bounds.min_lat << _area_data.geo_bounds.min_lon;
  
  // определяем размер экрана
  QScreen *scr = QGuiApplication::primaryScreen();
  
  // определяем отношение сторон
  _area_data.lon2lon_distance = geo::lon2lon_distance(_area_data.geo_bounds.min_lon, _area_data.geo_bounds.max_lon, (_area_data.geo_bounds.max_lat + _area_data.geo_bounds.min_lat) / 2);
  _area_data.lat2lat_distance = geo::lat2lat_distance(_area_data.geo_bounds.min_lat, _area_data.geo_bounds.max_lat, (_area_data.geo_bounds.max_lon + _area_data.geo_bounds.min_lon) / 2);
  
  // если ширина карты больше высоты, то задаем ширину, а высоту подгоняем 
  if(_area_data.lon2lon_distance / _area_data.lat2lat_distance > 1) {
    
    int w = scr->availableSize().width() / 1.5;
    _area_data.area_base_size.setWidth(w);
    _area_data.area_base_size.setHeight(w * (_area_data.lat2lat_distance / _area_data.lon2lon_distance)); 
    
  }
  
  // иначе, если высота карты больше ширины, то задаем высоту, а ширину подгоняем
  else {

    int h = scr->availableSize().height();
    _area_data.area_base_size.setHeight(h / 1.5);
    _area_data.area_base_size.setWidth(h * (_area_data.lon2lon_distance / _area_data.lat2lat_distance)); 
  }
  
  
  scene = new SvAreaScene(&_area_data);
  view = new SvAreaView(widgetMap, &_area_data);
  view->setAreaScene(scene);
  
  _trackSelected = AppParams::readParam(this, QString("AREA_%1").arg(_area_data.area_name), "TrackSelected", true).toBool();
  for(AreaButton *btn: buttonsTop) {
    if(btn->type() == bntTrackSelected)
      btn->setChecked(_trackSelected);
  }
  
  /* задаем масштаб */
  _area_data.scale = AppParams::readParam(this, QString("AREA_%1").arg(_area_data.area_name), "Scale", 0.64).toReal();
  setScale(_area_data.scale);
  
  connect(view, SIGNAL(mouseMoved(QMouseEvent*)), this, SLOT(mouseMoved(QMouseEvent*)));
  connect(view, SIGNAL(mousePressed(QMouseEvent*)), this, SLOT(mousePressed(QMouseEvent*)));
  connect(view, SIGNAL(mouseReleased(QMouseEvent*)), this, SLOT(mouseReleased(QMouseEvent*)));
  
  
//  udp = new SvUdpReader("172.16.4.106", 35600, this);
////  connect(udp, SIGNAL(), socker, SLOT(terminate())
//  connect(udp, SIGNAL(dataReaded(qreal,qreal,int)), this, SLOT(trackAirplane(qreal,qreal,int)));
  
//  _trackPen.setColor(QColor(30, 130, 230, 255));
  _trackBrush.setColor(QColor(170, 85, 0, 170));
  _trackBrush.setStyle(Qt::SolidPattern);
  _trackPen.setStyle(Qt::NoPen); // SolidLine);
  _track_secs = _track_time.hour() * 3600 + _track_time.minute() * 60 + _track_time.second();
  
//  connect(this, SIGNAL(editing(bool)), this, SLOT(editing(bool)));
  
}

area::SvArea::~SvArea()
{
//  view->~SvAreaView();
//  scene->~SvAreaScene();
  
  AppParams::saveWindowParams(this, size(), pos(), windowState(), QString("AREA_%1").arg(_area_data.area_name));
  AppParams::saveParam(this, QString("AREA_%1").arg(_area_data.area_name), "Scale", _area_data.scale);
  AppParams::saveParam(this, QString("AREA_%1").arg(_area_data.area_name), "TrackSelected", _trackSelected);
  
  deleteLater();
}


void area::SvArea::trackAirplane(qreal lon, qreal lat, int angle)
{
  /**
  QPointF newPoint = geo2point(&_area_data, lon, lat);
  
  SvMapObjectRadius* mapRadius = nullptr;
  SvMapObjectAirplane* airplane = nullptr;
//  qreal min_distance = 0xFFFFFFFF;
  
  foreach(SvMapObject *obj, scene->mapObjects()) {
    
    switch (obj->type()) {
      
      case motAirplane: 
        
        airplane = (SvMapObjectAirplane*)obj;
        airplane->setGeo(lon, lat);
        airplane->setPos(newPoint);
//        qDebug() << "air"<< newPoint;
        airplane->setAngle(angle);
        
        break;
        
//      case motBeaconPlanned: // находим ближайший буй
//      {
//        qreal ab = qSqrt(qPow(newPoint.x() - obj->x(), 2.0) + qPow(newPoint.y() - obj->y(), 2.0));
////        if(qIsNaN(ab) || qIsInf(ab))
////          qDebug() << QString("newPoint.x()=%1  obj->x()=%2  newPoint.y()=%3  obj->y()=%4")
////                      .arg(newPoint.x()).arg(obj->x()).arg(newPoint.y()).arg(obj->y());
//        if(ab < min_distance)
//        {
//          min_distance = ab;
//          nearestBeacon = (SvMapObjectBeaconPlanned*)obj;
//        }
        
//        break;
//      }
        
      case motRadius:
        
        mapRadius = (SvMapObjectRadius*)obj;
        mapRadius->setPos(newPoint);
        
        break;
    }
  }
//  qreal distance = geo1_geo2_distance(lon, lat, nearestBeacon->lon(), nearestBeacon->lat());
  // находим ближайший буй
  SvMapObjectBeaconPlanned* nearestBeacon = findNearestPlanned(newPoint);
  if(airplane && mapRadius && nearestBeacon)
    mapRadius->setup(airplane, nearestBeacon); // nearestBeacon->pos().x(), nearestBeacon->pos().y(), distance, angle);
  
  // новая точка пути 
  SvMapObjectDirection* dir = new SvMapObjectDirection(this, lon, lat);
  dir->setPos(newPoint);
  dir->setAngle(angle);
  dir->setBrush(QBrush(QColor(0, 255, 100, 200))); //_trackBrush
  dir->setPen(QColor(0, 255, 100, 200).dark(), Qt::SolidLine, 1);
  scene->addMapObject(dir);
  
  _trackPoints.insert(geo::POSITION(lon, lat, angle, QDateTime::currentDateTime()), dir); // scene->addEllipse(newPoint.x(), newPoint.y(), 3, 3, _trackPen, _trackBrush));
  
  // лишние точки убираем 
  foreach (geo::POSITION geopos, _trackPoints.keys()) {
    if(geopos.utc().secsTo(QDateTime::currentDateTime()) > _track_secs)
    {
      scene->removeMapObject(_trackPoints.value(geopos)); // MapObject(_trackPoints.value(geo));
      _trackPoints.value(geopos)->~QGraphicsItem();
      _trackPoints.remove(geopos);      
    }
  }
  
  if(_trackSelfVessel)
    centerAirplane();
  **/
  scene->update();
}

SvMapObjectBeaconPlanned* area::SvArea::findNearestPlanned(QPointF fromPoint)
{
  SvMapObjectBeaconPlanned* nearestBeacon = nullptr;
  qreal min_distance = 0xFFFFFFFF;
  
  foreach(SvMapObject *obj, scene->mapObjects()) {
    if(obj->type() == motBeaconPlanned) {
      qreal ab = qSqrt(qPow(fromPoint.x() - obj->x(), 2.0) + qPow(fromPoint.y() - obj->y(), 2.0));

      if(ab < min_distance) {
        min_distance = ab;
        nearestBeacon = (SvMapObjectBeaconPlanned*)obj;
      }
    }
  }
  
  return nearestBeacon;
}

bool area::SvArea::readBounds(QString &fileName)
{
  
  /* читаем xml */
  QFile file(fileName);
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
      QMessageBox::warning(this, tr("Ошибка чтения XML"),
                           tr("Не удается прочитать файл %1:\n%2.")
                           .arg(fileName)
                           .arg(file.errorString()));
      return false;
  }
  
  QXmlStreamReader xml;
  xml.setDevice(&file);
  
  bool result = true;
  while (!xml.atEnd())
  {
    xml.readNextStartElement();

    if(xml.isStartElement() && xml.name() == "bounds")
    {
      bool k;
      _area_data.geo_bounds.min_lat = xml.attributes().value("minlat").toDouble(&k); result &= k;
      _area_data.geo_bounds.max_lat = xml.attributes().value("maxlat").toDouble(&k); result &= k;
      _area_data.geo_bounds.min_lon = xml.attributes().value("minlon").toDouble(&k); result &= k;
      _area_data.geo_bounds.max_lon = xml.attributes().value("maxlon").toDouble(&k); result &= k;
      
      break;
    }
  }
  
  return result;
}

bool area::SvArea::readMap(QString &fileName)
{
  bool result = false;
  
  /* читаем xml */
  QFile file(fileName);
  if (!file.open(QFile::ReadOnly | QFile::Text)) {
      QMessageBox::warning(this, tr("Ошибка чтения XML"),
                           tr("Не удается прочитать файл %1:\n%2.")
                           .arg(fileName)
                           .arg(file.errorString()));
      return false;
  }
  
  QXmlStreamReader xml;
  xml.setDevice(&file);
  
  /* читаем все точки */
  while (!xml.atEnd())
  {
    xml.readNextStartElement();

    if(xml.isStartElement() && xml.name() == "node")
    {
      bool o = true; bool k;
      quint64 node_id = xml.attributes().value("id").toLongLong(&k); o &= k;
      qreal lon = xml.attributes().value("lon").toDouble(&k); o &= k;
      qreal lat = xml.attributes().value("lat").toDouble(&k); o &= k;
      
      if(!o) {
        qDebug() << "wrong node" << xml.attributes().value("id");
        continue;
      }
      
      _area_data.NODES.insert(node_id, qMakePair(lon, lat));
    }
  }
  xml.clear();

  /* читаем пути */
  file.reset();
  file.flush();
  xml.setDevice(&file);
  
  while (!xml.atEnd())
  {
    xml.readNextStartElement();

    if(xml.isStartElement() && xml.name() == "way")
    {
      bool ok;

      quint64 way_id = xml.attributes().value("id").toLongLong(&ok);
      
      if(!ok) {
        qDebug() << "wrong way" << xml.attributes().value("id");
        continue;
      }
      
      xml.readNextStartElement();
      
      QList<QPair<qreal, qreal>> nodes_list;
      nodes_list.clear();
      
      while(!((xml.isEndElement() && xml.name() == "way") || xml.atEnd()))
      {
        if(xml.isStartElement() && xml.name() == "nd")
        {
          quint64 node_id = xml.attributes().value("ref").toLongLong(&ok);
          if(!ok)
            qDebug() << "wrong node reference" << xml.attributes().value("id");
          
          else
            nodes_list.append(_area_data.NODES.value(node_id));

        }
        xml.readNextStartElement();
        
      }
      _area_data.WAYS.insert(way_id, nodes_list);

    }
  }

  if (xml.hasError())
  {
    return false;
  }
  
  return true;
}

void area::SvArea::buttonPressed()
{
  AreaButton* button = (AreaButton*)(sender());
  
  switch (button->type()) {
    
    case bntZoomIn:
      scaleInc();
      break;
      
    case bntZoomOut:
      scaleDec();
      break;
      
    case bntZoomOriginal:
      _area_data.scale = 1;
      setScale(1);
      view->move(0, 0);
      break;
    
    case bntMoveLeft:
      moveHorizontal(-25);
      break;
      
    case bntMoveRight:
      moveHorizontal(25);
      break;
      
    case bntMoveUp:
      moveVertical(-25);
      break;
      
    case bntMoveDown:
      moveVertical(25);
      break;
      
    case bntAlignToLeftTop:
      view->move(0, 0);
//      view->repaint();
      break;
      
    case bntAddBeacon:
    {
//      foreach (AreaButton *btn, buttonsLeft) {
//        btn->setEnabled(_editMode || (btn == button));
//      }
      
//      button->setChecked(_editMode);
//      button->setIcon(!_editMode ? QIcon(":/buttons/Icons/Save.ico") : QIcon(":/buttons/Icons/Pen.ico"));
      
//      _editMode = !_editMode;
////      qDebug() << hint;
//      if(hint) {
//        hint->deleteLater();
//        hint = nullptr;
//      }
//      else {
//        hint = new QLabel("Щелкните на карте для добавления нового буя", nullptr, Qt::ToolTip);
//        hint->move(QApplication::activeWindow()->pos() + QPoint(QApplication::activeWindow()->width() / 2 - 150, 100));
//        hint->resize(300, 30);
//        hint->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
//        hint->setStyleSheet("background-color: rgb(255, 255, 127);");
//        hint->setVisible(true);
//      }
      
      break;
     }
      
    case bntReadSocket:
      
//      if(udp->isActive()) {
//        udp->stop();
        
//        while(udp->isActive()) QApplication::processEvents();
//      }
      
//      else 
//        udp->start();
      
//      button->setChecked(_socketIsActive);
      
//      _socketIsActive = !_socketIsActive;
//      qDebug() << "udp socket is NOT active" << _socketIsActive;

      break;
      
    case bntCenterOnAirplane:
      centerSelfVessel();
      break;
      
    case bntCenterSelected:
      centerSelected();
      break;
      
    case bntTrackAirplane:
      button->setChecked(_trackSelfVessel);
      _trackSelfVessel = !_trackSelfVessel;
      break;
      
    case bntTrackSelected:
      button->setChecked(_trackSelected);
      _trackSelected = !_trackSelected;
      break;      
     
    default:
      QMessageBox::information(this, "OGO", "Не назначено", QMessageBox::Ok);
      break;
  }
}

void area::SvArea::editing(bool editMode)
{
  switch (editMode) {
    case true:
      
      break;
      
    default:
      break;
  }
}

void area::SvArea::moveVertical(int val)
{
  view->move(view->x(), view->y() + val);
}

void area::SvArea::moveHorizontal(int val)
{
  view->move(view->x() + val, view->y());
}

void area::SvArea::mousePressed(QMouseEvent * event)
{
//  if(!event->modifiers().testFlag(Qt::ShiftModifier))
//    return;

  _mouseButton = event->button();
  _mousePos = event->pos();
  
//  if(_editMode && (_mouseButton == Qt::LeftButton))
//  {
//    QPointF coord = point2geo(&_area_data, _mousePos.x(), _mousePos.y());
    
//    BEACONEDITOR_UI = new SvBeaconEditor(this, -1, coord.x(), coord.y());
//    if(BEACONEDITOR_UI->exec() == QDialog::Accepted)
//    {
//      SvMapObjectBeaconPlanned* beacon = new SvMapObjectBeaconPlanned(this);
//      beacon->setId(BEACONEDITOR_UI->t_id);
//      beacon->setGeo(BEACONEDITOR_UI->t_lon, BEACONEDITOR_UI->t_lat);
//      beacon->setUid(BEACONEDITOR_UI->t_uid);
//      beacon->setDateTime(BEACONEDITOR_UI->t_date_time);
      
//      scene->addMapObject(beacon);
//      setScale(_area_data.scale);      
//    }
//    BEACONEDITOR_UI->~SvBeaconEditor();
    
//  }
}

void area::SvArea::mouseReleased(QMouseEvent * event)
{
  _mouseButton = Qt::NoButton;
}

void area::SvArea::mouseMoved(QMouseEvent * event)
{
  switch (_mouseButton) {
    case Qt::NoButton:
      break;
      
    case Qt::LeftButton:
      view->move((view->x() + event->pos().x() - _mousePos.x()), view->y() + event->pos().y() - _mousePos.y());
      break;
  }

  QWidget::mouseMoveEvent(event);
  
}

void area::SvArea::scaleInc()
{
  if(_area_data.scale >= 16)
    return;
  
  _area_data.scale *= 1.25;
  setScale(_area_data.scale);
  
  if(_trackSelfVessel)
    centerSelfVessel();
  
  if(_trackSelected)
    centerSelected();
  
}

void area::SvArea::scaleDec()
{
  if(_area_data.scale <= 0.5)
    return;
  
  _area_data.scale /= 1.25;

  setScale(_area_data.scale);
  
  if(_trackSelfVessel)
    centerSelfVessel();
  
  if(_trackSelected)
    centerSelected();
}

void area::SvArea::setScale(qreal scale)
{
  _area_data.area_curr_size.setWidth(_area_data.area_base_size.width() * _area_data.scale);
  _area_data.area_curr_size.setHeight(_area_data.area_base_size.height() * _area_data.scale);
  
  _area_data.koeff.lon = qreal(_area_data.area_curr_size.width())  / (_area_data.geo_bounds.max_lon - _area_data.geo_bounds.min_lon);
  _area_data.koeff.lat = qreal(_area_data.area_curr_size.height()) / (_area_data.geo_bounds.max_lat - _area_data.geo_bounds.min_lat);
  
  /* подбираем шаг сетки */
  updateGridStep();
  
  /* обновляем сцену */
  scene->setSceneRect(QRectF(0, 0, _area_data.area_curr_size.width() + BORDER_WIDTH * 2, _area_data.area_curr_size.height() + BORDER_WIDTH * 2));

  /* квадратики по углам */
//  qreal cs = _area_data.gridCellStep;
//  scene->ltsq->setPos(BORDER_WIDTH + 2, BORDER_WIDTH + 2);                                                                                     
//  scene->rtsq->setPos(_area_data.area_curr_size.width() - cs - 2 + BORDER_WIDTH, BORDER_WIDTH + 2);                                           
//  scene->lbsq->setPos(BORDER_WIDTH + 2, _area_data.area_curr_size.height() - cs - 2 + BORDER_WIDTH);                                          
//  scene->rbsq->setPos(_area_data.area_curr_size.width() - cs - 2 + BORDER_WIDTH, _area_data.area_curr_size.height() - cs - 2 + BORDER_WIDTH);

  view->resize(_area_data.area_curr_size.width() + BORDER_WIDTH * 2, _area_data.area_curr_size.height() + BORDER_WIDTH * 2);
  
  
  // пересчитываем координаты всех объектов 
  foreach (SvMapObject* item, scene->mapObjects()) {
    
    switch (item->type()) {
      case motVessel:
      case motSelfVessel:
      case motNavtek:
        
        scene->setMapObjectPos(item, item->geoPosition());

        break;
        
      default:
        break;
    }
  }
  
}

void area::SvArea::centerSelfVessel()
{
  /* центруем экран на свое судно */
  SvMapObject* found = nullptr;
  foreach (SvMapObject* item, scene->mapObjects()) {
    
    if(item->type() != motSelfVessel)
      continue;

    found = item;
    break;
  }
  
  if(found)
    centerTo(found->pos());
  
}

void area::SvArea::centerSelected()
{
  /* центруем экран на выделенный объект */
  SvMapObject* found = nullptr;
  foreach (SvMapObject* item, scene->mapObjects()) {
    
    if(!item->selection())
      continue;

    found = item;
    break;
  }
  
  if(found)
    centerTo(found->pos());
  
}

void area::SvArea::centerTo(QPointF pos)
{
  /* находим середину виджета */
  qreal xc = parentWidget()->width() / 2;
  qreal yc = parentWidget()->height() / 2;
  
  view->move(xc - pos.x(), yc - pos.y());
}

void area::SvArea::updateGridStep()
{
  // находим расстояние до следующей линии на карте с учетом масштаба, шага сетки
  // для горизонтальных линий направление 180 градусов
  geo::GEOPOSITION pos = geo::GEOPOSITION(_area_data.geo_bounds.min_lon, _area_data.geo_bounds.max_lat, 180, 0);
  pos = geo::get_next_coordinates(pos, MINOR_VGRID_DISTANCE / _area_data.scale);
  
  _area_data.gridYstep = geo2point(&_area_data, pos.longtitude, pos.latitude).y(); // - BORDER_WIDTH;
  
  // для вертикальных линий направление 90 градусов
  pos = geo::GEOPOSITION(_area_data.geo_bounds.min_lon, _area_data.geo_bounds.max_lat, 90, 0);
  pos = geo::get_next_coordinates(pos, MINOR_VGRID_DISTANCE / _area_data.scale);
    
  _area_data.gridXstep = geo2point(&_area_data, pos.longtitude, pos.latitude).x(); // - BORDER_WIDTH;

}

/** ****** AREA SCENE ****** **/
area::SvAreaScene::SvAreaScene(AREA_DATA *area_data)
{
  _area_data = area_data;
  
  qreal cs = _area_data->gridCellStep;

  //  lttxt = addText("1");
//  rttxt = addText("2");
//  lbtxt = addText("3");
//  rbtxt = addText("4");
  
//  rbsq = addRect(0, 0, cs, cs, QPen(QColor(0, 0, 0, 100)), QBrush(QColor(0, 0, 0, 100))); //(2, 2, cs, cs);
//  ltsq = addRect(0, 0, cs, cs, QPen(QColor(0, 0, 0, 100)), QBrush(QColor(0, 0, 0, 100))); //(_area_data->area_curr_size.width() - cs - 4, 2, cs, cs);                                           
//  rtsq = addRect(0, 0, cs, cs, QPen(QColor(0, 0, 0, 100)), QBrush(QColor(0, 0, 0, 100))); //(2, _area_data->area_curr_size.height() - cs - 4, cs, cs);                                       
//  lbsq = addRect(0, 0, cs, cs, QPen(QColor(0, 0, 0, 100)), QBrush(QColor(0, 0, 0, 100))); //(_area_data->area_curr_size.width() - cs - 4, _area_data->area_curr_size.height() - cs - 4, cs, cs);
      
//  resizeScene();
}

void area::SvAreaScene::setMapObjectPos(SvMapObject* mapObject, const geo::GEOPOSITION& geopos)
{
  QPointF new_pos = geo2point(_area_data, geopos.longtitude, geopos.latitude);
  mapObject->setPos(new_pos.x() + BORDER_WIDTH, new_pos.y() + BORDER_WIDTH);
  mapObject->setRotation(geopos.course);
  
  if(mapObject->identifier())
    mapObject->identifier()->setPos(new_pos.x() + BORDER_WIDTH, new_pos.y() + BORDER_WIDTH);
  
  if(!(mapObject->isSelected() && mapObject->selection()))
    return;
  
  mapObject->selection()->setPos(new_pos.x() + BORDER_WIDTH, new_pos.y() + BORDER_WIDTH);  
  
}

/** ****** AREA VIEW ******* **/
area::SvAreaView::SvAreaView(QWidget *parent, AREA_DATA *area_data) :
  QGraphicsView(parent)
{
  _area_data = area_data;
  
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  
//  this->setAlignment(Qt::AlignTop | Qt::AlignLeft); //!!
//  this->setAlignment(Qt::AlignCenter | Qt::AlignHCenter);
    
  setRenderHint(QPainter::Antialiasing, true);
  setDragMode(QGraphicsView::ScrollHandDrag); // RubberBandDrag  NoDrag  );
  setOptimizationFlags(QGraphicsView::DontSavePainterState);
  setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
  setStyleSheet("background-color: rgb(255, 255, 245)");
  setFocusPolicy(Qt::NoFocus);
  setCursor(Qt::ArrowCursor);
  
  _gridBorderColor = QColor(0x5672d8); // 0, 0, 255, 255);
  _gridMinorColor = QColor(0, 0, 255, 30);
  _gridMajorColor = QColor(0, 0, 255, 50);
  _mapCoastColor = QColor(0, 85, 127, 150);
  _fontColor = QColor(0, 0, 255, 200);
  _scaleColor = QColor("navy");

  _penBorder.setColor(_gridBorderColor);
  _penBorder.setStyle(Qt::SolidLine);
  _penBorder.setWidth(1);

  _penMajorLine.setColor(_gridMajorColor);
  _penMajorLine.setStyle(Qt::DashLine);
  _penMajorLine.setWidth(1);

  _penMinorLine.setColor(_gridMinorColor);
  _penMinorLine.setStyle(Qt::DotLine);
  _penMinorLine.setWidth(1);

  _pen_coastLine.setColor(_mapCoastColor);
  _pen_coastLine.setStyle(Qt::SolidLine);
  _pen_coastLine.setWidth(2);  
  
  _pen_scale.setColor(_scaleColor);
  _pen_scale.setStyle(Qt::SolidLine);
  _pen_scale.setWidth(1); 
  
  setVisible(true);
  
}


void area::SvAreaView::drawBackground(QPainter *painter, const QRectF &r)
{
  painter->setPen(_penBorder);
  painter->setBrush(QBrush());
  painter->drawRect(QRect(BORDER_WIDTH, BORDER_WIDTH, _area_data->area_curr_size.width(), _area_data->area_curr_size.height()));
  
  /* вертикальные линии */
  int i = 0;
  qreal x = BORDER_WIDTH; //_area_data->geo_bounds.min_lon;
  
  while(x < _area_data->area_curr_size.width() + BORDER_WIDTH) { 
    
    painter->setPen((i % 5 == 0) ? _penMajorLine : _penMinorLine);
    painter->drawLine(x, BORDER_WIDTH, x, _area_data->area_curr_size.height() + BORDER_WIDTH);
    
    i ++;
    x += _area_data->gridXstep;
 
  }

  /* горизонтальные линии */
  i = 0;
  qreal y = BORDER_WIDTH;
  
  while(y < _area_data->area_curr_size.height() + BORDER_WIDTH) {
    
    painter->setPen((i % 5 == 0) ? _penMajorLine : _penMinorLine);
    painter->drawLine(BORDER_WIDTH, y, _area_data->area_curr_size.width() + BORDER_WIDTH, y);
    
    i ++;
    y += _area_data->gridYstep;
    
  }

  /* рисуем шкалу масштаба */
  qreal x1 = _area_data->gridXstep;
  qreal y1 = _area_data->gridYstep * 2;
  qreal y2 = _area_data->gridYstep * 2 + 5;
   
  QPainterPath pathScale;
                      
  pathScale.moveTo(x1 * 5 + BORDER_WIDTH, y1 + BORDER_WIDTH);
  pathScale.lineTo(x1 * 15 + BORDER_WIDTH, y1 + BORDER_WIDTH);
  
  for(int i = 5; i < 16; i++){
    pathScale.moveTo(x1 * i + BORDER_WIDTH, y1 + BORDER_WIDTH);
    pathScale.lineTo(x1 * i + BORDER_WIDTH, y2 + BORDER_WIDTH + (i%5 == 0 ? 5 : 0));
  }
  
  QFont font("Courier New", 9, QFont::Normal);
  QString s = QString("%1%2").arg(qreal(MINOR_VGRID_DISTANCE * 10) / _area_data->scale / CMU.MetersCount, 0, 'f', 1)
              .arg(CMU.DistanceDesign);
  
  pathScale.addText(x1 * 5 + BORDER_WIDTH + 20, y2 + 16 + BORDER_WIDTH, font, s);
  
  painter->setPen(_pen_scale);
  painter->setBrush(QBrush());
  painter->drawPath(pathScale);
  
  
  /* заголовок карты */
  x1 = BORDER_WIDTH;
  y1 = BORDER_WIDTH - 6;
  y2 = _area_data->area_curr_size.height() + BORDER_WIDTH + 12;
  
  QPainterPath pathHeader;
                      
  pathHeader.moveTo(x1, y1);

//  font.setPixelSize(8);
  font.setBold(false);
  pathHeader.addText(x1, y1, font, QString("%1").arg(_area_data->area_name));
  pathHeader.addText(x1, y2, font, QString("Широта: %2 - %3 (%4%5). Долгота: %6 - %7 (%8%9)")
                                             .arg(_area_data->geo_bounds.min_lat, 0, 'f', 4)
                                             .arg(_area_data->geo_bounds.max_lat, 0, 'f', 4)
                                             .arg(_area_data->lat2lat_distance / CMU.MetersCount, 0, 'f', 2)
                                             .arg(CMU.DistanceDesign)
                                             .arg(_area_data->geo_bounds.min_lon, 0, 'f', 4)
                                             .arg(_area_data->geo_bounds.max_lon, 0, 'f', 4)
                                             .arg(_area_data->lon2lon_distance / CMU.MetersCount, 0, 'f', 2)
                                             .arg(CMU.DistanceDesign));

  
  
  
  painter->setPen(_pen_scale);
  painter->setBrush(QBrush());
  painter->drawPath(pathHeader);
  
}

void area::SvAreaView::mousePressEvent(QMouseEvent * event)
{
  emit mousePressed(event);
  QGraphicsView::mousePressEvent(event);
}

void area::SvAreaView::mouseReleaseEvent(QMouseEvent * event)
{
  emit mouseReleased(event);
  QGraphicsView::mouseReleaseEvent(event);
}

void area::SvAreaView::mouseMoveEvent(QMouseEvent * event)
{
  emit mouseMoved(event);
  QGraphicsView::mouseMoveEvent(event);
//  update();
}

//void area::SvAreaView::mouseDoubleClickEvent(QMouseEvent * event)
//{
  
//}

/** ******** RULERS ****** **/
area::SvHRuler::SvHRuler(QWidget *parent, float *scale, QSize* areaSize)
{


}

void area::SvHRuler::paintEvent(QPaintEvent * event)
{

}

area::SvVRuler::SvVRuler(QWidget *parent, float *scale, QSize *areaSize)
{

}

void area::SvVRuler::paintEvent(QPaintEvent * event)
{

}
