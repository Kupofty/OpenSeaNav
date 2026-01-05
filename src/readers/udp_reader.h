#pragma once

#include <QHostAddress>
#include <QObject>
#include <QUdpSocket>
#include <QDebug>

class UdpReader : public QObject
{
    Q_OBJECT

    public:
        explicit UdpReader(QObject *parent = nullptr);
        ~UdpReader();

        void updatePort(int port);
        QString getSenderDetails();
        bool isBounded();

        QString connect();
        QString disconnect();

    signals:
        void newLineReceived(const QByteArray &line);
        void newSenderDetails();

    private slots:
        void processPendingDatagrams();

    private:
        QUdpSocket *socket;
        int udpPort;
        QHostAddress senderIP;
        quint16 senderPort;
};
