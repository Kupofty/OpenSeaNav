QT += core gui
QT += serialport network
QT += quick quickwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

INCLUDEPATH += include

SOURCES += \
    src/interface.cpp \
    src/menubar_about.cpp \
    src/nmea_handler.cpp \
    src/main.cpp \
    src/output_writer.cpp \
    src/serial_reader.cpp \
    src/serial_writer.cpp \
    src/text_file_writter.cpp \
    src/udp_reader.cpp \
    src/udp_writer.cpp \
    src/utils.cpp \
    src/menubar_github.cpp

HEADERS += \
    include/interface.h \
    include/nmea_list.h \
    include/output_writer.h \
    include/serial_reader.h \
    include/nmea_handler.h \
    include/serial_writer.h \
    include/text_file_writter.h \
    include/udp_reader.h \
    include/udp_writer.h \
    include/utils.h \
    include/menubar_about.h \
    include/menubar_github.h

FORMS += \
    ui/menubar_about.ui \
    ui/interface.ui \
    ui/menubar_github.ui

RESOURCES += \
    resources/images.qrc \
    resources/qml.qrc


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

