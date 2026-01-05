#include "serial_reader.h"

/////////////
/// Class ///
/////////////
SerialReader::SerialReader()
{
    //Set serial parameters
    serial.setPortName("COM0");
    serial.setBaudRate(QSerialPort::Baud115200);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);

    //Connect
    connect(&serial, &QSerialPort::readyRead, this, &SerialReader::readData);
}

SerialReader::~SerialReader()
{
    closeSerialDevice();
}



//////////////////
/// Connection ///
//////////////////
bool SerialReader::openSerialDevice()
{
    return serial.open(QIODevice::ReadOnly);
}

void SerialReader::closeSerialDevice()
{
    if(isSerialOpen())
        serial.close();
}


///////////////////
/// Handle data ///
///////////////////
void SerialReader::readData()
{
    const QByteArray data = serial.readAll();
    buffer.append(data);

    while (buffer.contains('\n'))
    {
        int index = buffer.indexOf('\n');
        QByteArray line = buffer.left(index).trimmed();
        buffer.remove(0, index + 1);

        emit newLineReceived(line);
    }
}



//////////////////////
/// Set parameters ///
//////////////////////
void SerialReader::setPortName(const QString &portName)
{
    serial.setPortName(portName);
}

void SerialReader::setBaudRate(qint32 baudRate)
{
    serial.setBaudRate(baudRate);
}



////////////////
/// Get data ///
////////////////
QString SerialReader::getPortName()
{
    return serial.portName();
}

QString SerialReader::getErrorString()
{
    return serial.errorString();
}

bool SerialReader::isSerialOpen()
{
    return serial.isOpen();
}

