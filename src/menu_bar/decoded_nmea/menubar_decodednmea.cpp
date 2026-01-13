#include "menubar_decodednmea.h"
#include "ui_menubar_decodednmea.h"


///////////////////
/// Class Setup ///
///////////////////
MenuBarDecodedNmea::MenuBarDecodedNmea(QWidget *parent) : QDialog(parent), ui(new Ui::MenuBarDecodedNmea)
{
    ui->setupUi(this);

    //Set title
    setWindowTitle(tr("Decoded NMEA"));

    // Enable minimize, maximize, close buttons
    Qt::WindowFlags flags = Qt::Dialog
                            | Qt::WindowMaximizeButtonHint
                            | Qt::WindowCloseButtonHint
                            | Qt::WindowStaysOnTopHint;
    setWindowFlags(flags);
    setAttribute(Qt::WA_ShowWithoutActivating);
}

MenuBarDecodedNmea::~MenuBarDecodedNmea()
{
    delete ui;
}


void MenuBarDecodedNmea::retranslate()
{
    ui->retranslateUi(this);
}

///////////////////
/// Update Data ///
///////////////////
void MenuBarDecodedNmea::updateDataGGA(QString time, double latitude, double longitude, int fixQuality, int numSatellites, double hdop, double altitude, double freqHz)
{
    ui->label_utcTime_gga->setText(time);
    ui->lcdNumber_latitude_gga->display(latitude);
    ui->lcdNumber_longitude_gga->display(longitude);
    ui->lcdNumber_fixQuality_gga->display(fixQuality);
    ui->lcdNumber_satellites_gga->display(numSatellites);
    ui->lcdNumber_hdop_gga->display(hdop);
    ui->lcdNumber_altitude_gga->display(altitude);
    ui->lcdNumber_frequency_gga->display(freqHz);
}

void MenuBarDecodedNmea::updateDataGLL(QString utc, double latitude, double longitude, double freqHz)
{
    ui->label_utcTime_gll->setText(utc);
    ui->lcdNumber_latitude_gll->display(latitude);
    ui->lcdNumber_longitude_gll->display(longitude);
    ui->lcdNumber_frequency_gll->display(freqHz);
}

void MenuBarDecodedNmea::updateDataGSV(int satellitesInView, double frequency)
{
    ui->lcdNumber_satellites_gsv->display(satellitesInView);
    ui->lcdNumber_frequency_gsv->display(frequency);
}

void MenuBarDecodedNmea::updateDataVTG(double track_true, double track_mag, double speed_knot, double speedKmh, double frequency)
{
    ui->lcdNumber_track_true_vtg->display(track_true);
    ui->lcdNumber_track_mag_vtg->display(track_mag);
    ui->lcdNumber_speed_knot_vtg->display(speed_knot);
    ui->lcdNumber_speed_kmh_vtg->display(speedKmh);
    ui->lcdNumber_frequency_vtg->display(frequency);
}

void MenuBarDecodedNmea::updateDataGSA(double pdop, double hdop, double vdop, double freqHz)
{
    ui->lcdNumber_pdop_gsa->display(pdop);
    ui->lcdNumber_hdop_gsa->display(hdop);
    ui->lcdNumber_vdop_gsa->display(vdop);
    ui->lcdNumber_frequency_gsa->display(freqHz);
}

void MenuBarDecodedNmea::updateDataRMC(QString utcDate, QString utcTime, double latitude, double longitude, double speedMps, double course, double magVar, double freqHz)
{
    ui->label_date_rmc->setText(utcDate);
    ui->label_utcTime_rmc->setText(utcTime);
    ui->lcdNumber_latitude_rmc->display(latitude);
    ui->lcdNumber_longitude_rmc->display(longitude);
    ui->lcdNumber_sog_rmc->display(speedMps);
    ui->lcdNumber_cog_rmc->display(course);
    ui->lcdNumber_magVar_rmc->display(magVar);
    ui->lcdNumber_frequency_rmc->display(freqHz);
}

void MenuBarDecodedNmea::updateDataHDT(double heading, double freqHz)
{
    ui->lcdNumber_heading_hdt->display(heading);
    ui->lcdNumber_frequency_hdt->display(freqHz);
}

void MenuBarDecodedNmea::updateDataDBT(double depthFeet, double depthMeters, double depthFathom, double freqHz)
{
    ui->lcdNumber_depth_feet_dbt->display(depthFeet);
    ui->lcdNumber_depth_meter_dbt->display(depthMeters);
    ui->lcdNumber_depth_fathom_dbt->display(depthFathom);
    ui->lcdNumber_frequency_dbt->display(freqHz);
}

void MenuBarDecodedNmea::updateDataVHW(double headingTrue, double headingMag, double speedKnots, double speedKmh, double freqHz)
{
    ui->lcdNumber_heading_true_vhw->display(headingTrue);
    ui->lcdNumber_heading_mag_vhw->display(headingMag);
    ui->lcdNumber_speed_knot_vhw->display(speedKnots);
    ui->lcdNumber_speed_kmh_vhw->display(speedKmh);
    ui->lcdNumber_frequency_vhw->display(freqHz);
}

void MenuBarDecodedNmea::updateDataZDA(QString date, QString time, QString offsetTime, double freqHz)
{
    ui->label_date_zda->setText(date);
    ui->label_utcTime_zda->setText(time);
    ui->label_localZone_zda->setText(offsetTime);
    ui->lcdNumber_frequency_zda->display(freqHz);
}

void MenuBarDecodedNmea::updateDataDPT(double depth, double offset, double freqHz)
{
    ui->lcdNumber_depth_dpt->display(depth);
    ui->lcdNumber_depth_offset_dpt->display(offset);
    ui->lcdNumber_frequency_dpt->display(freqHz);
}

void MenuBarDecodedNmea::updateDataMWD(double dir1, QString dir1Unit, double dir2, QString dir2Unit, double speed1, QString speed1Unit, double speed2, QString speed2Unit, double freqHz)
{
    ui->lcdNumber_windDirection_mwd->display(dir1);
    ui->label_windDirectionUnit_mwd->setText(tr("Wind direction") + "(" + dir1Unit + ") :" );
    ui->lcdNumber_windDirection_mwd_2->display(dir2);
    ui->label_windDirectionUnit_mwd_2->setText(tr("Wind direction") + "(" + dir2Unit + ") :" );
    ui->lcdNumber_windSpeed_mwd->display(speed1);
    ui->label_windSpeedUnit_mwd->setText(tr("Wind speed") + "(" + speed1Unit + ") :" );
    ui->lcdNumber_windSpeed_mwd_2->display(speed2);
    ui->label_windSpeedUnit_mwd_2->setText(tr("Wind speed") + "(" + speed2Unit + ") :" );
    ui->lcdNumber_frequency_mwd->display(freqHz);
}

void MenuBarDecodedNmea::updateDataMTW(double temp, QString tempUnit, double freqHz)
{
    ui->lcdNumber_waterTemp_mtw->display(temp);
    ui->label_waterTempUnit_mtw->setText(tr("Water temp.") + "(°" + tempUnit + ") :");
    ui->lcdNumber_frequency_mtw->display(freqHz);
}

void MenuBarDecodedNmea::updateDataMWV(double angle, QString ref, double speed, QString unit, double freqHz)
{
    ui->lcdNumber_windAngle_mwv->display(angle);
    ui->label_windAngleUnit_mwv->setText(tr("Wind angle") + "(" + ref + ") :" );
    ui->lcdNumber_windSpeed_mwv->display(speed);
    ui->label_windSpeedUnit_mwv->setText(tr("Wind speed") + "(" + unit + ") :" );
    ui->lcdNumber_frequency_mwv->display(freqHz);
}


//Clear
void MenuBarDecodedNmea::clearDecodedDataScreens()
{
    //RMC
    ui->label_utcTime_rmc->setText(tr("No data"));
    ui->label_date_rmc->setText(tr("No data"));
    ui->lcdNumber_latitude_rmc->display(0);
    ui->lcdNumber_longitude_rmc->display(0);
    ui->lcdNumber_sog_rmc->display(0);
    ui->lcdNumber_cog_rmc->display(0);
    ui->lcdNumber_magVar_rmc->display(0);
    ui->lcdNumber_frequency_rmc->display(0);

    //GGA
    ui->label_utcTime_gga->clear();
    ui->lcdNumber_latitude_gga->display(0);
    ui->lcdNumber_longitude_gga->display(0);
    ui->lcdNumber_satellites_gga->display(0);
    ui->lcdNumber_fixQuality_gga->display(0);
    ui->lcdNumber_hdop_gga->display(0);
    ui->lcdNumber_altitude_gga->display(0);
    ui->lcdNumber_frequency_gga->display(0);

    //GSA
    ui->lcdNumber_pdop_gsa->display(0);
    ui->lcdNumber_hdop_gsa->display(0);
    ui->lcdNumber_vdop_gsa->display(0);
    ui->lcdNumber_frequency_gsa->display(0);

    //GLL
    ui->label_utcTime_gll->setText(tr("No data"));
    ui->lcdNumber_latitude_gll->display(0);
    ui->lcdNumber_longitude_gll->display(0);
    ui->lcdNumber_frequency_gll->display(0);

    //GSV
    ui->lcdNumber_satellites_gsv->display(0);
    ui->lcdNumber_frequency_gsv->display(0);

    //VTG
    ui->lcdNumber_track_true_vtg->display(0);
    ui->lcdNumber_track_mag_vtg->display(0);
    ui->lcdNumber_speed_kmh_vtg->display(0);
    ui->lcdNumber_speed_knot_vtg->display(0);
    ui->lcdNumber_frequency_vtg->display(0);

    //HDT
    ui->lcdNumber_heading_hdt->display(0);
    ui->lcdNumber_frequency_hdt->display(0);

    //DBT
    ui->lcdNumber_depth_feet_dbt->display(0);
    ui->lcdNumber_depth_meter_dbt->display(0);
    ui->lcdNumber_depth_fathom_dbt->display(0);
    ui->lcdNumber_frequency_dbt->display(0);

    //DPT
    ui->lcdNumber_depth_dpt->display(0);
    ui->lcdNumber_depth_offset_dpt->display(0);
    ui->lcdNumber_frequency_dpt->display(0);

    //VHW
    ui->lcdNumber_heading_true_vhw->display(0);
    ui->lcdNumber_heading_mag_vhw->display(0);
    ui->lcdNumber_speed_knot_vhw->display(0);
    ui->lcdNumber_speed_kmh_vhw->display(0);
    ui->lcdNumber_frequency_vhw->display(0);

    //ZDA
    ui->label_utcTime_zda->setText(tr("No data"));
    ui->label_date_zda->setText(tr("No data"));
    ui->label_localZone_zda->setText(tr("No data"));
    ui->lcdNumber_frequency_zda->display(0);

    //MWD
    ui->lcdNumber_windDirection_mwd->display(0);
    ui->lcdNumber_windDirection_mwd_2->display(0);
    ui->lcdNumber_windSpeed_mwd->display(0);
    ui->lcdNumber_windSpeed_mwd_2->display(0);
    ui->lcdNumber_frequency_mwd->display(0);

    //MTW
    ui->lcdNumber_waterTemp_mtw->display(0);
    ui->lcdNumber_frequency_mtw->display(0);

    //MWV
    ui->lcdNumber_windAngle_mwv->display(0);
    ui->lcdNumber_windSpeed_mwv->display(0);
    ui->lcdNumber_frequency_mwv->display(0);
}

void MenuBarDecodedNmea::on_pushButton_clear_data_clicked()
{
    clearDecodedDataScreens();
}


