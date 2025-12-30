#pragma once

#include <QString>
#include <QDateTime>


/////////////////////////
/// General Functions ///
/////////////////////////

// Return actual timestamp as [yyyy-MM-dd HH:mm:ss:zzz]
QString getTimeStamp();

// Check if NMEA sentence integrity
bool isNmeaChecksumValid(const QString &nmea);
