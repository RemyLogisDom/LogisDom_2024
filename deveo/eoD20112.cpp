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






#include "globalvar.h"
#include "server.h"
#include "eoD20112.h"




eoD20112::eoD20112(net1wire *Master, logisdom *Parent, QString RomID) : onewiredevice(Master, Parent, RomID)
{
    ui.setupUi(this);
    ui.gridLayout->addWidget(&trafficUi, 3, 1, 1, 2);
    LastRadioMessage.setText("Last message : N/A");
    setupLayout.addWidget(&LastRadioMessage, layoutIndex++, 0, 1, logisdom::PaletteWidth);
    ButtonOn.setText(cstr::toStr(cstr::ON));
    ButtonOff.setText(cstr::toStr(cstr::OFF));
    setupLayout.addWidget(&ButtonOn, layoutIndex, 0, 1, 1);
    setupLayout.addWidget(&ButtonOff, layoutIndex++, 1, 1, 1);
    connect(&ButtonOn, SIGNAL(clicked()), this, SLOT(set_On()));
    connect(&ButtonOff, SIGNAL(clicked()), this, SLOT(set_Off()));
    lastsavevalue = logisdom::NA;
    romid = RomID;
    family = romid.right(10);
    ui.labelromid->setText("RomID : " + romid);
    ui.labelfamily->setText(tr("EnOcean ") + RomID.mid(8, 2) + "-" + RomID.mid(10, 2) + "-" + RomID.mid(12, 2));
    if (master) ui.labelmaster->setText(master->getipaddress());
    htmlBind->addCommand(cstr::toStr(cstr::ON), SwitchON);
    htmlBind->addCommand(cstr::toStr(cstr::OFF), SwitchOFF);
    htmlBind->addParameterCommand("Mode", cstr::toStr(cstr::ON), SwitchON);
    htmlBind->addParameterCommand("Mode", cstr::toStr(cstr::OFF), SwitchOFF);
    MainValue = logisdom::NA;
    saveInterval.setToAuto();
    logEnabled.hide();
    WarnEnabled.hide();
    Unit.setText("");
    timeOut.setRange(5, 9999);
    timeOut.setValue(60);
    timeOut.setPrefix(tr("Time Out : "));
    timeOut.setSuffix(tr(" minutes"));
    setupLayout.addWidget(&timeOut, layoutIndex++, 0, 1, logisdom::PaletteWidth);
    connect(&checkLastMessage, SIGNAL(timeout()), this, SLOT(checkElaspedTime()));
    checkLastMessage.start(60000);
    if (master) getStatus();
}


void eoD20112::getStatus()
{
    if (!parent->isRemoteMode())
    {
        if (family == QString(familyeoD20112_A))
        {
            QString order = master->getFifoString("D20300", romid.left(8), "");
            master->addtofifo(order);
        }
        if (family == QString(familyeoD20112_B))
        {
            QString order = master->getFifoString("D20301", romid.left(8), "");
            master->addtofifo(order);
        }
    }
}


void eoD20112::checkElaspedTime()
{
    if (lastMessage.hasExpired(timeOut.value() * 30000) || (!lastMessage.isValid())) getStatus();
    if (lastMessage.hasExpired(timeOut.value() * 60000) || (!lastMessage.isValid()))
    {
        RetryWarning = 3;
        setTraffic(dataNotValid);
        setMainValue(logisdom::NA, false);
    }
}


void eoD20112::contextMenuEvent(QContextMenuEvent * event)
{
    QMenu contextualmenu;
    QAction Nom(tr("&Name"), this);
    QAction ActionON(cstr::toStr(cstr::ON), this);
    QAction ActionOFF(cstr::toStr(cstr::OFF), this);
    if (!parent->isRemoteMode())
    {
        contextualmenu.addAction(&Nom);
        contextualmenu.addAction(&ActionON);
        contextualmenu.addAction(&ActionOFF);
    }
    QAction *selection;
    selection = contextualmenu.exec(event->globalPos());
    if (selection == &Nom) changename();
    if (selection == &ActionON) set_On();
    if (selection == &ActionOFF) set_Off();
}


//0461E4
//046180

bool eoD20112::setscratchpad(const QString &V, bool enregistremode)
{
    if ((lastMessage.elapsed() < 1000) && (V == lastScratchpad)) return true;   // in case of radio received from multiple EnOcean master
    ScratchPad_Show.setText(V);
    bool ok;
    logMsg(V);
    int cmd = V.mid(0, 2).toInt(&ok, 16) & 0x0F;
    if (ok)
    {
        lastScratchpad = V;
        switch (cmd)
        {
            case 0x01 :
            {
            } break;
            case 0x02 :
            {
            } break;
            case 0x03 :
            {
            } break;
            case 0x04 :
            {
                int OV = V.mid(4, 2).toInt(&ok, 16) & 0x7F;
                if (ok)
                {
                    if (OV <= 0x64)
                    {
                        int Chl = V.mid(2, 2).toInt(&ok, 16) & 0x1F;
                        if (ok)
                        {
                            if (family == QString(familyeoD20112_A))
                            {
                                if (Chl == 0)
                                {
                                    MainValue = OV;
                                    htmlBind->setValue(MainValueToStr());
                                    htmlBind->setParameter("Value", MainValueToStr().remove(getunit()));
                                    if (enregistremode) savevalue(QDateTime::currentDateTime(), MainValue);
                                    ClearWarning();
                                    MainValueToStr();
                                    setValid(dataValid);
                                    emitDeviceValueChanged();
                                    QString T = QDateTime::currentDateTime().toString("ddd HH:mm:ss");
                                    setWindowTitle(name + " " + T);
                                    LastRadioMessage.setText("Last message : " + T);
                                    lastMessage.restart();
                                    return true;
                                } else return true;
                            }
                            if (family == QString(familyeoD20112_B))
                            {
                                if (Chl == 1)
                                {
                                    MainValue = OV;
                                    htmlBind->setValue(MainValueToStr());
                                    htmlBind->setParameter("Value", MainValueToStr().remove(getunit()));
                                    if (enregistremode) savevalue(QDateTime::currentDateTime(), MainValue);
                                    ClearWarning();
                                    MainValueToStr();
                                    setValid(dataValid);
                                    emitDeviceValueChanged();
                                    QString T = QDateTime::currentDateTime().toString("ddd HH:mm:ss");
                                    setWindowTitle(name + " " + T);
                                    LastRadioMessage.setText("Last message : " + T);
                                    lastMessage.restart();
                                    return true;                                } else return true;
                            }
                        }
                    }
                }
            } break;
            case 0x05 :
            {
            } break;
        }
}
    return false;
}



void eoD20112::GetConfigStr(QString &str)
{
    str += logisdom::saveformat("TimeOut", QString("%1").arg(timeOut.value()));
}


void eoD20112::setconfig(const QString &strsearch)
{
    //qDebug() << strsearch;
    QString Name = logisdom::getvalue("Name", strsearch);
    if (!Name.isEmpty()) setname(assignname(Name));
    else setname(assignname(tr("Switch ")));
    bool ok;
    int T = logisdom::getvalue("TimecheckLastMessageOut", strsearch).toInt(&ok);
    if (ok) timeOut.setValue(T);
}



QString eoD20112::getSecondaryValue()
{
    if (lastMessage.isValid())
    {
        qint64 t = lastMessage.elapsed() / 1000;
        if (t < 60) secondaryValue = "< 1 mn";
        else if (t < 3600) secondaryValue = QString("%1 mn").arg(t/60 + 1);
        else secondaryValue = QString("%1 h").arg(t/3600);
    }
    else secondaryValue = cstr::toStr(logisdom::NA);
    return secondaryValue;
}



void eoD20112::remoteCommandExtra(const QString &command)
{
    if (command == SwitchON) set_On();
    if (command == SwitchOFF) set_Off();
}





void eoD20112::On(bool send)
{
    if (!master) return;
    if (family == QString(familyeoD20112_A))
    {
        QString order = master->getFifoString("D2010064", romid.left(8), "");
        if (send) master->addtofifo(order);
    }
    if (family == QString(familyeoD20112_B))
    {
        QString order = master->getFifoString("D2010164", romid.left(8), "");
        if (send) master->addtofifo(order);
    }
}





void eoD20112::Off(bool send)
{
    if (!master) return;
    if (family == QString(familyeoD20112_A))
    {
        QString order = master->getFifoString("D2010000", romid.left(8), "");
        if (send) master->addtofifo(order);
    }
    if (family == QString(familyeoD20112_B))
    {
        QString order = master->getFifoString("D2010100", romid.left(8), "");
        if (send) master->addtofifo(order);
    }
}



bool eoD20112::isSwitchFamily()
{
    return true;
}



QString eoD20112::MainValueToStrLocal(const QString &str)
{
    QString S;
    if (str.isEmpty())
    {
        if (int(MainValue) == logisdom::NA) S = cstr::toStr(cstr::NA);
        else if (int(MainValue) == 0) S = QString(cstr::toStr(cstr::OFF));
        else if (int(MainValue) == 100) S = QString(cstr::toStr(cstr::ON));
        else S = QString("%1 %").arg(MainValue, 0, 'f', 0);
    }
    else S = str;
    ui.valueui->setText(S);
    return S;
}
