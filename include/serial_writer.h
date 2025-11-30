#pragma once

#include "output_writer.h"

#include <QSerialPort>

//Need to create serial class and inherit from it & from output_writer
// problem -> inherit 2 time from QObject

class SerialWriter : public OutputWriter
{
    Q_OBJECT

    public:
        explicit SerialWriter(QObject *parent = nullptr);
        ~SerialWriter();

        void sendData(const QString &data) override;

        bool openSerialDevice();
        void closeSerialDevice();

        //Set
        void setPortName(const QString &portName);
        void setBaudRate(qint32 baudRate);

        //Get
        QString getPortName();
        QString getErrorString();
        bool isSerialOpen();

    private:
        QSerialPort serial;
        QByteArray buffer;
};
