#pragma once

#include <QSerialPort>

#include "writers/output_writer.h"


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
