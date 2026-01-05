#include "serial_writer.h"

//////////////
/// Class  ///
//////////////
SerialWriter::SerialWriter(QObject *parent) : OutputWriter(parent)
{

}

SerialWriter::~SerialWriter()
{

}



/////////////////
/// Functions ///
/////////////////
void SerialWriter::sendData(const QString &line)
{
    if(getSocketOutputActivated())
    {
        QByteArray data = line.toUtf8() + "\r\n";
        serial.write(data);
    }
}



//////////////////
/// Connection ///
//////////////////
bool SerialWriter::openSerialDevice()
{
    return serial.open(QIODevice::WriteOnly);
}

void SerialWriter::closeSerialDevice()
{
    if(isSerialOpen())
        serial.close();
}



//////////////////////
/// Set parameters ///
//////////////////////
void SerialWriter::setPortName(const QString &portName)
{
    serial.setPortName(portName);
}

void SerialWriter::setBaudRate(qint32 baudRate)
{
    serial.setBaudRate(baudRate);
}



////////////////
/// Get data ///
////////////////
QString SerialWriter::getPortName()
{
    return serial.portName();
}

QString SerialWriter::getErrorString()
{
    return serial.errorString();
}

bool SerialWriter::isSerialOpen()
{
    return serial.isOpen();
}

