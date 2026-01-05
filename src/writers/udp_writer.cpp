#include "udp_writer.h"

/////////////
/// Class ///
/////////////
UdpWriter::UdpWriter(QObject *parent) : OutputWriter(parent)
{

}

UdpWriter::~UdpWriter()
{

}



/////////////////
/// Functions ///
/////////////////
void UdpWriter::sendData(const QString &line)
{
    if(getSocketOutputActivated())
    {
        QByteArray data = line.toUtf8();
        udpSocket.writeDatagram(data, udpAddress, udpPort);
    }
}



///////////////////////
/// Update settings ///
///////////////////////
void UdpWriter::updateUdpPort(int port)
{
    udpPort = port;
}

void UdpWriter::updateUdpMethod(const QHostAddress &method)
{
    udpAddress = method;
}

