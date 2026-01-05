#pragma once

#include <QString>
#include <QDateTime>


/////////////////////////
/// General Functions ///
/////////////////////////

// Return actual timestamp as [yyyy-MM-dd HH:mm:ss:zzz]
QString getTimeStamp();


// NMEA
bool isNmeaChecksumValid(const QString &nmea);

QString getNmeaType(const QString& nmeaText);

quint8 calculateChecksum(const QString &payload);
