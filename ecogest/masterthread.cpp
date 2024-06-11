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



#include <QElapsedTimer>
#include <QTimer>
#include "net1wire.h"
#include "masterthread.h"
#include "globalvar.h"
#ifdef Q_OS_LINUX
#include "sys/socket.h"
#endif
#ifdef Q_OS_WIN32
#include "Winsock2.h"
#endif


masterthread::masterthread()
{
    logEnabled = false;
    GlobalConvert = false;
    endLessLoop = true;
    modeProcess = -1;
    fileIndex = 0;
    socket = nullptr;
    moveToThread(this);
}




masterthread::~masterthread()
{
    endLessLoop = false;
    if (socket) socket->abort();
}




void masterthread::run()
{
#define addTimeTag "\r" + QDateTime::currentDateTime().toString("HH:mm:ss:zzz  ") +
    QTcpSocket MySocket;
    socket = &MySocket;
    MySocket.moveToThread(this);
    int sd = MySocket.socketDescriptor();
    int set = 1;
#ifdef Q_OS_LINUX
    setsockopt(sd, SOL_SOCKET, MSG_NOSIGNAL, (const char *)&set, sizeof(int));
#endif
#ifdef Q_OS_WIN32
    //setsockopt(sd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
#endif
    endLessLoop = true;
    QElapsedTimer timer;
    timer.start();
    QString msg = "Thread starts, first connection";
    TCPconnect(MySocket, msg);
    while (endLessLoop)
    {
        log.clear();
        // One Wire Search request if no request ongoing
        if (FIFO.isEmpty() && !MySocket.bytesAvailable())
        {
            checkSearch();
            FIFOAppend(GetScratchPads, NetRequestMsg[GetScratchPads], 0);
            FIFOAppend(GetStatus, NetRequestMsg[GetStatus], 0);
            log += addTimeTag "********************    Fill FIFO with device Reading  *******************************";
            addDeviceReadings();
            for (int n=0; n<FIFO.count(); n++) log += addTimeTag FIFO.at(n)->Request;
            log += addTimeTag "*************************************************************************************";
            processFIFO(MySocket);	// process FIFO
        }
        log += addTimeTag "****************************    Device Listing  **************************************";
        for (int n=0; n<devices.count(); n++) log += addTimeTag "" + QString ("Device_%1 RomID : ").arg(n) + devices.at(n)->RomID + " Channel : " + devices.at(n)->channel + QString("  Local name : %1  ").arg(devices.at(n)->localName) + "Original Scratchpad : " + devices.at(n)->Scratchpad;
        for (int n=0; n<FIFO.count(); n++) log += addTimeTag "" + FIFO.at(n)->RomID + " : " + FIFO.at(n)->Request + QString ("   Scratchpad_ID : %1").arg(FIFO.at(n)->scratchpad_ID);
        log += addTimeTag "**************************************************************************************";
        processFIFO(MySocket);
        saveLog();
        int minute = QDateTime::currentDateTime().time().minute();
        while ((minute == QDateTime::currentDateTime().time().minute()) && endLessLoop)
        {
            if (MySocket.bytesAvailable())
            {
                checkSendData(MySocket);
                saveLog();
            }
            if (!FIFOSpecial.isEmpty())
            {
                processFIFOSpecial(MySocket);
                saveLog();
            }
            if (FIFO.isEmpty())
            {
                if (FIFO.isEmpty() && !MySocket.bytesAvailable())	// if FIFO is empty add all request otherwise, finish las bunch
                {
                    log += addTimeTag "***************    Fill FIFO with device Reading for reading till next minute  *********************";
                    if (searchDelay == 0) checkSearch();
                    addDeviceReadings(false);
                }
                for (int n=0; n<FIFO.count(); n++) log += addTimeTag "" + FIFO.at(n)->RomID + " : " + FIFO.at(n)->Request + QString ("   Scratchpad_ID : %1").arg(FIFO.at(n)->scratchpad_ID);
                log += addTimeTag "**************************************************************************************";
            }
            processFIFO(MySocket, false);
            if (FIFO.isEmpty()) saveLog();
            if (FIFOSpecial.isEmpty()) sleep(1);
        }
        // Clear FIFO
        mutexData.lock();
        while (!FIFO.isEmpty())
        {
            delete FIFO.first();
            FIFO.removeFirst();
        }
        mutexData.unlock();
        saveLog();
    }
    MySocket.disconnectFromHost();
    if (MySocket.state() != QAbstractSocket::UnconnectedState) MySocket.waitForDisconnected();
    tcpStatus = MySocket.state();
    emit(tcpStatusChange());
}





void masterthread::TCPconnect(QTcpSocket &Socket, const QString &Status)
{
    log += addTimeTag "TCP Connect  " + Status;
    int tryConnect = 1;
    do
    {
        log += addTimeTag QString("TCP try connect %1").arg(tryConnect);
        if (Socket.state() == QAbstractSocket::ConnectedState)
        {
            Socket.disconnectFromHost();
            log += addTimeTag "Try Disconnect";
            if (Socket.state() == QAbstractSocket::ConnectedState) Socket.waitForDisconnected();
        }
        if (Socket.state() == QAbstractSocket::UnconnectedState)
        {
            log += addTimeTag "Disconnect OK";
        }
        tcpStatus = Socket.state();
        emit(tcpStatusChange());
        if (endLessLoop)
        {
            Socket.connectToHost(moduleipaddress, port);
            log += addTimeTag "Try Connect";
            Socket.waitForConnected();
            if (Socket.state() == QAbstractSocket::ConnectedState)
            {
                log += addTimeTag "Connect OK";
            }
            tcpStatus = Socket.state();
            emit(tcpStatusChange());
            tryConnect ++;
        }
        if (tryConnect > 10)
        {
            log.clear();
            tryConnect = 0;
        }
    }
    while ((Socket.state() != QAbstractSocket::ConnectedState) && (endLessLoop));
}




int masterthread::getConvertTime(device *dev)
{
    QString family = dev->RomID.right(2);
    if (family == family1820) return 200;
    if ((family == family1822) or (family == family18B20) or (family == family1825))
    {
        if (dev->Scratchpad.isEmpty()) return 750;
        bool ok;
        int resolution = dev->Scratchpad.mid(8, 2).toInt(&ok, 16);
        if (!ok) return 750;
        int P = resolution & 0x60;
        P /= 32;
        if (P == 3) return 750;
        if (P == 2) return 375;
        if (P == 1) return 190;
        if (P == 0) return 95;
        return 750;
    }
    return 0;
}



#define incCounterEmit emit(deviceReturn(devices.at(deviceIndex)->RomID, devices.at(deviceIndex)->Scratchpad));


void masterthread::readFIFOBuffer(const QString &extract)
{
    QString devicescratchpad = logisdom::getvalue("ScratchPad", extract);
    QString R2 = devicescratchpad.right(2);
    QString R14 = devicescratchpad.right(14);
    QString R18 = devicescratchpad.right(18);
    QString R26 = devicescratchpad.right(26);
    int deviceIndex = FIFO.first()->device_ID;
    switch (FIFO.first()->Request_ID)
    {
        case LocalSearch	: LocalSearchAnalysis(extract); break;
        case GlobalSearch	: GlobalSearchAnalysis(extract); break;
        case SearchReset	: LocalSearchAnalysis(extract); break;
        case GetScratchPads	: GetScratchPadsAnalysis(extract); break;
        case GetStatus		: GetStatusAnalysis(extract); break;
        case ConvertTemp	:
                    log += addTimeTag QString(" Wait for convesion : 1 second");
                    sleep(1); break;
        case ConvertTempRomID	:
                    if ((deviceIndex >= 0) && (deviceIndex < devices.count())) {
                    int convTime = getConvertTime(devices.at(deviceIndex));
                    log += addTimeTag QString(" Wait for convesion : %1ms").arg(convTime);
                    msleep(convTime + 250); }
                    break;
        case ReadTemp           : if ((deviceIndex >= 0) && (deviceIndex < devices.count())) { devices.at(deviceIndex)->Scratchpad = R18; incCounterEmit } break;
        case ReadPIO            : if ((deviceIndex >= 0) && (deviceIndex < devices.count())) { devices.at(deviceIndex)->Scratchpad = R26; incCounterEmit } break;
        case ReadCounter        : if ((deviceIndex >= 0) && (deviceIndex < devices.count()))
                                    {
                                        if (FIFO.first()->scratchpad_ID == 0)
                                        {
                                           // if (devices.at(deviceIndex)->Scratchpad != devicescratchpad) incCounterEmit
                                            devices.at(deviceIndex)->Scratchpad = devicescratchpad;
                                        }
                                        else if (FIFO.first()->scratchpad_ID == 1) devices.at(deviceIndex)->Scratchpad_1 = devicescratchpad;
                                    } break;
        case ReadLCD            : if ((deviceIndex >= 0) && (deviceIndex < devices.count())) { devices.at(deviceIndex)->Scratchpad = devicescratchpad; incCounterEmit } break;
        case ReadDualSwitch     : if ((deviceIndex >= 0) && (deviceIndex < devices.count())) { devices.at(deviceIndex)->Scratchpad = R2; } break;
        case ChannelAccessRead	: if ((deviceIndex >= 0) && (deviceIndex < devices.count())) { devices.at(deviceIndex)->Scratchpad = R14; } break;
        case ReadADC            : if ((deviceIndex >= 0) && (deviceIndex < devices.count())) { devices.at(deviceIndex)->Scratchpad = R26; incCounterEmit} break;
        case ReadADCPage01h     : if ((deviceIndex >= 0) && (deviceIndex < devices.count())) { devices.at(deviceIndex)->Scratchpad_1 = "page01h=(" + R18 + ")"; } break;
        case ReadPage           : if ((deviceIndex >= 0) && (deviceIndex < devices.count())) { devices.at(deviceIndex)->Scratchpad = R18; } break;
        case ReadPage00h        : if ((deviceIndex >= 0) && (deviceIndex < devices.count())) { devices.at(deviceIndex)->Scratchpad = R18; incCounterEmit} break;
        case ReadPage01h        : if ((deviceIndex >= 0) && (deviceIndex < devices.count())) { devices.at(deviceIndex)->Scratchpad_1 = "page01h=(" + R18 + ")"; } break;
    }
}




void masterthread::readFIFOSpecialBuffer(const QString &extract)
{
    QString devicescratchpad = logisdom::getvalue("ScratchPad", extract);
    if (devicescratchpad.isEmpty())
    {
        if (extract.startsWith("Done")) GetStatusAnalysis(extract);
        return;
    }
    QString R8 = devicescratchpad.right(8);
    QString R16 = devicescratchpad.right(16);
    int deviceIndex = FIFOSpecial.first()->device_ID;
    switch (FIFOSpecial.first()->Request_ID)
    {
        case WritePIO	:
        case setChannelOn	:
        case setChannelOff	:	if ((deviceIndex >= 0) && (deviceIndex < devices.count()))
                        {
                            devices.at(deviceIndex)->Return = R8.right(2);
                            devices.at(deviceIndex)->Scratchpad = ""; //.mid(2, 4);
                            emit(deviceReturn(devices.at(deviceIndex)->RomID, devices.at(deviceIndex)->Return));
                        } break;
        case ChannelAccessWrite	: if ((deviceIndex >= 0) && (deviceIndex < devices.count()))
                        {
                            devices.at(deviceIndex)->Scratchpad = R16;
                            incCounterEmit
                        } break;
        case ReadMemoryLCD	: if ((deviceIndex >= 0) && (deviceIndex < devices.count()))
                        {
                            devices.at(deviceIndex)->Scratchpad_1 = devicescratchpad;
                            emit(deviceReturn(devices.at(deviceIndex)->RomID, devices.at(deviceIndex)->Scratchpad_1));
                        } break;
        case WriteLed	: if ((deviceIndex >= 0) && (deviceIndex < devices.count()))
                        {
                            devices.at(deviceIndex)->Scratchpad = devicescratchpad;
                            incCounterEmit
                        } break;
        default :
                if (extract.startsWith("Status")) GetStatusAnalysis(extract);
        break;
    }
}



QString masterthread::getScratchPad(const QString &RomID, int scratchpad_ID)
{
    if (!isRunning()) return "";
    QMutexLocker locker(&mutexData);
    for (int n=0; n<devices.count(); n++)
    {
        if (devices.at(n)->RomID == RomID)
        {
            if (scratchpad_ID == 0)	return devices.at(n)->Scratchpad;
            if (scratchpad_ID == 1)	return devices.at(n)->Scratchpad_1;
        }
    }
    return "";
}






void masterthread::saveLog()
{
    if (logEnabled)
    {
        log += addTimeTag "*******************************    Device Scratchpad  **********************************";
        for (int n=0; n<devices.count(); n++) log += addTimeTag "" + devices.at(n)->RomID + "  Scratchpad : " + devices.at(n)->Scratchpad;
        log += addTimeTag "_________________________________FIN__________________________________\r\r";
        QString filename = moduleipaddress;
        filename.remove(".");
        filename += QString("_%1.txt").arg(fileIndex);
        QFile file(filename);
        QTextStream out(&file);
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        out << logfile;
        file.close();
        fileIndex++;
        if (fileIndex > 999) fileIndex = 0;
        emit(tcpStatusUpdate(log));
    }
    log.clear();
    logfile.clear();
}





void masterthread::checkSearch()
{
    bool search = true;
    if (lastSearch.isValid())
    {
        if (searchDelay == -1) search = false;
        else if (searchDelay == 0) search = true;
        else if (lastSearch.secsTo(QDateTime::currentDateTime()) >= searchDelay)
        {
            lastSearch = QDateTime::currentDateTime();
            search = true;
        }
        else search = false;
    }
    else
    {
        lastSearch = QDateTime::currentDateTime();
    }
    if (search)
    {
        FIFOAppend(LocalSearch, NetRequestMsg[LocalSearch], 0); // Local search first to set local flag correctly
        FIFOAppend(GlobalSearch, NetRequestMsg[GlobalSearch], 0);
    }
}



void masterthread::processFIFO(QTcpSocket &MySocket, bool ALL)
{
    QElapsedTimer timer;
    QString extract;
    if (FIFO.isEmpty())
    {
        log += addTimeTag " FIFO Empty";
        return;
    }
    while (!FIFO.isEmpty() && FIFOSpecial.isEmpty() && endLessLoop && !MySocket.bytesAvailable())
    {
        log += addTimeTag "------------------------------------------------------------------------------";
        QString Req = FIFO.first()->Request;
        log += addTimeTag " SEND : " + Req;
        Req += "\r";
        if (MySocket.state() == QAbstractSocket::ConnectedState)
        {
                        if (MySocket.isValid()) MySocket.write(Req.toLatin1()); else return;
            if (MySocket.waitForBytesWritten())
            {
                if (MySocket.waitForReadyRead())
                {
                    timer.start();
                    do
                    {
                        extract = extractBuffer(MySocket);
                        if (extract.isEmpty())
                        {
                            msleep(500);
                            if (!MySocket.waitForReadyRead(10000))
                            {
                                QString msg = "Socket error , reconnect, request = " + Req;
                                TCPconnect(MySocket, msg);
                            }
                        }
                    } while (extract.isEmpty() and (timer.elapsed() < 10000));
                    log += addTimeTag " GET : " + extract;
                    readFIFOBuffer(extract);
                    msleep(250);
                    mutexData.lock();
                    delete FIFO.first();
                    FIFO.removeFirst();
                    mutexData.unlock();
                    if (logEnabled) { emit(tcpStatusUpdate(log)); logfile.append(log); log.clear(); }
                    if (!ALL) return;
                }
                else
                {
                    QString msg = "Socket error , reconnect, request = " + Req;
                    TCPconnect(MySocket, msg);
                }
            }
            else
            {
                QString msg = "Socket error Error writing data = " + Req;
                TCPconnect(MySocket, msg);
            }
        }
        else
        {
            QString msg = "Socket error , reconnect, request = " + Req;
            TCPconnect(MySocket, msg);
        }
    }
}




void masterthread::processFIFOSpecial(QTcpSocket &MySocket)
{
    QElapsedTimer timer;
    QString extract;
    if (FIFOSpecial.isEmpty())
    {
        log += addTimeTag " FIFOSpecial Empty";
        return;
    }
    log += addTimeTag "********************     FIFOSpecial contains   *************************************";
    for (int n=0; n<FIFOSpecial.count(); n++) log += addTimeTag "" + FIFOSpecial.at(n)->Request;
    log += addTimeTag "*************************************************************************************";
    while (!FIFOSpecial.isEmpty() && endLessLoop && !MySocket.bytesAvailable())
    {
        log += addTimeTag "------------------------------------------------------------------------------";
        Buffer.clear();
        QString Req = FIFOSpecial.first()->Request;
        log += addTimeTag " SEND : " + Req;
        Req += "\r";
        if (MySocket.state() == QAbstractSocket::ConnectedState)
        {
            if (MySocket.isValid()) MySocket.write(Req.toLatin1()); else return;
            if (MySocket.waitForBytesWritten())
            {
                if (MySocket.waitForReadyRead())
                {
                    timer.start();
                    do
                    {
                        extract = extractBuffer(MySocket);
                        if (extract.isEmpty())
                        {
                            msleep(500);
                            if (!MySocket.waitForReadyRead(10000))
                            {
                                QString msg = "Socket error , reconnect, request = " + Req;
                                TCPconnect(MySocket, msg);
                            }
                        }
                    } while (extract.isEmpty() and (timer.elapsed() < 10000));
                    log += addTimeTag " GET : " + extract;
                    readFIFOSpecialBuffer(extract);
                    msleep(100);
                    mutexData.lock();
                    delete FIFOSpecial.first();
                    FIFOSpecial.removeFirst();
                    mutexData.unlock();
                    if (logEnabled) { emit(tcpStatusUpdate(log)); logfile.append(log); log.clear(); }
                }
                else
                {
                    QString msg = "Socket error , reconnect, request = " + Req;
                    TCPconnect(MySocket, msg);
                }
            }
            else
            {
                QString msg = "Error writing data, request = " + Req;
                TCPconnect(MySocket, msg);
            }
        }
        else
        {
            QString msg = "Socket error , reconnect, request = " + Req;
            TCPconnect(MySocket, msg);
        }
    }
    //FIFOAppend(GetScratchPads, NetRequestMsg[GetScratchPads], 0);
    //FIFOAppend(GetStatus, NetRequestMsg[GetStatus], 0);
    //processFIFO(MySocket);
}





void masterthread::checkSendData(QTcpSocket &MySocket)
{
    QString extract;
    extract = extractBuffer(MySocket);
    if (!extract.isEmpty())
    {
        log += addTimeTag "********************    Master send information  *******************************";
        log += addTimeTag extract;
        GetStatusAnalysis(extract);
    }
}



QString masterthread::extractBuffer(QTcpSocket &socket)
{
    QString extract = "";
    int chdeb, chfin, L;
    Buffer += socket.readAll();
    chdeb = Buffer.indexOf("<");
    chfin = Buffer.indexOf(">");
    L = Buffer.length();
    if ((chdeb == -1) and (chfin == -1)) return "";		//GenMsg("No begin No end found : ");		//GenMsg("Buffer : " + Buffer);
    if ((chdeb != -1) and (chfin == -1)) return "";		//GenMsg("No end found : ");	//GenMsg("Buffer : " + Buffer);
    if (chdeb > chfin)   //GenMsg("No begin found : ");//GenMsg("Buffer : " + Buffer);
    {
        Buffer = Buffer.right(L - chdeb);
    }
    if ((chdeb != -1) and (chfin != -1))
    {
        extract = Buffer.mid(chdeb + 1, chfin - chdeb - 1);
        Buffer = Buffer.right(L - chfin - 1);
    }
    return extract;
}


// ALL is used to skip counters to process a reading regularly and not in real time to avoid counting fluctuations

void masterthread::addDeviceReadings(bool ALL)
{
    if (GlobalConvert)
    {
        FIFOAppend(ConvertTemp, "WCS=0&Dat=CC44"); // Channel 0, 1 or 2
        FIFOAppend(ConvertTemp, "WCS=1&Dat=CC44");
        FIFOAppend(ConvertTemp, "WCS=2&Dat=CC44");
    }
    for(int index=0; index<devices.count(); index++)
    {
        if (devices.at(index)->localName.isEmpty())
        {
            QString RomID = devices.at(index)->RomID;
            QString family = RomID.right(2);
            if (!GlobalConvert)
            {
                int convTime = getConvertTime(devices.at(index));
                if (convTime) FIFOAppend(ConvertTempRomID, "WCS=" + devices.at(index)->channel + "&Adr=" + RomID.left(16) + "&Dat=44", index);
            }
            if (family == family1822) FIFOAppend(ReadTemp, "WCh=" + devices.at(index)->channel + "&Adr=" + RomID + "&Dat=BEFFFFFFFFFFFFFFFFFF", index);
            else if (family == family1820)  FIFOAppend(ReadTemp, "WCh=" + devices.at(index)->channel + "&Adr=" + RomID + "&Dat=BEFFFFFFFFFFFFFFFFFF", index);
            else if (family == family1825)  FIFOAppend(ReadTemp, "WCh=" + devices.at(index)->channel + "&Adr=" + RomID + "&Dat=BEFFFFFFFFFFFFFFFFFF", index);
            else if (family == family18B20) FIFOAppend(ReadTemp, "WCh=" + devices.at(index)->channel + "&Adr=" + RomID + "&Dat=BEFFFFFFFFFFFFFFFFFF", index);
            else if (family == familyLedOW) FIFOAppend(ReadTemp, "WCh=" + devices.at(index)->channel + "&Adr=" + RomID.left(16) + "&Dat=BEFFFFFFFFFFFFFFFFFF", index, 0);
            else if (family == familyLCDOW) FIFOAppend(ReadLCD, "WCh=" + devices.at(index)->channel + "&Adr=" + RomID.left(16) + "&Dat=A5DF01FFFFFFFFFFFFFFFFFFFFFF", index, 0);
            else if (family == family2406)  FIFOAppend(ChannelAccessRead, "WCh=" + devices.at(index)->channel + "&Adr=" + RomID.left(16) + "&Dat=F545FFFFFFFFFF", index);
            else if (family == family2408)  FIFOAppend(ReadPIO, "WCh=" + devices.at(index)->channel + "&Adr=" + RomID.left(16) + "&Dat=F08800FFFFFFFFFFFFFFFFFFFF", index);
            else if (family == family2413)  FIFOAppend(ReadDualSwitch, "WCh=" + devices.at(index)->channel + "&Adr=" + RomID.left(16) + "&Dat=F5FF", index);
            else if (family == family3A2100H)  FIFOAppend(ReadDualSwitch, "WCh=" + devices.at(index)->channel + "&Adr=" + RomID.left(16) + "&Dat=F5FF", index);
            else if ((family == family2423) && ALL)
            {
                FIFOAppend(ReadCounter, "WCh=" + devices.at(index)->channel + "&Adr=" + RomID.left(16) + "&Dat=A5DF01FFFFFFFFFFFFFFFFFFFFFF", index, 0);
                FIFOAppend(ReadCounter, "WCh=" + devices.at(index)->channel + "&Adr=" + RomID.left(16) + "&Dat=A5FF01FFFFFFFFFFFFFFFFFFFFFF", index, 1);
            }
            else if (family == family2438)
            {
                FIFOAppend(ConvertTempRomID,	"WCS=" + devices.at(index)->channel + "&Adr=" + RomID.left(16) + "&Dat=44", index);
                FIFOAppend(ConvertV,		"WCS=" + devices.at(index)->channel + "&Adr=" + RomID.left(16) + "&Dat=B4", index);
                FIFOAppend(RecallMemPage00h,	"WCh=" + devices.at(index)->channel + "&Adr=" + RomID.left(16) + "&Dat=B800", index);
                FIFOAppend(ReadPage00h,		"WCh=" + devices.at(index)->channel + "&Adr=" + RomID.left(16) + "&Dat=BE00FFFFFFFFFFFFFFFFFF", index);
                FIFOAppend(RecallMemPage01h,	"WCh=" + devices.at(index)->channel + "&Adr=" + RomID.left(16) + "&Dat=B801", index);
                FIFOAppend(ReadPage01h,		"WCh=" + devices.at(index)->channel + "&Adr=" + RomID.left(16) + "&Dat=BE01FFFFFFFFFFFFFFFFFF", index);
            }
            else if (family == family2450)
            {
                FIFOAppend(ReadADCPage01h, "WCh=" + devices.at(index)->channel + "&Adr=" + RomID.left(16) + "&Dat=AA0800FFFFFFFFFFFFFFFFFFFF", index, 1);
                FIFOAppend(ConvertADC, "WCS=" + devices.at(index)->channel + "&Adr=" + RomID.left(16) + "&Dat=3C0F00FFFF", index);
                FIFOAppend(ReadADC, "WCh=" + devices.at(index)->channel + "&Adr=" + RomID.left(16) + "&Dat=AA0000FFFFFFFFFFFFFFFFFFFF", index);
            }
        }
    }
    FIFOAppend(GetScratchPads, NetRequestMsg[GetScratchPads], 0);
    FIFOAppend(GetStatus, NetRequestMsg[GetStatus], 0);
}


//WCh=1&Adr=A759586B62C7197E&Dat=FA0000FFFFFFFFFFFFFFFFFF
//<[126][25][199][98][107][88][89][167]<ScratchPad = (
//FA0000002200000001020152
//FA00000144000000020402A4
//WCh=1&Adr=A759586B62C7197E&Dat=FA0000FFFFFFFFFFFFFFFFFF


void masterthread::addToFIFOSpecial(const QString &RomID, const QString &data, int Request_ID)
{
    if (FIFOSpecial.count() > 100) return;
    QString S;
    if (RomID.isEmpty())
    {
        S.clear();
        FIFOSpecialAppend(Request_ID, data, S, 0);
    }
    else
    {
        for (int index=0; index<devices.count(); index++)
        {
            if (devices.at(index)->RomID == RomID.left(16))
            {
                switch (Request_ID)
                {
                    case setChannelOn :
                        if (data.isEmpty()) log += addTimeTag RomID + " setChannelOn empty data";
                        else
                        {
                            S = "WCh=" + devices.at(index)->channel + "&Adr=" + RomID.left(16) + "&Dat=5A" + data + "FF";
                            FIFOSpecialAppend(setChannelOn, S, RomID, index);
                        }
                        break;
                    case setChannelOff :
                        if (data.isEmpty()) log += addTimeTag RomID + " setChannelOff empty data";
                        else
                        {
                            S = "WCh=" + devices.at(index)->channel + "&Adr=" + RomID.left(16) + "&Dat=5A" + data + "FF";
                            FIFOSpecialAppend(setChannelOff, S, RomID, index);
                        }
                        break;
                    case WriteScratchpad00	:
                        if (data.isEmpty()) log += addTimeTag RomID + " WriteScratchpad00 empty data";
                        else
                        {
                            S = "WCh=" + devices.at(index)->channel + "&Adr=" + RomID.left(16) + "&Dat=4E00" + data;
                            FIFOSpecialAppend(WriteScratchpad00, S, RomID, index);
                        }
                        break;
                    case CopyScratchpad00	:
                            S = "WCh=" + devices.at(index)->channel + "&Adr=" + RomID.left(16) + "&Dat=4800";
                            FIFOSpecialAppend(CopyScratchpad00, S, RomID, index);
                        break;
                    case WriteMemory	:
                        if (data.isEmpty()) log += addTimeTag RomID + " WriteMemory empty data";
                        else
                        {
                            S = "WCh=" + devices.at(index)->channel + "&Adr=" + RomID.left(16) + "&Dat=55" + data;
                            FIFOSpecialAppend(WriteMemory, S, RomID, index);
                        }
                        break;
                    case WriteValText	:
                        if (data.isEmpty()) log += addTimeTag RomID + " WriteValText empty data";
                        else
                        {
                            S = "WCh=" + devices.at(index)->channel + "&Adr=" + RomID.left(16) + "&Dat=" + data;
                            FIFOSpecialAppend(WriteMemory, S, RomID, index);
                        }
                        break;
                    case WriteLed	:
                        if (data.isEmpty()) log += addTimeTag RomID + " WriteLed empty data";
                        else
                        {
                            S = "WCh=" + devices.at(index)->channel + "&Adr=" + RomID.left(16) + "&Dat=" + data;
                            FIFOSpecialAppend(WriteLed, S, RomID, index);
                        }
                        break;
                    case ReadMemoryLCD	:
                        if (data.isEmpty()) log += addTimeTag RomID + " ReadMemoryLCD empty data";
                        else
                        {
                            S = "WCh=" + devices.at(index)->channel + "&Adr=" + RomID.left(16) + "&Dat=" + data;
                            FIFOSpecialAppend(ReadMemoryLCD, S, RomID, index);
                        }
                        break;
                    case WritePIO :
                        if (data.isEmpty()) log += addTimeTag RomID + " WritePIO empty data";
                        else
                        {
                            S = "WCh=" + devices.at(index)->channel + "&Adr=" + RomID.left(16) + "&Dat=5A" + data + "FF";
                            FIFOSpecialAppend(WritePIO, S, RomID, index);
                        }
                        break;
                    case ChannelAccessWrite :
                        if (data.isEmpty()) log += addTimeTag RomID + " ChannelAccessWrite empty data";
                        else
                        {
                            S = "WCh=" + devices.at(index)->channel + "&Adr=" + RomID.left(16) + "&Dat=F5" + data + "FFFF";
                            FIFOSpecialAppend(ChannelAccessWrite, S, RomID, index);
                        }
                        break;
                }
            }
        }
    }
}




void masterthread::addToFIFOSpecial(const QString &data)
{
    if (lastFIFOAdd == data) return;
    lastFIFOAdd = data;
    if (FIFOSpecial.count() > 100) return;
    FIFOStruc *newFIFO = new FIFOStruc;
    newFIFO->RomID = "";
    newFIFO->Request_ID = -1;
    newFIFO->Request = data;
    newFIFO->scratchpad_ID = -1;
    newFIFO->device_ID = -1;
    mutexData.lock();
    FIFOSpecial.append(newFIFO);
    mutexData.unlock();
}



void masterthread::addToFIFO(int reqID)
{
    FIFOStruc *newFIFO = new FIFOStruc;
    newFIFO->Request_ID = reqID;
    newFIFO->Request = "";
    newFIFO->scratchpad_ID = -1;
    newFIFO->RomID = "";
    mutexData.lock();
    FIFO.append(newFIFO);
    mutexData.unlock();
}


void masterthread::FIFOAppend(int reqID, const QString &reqStr, int device_ID, int scratchpad_ID)
{
    FIFOStruc *newFIFO = new FIFOStruc;
    newFIFO->Request_ID = reqID;
    newFIFO->Request = reqStr;
    newFIFO->scratchpad_ID = scratchpad_ID;
    if ((device_ID >= 0) && (device_ID < devices.count())) newFIFO->RomID = devices.at(device_ID)->RomID; else newFIFO->RomID = "";
    newFIFO->device_ID = device_ID;
    mutexData.lock();
    FIFO.append(newFIFO);
    mutexData.unlock();
}





void masterthread::removeDuplicates()
{
    int count = FIFOSpecial.count();
    if (count < 2) return;
    QString lastRomID, lastRequest;
    lastRomID = FIFOSpecial.last()->RomID;
    lastRequest = FIFOSpecial.last()->Request;
    for (int n=0; n<(count-2); n++)
    {
        if (lastRomID == FIFOSpecial.at(n)->RomID)
            if (lastRequest == FIFOSpecial.at(n)->Request)
            {
                FIFOSpecial.removeAt(n);
            }
    }
}




void masterthread::FIFOSpecialAppend(int reqID, const QString &reqStr, const QString &RomID, int device_ID, int scratchpad_ID)
{
    if ((device_ID >= 0) && (device_ID < devices.count()))
    {
        FIFOStruc *newFIFO = new FIFOStruc;
        newFIFO->RomID = RomID;
        newFIFO->Request_ID = reqID;
        newFIFO->Request = reqStr;
        newFIFO->scratchpad_ID = scratchpad_ID;
        newFIFO->device_ID = device_ID;
        mutexData.lock();
        FIFOSpecial.append(newFIFO);
        removeDuplicates();
        if (FIFOSpecial.count() > 100) delete FIFOSpecial.takeFirst();
        mutexData.unlock();
    }
}




QString masterthread::getStr(int index)
{
    QString str;
    switch (index)
    {
        case TankLow : str = "TankLow"; break;
        case TankHigh : str = "TankHigh"; break;
        case HeaterIn : str = "HeaterIn"; break;
        case HeaterOut : str = "HeaterOut"; break;
        case HeatingOut : str = "HeatingOut"; break;
        case SolarIn : str = "SolarIn"; break;
        case SolarOut : str = "SolarOut"; break;
    }
    return str;
}





void masterthread::GetStatusAnalysis(const QString &data)
{
        emit(newStatus(data));
}




void masterthread::GetScratchPadsAnalysis(const QString &DATA)
{
    QStringList list = DATA.split("One_Wire_Device");
    for (int i=0; i<list.count(); i++)
    {
        QString data = list.at(i);
        QString RomID = logisdom::getvalue("RomID", data);
        QString Scratchpad = logisdom::getvalue("Scratchpad", data);
        if (Scratchpad.length() == 18)
        {
            for(int index=0; index<devices.count(); index++)
            {
                if (devices.at(index)->RomID == RomID)
                {
                    devices.at(index)->Scratchpad = Scratchpad;
                    emit(deviceReturn(devices.at(index)->RomID, devices.at(index)->Scratchpad));
                    break;
                }
            }
        }
    }
}



void masterthread::GlobalSearchAnalysis(const QString &data)
{
    QString RomID, channel;
    int Ch0, Ch1, Ch2;
    int index = 0;
//	int busShorted = channel.indexOf("Bus Shorted");
//	if (busShorted != -1) GenError(56, "Bus shorted");
    Ch0 = data.indexOf("Channel = (0)");
    Ch1 = data.indexOf("Channel = (1)");
    Ch2 = data.indexOf("Channel = (2)");
    if ((Ch0 != -1) and (Ch1 != -1))
    {
        channel = data.mid(Ch0, Ch1 - Ch0);
        RomID = logisdom::getvalue(QString("ID%1").arg(index), channel);
        while (!RomID.isEmpty())
        {
            QString checkRomIDNull = RomID;
            checkRomIDNull.remove("0");
            if ((RomID.length() > 15) && (!checkRomIDNull.isEmpty()))
            {
                bool found = false;
                for (int n=0; n<devices.count(); n++)
                {
                    if (devices.at(n)->RomID == RomID)
                    {
                        found = true;
                        devices.at(n)->channel = "0";
                    }
                }
                if (!found)
                {
                    device *dev = new device;
                    dev->RomID = RomID;
                    dev->localName = "";
                    dev->channel = "0";
                    devices.append(dev);
                    emit(newDevice(devices.last()->RomID, dev->localName));
                }
            }
            index ++;
            RomID = logisdom::getvalue(QString("ID%1").arg(index), channel);
        }
    }
    if ((Ch1 != -1) and (Ch2 != -1))
    {
        channel = data.mid(Ch1, Ch2 - Ch1);
        RomID = logisdom::getvalue(QString("ID%1").arg(index), channel);
        while (!RomID.isEmpty())
        {
            QString checkRomIDNull = RomID;
            checkRomIDNull.remove("0");
            if ((RomID.length() > 15) && (!checkRomIDNull.isEmpty()))
            {
                bool found = false;
                for (int n=0; n<devices.count(); n++)
                {
                    if (devices.at(n)->RomID == RomID)
                    {
                        found = true;
                        devices.at(n)->channel = "1";
                    }
                }
                if (!found)
                {
                    device *dev = new device;
                    dev->RomID = RomID;
                    dev->localName = "";
                    dev->channel = "1";
                    devices.append(dev);
                    emit(newDevice(devices.last()->RomID, dev->localName));
                }
            }
            index ++;
            RomID = logisdom::getvalue(QString("ID%1").arg(index), channel);
        }
    }
    if (Ch2 != -1)
    {
        channel = data.mid(Ch2, channel.length() - Ch2);
        RomID = logisdom::getvalue(QString("ID%1").arg(index), channel);
        while (!RomID.isEmpty())
        {
            QString checkRomIDNull = RomID;
            checkRomIDNull.remove("0");
            if ((RomID.length() > 15) && (!checkRomIDNull.isEmpty()))
            {
                bool found = false;
                for (int n=0; n<devices.count(); n++)
                {
                    if (devices.at(n)->RomID == RomID)
                    {
                        found = true;
                        devices.at(n)->channel = "2";
                    }
                }
                if (!found)
                {
                    device *dev = new device;
                    dev->RomID = RomID;
                    dev->localName = "";
                    dev->channel = "2";
                    devices.append(dev);
                    emit(newDevice(devices.last()->RomID, dev->localName));
                }
            }
            index ++;
            RomID = logisdom::getvalue(QString("ID%1").arg(index), channel);
        }
    }
    emit(searchFinished());
}




void masterthread::LocalSearchAnalysis(const QString &dat)
{
    QString RomID;
    QStringList list = dat.split("\n");
    for (int i=0; i<list.count(); i++)
    {
        QString data = list[i];
        for (int n=0; n<LastDetector; n++)
        {
            RomID = logisdom::getvalue(getStr(n), data);
            QString checkRomIDNull = RomID;
            checkRomIDNull.remove("0");
            if ((RomID.length() > 15) && (!checkRomIDNull.isEmpty()))
            {
                bool found = false;
                for (int i=0; i<devices.count(); i++)
                {
                    if (devices.at(i)->RomID == RomID)
                    {
                        found = true;
                        devices.at(i)->localName = getStr(n);
                    }
                }
                if (!found)
                {
                    device *dev = new device;
                    dev->RomID = RomID;
                    dev->localName = getStr(n);
                    dev->channel = "";
                    devices.append(dev);
                    emit(newDevice(devices.last()->RomID, devices.last()->localName));
                }
            }
        }
        bool ok;
        int mode = logisdom::getvalue("modeProcess", data).toInt(&ok);
        if (ok)
        {
            if (modeProcess != mode)
            {
                modeProcess = mode;
                emit(modeChanged(mode));
            }
        }
    }
    emit(searchFinished());
}

