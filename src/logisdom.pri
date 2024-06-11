DESTDIR = $${BUILD_PATH}
CODECFORTR = UTF-8
CODECFORSRC = UTF-8
#QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_STL
unix:QMAKE_CXXFLAGS += -Wno-deprecated-copy
TEMPLATE = app
DEPENDPATH += $${QWT_PATH}
INCLUDEPATH += $${PWD} $${QWT_PATH}/src $${QUAZIP_PATH}/quazip $$[QT_INSTALL_HEADERS]/QtZlib
UI_DIR = $$OUT_PWD/build/ui
MOC_DIR = $$OUT_PWD/build/moc
RCC_DIR = $$OUT_PWD/build/rcc
OBJECTS_DIR = $$OUT_PWD/build/obj
TRANSLATIONS += trans/logisdom_fr.ts
QT += network core gui xml widgets serialport mqtt
win32:QT += core5compat
CONFIG += qt qwt quazip zlib thread exceptions
RESOURCES += src/onewire.qrc
HEADERS += ../plugins/interface.h \
 header/devicelistmodel.h \
 header/addDaily.h \
 header/filesave.h \
 header/addicons.h \
 header/addProgram.h \
 header/alarmwarn.h \
 header/axb.h \
 header/backup.h \
 header/calcthread.h \
 header/calc.h \
 header/chauffageunit.h \
 header/histo.h \
 header/configmanager.h \
 header/configwindow.h \
 header/commonstring.h \
 header/connection.h \
 header/curve.h \
 header/dataloader.h \
 header/daily.h \
 header/deadevice.h \
 header/devchooser.h \
 header/devfinder.h \
 header/devrps2.h \
 header/devresol.h \
 header/devvirtual.h \
 header/formula.h \
 header/energiesolaire.h \
 header/errlog.h \
 header/global.h \
 header/globalvar.h \
 header/graph.h \
 header/graphconfig.h \
 header/highlighter.h \
 header/htmlbinder.h \
 header/iconearea.h \
 header/iconf.h \
 header/icont.h \
 header/inputdialog.h \
 header/interval.h \
 header/lcdonewire.h \
 header/ledonewire.h \
 header/linecoef.h \
 header/logisdom.h \
 header/mailsender.h \
 header/messagebox.h \
 header/meteo.h \
 header/net1wire.h \
 header/onewire.h \
 header/pieceslist.h \
 header/pngthread.h \
 header/plot.h \
 header/programevent.h \
 header/remote.h \
 header/reprocessthread.h \
 header/rps2.h \
 header/resol.h \
 header/server.h \
 header/sendmailthread.h \
 header/sendsmsthread.h \
 header/simplecrypt.h \
 header/tableau.h \
 header/tableauconfig.h \
 header/tcpdata.h \
 header/textedit.h \
 header/treehtmlwidget.h \
 header/vmc.h \
 header/weathercom.h \
 header/remotethread.h

FORMS += forms/addProgram.ui \
 $$PWD/configwind.ui \
 forms/alarmwarn.ui \
 forms/configgui.ui \
 forms/devvirtual.ui \
 forms/dailyui.ui \
 forms/devfinder.ui \
 forms/deadevice.ui \
 forms/energiesolaire.ui \
 forms/errlog.ui \
 forms/formchauffage.ui \
 forms/formula.ui \
 forms/guinet1wire.ui \
 forms/lcdonewire.ui \
 forms/ledonewire.ui \
 forms/mainw.ui \
 forms/onewiredevice.ui \
 forms/loadicon.ui \
 forms/stdui.ui \
 forms/switchui.ui \
 forms/tableau.ui


SOURCES += src/addDaily.cpp \
 $$PWD/devicelistmodel.cpp \
 src/filesave.cpp \
 src/addicons.cpp \
 src/addProgram.cpp \
 src/alarmwarn.cpp \
 src/axb.cpp \
 src/backup.cpp \
 src/chauffageunit.cpp \
 src/histo.cpp \
 src/configmanager.cpp \
 src/configwindow.cpp \
 src/connection.cpp \
 src/commonstring.cpp \
 src/curve.cpp \
 src/daily.cpp \
 src/dataloader.cpp \
 src/devfinder.cpp \
 src/devrps2.cpp \
 src/devresol.cpp \
 src/devvirtual.cpp \
 src/deadevice.cpp \
 src/devchooser.cpp \
 src/energiesolaire.cpp \
 src/errlog.cpp \
 src/formula.cpp \
 src/calcthread.cpp \
 src/calc.cpp \
 src/graph.cpp \
 src/graphconfig.cpp \
 src/highlighter.cpp \
 src/htmlbinder.cpp \
 src/iconearea.cpp \
 src/iconf.cpp \
 src/icont.cpp \
 src/inputdialog.cpp \
 src/interval.cpp \
 src/lcdonewire.cpp \
 src/ledonewire.cpp \
 src/linecoef.cpp \
 src/mailsender.cpp \
 src/main.cpp \
 src/meteo.cpp \
 src/messagebox.cpp \
 src/logisdom.cpp \
 src/net1wire.cpp \
 src/onewire.cpp \
 src/pieceslist.cpp \
 src/pngthread.cpp \
 src/plot.cpp \
 src/programevent.cpp \
 src/remote.cpp \
 src/reprocessthread.cpp \
 src/rps2.cpp \
 src/resol.cpp \
 src/server.cpp \
 src/sendmailthread.cpp \
 src/sendsmsthread.cpp \
 src/simplecrypt.cpp \
 src/tableau.cpp \
 src/tableauconfig.cpp \
 src/tcpdata.cpp \
 src/textedit.cpp \
 src/treehtmlwidget.cpp \
 src/vmc.cpp \
 src/weathercom.cpp \
 src/remotethread.cpp
