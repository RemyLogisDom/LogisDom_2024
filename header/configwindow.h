/****************************************************************************
**
** Copyright (C) 2022 Remy CARISIO.
**
** This file is part of the LogisDom project from Remy CARISIO.
** remy.carisio@orange.fr   http://logisdom.fr
** LogisDom is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.

** LogisDom is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.

** You should have received a copy of the GNU General Public License
** along with LogisDom.  If not, see <https://www.gnu.org/licenses/>
**
****************************************************************************/



#ifndef CONFIGWINDOW_H
#define CONFIGWINDOW_H

#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QTreeWidgetItem>
#include <QtCore>
#include <QtGui>

#include "ui_configgui.h"
#include "backup.h"
#include "mqtt/mqtt.h"
#include "pngthread.h"
#include "devfinder.h"
#include "sendmailthread.h"
#include "sendsmsthread.h"
#include "../interface.h"

class net1wire;
class Server;
class configwindow;
class configManager;
class logisdom;
class Connection;
class onewiredevice;
class ProgramEvent;
class ProgramData;
class daily;
class htmlBinder;
class treeHtmlWidget;
class eocean;
class LogisDomInterface;


struct ipInt {
    QString ip;
    uint count;
};

class configwindow : public QDialog
{
	Q_OBJECT
    friend class devfinder;
    friend class devvirtual;
    friend class devchooser;
    friend class formula;
    friend class fts800;
    friend class IconeArea;
    friend class logisdom;
    friend class net1wire;
    friend class Plot;
    friend class tableau;
    friend class x10;
    friend class enocean;
#define TabControl 0
#define TabValues 1
#define TabValuesTr 3
#define cryptKey LoGiDoMiSgReAt
#define sizeMax 10000
enum priv { Limited = 1, Admin, HtmlPreviewLevel};
enum menuhtml { Main, AllValues, Light, Temperature, Heating, Program, Config, Restart, LastMenuHtml };
public:
struct htmlSetup
{
	QString welcomeMessage;
	QString htmlHeader;
	QString htmlTitle;
	QString htmlMenu;
	QString htmlTime;
	QString htmlList;
	QString htmlDetector;
	QString htmlValue;
	QString htmlCommand;
};
	QList <htmlSetup*> htmlLayout;
	configwindow(logisdom *Parent);
	~configwindow();
	Ui::configgui ui;
// Palette setup
	QWidget setup;
    QWidget *Extra;
    QGridLayout setupLayout;
	QListWidget OneWireList;
	QTreeWidget OneWireTree;
    QLineEdit Listfilter;
	QToolButton DevicesTool;
	QCheckBox checkBoxTree;
    htmlBinder *htmlBindNetworkMenu = nullptr;
    htmlBinder *htmlBindDeviceMenu = nullptr;
    htmlBinder *htmlBindWebMenu = nullptr;
    typedef std::vector<uint8_t> byteVec_t;
    onewiredevice *NewPluginDevice(const QString &newRomID, LogisDomInterface *);
    onewiredevice *NewMqttDevice(const QString &newRomID);
    onewiredevice *NewDevice(const QString &RomID, net1wire *master);
    void UpdateRemoteDevice(const QString &configdata);
	void updateDeviceList();
    QTimer backupTimer;
    QStringList fileToBackup;
    QList<ipInt*> suspectIP;
    bool deviceexist(const QString &RomID);
	bool deviceexist(onewiredevice *device);
	onewiredevice *DeviceExist(const QString &RomID);
    onewiredevice *EoDeviceExist(const QString &RomID);
    bool devicenameexist(const QString &Name);
	onewiredevice *Devicenameexist(const QString &Name);
	QString Header;
	int DeviceCount();
	bool isUploading();
    QDateTime nextBackup;
	void GetAllMenuHtml(QString *str, QString &ID, int Privilege);
	void GetDevicesStr(QString &str);
	void GetDevicesScratchpad(QString &str);
    void SetDevicesScratchpad(const QString &configdata, bool save);
    void SetDevicesMainValue(const QString &configdata, bool save);
	void GetDevicesMainValue(QString &str);
	void GetMenuHtml(QString *str, QString &ID, int Privilege, QString Menu);
	net1wire *MasterExist(const QString &IPHex);
	void LectureAll();
	void LectureRecAll();
	void convert();
	void LectureVD();
	void LectureRecVD();
	void generatePng();
	void closeNet1Wire();
    void startNetwork();
	QDateTime lastPng;
	void readconfigfile(QString &configdata);
    void readPathConfig(const QString &configdata);
	void enablePathConfig();
    // Qt6 deprecated QTextCodec *textCodec;
    // Qt6 deprecated void findCodecs();
    // Qt6 deprecated void setCodec(QString codec = "");
    // Qt6 deprecated QMap<QString, QTextCodec *> codecMap;
    // Qt6 deprecated QList<QTextCodec *> codecs;
    // Qt6 deprecated QString getCodecName();
    void readPngConfig(QString &configdata);
	void readGeneralConfig(QString &configdata);
	void readHtmlConfig(QString &configdata);
	void createVirtualDevices(QString &configdata);
	void setRemoteMode(QString &configdata);
	void setServerMode(QString &configdata);
	bool hasSimulatedMaster();
	QString getRemoteHost();
	QString getDeviceName(QString &RomID);
	void SaveConfigStr(QString &str);
	bool isSimulated();
    bool saveOnChange();
    bool serverIsListening();
	configManager *configmanager;
	Server *server;
	void checkDatZipFile();
	void setNextSave();
	void replacewebid(QString &datHtml, const QString &id);
	void htmlTranslate(QString &datHtml, const QString &id);
	void valueTranslate(QString &txt);
    QMutex mutexTranslate;
    QMutex ConnectionMutex;
    onewiredevice *chooseDevice();
    sendMailThread eMailSender;
    sendSmsThread smsSender;
    int LocalTabsNumber = 2;
    lMqtt *mqttUI;
private:
    int iconPngIndex, graphPngIndex, chartPngIndex;
    QMutex mutex;
	QMutex mutexReadConfig;
	QMutex mutexNewDevice;
	QMutex mutexFifoEmpty;
    //configManager *configmanagerTr;
	logisdom *parent;
	QString toStr(int index);
    void updateBanIPList();
    void updateUsersList();
    void readconfigfilefordevice(const QString &configdata, onewiredevice *device);
	QList <net1wire*> net1wirearray;
    QList <onewiredevice*> devicePtArray;
    QHash <QString, onewiredevice*> deviceList;
    QComboBox net1wireList;
	net1wire *newmaster(QString Name, int Type);
	void addUser();
	void removeUser(int index);
	void editUser(int index);
	void addHtmlMenu(QString name);
	QString getColorCode(QString head);
	bool flagCheckZipFiles;
    onewiredevice *addVD(QString name = "");
    pngthread savePng;
    backup fileBackup;
    QString boutonBackupStr;
    QString mailPsw;
    void Lock(bool state);
    void readDevicePublish(const QString &configdata);
    QStringList deviceToPublish;    // used during startup to set publish status when device is identified or discovered, once identified, it is removed from this list
private slots:
    void supprimer();
    void addVDev();
    void ajouter();
    void removeVD();
    void loadVD();
    void choosePngFolder();
    void clearServerLog();
    void clearIPList();
    void rightclicklList(const QPoint & pos);
	void ShowDevice();
	void DisplayDevice(int index);
	void startstopserver(int state);
    void NewClient(Connection*);
    void logServer(QString);
    void newRequest(QString);
    void ClientisGone(Connection*);
    void setPalette(int index);
    void setTreePalette(QTreeWidgetItem *item, int);
    void setPalette(QListWidgetItem* item);
    void pngEnabled(int state);
	void htmlEnabled(int state);
	void pngResize(int state);
	void treeViewSwitch(int state);
    void filterChanged(QString);
    void addHtmlMenu();
	void removeHtmlMenu();
	void TcpStateConnected(net1wire*);
	void TcpStateDisonnected(net1wire*);
	void menuEventfor(QLineEdit *line, const QPoint &pos);
	void DataPathClicked();
	void ZipPathClicked();
	void BackupPathClicked();
	void HtmlPathClicked();
	void DataPathChanged();
	void ZipPathChanged();
	void BackupPathChanged();
	void HtmlPathChanged();
    // Qt 6 void codecIndexChanged(int);
    void sendHtmlMail();
    void updateNow();
    void backupNow();
    void backupFinished();
    void getHtmlLinks(QString user, QString password, QString &str);
    void backupTimerTimeOut();
    void logMailSender(QString);
    void logSMSSender(QString);
    void publishDevice();
    void publishDevice(QTreeWidgetItem *item, int);
    void unpublishDevice();
    void unpublishDevice(QTreeWidgetItem *item, int);
    void mqttPublishItemChanged(QTreeWidgetItem *item, int);
public slots:
	void DeviceConfigChanged(onewiredevice *device);
	void DeviceValueChanged(onewiredevice *device);
	void ProgHasChanged(ProgramData *prog);
	void RemovePrgEvt(ProgramData *prog);
	void dailyHasChange(daily *Daily);
	void RemoveDaily(daily *Daily);
	void HideHeatingTab(int);
    void changeSMTPServer(QString);
    void changeSMTPPassword(QString);
    void changeSMTPLogin(QString);
    void changeSMTPPort(QString);
    void changeSMTPSecure(int);
    void renameTab(int);
    void CRCCalChanged(QString);
    void CRCCLicked();
signals:
	void newDeviceAdded(onewiredevice *device);
	void DeviceRemove(onewiredevice *device);
	void DeviceChanged(onewiredevice *device);
	void ProgChanged(ProgramData *prog);
	void Allfifoempty();
protected:
};



#endif

