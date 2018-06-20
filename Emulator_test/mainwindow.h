#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileInfo>

#include "sql_defs.h"
#include "sv_area.h"
#include "../../svlib/sv_exception.h"
#include "geo.h"
#include "nmea.h"
#include "../../svlib/sv_log.h"

#include "qcustomplot.h"

#include "sv_vessel.h"
#include "sv_idevice.h"
#include "sv_gps.h"
#include "sv_ais.h"

#include "sv_vesseleditor.h"
#include "sv_navtexeditor.h"
#include "sv_serialeditor.h"
#include "sv_networkeditor.h"
#include "sv_navstateditor.h"



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QString ARG_LAG_MSGTYPE = "msgtype";
    QString ARG_AIS_RECEIVERANGE = "receive_range";
    QString ARG_NAV_RECV_FREQ = "recv_freq";
    QString ARG_ECHO_BEAM_COUNT = "beam_count";
    QString ARG_ECHO_FISH_COUNT = "fish_count";

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bool init();

    svlog::SvLog log;

private:
    enum States {
      sRunning,
      sStopping,
      sRunned,
      sStopped
    };

    Ui::MainWindow *ui;

    area::SvArea* _area;
    
    QString _db_file_name;
    QString _depth_file_name;
    
    geo::BOUNDS _bounds;

    QCPGraph* _curve;
    QCPFinancial* _scatt;
    QCPGraph* _fish;
    
//    QSqlQuery* _query;
    SvException _exception;

    QFont _font_default;
    QFont _font_inactive;
    QFont _font_nolink;
    
    SerialPortParams _lag_serial_params = SerialPortParams(idev::sdtLAG);
    SerialPortParams _ais_serial_params = SerialPortParams(idev::sdtSelfAIS);
    SerialPortParams _navtex_serial_params = SerialPortParams(idev::sdtNavtex);
    idev::NetworkParams _echo_multi_network_params = idev::NetworkParams(idev::sdtEchoMulti);
    idev::NetworkParams _echo_fish_network_params = idev::NetworkParams(idev::sdtEchoFish);
    idev::NetworkParams _gps_network_params = idev::NetworkParams(idev::sdtGPS);
    
    gps::SvGPS* _self_gps = nullptr;
    gps::SvGPSNetworkInterface* _self_gps_ifc = nullptr;
    ais::SvSelfAIS* _self_ais = nullptr;
    vsl::SvVessel* _self_vessel = nullptr;
    lag::SvLAG* _self_lag = nullptr;
    ech::SvECHOMulti* _self_multi_echo = nullptr;
    ech::SvECHOFish* _self_fish_echo = nullptr;
    nav::SvNAVTEX* _navtex = nullptr;
    
    QVariant parse_args(QString args, QString arg);
    
    ais::aisStaticVoyageData readAISStaticVoyageData(QSqlQuery* q);
    ais::aisDynamicData readAISDynamicData(QSqlQuery* q);
    gps::gpsInitParams readGPSInitParams(QSqlQuery* q, ais::aisDynamicData &dynamic_data); //, QDateTime lastUpdate);
    ais::aisNavStat readNavStat(QSqlQuery* q);
    
    int _selected_vessel_id = -1;
    States _current_state = sStopped;
    
    QTimer _timer_x10;
    QTimer _timer_stepByStep;
    
    
    bool createSelfVessel();
    vsl::SvVessel* createOtherVessel(QSqlQuery* q);
    bool createNavtex();
    
    bool read_devices_params();
    void save_devices_params();
    
    void update_NAVTEX_data();
    
    void updateGPSInitParams(gps::SvGPS* g);
    
private slots:
    void editVessel(int id);
    void editVesselNavStat(int id);
    
    void on_updateMapObjectInfo(SvMapObject* mapObject = nullptr);
    void on_updateVesselById(int id);
    void on_updateVesselActive(int id);
    
    void on_areaSelectionChanged();
    void currentVesselListItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_listVessels_doubleClicked(const QModelIndex &index);
    
    void on_bnAISEditSerialParams_clicked();
    void on_bnLAGEditSerialParams_clicked();
    void on_bnNAVTEXEditSerialParams_clicked();
    void on_bnECHOMultiEditNetworkParams_clicked();
    void on_bnECHOFishEditNetworkParams_clicked();
    
    void on_actionNewVessel_triggered();
    void on_actionEditVessel_triggered();
    void on_actionRemoveVessel_triggered();
    
    void stateChanged(States state);
    
    void on_timer_X10();
    void on_timer_stepByStep();
    void on_bnStart_pressed();
    void on_bnStart_released();
    
    void on_bnDropDynamicData_clicked();
    
    void on_bnNAVTEXAlarmSend_clicked();
    void on_bnLAGAlarmSend_clicked();
    void on_bnAISAlarmSend_clicked();
    
    void on_cbLAGMessageType_currentIndexChanged(int index);
    void on_bnEditNAVTEX_clicked();
    
    void on_bnStart_clicked();
    
    void on_echoFishUpdated(ech::HeaderFish *h);
    
    void on_cbMeasureUnits_currentIndexChanged(int index);
    
    void on_bnSetActive_clicked();
    
    void on_actionNavStat_triggered();
    
    void on_bnGPSEditNetworkParams_clicked();
    
    void on_bnStep_clicked();
    
signals:
  void newState(States state);
  
  void setMultiplier(quint32 multiplier);
  void setStepByStep(bool step_by_step);
  void nextStep();
  
  void startGPSEmulation(quint32 msecs);
  void startAISEmulation(quint32 msecs);
  void startLAGEmulation(quint32 msecs);
  void startNAVTEXEmulation(quint32 msecs);
  void startECHOMultiEmulation(quint32 msecs);
  void startECHOFishEmulation(quint32 msecs);
  
  void stopGPSEmulation();
  void stopAISEmulation();
  void stopLAGEmulation();
  void stopNAVTEXEmulation();
  void stopECHOMultiEmulation();
  void stopECHOFishEmulation();
  
  void new_lag_message_type(lag::MessageType msgtype);
};

#endif // MAINWINDOW_H
