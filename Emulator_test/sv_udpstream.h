#ifndef SVUDPSTREAM_H
#define SVUDPSTREAM_H

#include <QObject>
#include <QUdpSocket>
#include <QThread>
#include <QByteArray>
#include <QHostAddress>
#include <random>
#include <QTime>
#include <qmath.h>

//#include "../beacon_2/converting.h"

struct Data {
    char c1;
    char c2;
    qreal lon;
    qreal lat;
    int angle;
};


class SvUdpWriter : public QThread
{
    Q_OBJECT
  
  public:
    SvUdpWriter(QString ip, quint16 port, 
                quint16 speed, quint16 maxAngle, 
                quint16 const_course_length, quint16 timeout, 
                QObject *parent = 0);
    ~SvUdpWriter();

    bool isAsctive() { return _isActive; }
    void setTimeout(int timeout) { _timeout = timeout; }
    
  protected:
    void run() Q_DECL_OVERRIDE;

  private:
    QUdpSocket *udp;
    bool _isActive = false;
    QString _ip;
    quint16 _port;
  
    quint16 _speed;
    quint16 _maxAngle;
    Data _data;
    
    qreal _min_lon;
    qreal _max_lon;
    qreal _min_lat;
    qreal _max_lat;
    
    qreal _segment_length;
    qreal _const_course_length;
    qreal _lon1m;
    qreal _lat1m;
    
    int _timeout = 1000;
    
    qreal inline lonOffset();
    qreal inline latOffset();
    
    int quarter();
    void normal_angle();
    
  public slots:
    void stop() { _isActive = false; }
    
};


class SvUdpReader : public QThread
{
    Q_OBJECT
  
  public:
    explicit SvUdpReader(const QString &ip, const quint16 port, QObject *parent = 0);
    ~SvUdpReader();

  bool isActive() { return _isActive; }
  
  private:
    QUdpSocket* _socket;
    QByteArray _buff;
    bool _isActive = false;
    
    Data _data;
    
    QString _ip;
    quint16 _port;
    
    void run() Q_DECL_OVERRIDE ;
    
  signals:
    void dataReaded(qreal lon, qreal lat, int angle);

  public slots:
    void stop() { _isActive = false; }

};

#endif // SVUDPSTREAM_H
