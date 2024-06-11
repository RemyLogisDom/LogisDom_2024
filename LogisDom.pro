message(Project path : $$OUT_PWD)
message(Source path : $$PWD)

QWT_PATH = $${OUT_PWD}/qwt-6.2.0
message(QWT path : $$QWT_PATH)
LIBS += -lqwt -L$${QWT_PATH}/lib
win32:LIBS += -llibssl -LC:\Qt\Tools\OpenSSLv3\Win_x64\lib
INCLUDEPATH += header

win32:QUAZIP_PATH = $${OUT_PWD}/quazip-qt6
win32:INCLUDEPATH += ../Desktop_Qt_6_5_3_MinGW_64_bit-Release/quazip-qt6/quazip
unix:QUAZIP_PATH = $${OUT_PWD}/quazip-0.7.3
unix:INCLUDEPATH += $${OUT_PWD}/quazip-0.7.3
message(Quazip path : $$QUAZIP_PATH)
#win32:LIBS *= -lquazip1-qt6 -L$${QUAZIP_PATH}/lib
LIBS += -lquazip -L$${QUAZIP_PATH}/lib
#QT_NO_DEBUG_OUTPUT
debug::BUILD_PATH = $$OUT_PWD/debug
release::BUILD_PATH = $$OUT_PWD/release

TARGET = LogisDom
#DEFINES += _TTY_POSIX_ POSIX


include (teleinfo/teleinfo.pri)
include (devonewire/devonewire.pri)
include (deveo/deveo.pri)
include (ha7s/ha7s.pri)
include (mqtt/mqtt.pri)
include (ha7net/ha7net.pri)
include (enocean/enocean.pri)
include (mbus/mbus.pri)
include (fts800/fts800.pri)
include (modbus/modbus.pri)
include (ecogest/ecogest.pri)
include (mail/mail.pri)
include (src/logisdom.pri)

RESOURCES += \
    mqtt/mqtt.qrc

