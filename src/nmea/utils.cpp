#include "utils.h"


/////////////////////////
/// General Functions ///
/////////////////////////
QString getTimeStamp()
{
    return "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss:zzz") + "] ";
}



//////////////////////////////
/// NMEA Related Functions ///
//////////////////////////////
bool isNmeaChecksumValid(const QString &nmea)
{
    int dollarIdx = nmea.indexOf('$');
    int starIdx   = nmea.indexOf('*');

    if (dollarIdx == -1 || starIdx == -1 || starIdx <= dollarIdx + 1)
        return false;

    quint8 checksum = 0;
    for (int i = dollarIdx + 1; i < starIdx; ++i)
        checksum ^= nmea.at(i).toLatin1();

    bool ok = false;
    quint8 received = nmea.mid(starIdx + 1, 2).toUInt(&ok, 16);
    return ok && (checksum == received);
}

QString getNmeaType(const QString& nmeaText)
{
    QString nmeaType;
    int dollarIdx = nmeaText.indexOf('$');
    int commaIdx  = nmeaText.indexOf(',', dollarIdx);
    if (dollarIdx != -1 && commaIdx != -1 && commaIdx > dollarIdx)
    {
        nmeaType = nmeaText.mid(dollarIdx + 1, commaIdx - dollarIdx - 1).toUpper();
    }

    return nmeaType;
}

quint8 calculateChecksum(const QString &payload)
{
    quint8 checksum = 0;
    for (const char c : payload.toUtf8()) {
        checksum ^= static_cast<quint8>(c);
    }
    return checksum;
}
