QT += core gui
QT += serialport network
QT += quick quickwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

INCLUDEPATH += src

SOURCES += \
    src/main.cpp \
    src/interface/interface.cpp \
    src/nmea/nmea_handler.cpp \
    src/readers/tcp_reader.cpp \
    src/readers/udp_reader.cpp \
    src/readers/serial_reader.cpp \
    src/writers/output_writer.cpp \
    src/writers/serial_writer.cpp \
    src/writers/text_file_writter.cpp \
    src/writers/udp_writer.cpp \
    src/nmea/utils.cpp \
    src/menu_bar/about/menubar_about.cpp \
    src/menu_bar/simu/menubar_simdata.cpp \
    src/menu_bar/faq/menubar_faq.cpp \
    src/menu_bar/data_monitor/menubar_datamonitor.cpp \
    src/menu_bar/decoded_nmea/menubar_decodednmea.cpp \
    src/menu_bar/txt_logger/menubar_txtlogger.cpp \
    src/menu_bar/connections/menubar_connections.cpp

HEADERS += \
    src/interface/interface.h \
    src/nmea/nmea_handler.h \
    src/readers/serial_reader.h \
    src/readers/tcp_reader.h \
    src/readers/udp_reader.h \
    src/writers/output_writer.h \
    src/writers/serial_writer.h \
    src/writers/text_file_writter.h \
    src/writers/udp_writer.h \
    src/nmea/utils.h \
    src/menu_bar/about/menubar_about.h \
    src/menu_bar/simu/menubar_simdata.h \
    src/menu_bar/faq/menubar_faq.h \
    src/menu_bar/data_monitor/menubar_datamonitor.h \
    src/menu_bar/decoded_nmea/menubar_decodednmea.h \
    src/menu_bar/txt_logger/menubar_txtlogger.h \
    src/menu_bar/connections/menubar_connections.h

FORMS += \
    src/interface/interface.ui \
    src/menu_bar/about/menubar_about.ui \
    src/menu_bar/simu/menubar_simdata.ui \
    src/menu_bar/faq/menubar_faq.ui \
    src/menu_bar/data_monitor/menubar_datamonitor.ui \
    src/menu_bar/decoded_nmea/menubar_decodednmea.ui \
    src/menu_bar/txt_logger/menubar_txtlogger.ui \
    src/menu_bar/connections/menubar_connections.ui

TRANSLATIONS += \
    resources/translations/OpenSeaNav_fr_FR.ts

RESOURCES += \
    resources/pictures/pictures.qrc \
    resources/qmls/qmls.qrc \
    resources/translations/translations.qrc


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
