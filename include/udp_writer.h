#pragma once

#include "output_writer.h"

#include <QUdpSocket>
#include <QHostAddress>

class UdpWriter : public OutputWriter
{
    Q_OBJECT

    public:
        explicit UdpWriter(QObject *parent = nullptr);
        ~UdpWriter();

        void sendData(const QString &data) override;

        void updateUdpPort(int port);
        void updateUdpMethod(const QHostAddress &udpAddress);

    private:
        QUdpSocket udpSocket;
        QHostAddress udpAddress = QHostAddress::Broadcast;
        int udpPort = 1024;
};
