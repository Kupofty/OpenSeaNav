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
    src/readers/udp_reader.cpp \
    src/readers/serial_reader.cpp \
    src/writers/output_writer.cpp \
    src/writers/serial_writer.cpp \
    src/writers/text_file_writter.cpp \
    src/writers/udp_writer.cpp \
    src/nmea/utils.cpp \
    src/menu_bar/about/menubar_about.cpp \
    src/menu_bar/simu/menubar_simdata.cpp \
    src/menu_bar/faq/menubar_faq.cpp

HEADERS += \
    src/interface/interface.h \
    src/nmea/nmea_handler.h \
    src/readers/serial_reader.h \
    src/readers/udp_reader.h \
    src/writers/output_writer.h \
    src/writers/serial_writer.h \
    src/writers/text_file_writter.h \
    src/writers/udp_writer.h \
    src/nmea/utils.h \
    src/menu_bar/about/menubar_about.h \
    src/menu_bar/simu/menubar_simdata.h \
    src/menu_bar/faq/menubar_faq.h

FORMS += \
    src/interface/interface.ui \
    src/menu_bar/about/menubar_about.ui \
    src/menu_bar/simu/menubar_simdata.ui \
    src/menu_bar/faq/menubar_faq.ui


RESOURCES += \
    resources/images.qrc \
    resources/qml.qrc


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

