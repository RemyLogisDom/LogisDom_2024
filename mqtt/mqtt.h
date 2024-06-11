#ifndef MQTT_H
#define MQTT_H

#include <QObject>
#include <QtPlugin>
#include <QTabWidget>
#include <QTcpSocket>
#include <QThread>
#include <QTreeWidget>
#include <QComboBox>
#include <QTimer>
#include <QNetworkReply>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMutex>
#include <QtMqtt/QMqttClient>
#include "ui_mqtt.h"
#include "qjsonmodel.h"



#define Mqtt_Config_Begin "Mqtt_Config_Begin"
#define Mqtt_Config_End "Mqtt_Config_End"


class lMqtt : public QWidget
{
    Q_OBJECT
public:
    lMqtt( QWidget *parent = nullptr );
    ~lMqtt();
    QObject* getObject();
    QWidget *widgetUi();
    QWidget *getDevWidget(QString RomID);
    void setConfigFileName(const QString);
    QString getDeviceCommands(const QString &device);
    void SaveConfigStr(QString &str);
    void readConfig(const QString &configdata);
    void setLockedState(bool);
    bool lockedState = true;
    void setDeviceConfig(const QString &, const QString &);
    void setStatus(const QString);
    bool isDimmable(const QString);
    bool isManual(const QString);
    double getMaxValue(const QString);
    static QString ip2Hex(const QString &ip);
    QTimer readTimer;
    QMqttClient *m_client;
    int lastMinute = -1;
    int idle = 0;
    bool loadConfig = false;
    QWidget *ui;
    Ui::MqttUI *mui;
signals:
    void newDeviceValue(QString, QString);
    void newDevice(QString);
    void deviceSelected(QString);
    void connectionStatus(QAbstractSocket::SocketState);
private:
    QStringList devicePublishedLoaded;
    QString configFileName;
    QList<mqttDevice*> shellyDevicesMqtt, deviceSetTable;
    QString ipaddress;
    quint16 port = 80;
    QString lastStatus;
    mqttDevice *addMqttDevice(const QString &, QString Name = "");
    QJsonTreeItem *getMqttTreeItem(QString);
    QString translateValue(QString value);
    QString buildRomID(int n);
    void readMqttDevices(QList<mqttDevice*>& devList);
    QString selectedMqttPath();
    QJsonModel *jsonmodel;
    QTreeView *mqttTree;
public slots:
    void publishDevice(QString &, QString &);
private slots:
    void AddMqttParameterClick();
    void MqttParameterCopy();
    void hideLog();
    void getStatus();
    void getStatus(mqttDevice *);
    void readAllNow();
    void startMqtt();
    void displayMqttDevice(int, int);
    void ProvideContexMenu(const QPoint&);
    void on_buttonConnect_clicked();
    void on_buttonPublish_clicked();
    void on_buttonSubscribe_clicked();
    void handleMessage(const QMqttMessage &qmsg);
    void setClientPort(int p);
    void brokerConnected();
    void brokerDisconnected();
    void updateLogStateChange();
    void newMqttShellyDevice(const mqttDevice*, QString);
    void showMqttDevice(const mqttDevice*);
    void mqttLogThis(const QString &);
    void mqttLogCommand(const QString &);
    void mqttPathSelected();
    void mqttLogChanged();
    void clearMqttLog();
    void setLockedState();
    void newValue(QString, QString);
};


#endif

