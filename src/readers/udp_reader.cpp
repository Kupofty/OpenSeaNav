#include "udp_reader.h"

/////////////
/// Class ///
/////////////
UdpReader::UdpReader(QObject *parent) : QObject(parent), socket(new QUdpSocket(this))
{
    QObject::connect(socket, &QUdpSocket::readyRead, this, &UdpReader::processPendingDatagrams);
}

UdpReader::~UdpReader()
{
    socket->close();
}



//////////////////
/// Connection ///
//////////////////
QString UdpReader::connect()
{
    QString result;
    if (socket->bind(QHostAddress::AnyIPv4, udpPort))
        result = tr("UDP bind succesfull");
    else
         result = tr("UDP bind failed : ") + socket->errorString();

     return result;
}

QString UdpReader::disconnect()
{
    if(isBounded())
    {
        socket->close();
        return tr("UDP socket closed");
    }
    else
        return tr("UDP socket not opened");
}



//////////////////////
/// Set parameters ///
//////////////////////
void UdpReader::updatePort(int port)
{
    udpPort = port;
}



///////////
/// Get ///
///////////
QString UdpReader::getSenderDetails()
{
    QString ipStr;
    if (senderIP.protocol() == QAbstractSocket::IPv6Protocol && senderIP.toString().startsWith("::ffff:"))
        ipStr = senderIP.toString().mid(0, 7);  // Remove "::ffff:"
    else
        ipStr = senderIP.toString();

    return ipStr + ":" + QString::number(senderPort);
}

bool UdpReader::isBounded()
{
    return (socket->state() == QAbstractSocket::BoundState);
}



///////////////////
/// Handle data ///
///////////////////
void UdpReader::processPendingDatagrams()
{
    QByteArray datagram;

    while (socket->hasPendingDatagrams())
    {
        datagram.resize(int(socket->pendingDatagramSize()));
        socket->readDatagram(datagram.data(), datagram.size(), &senderIP, &senderPort);

        datagram = datagram.trimmed();

        QString sender = tr("UDP → ") +  QString::number(udpPort);
        emit newLineReceived(sender, datagram);
        emit newSenderDetails();
    }
}

