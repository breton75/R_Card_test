#ifndef MAPOBJECTS_H
#define MAPOBJECTS_H

#include <QObject>
#include <QtWidgets>
#include <QRectF>
#include <QColor>
#include <QWidget>
#include <QMap>
#include <QColor>
#include <QGraphicsItem>
#include <QString>
#include <QGraphicsSceneHoverEvent>

#include <QGradient>
#include <QRadialGradient>

//#include "sv_mapobjects.h"
#include "../../svlib/sv_sqlite.h"
//#include "sv_beaconeditor.h"
#include "geo.h"
//#include "sv_vessel.h"

enum SvMapObjectTypes
{
  motVessel = 65537,
  motSelfVessel,
  motNavtek,
  motNode,
  motRadius,
  motDirection,
  motZone,
  motBeaconPlanned,
  motBeaconActive,
  motAirplane,
  motSelection,
  motIdentifier
};

#define DEFAULT_SELECTION_COLOR 0xffef0b

#define INACTIVE_VESSEL_COLOR "firebrick"   // QColor("firebrick")
#define OUTDATED_VESSEL_COLOR "lightgrey"
#define OUTDATED_VESSEL_PEN_COLOR "darkslategrey"
#define DEFAULT_VESSEL_COLOR "mediumturquoise"
#define DEFAULT_VESSEL_PEN_COLOR "darkslategrey"

class SvSignalHandler : public QObject
{
  Q_OBJECT
  
public:
  explicit SvSignalHandler() { }
  void mapObjectMouseDoubleClicked(int mapObjectId) { emit mouseDoubleClick(mapObjectId); }
  
signals:
  void mouseDoubleClick(int id);
  
};

class SvMapObjectSelection;
class SvMapObjectIdentifier;


class SvMapObject : public QGraphicsItem
{
//  static QColor defaultSelectionColor = QColor(0xffef0b);
  
  public:
  
    explicit SvMapObject(QWidget* parent);
    ~SvMapObject();
    
    virtual int type() const { return -1; }
    
    virtual int id() { return _id; }
    virtual void setId(int id) { _id = id; }
  
    virtual void setOutdated(bool isOutdate) { }
    virtual void setActive(bool isActive) { }
  
    geo::GEOPOSITION geoPosition() { return _geo_position; }
    void setGeoPosition(const geo::GEOPOSITION& geopos) { _geo_position = geopos; }
    
    SvSignalHandler signalHandler;
    
    QPainterPath* path() const { return _path; }
    
    QBrush brush() { return _brush; }
    virtual void setBrush(const QBrush &brush, const QColor &selectionColor = QColor(DEFAULT_SELECTION_COLOR)) {
      _brush = QBrush(brush);
      _selectionColor = selectionColor;
    }
  
    virtual void setBrush(const QColor &brushColor, const Qt::BrushStyle brushStyle = Qt::SolidPattern,
                          const QColor &selectionColor = QColor(DEFAULT_SELECTION_COLOR)) {
      _brush = QBrush(brushColor, brushStyle);
      _selectionColor = selectionColor;
    }
    
    QPen pen() const { return _pen; }
    virtual void setPen(const QPen &pen) {
      _pen = QPen(pen);
    }
    
    virtual void setPen(const QColor &penColor, const Qt::PenStyle penStyle = Qt::SolidLine, const int penWidth = 2) {
      _pen = QPen(penStyle);
      _pen.setColor(penColor);
      _pen.setWidth(penWidth);         
    }
  
    QColor selectionColor() const { return _selectionColor; }
    virtual void setSelectionColor(const QColor &selectionColor) {
      _selectionColor = selectionColor;
    }
  
    bool isEditable() const { return _isEditable; }
    void setEditable(const bool editable) { _isEditable = editable; }

    SvMapObjectSelection* selection() { return _selection; }
    void setSelection(SvMapObjectSelection* selection) { _selection = selection; }
    void deleteSelection() {
      if(_selection)
        delete _selection; 
      
      _selection = nullptr; 
    }
    
    SvMapObjectIdentifier* identifier() { return _identifier; }
    void setIdentifier(SvMapObjectIdentifier* identifier) { _identifier = identifier; }
    
    virtual QRectF boundingRect() const Q_DECL_OVERRIDE;
    virtual QPainterPath shape() const Q_DECL_OVERRIDE;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;
    
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *) Q_DECL_OVERRIDE;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *) Q_DECL_OVERRIDE;
  
  private:
    geo::GEOPOSITION _geo_position;
    
    int _id = -1;
    
    bool _isEditable = false;
    QPainterPath* _path = nullptr;
    
    QBrush _brush = QBrush();
    QPen _pen = QPen();
    
    QColor _selectionColor = QColor(DEFAULT_SELECTION_COLOR);
    
    SvMapObjectSelection* _selection = nullptr;
    SvMapObjectIdentifier* _identifier = nullptr;
    
 
};


/** ************************************************/
/** ************** AIRPLANE ************************/
/** ************************************************/

class SvMapObjectAirplane : public SvMapObject/*, public QObject*/
{
  const QPointF points[22] =
  {
    QPointF(0.0, -15.0), 
    QPointF(2.0, -13.0), 
    QPointF(2.0, -2.0),  
    QPointF(17.0, 3.0),  
    QPointF(17.0, 5.0),  
    QPointF(16.0, 6.0),  
    QPointF(2.0, 5.0),   
    QPointF(2.0, 11.0),  
    QPointF(7.0, 14.0),  
    QPointF(7.0, 15.0),  
    QPointF(2.0, 14.0),  
    QPointF(0.0, 15.0),  
    QPointF(-2.0, 14.0), 
    QPointF(-7.0, 15.0), 
    QPointF(-7.0, 14.0), 
    QPointF(-2.0, 11.0), 
    QPointF(-2.0, 5.0),  
    QPointF(-16.0, 6.0), 
    QPointF(-17.0, 5.0), 
    QPointF(-17.0, 3.0), 
    QPointF(-2.0, -2.0), 
    QPointF(-2.0, -13.0) 
  };
  
  public:
    explicit SvMapObjectAirplane(QWidget* parent);
    
    int type() const { return motVessel; }

};


/** ************************************************/
/** ************** BEACON **************************/
/** ************************************************/

class SvMapObjectBeaconAbstract : public SvMapObject
{
  public:
    
    explicit SvMapObjectBeaconAbstract(QWidget* parent):SvMapObject(parent) {  }
  
//    explicit SvMapObjectBeaconAbstract(QWidget* parent, const qreal lon, const qreal lat,
//                                       const int id, const QString &uid, const QDateTime &dateTime);
  
    int id() const { return _id; }
    void setId(const int id) { _id = id; }
    
    QString uid() const { return _uid; }
    void setUid(const QString &uid) { _uid = uid; }
    
    QDateTime date_time() const { return _date_time; }
    void setDateTime(const QDateTime date_time) { _date_time = date_time; }
    
private:
    int _id;
    QString _uid;
    QDateTime _date_time;
    
};

class SvMapObjectBeaconPlanned : public SvMapObjectBeaconAbstract
{
  const QPointF _points[6] =
    {
      QPointF(0.0, 0.0),    // 0 
      QPointF(-3.0, -10.0), // 1 
      QPointF(3.0, -10.0),  // 2 
      QPointF(0.0, -22.0)   // 3 
    } ;
  
  public:
    explicit SvMapObjectBeaconPlanned(QWidget* parent);
    
    int type() const { return motBeaconPlanned; }
    
    QAction *removeAction;
    QAction* editAction;

    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
};

class SvMapObjectBeaconActive : public SvMapObjectBeaconAbstract
{
  const QPointF _points[6] =
    {
      QPointF(0.0, 0.0),    // 0 
      QPointF(-3.0, -10.0), // 1 
      QPointF(3.0, -10.0),  // 2 
      QPointF(0.0, -22.0)   // 3 
    } ;
  
  public:
    explicit SvMapObjectBeaconActive(QWidget* parent, int number = -1);

    int type() const { return motBeaconActive; }
   
    int number() const { return _number; }
    void setNumber(const int number) { _number = number; }
    
    bool isCovereged() const { return _isCovereged; }
    void setCovereged(const bool covereged) { _isCovereged = covereged; }
    
    qreal signalLevel() const { return _signalLevel; }
    void setSignalLevel(const qreal signalLevel) { _signalLevel = signalLevel; }
    
    void setColors(const QColor &defaultColor, const QColor &signalColor, const QColor &textColor)
    {
      _defaultColor = defaultColor;
      _signalColor = signalColor;
      _textColor = textColor;
    }
    
  private:
    QPainterPath _pathNum;
//    QPainterPath _pathSignal;

    QGradient gradient = QGradient();

    QColor _defaultColor = QColor(0x216fb8);
    QColor _signalColor = QColor(0x14c546);
    QColor _textColor = Qt::white; // QColor(0xffef0b);
    
    int _number;
    bool _isCovereged;
    
    qreal _signalLevel = 0.0;
    
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) Q_DECL_OVERRIDE;

};

class SvMapObjectRadius : public SvMapObject
{
  public:
    explicit SvMapObjectRadius(QWidget* parent);
    
    int type() const { return motRadius; }
    qreal radius() const { return _radius; }
    void setup(SvMapObjectAirplane *airplane, SvMapObjectBeaconPlanned *beacon);
    
    
  private:
    QColor _brushStyle = Qt::NoBrush;
    QColor _penCircleColor = QColor(170, 170, 0, 170);
    QColor _penRadiusColor = QColor(170, 170, 0, 255);
    QColor _penCourseColor = QColor(30, 130, 230, 255);
    QColor _penTextColor = QColor(0, 0, 0, 255);
    Qt::PenStyle _penCircleStyle = Qt::DashLine;
    Qt::PenStyle _penRadiusStyle = Qt::DashDotDotLine;
    Qt::PenStyle _penCourseStyle = Qt::DotLine;
    Qt::PenStyle _penTextStyle = Qt::SolidLine;
    
    QPainterPath _pathCircle;
    QPainterPath _pathRadius;
    QPainterPath _pathCourse;
    QPainterPath _pathText;
    
    qreal _radius;
    qreal _course;
    
    QRectF boundingRect() const Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) Q_DECL_OVERRIDE;
    
};

/**
class SvMapObjectDirection : public SvMapObject
{
  public:
    explicit SvMapObjectDirection(QWidget* parent, qreal lon, qreal lat);
    
    int type() const { return motDirection; }
     
};
**/


/** ********************************************** **/
/** ***************** VESSEL ********************* **/
/** ********************************************** **/

class SvMapObjectVesselAbstract : public SvMapObject
{
  public:
    
    explicit SvMapObjectVesselAbstract(QWidget* parent, int id):
      SvMapObject(parent) { setId(id); }
  
//    int id() const { return _id; }
     
//  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE { qDebug() << 11; signalHandler.mapObjectMouseDoubleClicked(id()); }
  
//  virtual void setOutdated(bool isOutdate) = 0;
//  virtual void setActive(bool isActive) = 0;
  
//    vsl::SvVessel* vessel() { return _vessel; }
//    int vesselId() const { return _vessel->id; }
    
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    
private:
//    vsl::SvVessel* _vessel = nullptr;
//  int _id = -1;
    
};

class SvMapObjectOtherVessel : public SvMapObjectVesselAbstract
{
  const QPointF _points[7] =
  {
    QPointF(0.0, 0.0),    // 0 
    QPointF(0.0, -10.0),  // 1
    QPointF(-5.0, -3.0),  // 2
    QPointF(-5.0, 10.0),  // 3
    QPointF(0.0, 6.0),    // 4
    QPointF(5.0, 10.0),   // 5
    QPointF(5.0, -3.0)    // 6
  } ;
  
  QPen _default_pen = QPen(QBrush(QColor(DEFAULT_VESSEL_PEN_COLOR)), 2, Qt::SolidLine);
  QBrush _default_brush = QBrush(QColor(DEFAULT_VESSEL_COLOR), Qt::SolidPattern);
  
  public:
    explicit SvMapObjectOtherVessel(QWidget* parent, int id);
    
    int type() const { return motVessel; }
    
    void setOutdated(bool isOutdate) 
    { 
      QPen p(QBrush(QColor(OUTDATED_VESSEL_PEN_COLOR)), 1, Qt::SolidLine);
      QBrush b(QColor(OUTDATED_VESSEL_COLOR), Qt::SolidPattern);
      
      setBrush(isOutdate ? b : _default_brush);
      setPen(isOutdate ? p : _default_pen);
      this->update(this->boundingRect());
    }
    
    void setActive(bool isActive) 
    { 
      QPen p(QBrush(QColor(INACTIVE_VESSEL_COLOR)), 2);
      QBrush b(Qt::NoBrush);
      
      setBrush(isActive ? brush() : b);
      setPen(isActive ? pen() : p);
      this->update(this->boundingRect());
    }
    
    QAction *removeAction;
    QAction *editAction;

//    SvSignalHandler signalHandler;
    
//    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
//    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
};

class SvMapObjectSelfVessel : public SvMapObjectVesselAbstract
{
  const QPointF _points[7] =
  {
    QPointF(0.0, 0.0),    // 0 
    QPointF(0.0, -10.0),  // 1
    QPointF(-5.0, -3.0),  // 2
    QPointF(-5.0, 10.0),  // 3
    QPointF(0.0, 6.0),    // 4
    QPointF(5.0, 10.0),   // 5
    QPointF(5.0, -3.0)    // 6
  } ;
  
  QPen _default_pen = QPen(QBrush(QColor("darkslategrey")), 1, Qt::SolidLine);
  QBrush _default_brush = QBrush(QColor("dodgerblue"), Qt::SolidPattern); // orangered
  
  public:
    explicit SvMapObjectSelfVessel(QWidget* parent, int id);
    
    int type() const { return motSelfVessel; }
    
//    void setOutdated(bool isOutdate) { setBrush(isOutdate ? QColor(Qt::gray) : QColor("tomato")); }
    
    QAction *removeAction;
    QAction *editAction;

//    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
//    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
};


/** ********************************************** **/
/** ***************** SELECTION ****************** **/
/** ********************************************** **/

class SvMapObjectSelection : public SvMapObject
{
  public:
    
    explicit SvMapObjectSelection(QWidget* parent, SvMapObject* mapobj);
    
    int type() { return motSelection; }
  
private:
  SvMapObject* _mapobj = nullptr;
  
  
};


/** ********************************************** **/
/** **************** Identifier ****************** **/
/** ********************************************** **/

class SvMapObjectIdentifier : public SvMapObject
{
  public:
    
    explicit SvMapObjectIdentifier(QWidget* parent, SvMapObject* mapobj);
  
    int type() { return motIdentifier; }
  
private:
  SvMapObject* _mapobj = nullptr;
//  int _id;
  
//  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;
  
};

#endif // MAPOBJECTS_H
