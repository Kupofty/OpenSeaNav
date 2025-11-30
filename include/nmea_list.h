#pragma once

#include <QApplication>
#include <QStringList>


inline const QStringList acceptedNmeaList = {
    "DBT", "DPT", "GGA", "GLL", "GSA",
    "GSV", "HDT", "MTW", "MWD", "MWV",
    "OTHER", "RMC", "TXT", "VHW", "VTG",
    "ZDA"
};
