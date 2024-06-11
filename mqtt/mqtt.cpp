#include <QtWidgets>

#include "mqtt.h"
#include "logisdom.h"
#include "globalvar.h"
#include "simplecrypt.h"


lMqtt::lMqtt(QWidget *parent) : QWidget(parent)
{
    ui = new QWidget();
    QGridLayout *layout = new QGridLayout(ui);
    QWidget *w = new QWidget();
    mui = new Ui::MqttUI;
    mui->setupUi(w);
    layout->addWidget(w);
    mui->logTxtMqtt->hide();
    jsonmodel = new QJsonModel;
    mqttTree = new QTreeView(mui->frame);
    mui->gridLayout_5->removeWidget(mui->tableWidgetMqtt);
    mui->tableWidgetMqtt->hide();
    mui->gridLayout_5->addWidget(mqttTree, 0, 0, 1, 1);
    mqttTree->setModel(jsonmodel);
    m_client = new QMqttClient(this);
    m_client->setHostname(mui->lineEditHost->text());
    m_client->setPort(static_cast<quint16>(mui->spinBoxPort->value()));

    mui->buttonSubscribe->setEnabled(false);
    mui->buttonPublish->setEnabled(false);

    connect(m_client, &QMqttClient::messageReceived, this, [this](const QByteArray &message, const QMqttTopicName &topic) {
        const QString content = QDateTime::currentDateTime().toString() + " Received Topic : " + topic.name() + " Payload : " + message + u'\n';
        if (mui->lineEditFilter->text().isEmpty()) mqttLogThis(content); else if (topic.name().contains(mui->lineEditFilter->text())) mqttLogThis(content);
    });

    mui->pushButtonAddMqttParameter->setEnabled(false);

    connect(mui->checkBoxMqttLog, SIGNAL(stateChanged(int)), this, SLOT(mqttLogChanged()));
    connect(mui->pushButtonClearMqttLog, SIGNAL(clicked()), this, SLOT(clearMqttLog()));
    connect(mui->pushButtonLockMqtt, SIGNAL(clicked()), this, SLOT(setLockedState()));
    connect(mui->buttonConnect, SIGNAL(clicked()), this, SLOT(on_buttonConnect_clicked()));
    connect(mui->buttonPublish, SIGNAL(clicked()), this, SLOT(on_buttonPublish_clicked()));
    connect(mui->buttonSubscribe, SIGNAL(clicked()), this, SLOT(on_buttonSubscribe_clicked()));
    connect(mui->lineEditHost, &QLineEdit::textChanged, m_client, &QMqttClient::setHostname);
    connect(mui->spinBoxPort, QOverload<int>::of(&QSpinBox::valueChanged), this, &lMqtt::setClientPort);
    connect(m_client, &QMqttClient::connected, this, &lMqtt::brokerConnected);
    connect(m_client, &QMqttClient::disconnected, this, &lMqtt::brokerDisconnected);
    connect(m_client, &QMqttClient::stateChanged, this, &lMqtt::updateLogStateChange);
    connect(mqttTree, SIGNAL(clicked(const QModelIndex &)), this, SLOT(mqttPathSelected()));

    mui->tableWidgetMqtt->setColumnCount(8);
    mui->tableWidgetMqtt->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("RomID")));
    mui->tableWidgetMqtt->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Name")));
    mui->tableWidgetMqtt->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("Path")));
    mui->tableWidgetMqtt->setHorizontalHeaderItem(3, new QTableWidgetItem(tr("Value")));
    mui->tableWidgetMqtt->setHorizontalHeaderItem(4, new QTableWidgetItem(tr("Last Read")));
    mui->tableWidgetMqtt->setHorizontalHeaderItem(5, new QTableWidgetItem(tr("Command")));
    mui->tableWidgetMqtt->setHorizontalHeaderItem(6, new QTableWidgetItem(tr("Payload")));
    mui->tableWidgetMqtt->setHorizontalHeaderItem(7, new QTableWidgetItem(tr("Read interval")));
    connect(mui->tableWidgetMqtt, SIGNAL(cellPressed(int, int)),SLOT(displayMqttDevice(int, int)));
    mui->tableWidgetMqtt->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(mui->tableWidgetMqtt, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(ProvideContexMenu(const QPoint&)));
    //connect(m_client, &QMqttClient::messageReceived, this, [this](const QByteArray &message, const QMqttTopicName &topic)
    //connect(sub, &QMqttSubscription::messageReceived, this, &QmlMqttSubscription::handleMessage);
    mqttTree->header()->resizeSection(0, 350);
    mui->availableDevices->setColumnCount(2);
    mui->devicePublishList->setColumnCount(3);
    QStringList s;
    s.append("RomID");
    s.append("Name");
    s.append("Topic");
    mui->devicePublishList->setHeaderLabels(s);
    connect(mui->pushButtonAddMqttParameter, SIGNAL(clicked()), this, SLOT(AddMqttParameterClick()));
    connect(mui->pushButtonMqttCopyPath, SIGNAL(clicked()), this, SLOT(MqttParameterCopy()));
    connect(mui->pushButtonMqttGetStatus, SIGNAL(clicked()), this, SLOT(getStatus()));
    connect(mui->HideLog, SIGNAL(clicked()), this, SLOT(hideLog()));
    connect(&readTimer, SIGNAL(timeout()),SLOT(startMqtt()));
    readTimer.start(10000);
}


lMqtt::~lMqtt()
{
}


QObject *lMqtt::getObject()
{
    return this;
}


QWidget *lMqtt::widgetUi()
{
    return ui;
}

QWidget *lMqtt::getDevWidget(QString)
{
    return nullptr;
}

void lMqtt::setConfigFileName(const QString fileName)
{
    configFileName = fileName;
    configFileName.chop(3);
    configFileName.append(".cfg");
}


QString lMqtt::getDeviceCommands(const QString &)
{
    return "on=" + tr("On") + "|off=" + tr("Off");
}



void lMqtt::on_buttonConnect_clicked()
{
    if (m_client->state() == QMqttClient::Disconnected) {
        m_client->setUsername(mui->lineEditUser->text());
        m_client->setPassword(mui->lineEditPassword->text());
        m_client->connectToHost();
    } else {
        m_client->disconnectFromHost();
    }
}


void lMqtt::setClientPort(int p)
{
    m_client->setPort(static_cast<quint16>(p));
}


void lMqtt::brokerConnected()
{
    mui->lineEditHost->setEnabled(false);
    mui->spinBoxPort->setEnabled(false);
    mui->lineEditUser->setEnabled(false);
    mui->lineEditPassword->setEnabled(false);
    mui->buttonSubscribe->setEnabled(true);
    mui->buttonPublish->setEnabled(true);
    mui->buttonConnect->setText(tr("Disconnect"));
    on_buttonSubscribe_clicked();
}



void lMqtt::brokerDisconnected()
{
    mui->lineEditHost->setEnabled(true);
    mui->spinBoxPort->setEnabled(true);
    mui->buttonConnect->setText(tr("Connect"));
    mui->buttonSubscribe->setEnabled(false);
    mui->buttonPublish->setEnabled(false);
    mui->lineEditUser->setEnabled(true);
    mui->lineEditPassword->setEnabled(true);
}



void lMqtt::updateLogStateChange()
{
    const QString content = QDateTime::currentDateTime().toString() + ": State Change" + QString::number(m_client->state()) + u'\n';
    mqttLogThis(content);
}




void lMqtt::publishDevice(QString &topic, QString &payload)
{
    if (mui->checkBoxMqttLog->isChecked()) mqttLogCommand("Publish topic : " + topic + " Payload : " + payload);
    if (m_client->publish(topic, payload.toUtf8()) == -1)
    {
        if (mui->checkBoxMqttLog->isChecked()) mqttLogCommand(" Failed\n");
        //QMessageBox::critical(this, "Error", "Could not publish message");
    }
    else
    {
        if (mui->checkBoxMqttLog->isChecked()) mqttLogCommand(" Succesfull\n");
    }
}



void lMqtt::on_buttonPublish_clicked()
{
    if (mui->checkBoxMqttLog->isChecked()) mqttLogCommand("Publish topic : " + mui->lineEditCommandTopic->text() + " Payload : " + mui->lineEditPayLoad->text());
    if (m_client->publish(mui->lineEditCommandTopic->text(), mui->lineEditPayLoad->text().toUtf8()) == -1)
    {
        if (mui->checkBoxMqttLog->isChecked()) mqttLogCommand(" Failed\n");
        //QMessageBox::critical(this, "Error", "Could not publish message");
    }
    else
    {
        if (mui->checkBoxMqttLog->isChecked()) mqttLogCommand(" Succesfull\n");
    }
}


void lMqtt::on_buttonSubscribe_clicked()
{
    if (mui->lineEditSubTopic->isEnabled())
    {
        QMqttTopicFilter topic;
        topic.setFilter(mui->lineEditSubTopic->text());
        auto subscription = m_client->subscribe(topic);
        if (!subscription) {
            //QMessageBox::critical(this, "Error", "Could not subscribe. Is there a valid connection?");
            return;
        }
        mui->lineEditSubTopic->setEnabled(false);
        mui->buttonSubscribe->setText("Unsubscribe");
        connect(subscription, &QMqttSubscription::messageReceived, this, &lMqtt::handleMessage);
    }
    else
    {
        m_client->unsubscribe(mui->lineEditSubTopic->text());
        mui->lineEditSubTopic->setEnabled(true);
        mui->buttonSubscribe->setText("Subscribe");
    }
}



void lMqtt::handleMessage(const QMqttMessage &qmsg)
{
    jsonmodel->appendJson(qmsg.topic().name(), qmsg.payload());
    foreach (mqttDevice *dev, deviceSetTable) {
        if (dev->path_item.text().contains(qmsg.topic().name()))
        {
            QJsonTreeItem *item = getMqttTreeItem(dev->path_item.text());
            if (item) {
                item->tableValue = &dev->valueMqtt;
                item->tableValue->RomID = dev->RomID;
                dev->value = item->value().toString();
                dev->valueMqtt.tableWidgetItem.setText(item->value().toString());
                dev->valueMqtt.lastRead_item.setText(QDateTime::currentDateTime().toString("HH:mm:ss"));
                deviceSetTable.removeAll(dev);
                connect(item->tableValue, SIGNAL(newItemValue(QString, QString)), this, SLOT(newValue(QString, QString)));
                //qDebug() << "handleMessage dev->RomID : " + dev->RomID + " : " + dev->value;
                bool ok;
                QString value = dev->value;
                value.toDouble(&ok);
                if (!ok) value = translateValue(value);
                emit(newDeviceValue(dev->RomID, value));
            }
        }
    }
}



void lMqtt::newValue(QString RomID, QString value)
{
    bool ok;
    value.toDouble(&ok);
    if (!ok) value = translateValue(value);
    emit(newDeviceValue(RomID, value));
}


void lMqtt::SaveConfigStr(QString &str)
{
    QSettings settings(configFileName, QSettings::IniFormat);

    SimpleCrypt crypto(Q_UINT64_C(0x15cad52ccb57d84c)); //some random number
    QString passwordEncrypted = crypto.encryptToString(mui->lineEditPassword->text());
    str += "\n" Mqtt_Config_Begin "\n";
        str += logisdom::saveformat("Mqtt_Host", mui->lineEditHost->text());
        QString port = QString("%1").arg(mui->spinBoxPort->value());
        str += logisdom::saveformat("Mqtt_Port", port);
        str += logisdom::saveformat("Mqtt_User", mui->lineEditUser->text());
        str += logisdom::saveformat("Mqtt_PassWord", passwordEncrypted);
        int devIndex = 0;
    foreach (mqttDevice *dev, shellyDevicesMqtt) {
        QString devID = QString("mqttDevice_%1").arg(++devIndex, 3, 10, QChar('0'));
        QString s = dev->path_item.text();
        str += logisdom::saveformat(devID + "/pathLocation", s.replace("/", "*"));
        str += logisdom::saveformat(devID + "/maxValue", dev->maxValue_item.text());
        str += logisdom::saveformat(devID + "/Command", dev->command_item.text());
        str += logisdom::saveformat(devID + "/Payload", dev->payload_item.text());
        str += logisdom::saveformat(devID + "/ReadInterval", dev->readInterval.currentText());
    }
    str += Mqtt_Config_End;
}


void lMqtt::readConfig(const QString &configdata)
{
    loadConfig = true;
    QString TAG_Begin = Mqtt_Config_Begin;
    QString TAG_End = Mqtt_Config_End;
    SearchLoopBegin
        if (!strsearch.isEmpty())
        {
            QString Mqtt_Host = logisdom::getvalue("Mqtt_Host", strsearch);
            if (!Mqtt_Host.isEmpty()) mui->lineEditHost->setText(Mqtt_Host);
            bool ok;
            int port = logisdom::getvalue("Mqtt_Port", strsearch).toInt(&ok, 10);
            if (ok) mui->spinBoxPort->setValue(port);
            QString Mqtt_User = logisdom::getvalue("Mqtt_User", strsearch);
            mui->lineEditUser->setText(Mqtt_User);
            QString Mqtt_Pasword_Encrypted = logisdom::getvalue("Mqtt_PassWord", strsearch);
            QString Mqtt_Pasword;
            SimpleCrypt crypto(Q_UINT64_C(0x15cad52ccb57d84c)); //some random number
            if (!Mqtt_Pasword_Encrypted.isEmpty())
            {
                Mqtt_Pasword = crypto.decryptToString(Mqtt_Pasword_Encrypted);
                mui->lineEditPassword->setText(Mqtt_Pasword);
            }
        int devIndex = 1;
        QString devParameter = QString("mqttDevice_%1").arg(devIndex++, 3, 10, QChar('0'));
        QString devParamterPathLocation = logisdom::getvalue(devParameter + "/pathLocation", strsearch);
        while (!devParamterPathLocation.isEmpty()) {
            devParamterPathLocation.replace("*", "/");
            mqttDevice *shellyDev = addMqttDevice(devParamterPathLocation);

            QString devParamterMaxValue = logisdom::getvalue(devParameter + "/maxValue", strsearch);
            shellyDev->maxValue_item.setText(devParamterMaxValue);

            QString devCommand = logisdom::getvalue(devParameter + "/Command", strsearch);
            shellyDev->command_item.setText(devCommand);

            QString devPayload = logisdom::getvalue(devParameter + "/Payload", strsearch);
            shellyDev->payload_item.setText(devPayload);

            QString devReadInterval = logisdom::getvalue(devParameter + "/ReadInterval", strsearch);
            shellyDev->readInterval.setCurrentText(devReadInterval);

            devParameter = QString("mqttDevice_%1").arg(devIndex++, 3, 10, QChar('0'));
            devParamterPathLocation = logisdom::getvalue(devParameter + "/pathLocation", strsearch);
            QString RomID = shellyDev->RomID;
            emit(newDevice(RomID));
        }
    }
    SearchLoopEnd
    loadConfig = false;
}





void lMqtt::clearMqttLog()
{
    mui->logTxtMqtt->clear();
}




void lMqtt::mqttLogChanged()
{
    if (mui->checkBoxMqttLog->isChecked()) {
        mui->logTxtMqtt->show();
        mui->checkBoxWriteMqtt->setEnabled(true);
    }
    else {
        mui->checkBoxWriteMqtt->setEnabled(false);
    }
}



void lMqtt::hideLog()
{
    if (mui->logTxtMqtt->isHidden()) mui->logTxtMqtt->show();
    else mui->logTxtMqtt->hide();
}



void lMqtt::setLockedState()
{
    lockedState = !lockedState;
    setLockedState(lockedState);
}


void lMqtt::setLockedState(bool state)
{
    lockedState = state;
    if (state) {
        mui->pushButtonLockMqtt->setIcon(QIcon(QPixmap(QString::fromUtf8(":/images/lock.png"))));
        mui->gridLayout_5->removeWidget(mqttTree);
        mqttTree->hide();
        mui->gridLayout_5->addWidget(mui->tableWidgetMqtt, 0, 0, 1, 1);
        mui->tableWidgetMqtt->show();
        mui->pushButtonMqttGetStatus->show();
    }
    else
    {
        mui->pushButtonLockMqtt->setIcon(QIcon(QPixmap(QString::fromUtf8(":/images/unlock.png"))));
        mui->gridLayout_5->removeWidget(mui->tableWidgetMqtt);
        mui->tableWidgetMqtt->hide();
        mui->gridLayout_5->addWidget(mqttTree, 0, 0, 1, 1);
        mqttTree->show();
        mui->pushButtonMqttGetStatus->hide();
    }
}




void lMqtt::setDeviceConfig(const QString &config, const QString &data)
{
    // if (mqtt) mqtt->setDeviceConfig("setDeviceName",  romid + "//" + name);
    if (config == "setDeviceName") {
        QStringList d = data.split("//");
        if (d.count() == 2) {
            QString  RomID = d.first();
            QString  Name = d.last();
            foreach (mqttDevice *dev, shellyDevicesMqtt)
                if (dev->RomID == RomID) dev->name_item.setText(Name);
        }
    }
}




double lMqtt::getMaxValue(const QString)
{
    return 1;
}


bool lMqtt::isManual(const QString)
{
    return false;
}


bool lMqtt::isDimmable(const QString)
{
    return false;
}




void lMqtt::setStatus(const QString status)
{
    mqttLogThis("setStatus : " + status);
    QStringList split = status.split("=");
    QString RomID = split.first();
    QString command = split.last();
    if (split.count() != 2) return;
    foreach (mqttDevice *dev, shellyDevicesMqtt) {
    if (dev->RomID == RomID) {
        if (mui->checkBoxMqttLog->isChecked()) mqttLogCommand("Publish topic : " + dev->command_item.text() + " Payload : " + dev->payload_item.text() + command);
        QByteArray payload = (dev->payload_item.text() + command).toUtf8();
        if (m_client->publish(dev->command_item.text(), payload) == -1)
        {
                if (mui->checkBoxMqttLog->isChecked()) mqttLogCommand(" Failed\n");
        }
        else
        {
            if (mui->checkBoxMqttLog->isChecked()) mqttLogCommand(" Succesfull\n");
        }
        return; }
    }
}


QString lMqtt::selectedMqttPath()
{
        QModelIndex md = mqttTree->currentIndex();
        if (md.column() != 0) md = md.sibling(md.row(), 0);
        QString path;
        if (md.isValid()) {
        path = md.data(Qt::DisplayRole).toString();
        while(md.parent().isValid()) {
        path = md.parent().data().toString() + "/" + path;
        md = md.parent(); }
        }
        // check if device exists :
        bool found = false;
        foreach (mqttDevice *dev, shellyDevicesMqtt) {
                if (dev->path_item.text() == path) found = true; }
        if (!found) return path;
        return "";
}




void lMqtt::mqttPathSelected()
{
    QString path = selectedMqttPath();
    if (path.isEmpty())
    {
        mui->pushButtonAddMqttParameter->setEnabled(false);
        mui->pushButtonAddMqttParameter->setToolTip("");
    }
    else
    {
        mui->pushButtonAddMqttParameter->setEnabled(true);
        mui->pushButtonAddMqttParameter->setToolTip(path);
    }
}



void lMqtt::AddMqttParameterClick()
{
        QString path = selectedMqttPath();
        QStringList pList = path.split("/");
        QString Name = pList.first() + "_" + pList.last();
        addMqttDevice(path, Name);
        mui->pushButtonAddMqttParameter->setEnabled(false);
        mui->pushButtonAddMqttParameter->setToolTip("");
}




void lMqtt::MqttParameterCopy()
{
    QString path = selectedMqttPath();
    qApp->clipboard()->setText(path);
    mui->pushButtonMqttCopyPath->setToolTip(path);
}



mqttDevice *lMqtt::addMqttDevice(const QString &parameterPath, QString Name)
{
        mqttDevice *newDev = new  mqttDevice;
        if (newDev) {
            int index = shellyDevicesMqtt.count();
            shellyDevicesMqtt.append(newDev);
            newDev->RomID = buildRomID(shellyDevicesMqtt.count());
            newDev->path_item.text() = parameterPath;
            QJsonTreeItem *item = getMqttTreeItem(parameterPath);
            if (item) {
                        item->tableValue = &newDev->valueMqtt;
                        item->tableValue->RomID = newDev->RomID;
                        newDev->value = item->value().toString();
                        newDev->valueMqtt.tableWidgetItem.setText(item->value().toString());
                        qDebug() << "connect1 : " + item->tableValue->RomID;
                        connect(item->tableValue, SIGNAL(newItemValue(QString, QString)), this, SLOT(newValue(QString, QString)));
                        newDev->valueMqtt.lastRead_item.setText(QDateTime::currentDateTime().toString("HH:mm:ss"));
                        bool ok;
                        QString value = newDev->value;
                        value.toDouble(&ok);
                        if (!ok) value = translateValue(value);
                        emit(newDeviceValue(newDev->RomID, newDev->value));
            }
            else deviceSetTable.append(newDev);
            mui->tableWidgetMqtt->insertRow(index);
            mui->tableWidgetMqtt->setItem(index, 0, &newDev->RomID_item);
            newDev->RomID_item.setText(newDev->RomID);
            newDev->name_item.setFlags(newDev->name_item.flags() ^ Qt::ItemIsEditable);
            mui->tableWidgetMqtt->setItem(index, 1, &newDev->name_item);
            newDev->RomID_item.setFlags(newDev->RomID_item.flags() ^ Qt::ItemIsEditable);
            mui->tableWidgetMqtt->setItem(index, 2, &newDev->path_item);
            newDev->path_item.setText(parameterPath);
            newDev->path_item.setFlags(newDev->path_item.flags() ^ Qt::ItemIsEditable);
            mui->tableWidgetMqtt->setItem(index, 3, &newDev->valueMqtt.tableWidgetItem);
            newDev->valueMqtt.tableWidgetItem.setText(newDev->value);
            newDev->valueMqtt.tableWidgetItem.setFlags(newDev->valueMqtt.tableWidgetItem.flags() ^ Qt::ItemIsEditable);
            mui->tableWidgetMqtt->setItem(index, 4, &newDev->valueMqtt.lastRead_item);
            mui->tableWidgetMqtt->setItem(index, 5, &newDev->command_item);
            mui->tableWidgetMqtt->setItem(index, 6, &newDev->payload_item);
            mui->tableWidgetMqtt->setCellWidget(index, 7, &newDev->readInterval);
            newMqttShellyDevice(newDev, Name);
            return newDev;
        }
        return nullptr;
}



QString lMqtt::translateValue(QString value)
{
    if (value.contains("true", Qt::CaseInsensitive)) return "1";
    if (value.contains("on", Qt::CaseInsensitive)) return "1";
    if (value.contains("false", Qt::CaseInsensitive)) return "0";
    if (value.contains("off", Qt::CaseInsensitive)) return "0";
    return "NA";
}


QJsonTreeItem *lMqtt::getMqttTreeItem(QString path)
{
    QStringList listParameters = path.split("/");
    int parametersCount = listParameters.count();
    if (!listParameters.isEmpty()) {

    if (parametersCount == 1) {
        for (int n = 0; n<jsonmodel->rowCount(); n++) {
                QModelIndex m = jsonmodel->index(n, 0);
                if (m.data().toString() == path) {
                    QJsonTreeItem *item = static_cast<QJsonTreeItem*>(m.internalPointer());
                    return item; } } }

        if (parametersCount > 1) {
            for (int n = 0; n<jsonmodel->rowCount(); n++) {
                    QModelIndex mRoot = jsonmodel->index(n, 0);
                    if (mRoot.data().toString() == listParameters.first()) {
                    int i = 0;
                    int pIndex = 1;
                    QModelIndex m = jsonmodel->index(i, 0, mRoot);
                oneMoreStep:
                    while (m.isValid()) {
                        if (m.data().toString() == listParameters.at(pIndex)) {
                                if (++pIndex == listParameters.count()) {
                                    QJsonTreeItem *item = static_cast<QJsonTreeItem*>(jsonmodel->index(i, 1, mRoot).internalPointer());
                                    return item;
                            }
                            mRoot = m;
                            i = 0;
                            m = jsonmodel->index(i, 0, mRoot);
                            goto oneMoreStep;
                        }
                        m = jsonmodel->index(++i, 0, mRoot);
                    }
                }
            }
        }
        return nullptr;
    }
    return nullptr;
}



QString lMqtt::ip2Hex(const QString &ip)
{
    bool ok;
    int p1 = ip.indexOf(".");		// get first point position
    if (p1 == -1) return "";
    int p2 = ip.indexOf(".", p1 + 1);	// get second point position
    if (p2 == -1) return "";
    int p3 = ip.indexOf(".", p2 + 1);	// get third point position
    if (p3 == -1) return "";
    int l = ip.length();
    QString N1 = ip.mid(0, p1);
    if (N1 == "") return "";
    QString N2 = ip.mid(p1 + 1, p2 - p1 - 1);
    if (N2 == "") return "";
    QString N3 = ip.mid(p2 + 1, p3 - p2 - 1);
    if (N3 == "") return "";
    QString N4 = ip.mid(p3 + 1, l - p3 - 1);
    if (N4 == "") return "";
    int n1 = N1.toInt(&ok);
    if (!ok) return "";
    int n2 = N2.toInt(&ok);
    if (!ok) return "";
    int n3 = N3.toInt(&ok);
    if (!ok) return "";
    int n4 = N4.toInt(&ok);
    if (!ok) return "";
    return QString("%1%2%3%4").arg(n1, 2, 16, QLatin1Char('0')).arg(n2, 2, 16, QLatin1Char('0')).arg(n3, 2, 16, QLatin1Char('0')).arg(n4, 2, 16, QLatin1Char('0')).toUpper();
}

QString lMqtt::buildRomID(int n)
{
        QString ipHex = ip2Hex(mui->lineEditHost->text());
        QString id = QString("%1").arg(n, 3, 10, QLatin1Char('0')).toUpper();
        QString RomID = ipHex + id + "MT";
        return RomID;
}



void lMqtt::newMqttShellyDevice(const mqttDevice* dev, QString Name)
{
    bool ok;
    QString value = dev->value;
    value.toDouble(&ok);
    if (!ok) value = translateValue(value);
    QString RomID = dev->RomID;
    if (!loadConfig) RomID.append("!");
    if (!Name.isEmpty()) RomID.append( ":" + Name);
    emit(newDevice(RomID));
    if (!loadConfig) {
        emit(newDeviceValue(dev->RomID, value));
        setLockedState();
    }
}




/*void lMqtt::newMqttShellyDeviceValue(const mqttDevice* dev)
{
    bool ok;
    QString value = dev->value;
    value.toDouble(&ok);
    if (!ok) value = translateValue(value);
    emit(newDeviceValue(dev->RomID, value));
}*/



void lMqtt::showMqttDevice(const mqttDevice* dev)
{
    emit(deviceSelected(dev->RomID));
}



void lMqtt::mqttLogThis(const QString &log)
{
    if (!mui->checkBoxMqttLog->isChecked()) return;
    if (mui->checkBoxMqttLog->isChecked() && mui->checkBoxWriteMqtt->isChecked()) return;
    QString txt = mui->logTxtMqtt->toPlainText();
    txt.append(QDateTime::currentDateTime().toString("HH:mm:ss ") + log + "\n");
    int l = txt.length();
    if (l > 10000) txt = txt.mid(l-10000, 10000);
    if (mui->checkBoxMqttLog->isChecked()) {
        mui->logTxtMqtt->setText(txt);
        mui->logTxtMqtt->moveCursor(QTextCursor::End);
    }
}




void lMqtt::mqttLogCommand(const QString &log)
{
    if (mui->checkBoxMqttLog->isChecked() && mui->checkBoxWriteMqtt->isChecked())
    {
        QString txt = mui->logTxtMqtt->toPlainText();
        txt.append(QDateTime::currentDateTime().toString("HH:mm:ss ") + log  + "\n");
        int l = txt.length();
        if (l > 10000) txt = txt.mid(l-10000, 10000);
        if (mui->checkBoxMqttLog->isChecked()) {
            mui->logTxtMqtt->setText(txt);
            mui->logTxtMqtt->moveCursor(QTextCursor::End);
        }
    }
}



void lMqtt::startMqtt()
{
    disconnect(&readTimer, 0, 0, 0);
    on_buttonConnect_clicked();
    connect(&readTimer, SIGNAL(timeout()),SLOT(readAllNow()));
}



void lMqtt::getStatus()
{
    int row = mui->tableWidgetMqtt->currentRow();
    if (row < 0) return;
    QList<mqttDevice*> mqttDevList;
    mqttDevice *dev = shellyDevicesMqtt.at(row);
    getStatus(dev);
}




void lMqtt::readMqttDevices(QList<mqttDevice*>& devList)
{
    foreach (mqttDevice *dev, devList) {
        getStatus(dev);
        //qDebug() << "Read " + dev->RomID;
        /*        QString topic = dev->command_item.text();
        QStringList commandList = topic.split("/");
        QString readCommand;
        foreach (QString c, commandList) {
            if (c == "command") { readCommand.append(c); break; }
            readCommand.append(c + "/");
        }
        //qDebug() << "Read : " + readCommand;
        QString payload = "status_update";
        if (mui->checkBoxMqttLog->isChecked()) mqttLogCommand("Publish topic : " + topic + " Payload : " + payload);
        if (m_client->publish(topic, payload.toUtf8()) == -1)
        {
            if (mui->checkBoxMqttLog->isChecked()) mqttLogCommand(" Failed\n");
        }
        else
        {
            if (mui->checkBoxMqttLog->isChecked()) mqttLogCommand(" Succesfull\n");
        }
    }*/
    }
}



void lMqtt::getStatus(mqttDevice *dev)
{
    if (!dev) return;
    QStringList pathList = dev->path_item.text().split("/");
    if (pathList.count() < 2) return;
    QString readCommand;
    if (dev->path_item.text().startsWith("shellies/")) readCommand = pathList.at(0) + "/" + pathList.at(1);
    else readCommand = pathList.at(0);
    readCommand.append("/command");
    QString payload = "status_update";
    if (mui->checkBoxMqttLog->isChecked()) mqttLogCommand("Publish topic : " + readCommand + " Payload : " + payload);
    if (m_client->publish(readCommand, payload.toUtf8()) == -1)
    {
        if (mui->checkBoxMqttLog->isChecked()) mqttLogCommand(" Failed\n");
    }
    else
    {
        if (mui->checkBoxMqttLog->isChecked()) mqttLogCommand(" Succesfull\n");
    }
}



void lMqtt::readAllNow()
{
    if (m_client->state() != QMqttClient::Connected) {
        m_client->setUsername(mui->lineEditUser->text());
        m_client->setPassword(mui->lineEditPassword->text());
        m_client->connectToHost();
        return;
    }
    QList<mqttDevice*> mqttDevList;
    if (lastMinute < 0) {
        lastMinute = QDateTime::currentDateTime().time().minute();
        foreach (mqttDevice *dev, shellyDevicesMqtt) {
            if (dev->readInterval.currentIndex() > 0) mqttDevList.append(dev);
        }
    }
    int m = QDateTime::currentDateTime().time().minute();
    if (m != lastMinute)
    {
        foreach (mqttDevice *dev, shellyDevicesMqtt) {
            switch (dev->readInterval.currentIndex())
            {
            case readMqtt1mn : mqttDevList.append(dev); break;
            case readMqtt2mn : if (m % 2 == 0) mqttDevList.append(dev); break;
            case readMqtt5mn : if (m % 5 == 0) mqttDevList.append(dev); break;
            case readMqtt10mn : if (m % 10 == 0) mqttDevList.append(dev); break;
            case readMqtt30mn : if (m % 30 == 0) mqttDevList.append(dev); break;
            case readMqtt1hour : if (m == 0) mqttDevList.append(dev); break;
            }
        }
        lastMinute = m;
    }
    readMqttDevices(mqttDevList);
}




void lMqtt::displayMqttDevice(int row, int)
{
    emit(showMqttDevice(shellyDevicesMqtt.at(row)));
}



void lMqtt::ProvideContexMenu(const QPoint&p)
{
    QPoint c = mui->tableWidgetMqtt->mapToGlobal(p);
    QMenu submenu;
    QAction copyAction(tr("Copy"));
    QAction lockAction(tr("Lock"));
    QAction unlockAction(tr("Unlock"));
    QList<QTableWidgetItem *> items=mui->tableWidgetMqtt->selectedItems();
    if (items.count()==1) {
        submenu.addAction(&copyAction);
        if (items[0]->column() == 2) {
            if (items[0]->flags() & Qt::ItemIsEditable) submenu.addAction(&lockAction); else submenu.addAction(&unlockAction); }
        QAction* rightClickItem = submenu.exec(c);
        if (rightClickItem == &copyAction) {
            qApp->clipboard()->setText(items[0]->text()); }
        if (rightClickItem == &lockAction) items[0]->setFlags(items[0]->flags() ^ Qt::ItemIsEditable);
        if (rightClickItem == &unlockAction) items[0]->setFlags(items[0]->flags() | Qt::ItemIsEditable);
        }
}

