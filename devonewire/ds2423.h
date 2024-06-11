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



#ifndef ds2423_H
#define ds2423_H
#include "onewire.h"
#include "ui_ds2423.h"

class ds2423 : public onewiredevice
{
	Q_OBJECT
enum counterMode { absoluteMode, relativeMode, offsetMode };
public:
	ds2423(net1wire *Master, logisdom *Parent, QString RomID);
	void lecture();
	void lecturerec();
	bool setscratchpad(const QString &scratchpad, bool enregistremode = false);
    void SetOrder(const QString &order);
	QComboBox counterMode;
	QSpinBox Offset;
	QDoubleSpinBox Coef;
	interval countInit;
	QCheckBox SaveOnUpdate;
	void GetConfigStr(QString &str);
	void setconfig(const QString &strsearch);
	long int LastCounter, Delta;
private:
	Ui::ds2423ui ui;
	void setResult(long int NewValue, bool enregistremode);
public slots:
	void contextMenuEvent(QContextMenuEvent * event);
private slots:
	void modeChanged(int);
	void saveOnUpdateChanged(int state);
};


#endif
