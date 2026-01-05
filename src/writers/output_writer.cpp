#include "output_writer.h"


/////////////
/// Class ///
/////////////

OutputWriter::OutputWriter(QObject *parent) : QObject{parent}
{
    //Define outputFlags map
    for (const QString &type : acceptedNmeaList) {
        outputFlags[type] = false;
    }
}

OutputWriter::~OutputWriter()
{

}



/////////////////
/// Functions ///
/////////////////
void OutputWriter::sendData(const QString &line)
{
    //virtual function
    Q_UNUSED(line);
}



////////////////
/// Get data ///
////////////////
bool OutputWriter::getSocketOutputActivated()
{
    return socketOutputActivated;
}



//////////////////////
/// Update outputs ///
//////////////////////
void OutputWriter::updateSocketOutputActivated(bool check)
{
    socketOutputActivated = check;
}

void OutputWriter::updateOutputNMEA(const QString &type, bool check)
{
    if (outputFlags.contains(type))
        outputFlags[type] = check;
    else
        qDebug() << "Unknown type";
}



////////////////////
/// Public slots ///
////////////////////
void OutputWriter::publishNMEA(const QString &type, const QString &nmeaText)
{
    if (outputFlags[type])
        sendData(nmeaText);
}


