#pragma once

#include <QCoreApplication>
#include <QObject>
#include <QDebug>
#include <QTime>

#include "utils.h"

class NMEA_Handler : public QObject
{
    Q_OBJECT

    public:
        NMEA_Handler();
        ~NMEA_Handler();

    public slots:
        void handleRawSentences(const QByteArray &line);

    private:
        //Frequency timers
        QElapsedTimer timer_gsv, timer_gga, timer_vtg, timer_gll, timer_gsa, timer_rmc, timer_hdt, timer_dbt, timer_vhw,
                      timer_zda, timer_dpt, timer_mwd, timer_mwv, timer_mtw;

        qint64 lastUpdateTimeGGA = -1, lastUpdateTimeGSV = -1, lastUpdateTimeRMC = -1, lastUpdateTimeVTG = -1,
               lastUpdateTimeGLL = -1, lastUpdateTimeGSA = -1, lastUpdateTimeHDT = -1, lastUpdateTimeDBT = -1,
               lastUpdateTimeVHW = -1, lastUpdateTimeDPT = -1, lastUpdateTimeZDA = -1, lastUpdateTimeMWD = -1,
               lastUpdateTimeMWV = -1, lastUpdateTimeMTW = -1;

    private:
        void handleGGA(const QList<QByteArray> &fields);
        void handleRMC(const QList<QByteArray> &fields);
        void handleGSV(const QList<QByteArray> &fields);
        void handleGLL(const QList<QByteArray> &fields);
        void handleGSA(const QList<QByteArray> &fields);
        void handleVTG(const QList<QByteArray> &fields);
        void handleHDT(const QList<QByteArray> &fields);
        void handleDBT(const QList<QByteArray> &fields);
        void handleVHW(const QList<QByteArray> &fields);
        void handleZDA(const QList<QByteArray> &fields);
        void handleDPT(const QList<QByteArray> &fields);
        void handleMWD(const QList<QByteArray> &fields);
        void handleMWV(const QList<QByteArray> &fields);
        void handleMTW(const QList<QByteArray> &fields);

        double calculateCoordinates(const QString &valueStr, const QString &direction);
        double calculateFrequency(QElapsedTimer &timer, qint64 &lastTime);
        QByteArray removeAsterisk(const QByteArray lastField);
        bool isNmeaMinimumSize(const QList<QByteArray> &fields, int minSize);

    signals:
        void newNMEASentence(const QString &type, const QString &nmeaText);

        void newDecodedGSA(double pdop, double hdop, double vdop, double freqHz);
        void newDecodedGSV(int totalSatellites, double freqHz);
        void newDecodedGLL(QString utc, double latitude, double longitude, double freqHz);
        void newDecodedGGA(QString utc, double latitude, double longitude, int fixQuality, int numSatellites, double hdop, double altitude, double freqHz);
        void newDecodedVTG(double track_true, double track_mag, double speed_kn, double speedKmh, double freqHz);
        void newDecodedRMC(QString utcDate, QString utcTime, double latitude, double longitude, double speedMps, double course, double magVar, double freqHz);
        void newDecodedHDT(double heading, double freqHz);
        void newDecodedDBT(double depthFeet, double depthMeters, double depthFathom, double freqHz);
        void newDecodedVHW(double headingTrue, double headingMag, double speedKnots, double speedKmh, double freqHz);
        void newDecodedZDA(QString dateStr, QString timeStr, QString offsetStr, double freqHz);
        void newDecodedDPT(double depth, double offset, double freqHz);
        void newDecodedMWD(double dir1, QString dir1Unit, double dir2, QString dir2Unit, double speed1, QString speed1Unit, double speed2, QString speed2Unit, double freqHz);
        void newDecodedMTW(double temp, QString tempUnit, double freqHz);
        void newDecodedMWV(double angle, QString ref, double speed, QString unit, double freqHz);
};
