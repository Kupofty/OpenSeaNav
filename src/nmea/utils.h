#pragma once

#include <QString>
#include <QDateTime>
#include <QApplication>
#include <QStringList>


/////////////////
/// Variables ///
/////////////////
inline const QStringList acceptedNmeaList = {
    "DBT", "DPT", "GGA", "GLL", "GSA",
    "GSV", "HDT", "MTW", "MWD", "MWV",
    "OTHER", "RMC", "TXT", "VHW", "VTG",
    "ZDA"
};


/////////////////////////
/// General Functions ///
/////////////////////////

// Return actual timestamp as [yyyy-MM-dd HH:mm:ss:zzz]
QString getTimeStamp();


// NMEA
bool isNmeaChecksumValid(const QString &nmea);

QString getNmeaType(const QString& nmeaText);

quint8 calculateChecksum(const QString &payload);
