#include "utils.h"


/////////////////////////
/// General Functions ///
/////////////////////////
QString getTimeStamp()
{
    return "[" + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss:zzz") + "] ";
}



/////////////////////////////
/// TCP Related Functions ///
/////////////////////////////
bool isIpAddressValid(QString ip_address)
{
    if(ip_address == "localhost")
        return true;

    QStringList octets = ip_address.split('.');
    bool valid_ip = true;

    //Check if 4 segments
    if (octets.size() != 4)
    {
        valid_ip = false;
    }

    //Check if each segment is between 0 and 255
    for (const QString &octet : octets)
    {
        bool ok;
        int value = octet.toInt(&ok);
        if (!ok || value < 0 || value > 255)
        {
            valid_ip = false;
        }
    }

    return valid_ip;
}


//////////////////////////////
/// NMEA Related Functions ///
//////////////////////////////
bool isNmeaChecksumValid(const QString &nmea)
{
    if (nmea.isEmpty())
        return false;

    // Sentence must start with '$' or '!'
    const QChar start = nmea.at(0);
    if (start != '$' && start != '!')
        return false;

    int startIdx = 0;
    int starIdx  = nmea.indexOf('*');

    if (starIdx <= startIdx + 1)
        return false;

    quint8 checksum = 0;
    for (int i = startIdx + 1; i < starIdx; ++i)
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
