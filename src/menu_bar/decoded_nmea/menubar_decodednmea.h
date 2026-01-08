#ifndef MENUBAR_DECODEDNMEA_H
#define MENUBAR_DECODEDNMEA_H

#include <QDialog>

namespace Ui {
class MenuBarDecodedNmea;
}

class MenuBarDecodedNmea : public QDialog
{
    Q_OBJECT

    public:
        explicit MenuBarDecodedNmea(QWidget *parent = nullptr);
        ~MenuBarDecodedNmea();
        void clearDecodedDataScreens();

    private:
        Ui::MenuBarDecodedNmea *ui;

    public slots:
        void updateDataGGA(QString time, double latitude, double longitude, int fixQuality, int numSatellites, double hdop, double altitude, double freqHz);
        void updateDataGLL(QString utc, double latitude, double longitude, double freqHz);
        void updateDataGSV(int satellitesInView, double frequency);
        void updateDataVTG(double track_true, double track_mag, double speed_knot, double speedKmh, double frequency);
        void updateDataGSA(double pdop, double hdop, double vdop, double freqHz);
        void updateDataRMC(QString utcDate, QString utcTime, double latitude, double longitude, double speedMps, double course, double magVar, double freqHz);
        void updateDataHDT(double heading, double freqHz);
        void updateDataDBT(double depthFeet, double depthMeters, double depthFathom, double freqHz);
        void updateDataVHW(double headingTrue, double headingMag, double speedKnots, double speedKmh, double freqHz);
        void updateDataZDA(QString date, QString time, QString offsetTime, double freqHz);
        void updateDataDPT(double depth, double offset, double freqHz);
        void updateDataMWD(double dir1, QString dir1Unit, double dir2, QString dir2Unit, double speed1, QString speed1Unit, double speed2, QString speed2Unit, double freqHz);
        void updateDataMTW(double temp, QString tempUnit, double freqHz);
        void updateDataMWV(double angle, QString ref, double speed, QString unit, double freqHz);

};

#endif // MENUBAR_DECODEDNMEA_H
