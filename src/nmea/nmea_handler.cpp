#include "nmea_handler.h"


/////////////
/// Class ///
/////////////
NMEA_Handler::NMEA_Handler()
{

}

NMEA_Handler::~NMEA_Handler()
{

}



////////////////////////////
/// Detect NMEA Sentence ///
////////////////////////////
void NMEA_Handler::handleRawSentences(const QByteArray &line)
{
    //Remove malformed sentences
    if (!line.startsWith('$'))
        return;

    QString nmeaText = QString::fromUtf8(line).trimmed();
    QList<QByteArray> fields = line.split(',');
    QString nmeaFormat = QString::fromUtf8(line.mid(3, 3));

    if (!acceptedNmeaList.contains(nmeaFormat))
        nmeaFormat ="OTHER";

    //Send sentences to
    emit newNMEASentence(nmeaFormat, nmeaText);

    //Do not handle data with incorrect checksums;
    if (!isNmeaChecksumValid(nmeaText))
        return;

    if(nmeaFormat == "GGA")
        handleGGA(fields);
    else if (nmeaFormat == "RMC")
        handleRMC(fields);
    else if (nmeaFormat == "GSV")
        handleGSV(fields);
    else if (nmeaFormat == "GLL")
        handleGLL(fields);
    else if (nmeaFormat == "GSA")
        handleGSA(fields);
    else if (nmeaFormat == "VTG")
        handleVTG(fields);
    else if (nmeaFormat == "HDT")
        handleHDT(fields);
    else if (nmeaFormat == "DBT")
        handleDBT(fields);
    else if (nmeaFormat == "VHW")
        handleVHW(fields);
    else if (nmeaFormat == "ZDA")
        handleZDA(fields);
    else if (nmeaFormat == "DPT")
        handleDPT(fields);
    else if (nmeaFormat == "MWD")
        handleMWD(fields);
    else if (nmeaFormat == "MWV")
        handleMWV(fields);
    else if (nmeaFormat == "MTW")
        handleMTW(fields);
}



///////////////////
/// Handle Data ///
///////////////////

void NMEA_Handler::handleGGA(const QList<QByteArray> &fields)
{
    //Check size
    if(!isNmeaMinimumSize(fields, 9))
        return;

    // Parse UTC time
    QString timeStr = fields[1];
    QTime utcTime = QTime::fromString(timeStr.left(6), "hhmmss");

    // Parse position
    QString latStr   = fields[2];
    QString latDir   = fields[3];
    QString lonStr   = fields[4];
    QString lonDir   = fields[5];
    double latitude  = calculateCoordinates(latStr, latDir);
    double longitude = calculateCoordinates(lonStr, lonDir);
    if (std::isnan(latitude) || std::isnan(longitude))
        return;

    // Parse fix quality
    int fixQuality = fields[6].toInt();

    // Parse number of satellites
    int numSatellites = fields[7].toInt();

    // Parse HDOP
    double hdop = fields[8].toDouble();

    // Parse altitude and units
    double altitude = fields[9].toDouble();

    //Calculate frequency
    double freqHz = calculateFrequency(timer_gga, lastUpdateTimeGGA);

    emit newDecodedGGA(utcTime.toString(), latitude, longitude, fixQuality, numSatellites, hdop, altitude, freqHz);
}

void NMEA_Handler::handleRMC(const QList<QByteArray> &fields)
{
    if(!isNmeaMinimumSize(fields, 12))
        return;

    // Parse time
    QString timeStr = fields[1];
    QTime utcTime = QTime::fromString(timeStr.left(6), "hhmmss");

    // Status: A=active, V=void
    QString status = fields[2];
    if (status != "A")
        return;

    // Parse latitude
    double latitude = calculateCoordinates(fields[3], fields[4]);

    // Parse longitude
    double longitude = calculateCoordinates(fields[5], fields[6]);

    // Speed over ground (knots) and convert to m/s
    double speedKnots = fields[7].toDouble();

    // Course over ground
    double course = fields[8].toDouble();

    // Parse date
    QString dateStr = fields[9];
    QDate utcDate = QDate::fromString(dateStr, "ddMMyy");
    if (utcDate.isValid())
    {
        int year = utcDate.year();
        if (year < 2000)
            utcDate = utcDate.addYears(100);
    }
    QString formattedDate = utcDate.toString("dd/MM/yyyy");

    // Magnetic variation
    double magVar = fields[10].toDouble();
    if (!fields[11].isEmpty())
    {
        QString varDir = fields[11];
        if (varDir == "W")
            magVar = -magVar;
    }

    //Freq
    double freqHz = calculateFrequency(timer_rmc, lastUpdateTimeRMC);

    // Emit or process parsed data
    emit newDecodedRMC(formattedDate, utcTime.toString(), latitude, longitude, speedKnots, course, magVar, freqHz);
}

void NMEA_Handler::handleGSV(const QList<QByteArray> &fields)
{
    //Check size
    if(!isNmeaMinimumSize(fields, 4))
        return;

    // Parse GSV
    int sentenceNumber  = fields[2].toInt();
    int totalSatellites = fields[3].toInt();

    //GSV is composed of multiples sub-messages
    if (sentenceNumber == 1 )
    {
        double freqHz = calculateFrequency(timer_gsv, lastUpdateTimeGSV);
        emit newDecodedGSV(totalSatellites, freqHz);
    }
}

void NMEA_Handler::handleGLL(const QList<QByteArray> &fields)
{
    //Check size
    if(!isNmeaMinimumSize(fields, 8))
        return;

    // Parse position
    QString latStr   = fields[1];
    QString latDir   = fields[2];
    QString lonStr   = fields[3];
    QString lonDir   = fields[4];
    double latitude  = calculateCoordinates(latStr, latDir);
    double longitude = calculateCoordinates(lonStr, lonDir);
    if (std::isnan(latitude) || std::isnan(longitude))
        return;

    // Parse UTC time
    QString timeStr = fields[5];
    QTime utcTime = QTime::fromString(timeStr.left(6), "hhmmss");

    // Status and mode
    QString status = removeAsterisk(fields[6]);

    //Calculate frequency
    double freqHz = calculateFrequency(timer_gll, lastUpdateTimeGLL);

    if (status != "A")
        return;

    emit newDecodedGLL(utcTime.toString(), latitude, longitude, freqHz);
}

void NMEA_Handler::handleGSA(const QList<QByteArray> &fields)
{
    if(!isNmeaMinimumSize(fields, 17))
        return;

    double pdop = fields[15].toDouble();
    double hdop = fields[16].toDouble();
    double vdop = removeAsterisk(fields[17]).toDouble();

    double freqHz = calculateFrequency(timer_gsa, lastUpdateTimeGSA);

    emit newDecodedGSA(pdop, hdop, vdop, freqHz);
}

void NMEA_Handler::handleVTG(const QList<QByteArray> &fields)
{
    if(!isNmeaMinimumSize(fields, 8))
        return;

    double trackTrue  = fields[1].toDouble();
    double trackMag   = fields[3].toDouble();
    double speedKnots = fields[5].toDouble();
    double speedKmh   = fields[7].toDouble();

    //Calculate frequency
    double freqHz = calculateFrequency(timer_vtg, lastUpdateTimeVTG);

    emit newDecodedVTG(trackTrue, trackMag, speedKnots, speedKmh, freqHz);
}

void NMEA_Handler::handleHDT(const QList<QByteArray> &fields)
{
    if(!isNmeaMinimumSize(fields, 3))
        return;

    double heading      = fields[1].toDouble();
    QString headingUnit = removeAsterisk(fields[2]);

    // Validate that the unit is 'T' (true heading)
    if (headingUnit != "T")
        return;

    //Calculate frequency
    double freqHz = calculateFrequency(timer_hdt, lastUpdateTimeHDT);

    emit newDecodedHDT(heading, freqHz);
}

void NMEA_Handler::handleDBT(const QList<QByteArray> &fields)
{
    if(!isNmeaMinimumSize(fields, 6))
        return;

    //Extract data
    double depthFeet   = fields[1].toDouble();
    double depthMeters = fields[3].toDouble();
    double depthFathom = fields[5].toDouble();

    //Calculate frequency
    double freqHz = calculateFrequency(timer_dbt, lastUpdateTimeDBT);

    emit newDecodedDBT(depthFeet, depthMeters, depthFathom, freqHz);
}

void NMEA_Handler::handleVHW(const QList<QByteArray> &fields)
{
    if(!isNmeaMinimumSize(fields, 8))
        return;

    double headingTrue = fields[1].toDouble();
    double headingMag  = fields[3].toDouble();
    double speedKnots  = fields[5].toDouble();
    double speedKmh    = fields[7].toDouble();

    //Calculate frequency
    double freqHz = calculateFrequency(timer_vhw, lastUpdateTimeVHW);

    emit newDecodedVHW(headingTrue, headingMag, speedKnots, speedKmh, freqHz);
}

void NMEA_Handler::handleZDA(const QList<QByteArray> &fields)
{
    if(!isNmeaMinimumSize(fields, 7))
        return;

    QString utcTime   = fields[1];
    QString day       = fields[2];
    QString month     = fields[3];
    QString year      = fields[4];
    QString tzHourStr = fields[5];
    QString tzMinStr  = fields[6];

    //Date & Time
    QDate date = QDate(year.toInt(), month.toInt(), day.toInt());
    QTime time = QTime::fromString(utcTime.left(6), "hhmmss");  // drop decimals
    QString dateStr = date.toString("dd/MM/yyyy");
    QString timeStr = time.toString("hh:mm:ss");

    // Parse offsets
    int tzHour = tzHourStr.toInt();
    int tzMin  = tzMinStr.toInt();

    // Handle sign: if tzHour is negative, apply sign to minutes
    QString sign = (tzHour < 0 || tzHourStr.startsWith("-")) ? "-" : "+";
    tzHour = std::abs(tzHour);
    tzMin  = std::abs(tzMin);

    QString offsetStr = QString("%1%2:%3").arg(sign).arg(tzHour, 2, 10, QChar('0')).arg(tzMin, 2, 10, QChar('0'));

    // Calculate frequency
    double freqHz = calculateFrequency(timer_zda, lastUpdateTimeZDA);

    emit newDecodedZDA(dateStr, timeStr, offsetStr, freqHz);
}

void NMEA_Handler::handleDPT(const QList<QByteArray> &fields)
{
    if(!isNmeaMinimumSize(fields, 2))
        return;

    double depth  = fields[1].toDouble();
    double offset = removeAsterisk(fields[2]).toDouble();

    double freqHz = calculateFrequency(timer_dpt, lastUpdateTimeDPT);

    emit newDecodedDPT(depth, offset, freqHz);
}

void NMEA_Handler::handleMWD(const QList<QByteArray> &fields)
{
    if(!isNmeaMinimumSize(fields, 8))
        return;

    double dir1        = fields[1].toDouble();
    QString dir1Unit   = fields[2];
    double dir2        = fields[3].toDouble();
    QString dir2Unit   = fields[4];
    double speed1      = fields[5].toDouble();
    QString speed1Unit = fields[6];
    double speed2      = fields[7].toDouble();
    QString speed2Unit = removeAsterisk(fields[8]);

    double freqHz = calculateFrequency(timer_mwd, lastUpdateTimeMWD);

    emit newDecodedMWD(dir1, dir1Unit, dir2, dir2Unit, speed1, speed1Unit, speed2, speed2Unit, freqHz);
}

void NMEA_Handler::handleMTW(const QList<QByteArray> &fields)
{
    isNmeaMinimumSize(fields, 2);

    double temp      = fields[1].toDouble();
    QString tempUnit = removeAsterisk(fields[2]);

    double freqHz = calculateFrequency(timer_mtw, lastUpdateTimeMTW);

    emit newDecodedMTW(temp, tempUnit, freqHz);
}

void NMEA_Handler::handleMWV(const QList<QByteArray> &fields)
{
    if(!isNmeaMinimumSize(fields, 5))
        return;

    double angle  = fields[1].toDouble();
    QString ref   = fields[2]; // R=relative, T=true
    double speed  = fields[3].toDouble();
    QString unit  = fields[4]; // N=knots, M=m/s, K=km/h
    QString valid = removeAsterisk(fields[5]); //A:valid / V=invalid
    double freqHz = calculateFrequency(timer_mwv, lastUpdateTimeMWV);

    if(valid != "A")
        return;

    emit newDecodedMWV(angle, ref, speed, unit, freqHz);
}




/////////////////////////
/// Generic Functions ///
/////////////////////////
double NMEA_Handler::calculateCoordinates(const QString &valueStr, const QString &direction)
{
    bool ok = false;
    double raw = valueStr.toDouble(&ok);
    if (!ok)
        return std::numeric_limits<double>::quiet_NaN();

    double degrees = floor(raw / 100.0);
    double minutes = raw - (degrees * 100.0);
    double result = degrees + (minutes / 60.0);

    if (direction == "S" || direction == "W")
        result *= -1.0;

    return result;
}

double NMEA_Handler::calculateFrequency(QElapsedTimer &timer, qint64 &lastUpdateTime)
{
    if (lastUpdateTime == -1)
    {
        timer.start();
        lastUpdateTime = 0;
        return 0;
    }

    qint64 now = timer.elapsed();
    qint64 delta_time = now - lastUpdateTime;
    lastUpdateTime = now;

    if (delta_time > 0)
    {
        double freqHz = 1000.0 / static_cast<double>(delta_time);
        return freqHz;
    }

    return 0;
}

QByteArray NMEA_Handler::removeAsterisk(const QByteArray lastField)
{
     return lastField.split('*').first();
}

bool NMEA_Handler::isNmeaMinimumSize(const QList<QByteArray> &fields, int minSize)
{
    return (fields.size() >= minSize);
}
