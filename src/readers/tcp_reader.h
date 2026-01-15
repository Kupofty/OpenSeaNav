#pragma once

#include <QObject>
#include <QTcpSocket>

class TcpReader : public QObject
{
    Q_OBJECT

    public:
        explicit TcpReader(QObject *parent = nullptr);
        ~TcpReader();

        void connectToHost(const QString &ip, quint16 port);
        void abortConnection();
        bool isConnectedOrConnecting();

    signals:
        void newLineReceived(QString tcpSender, const QByteArray &line);
        void dataReceived(const QByteArray &data);
        void updateState(QString state);

    private slots:
        void onReadyRead();
        void onConnected();
        void onDisconnected();
        void onError(QAbstractSocket::SocketError socketError);

    private:
        QTcpSocket *socket;
        QByteArray buffer;
        QString ip;
        quint16 port;

};
