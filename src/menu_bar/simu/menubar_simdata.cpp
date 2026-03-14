#include "menubar_simdata.h"
#include "ui_menubar_simdata.h"

///////////////////
/// Class Setup ///
///////////////////
MenuBarSimData::MenuBarSimData(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MenuBarSimData)
{
    //Setup UI
    ui->setupUi(this);
    ui->tabWidget->setCurrentWidget(ui->tab_auto_generation);

    setWindowTitle(tr("Data input simulator"));
    setAttribute(Qt::WA_DeleteOnClose);

    //Timers
    autoSendManualDataTimer.setInterval(defaultTimeMsTimer);
    autoSendSimuDataTimer.setInterval(defaultTimeMsTimer);

    connect(&autoSendManualDataTimer, &QTimer::timeout, this, &MenuBarSimData::sendManualInputData);
    connect(&autoSendSimuDataTimer, &QTimer::timeout, this, &MenuBarSimData::sendSimuData);
}

MenuBarSimData::~MenuBarSimData()
{
    delete ui;
}



/////////////////
/// Functions ///
/////////////////

//Common
QString MenuBarSimData::formatLatitude(double lat)
{
    lat = std::abs(lat);

    int degrees = int(lat);
    double minutes = (lat - degrees) * 60.0;

    double ddmm = degrees * 100.0 + minutes;

    return QString::number(ddmm, 'f', 8);
}

QString MenuBarSimData::formatLongitude(double lon)
{
    lon = std::abs(lon);

    int degrees = int(lon);
    double minutes = (lon - degrees) * 60.0;

    double dddmm = degrees * 100.0 + minutes;

    return QString::number(dddmm, 'f', 8);
}

QString MenuBarSimData::getTime()
{
    QDateTime now = QDateTime::currentDateTimeUtc();
    QString utcTime = now.toString("hhmmss.zzz").left(9);

    return utcTime;
}

void MenuBarSimData::sendNmeaData(const QString &payload)
{
    //Auto calculate checksum
    quint8 checksum = calculateChecksum(payload);

    //Create NMEA sentence = payload + checksum
    QString nmea = QString("$%1*%2")
                       .arg(payload)
                       .arg(checksum, 2, 16, QLatin1Char('0'));

    //Emit new sentence
    emit dataReady(sender, nmea.toUtf8());
}


// Send Data
void MenuBarSimData::sendManualInputData()
{
    QString payload = ui->lineEdit_manual_input->text().trimmed();
    QString checksumText = ui->label_checksum->text();

    if (payload.isEmpty() || checksumText.isEmpty())
        return;

    sendNmeaData(payload);
}

void MenuBarSimData::sendSimuData()
{
    //GLL (Position)
    if(ui->checkBox_GLL->isChecked())
        sendGLL();

    //DBT (Depth)
    if(ui->checkBox_DBT->isChecked())
        sendDBT();

    //DPT (Depth + Offset)
    if(ui->checkBox_DPT->isChecked())
        sendDPT();

    //GSV (Satellites)
    if(ui->checkBox_GSV->isChecked())
        sendGSV();

    //HDT (True Heading)
    if(ui->checkBox_HDT->isChecked())
        sendHDT();

    //MTW (Water Temperature)
    if(ui->checkBox_MTW->isChecked())
        sendMTW();

    //VTG (Speed & Heading)
    if(ui->checkBox_VTG->isChecked())
        sendVTG();

    //RMC (Position & SOG & COG & Date/Time)
    if(ui->checkBox_RMC->isChecked())
        sendRMC();

    //MWV (Winds speed & angle)
    if(ui->checkBox_MWV->isChecked())
        sendMWV();
}

void MenuBarSimData::sendRMC()
{
    QString date = QDateTime::currentDateTimeUtc().toString("ddMMyy");
    QString utcTime = getTime();

    QString statusIndicator = "A";

    QString dirLat = (this->latitude >= 0) ? "N" : "S";
    QString dirLon = (this->longitude >= 0) ? "E" : "W";

    QString latitude = formatLatitude(this->latitude);
    QString longitude = formatLongitude(this->longitude);

    QString dirMagVariation = (this->magneticVariation >= 0) ? "E" : "W";
    double magVar = std::abs(this->magneticVariation);

    QString payload = QString("xxRMC,%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11")
                          .arg(utcTime)
                          .arg(statusIndicator)
                          .arg(latitude)
                          .arg(dirLat)
                          .arg(longitude)
                          .arg(dirLon)
                          .arg(this->sog)
                          .arg(this->cog)
                          .arg(date)
                          .arg(magVar)
                          .arg(dirMagVariation);

    sendNmeaData(payload);
}

void MenuBarSimData::sendGLL()
{
    QString dirLat = (this->latitude >= 0) ? "N" : "S";
    QString dirLon = (this->longitude >= 0) ? "E" : "W";

    QString latitude = formatLatitude(this->latitude);
    QString longitude = formatLongitude(this->longitude);

    QString utcTime = getTime();

    QString statusIndicator = "A";
    QString modeIndicator = "S";

    QString payload = QString("xxGLL,%1,%2,%3,%4,%5,%6,%7")
                          .arg(latitude)
                          .arg(dirLat)
                          .arg(longitude)
                          .arg(dirLon)
                          .arg(utcTime)
                          .arg(statusIndicator)
                          .arg(modeIndicator);

    sendNmeaData(payload);
}

void MenuBarSimData::sendDBT()
{
    double depthFeet   = depthMeter * 3.28084;
    double depthFathom = depthMeter / 1.8288;

    QString payload = QString("xxDBT,%1,f,%2,M,%3,F")
                          .arg(depthFeet,   0, 'f', 1)
                          .arg(this->depthMeter,  0, 'f', 1)
                          .arg(depthFathom, 0, 'f', 1);

    sendNmeaData(payload);
}

void MenuBarSimData::sendDPT()
{
    int maximumRange = 100;

    QString payload = QString("xxDPT,%1,%2,%3")
                          .arg(this->depthMeter, 0, 'f', 1)
                          .arg(this->transducerOffet, 0, 'f', 1)
                          .arg(maximumRange);

    sendNmeaData(payload);
}

void MenuBarSimData::sendGSV()
{
    QString payload = QString("xxGSV,1,1,%1,1,0,0,0")
                          .arg(this->numberSatellites);

    sendNmeaData(payload);
}

void MenuBarSimData::sendHDT()
{
    QString payload = QString("xxHDT,%1,T")
                          .arg(this->trueHeading);

    sendNmeaData(payload);
}

void MenuBarSimData::sendMTW()
{
    QString payload = QString("xxMTW,%1,C")
                          .arg(this->waterTemp);

    sendNmeaData(payload);
}

void MenuBarSimData::sendVTG()
{
    double speedKPH = knotToKph(this->sog);
    QString modeIndicator = "S";

    QString payload = QString("xxVTG,%1,T,%2,M,%3,N,%4,K,%5")
                          .arg(this->trueHeading)
                          .arg(this->magneticHeading)
                          .arg(this->sog)
                          .arg(speedKPH)
                          .arg(modeIndicator);

    sendNmeaData(payload);
}

void MenuBarSimData::sendMWV()
{
    QString windDir = "R"; //relative
    QString windSpeedUnit = "N"; //knots
    QString validity = "A";

    QString payload = QString("xxMWV,%1,%2,%3,%4,%5")
                          .arg(this->windRelativeAngle)
                          .arg(windDir)
                          .arg(this->windSpeed)
                          .arg(windSpeedUnit)
                          .arg(validity);

    sendNmeaData(payload);
}


/////////////////
/// GUI Logic ///
/////////////////

// Manual Data Input
void MenuBarSimData::on_pushButton_send_manual_input_clicked()
{
    sendManualInputData();
}

void MenuBarSimData::on_lineEdit_manual_input_textChanged(const QString &payload)
{
    quint8 checksum = calculateChecksum(payload);
    QString checksumStr = QString("*%1").arg(checksum, 2, 16, QLatin1Char('0')).toUpper();
    ui->label_checksum->setText(checksumStr);
}

void MenuBarSimData::on_checkBox_automatic_send_stateChanged(int state)
{
    if (state == Qt::Checked)
        autoSendManualDataTimer.start();
    else
        autoSendManualDataTimer.stop();
}

void MenuBarSimData::on_doubleSpinBox_automatic_send_freq_valueChanged(double value)
{
    int intervalMs = 1000 / value;
    autoSendManualDataTimer.setInterval(intervalMs);
}


//Check sim outputs
void MenuBarSimData::activateAllOutputs(bool check)
{
    ui->checkBox_DBT->setChecked(check);
    ui->checkBox_DPT->setChecked(check);
    ui->checkBox_GLL->setChecked(check);
    ui->checkBox_GSV->setChecked(check);
    ui->checkBox_HDT->setChecked(check);
    ui->checkBox_MTW->setChecked(check);
    ui->checkBox_MWV->setChecked(check);
    ui->checkBox_RMC->setChecked(check);
    ui->checkBox_VTG->setChecked(check);
}

void MenuBarSimData::on_pushButton_checkAll_clicked()
{
    activateAllOutputs(true);
}

void MenuBarSimData::on_pushButton_uncheckAll_clicked()
{
    activateAllOutputs(false);
}


//Update simulator values
void MenuBarSimData::on_doubleSpinBox_simFreq_valueChanged(double value)
{
    int intervalMs = 1000 / value;
    autoSendSimuDataTimer.setInterval(intervalMs);
}

void MenuBarSimData::on_checkBox_simAutomaticSend_stateChanged(int state)
{
    if (state == Qt::Checked)
        autoSendSimuDataTimer.start();
    else
        autoSendSimuDataTimer.stop();
}

void MenuBarSimData::on_doubleSpinBox_latitude_valueChanged(double latitude)
{
    this->latitude = latitude;
}

void MenuBarSimData::on_doubleSpinBox_longitude_valueChanged(double longitude)
{
    this->longitude = longitude;
}

void MenuBarSimData::on_doubleSpinBox_depth_valueChanged(double depth)
{
    this->depthMeter = depth;
}

void MenuBarSimData::on_spinBox_valueChanged(int satellites)
{
    this->numberSatellites = satellites;
}

void MenuBarSimData::on_doubleSpinBox_transducerOffset_valueChanged(double offset)
{
    this->transducerOffet = offset;
}

void MenuBarSimData::on_doubleSpinBox_trueHeading_valueChanged(double heading)
{
    this->trueHeading = heading;
}

void MenuBarSimData::on_doubleSpinBox_waterTemp_valueChanged(double temp)
{
    this->waterTemp = temp;
}

void MenuBarSimData::on_doubleSpinBox_magHeading_valueChanged(double heading)
{
    this->magneticHeading = heading;
}

void MenuBarSimData::on_doubleSpinBox_speed_valueChanged(double speed)
{
    this->sog = speed;
}

void MenuBarSimData::on_doubleSpinBox_magVariation_valueChanged(double variation)
{
    this->magneticVariation = variation;
}

void MenuBarSimData::on_doubleSpinBox_cog_valueChanged(double course)
{
    this->cog = course;
}

void MenuBarSimData::on_doubleSpinBox_windSpeed_valueChanged(double speed)
{
    this->windSpeed = speed;
}

void MenuBarSimData::on_doubleSpinBox_windRelativeAngle_valueChanged(double angle)
{
    this->windRelativeAngle = angle;
}



