#include "tcp_reader.h"


///////////////////
/// Class Setup ///
///////////////////
TcpReader::TcpReader(QObject *parent) : QObject(parent), socket(new QTcpSocket(this))
{
    connect(socket, &QTcpSocket::readyRead, this, &TcpReader::onReadyRead);
    connect(socket, &QTcpSocket::connected, this, &TcpReader::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &TcpReader::onDisconnected);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred), this, &TcpReader::onError);
}

TcpReader::~TcpReader()
{
    disconnect();
}



///////////////////
/// Connections ///
///////////////////
void TcpReader::connectToHost(const QString &ip, quint16 port)
{
    this->ip = ip;
    this->port = port;
    socket->connectToHost(ip, port);
    emit updateState(tr("Trying to connect..."));
}

void TcpReader::abortConnection()
{
    if(!isConnectedOrConnecting())
        emit updateState(tr("Socket not connected"));
    else
    {
        socket->abort();
        emit updateState(tr("Disconnected"));
    }

}

bool TcpReader::isConnectedOrConnecting()
{
    return (socket->state() == QAbstractSocket::ConnectedState ||
            socket->state() == QAbstractSocket::ConnectingState);
}



/////////////
/// Slots ///
/////////////
void TcpReader::onReadyRead()
{
    const QString senderDetails = "TCP→" + ip + ":" + QString::number(port);

    buffer.append(socket->readAll());

    int index;
    while ((index = buffer.indexOf("\r\n")) != -1) {
        QByteArray line = buffer.left(index);
        buffer.remove(0, index + 2); // remove line + CRLF

        emit newLineReceived(senderDetails, line);
    }
}

void TcpReader::onConnected()
{
    emit updateState(tr("Connected"));
}

void TcpReader::onDisconnected()
{
    emit updateState(tr("Disconnected"));
}

void TcpReader::onError(QAbstractSocket::SocketError)
{
    emit updateState(tr("Error: ") + socket->errorString());
}

