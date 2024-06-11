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



#ifndef devA50401_H
#define devA50401_H
#include "onewire.h"
#include "ui_eoA504xx.h"

class eoA504xx : public onewiredevice
{
    Q_OBJECT
public:
    eoA504xx(net1wire *Master, logisdom *Parent, QString RomID);
    void setconfig(const QString &strsearch);
    void GetConfigStr(QString &str);
    QString MainValueToStrLocal(const QString &str);
    bool setscratchpad(const QString &scratchpad, bool enregistremode = false);
    QString getSecondaryValue();
// Palette setup
    QLabel LastRadioMessage;
    QSpinBox timeOut;
private:
    Ui::A504xxui ui;
    QString lastScratchpad;
    QTimer checkLastMessage;
public slots:
    void contextMenuEvent(QContextMenuEvent * event);
    void checkElaspedTime();
};


#endif
