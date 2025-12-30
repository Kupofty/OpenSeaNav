#include "interface.h"

/////////////
/// CLASS ///
/////////////
Interface::Interface(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::Interface),
    serial_reader(new SerialReader),
    serial_writer(new SerialWriter),
    nmea_handler(new NMEA_Handler),
    udp_reader(new UdpReader),
    udp_writer(new UdpWriter),
    text_file_writer(new TextFileWritter)
{
    //Setup UI
    ui->setupUi(this);
    ui->tabWidget->setCurrentWidget(ui->tab_connection);
    this->showMaximized();

    //QML Map
    ui->quickWidget_map->setSource(QUrl(QStringLiteral("qrc:/mainMap.qml")));
    ui->quickWidget_map->show();
    qmlMapObject = ui->quickWidget_map->rootObject();

    //Hide widgets
    hideGUI();

    //Setup GUI
    listAvailablePorts(ui->comboBox_serial_input_port_list);
    listAvailablePorts(ui->comboBox_serial_output_port_list);

    //Qt connects
    connectSignalSlot();

    //Create lists (NMEA, UI elements, etc)
    initializeLists();
}

Interface::~Interface()
{
    delete fileRecordingSizeTimer;
    delete serial_reader;
    delete nmea_handler;
    delete udp_writer;
    delete text_file_writer;
    delete ui;
}

void Interface::connectSignalSlot()
{
    //Inputs
    connect(serial_reader, &SerialReader::newLineReceived, nmea_handler, &NMEA_Handler::handleRawSentences);
    connect(udp_reader, &UdpReader::newLineReceived, nmea_handler, &NMEA_Handler::handleRawSentences);

    //Outputs
    connect(nmea_handler, &NMEA_Handler::newNMEASentence, udp_writer, &UdpWriter::publishNMEA);
    connect(nmea_handler, &NMEA_Handler::newNMEASentence, serial_writer, &SerialWriter::publishNMEA);
    connect(nmea_handler, &NMEA_Handler::newNMEASentence, text_file_writer, &TextFileWritter::writeRawSentences);

    //General Display Settings
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &Interface::scrollDownPlainText);
    connect(nmea_handler, &NMEA_Handler::newNMEASentence, this, &Interface::displayNmeaSentence);
    connect(udp_reader, &UdpReader::newSenderDetails, this, &Interface::updateUdpSenderDetails);

    //Display decoded NMEA data
    connect(nmea_handler, &NMEA_Handler::newDecodedGGA, this, &Interface::updateDataGGA);
    connect(nmea_handler, &NMEA_Handler::newDecodedGLL, this, &Interface::updateDataGLL);
    connect(nmea_handler, &NMEA_Handler::newDecodedGSV, this, &Interface::updateDataGSV);
    connect(nmea_handler, &NMEA_Handler::newDecodedVTG, this, &Interface::updateDataVTG);
    connect(nmea_handler, &NMEA_Handler::newDecodedGSA, this, &Interface::updateDataGSA);
    connect(nmea_handler, &NMEA_Handler::newDecodedRMC, this, &Interface::updateDataRMC);
    connect(nmea_handler, &NMEA_Handler::newDecodedHDT, this, &Interface::updateDataHDT);
    connect(nmea_handler, &NMEA_Handler::newDecodedDBT, this, &Interface::updateDataDBT);
    connect(nmea_handler, &NMEA_Handler::newDecodedVHW, this, &Interface::updateDataVHW);
    connect(nmea_handler, &NMEA_Handler::newDecodedZDA, this, &Interface::updateDataZDA);
    connect(nmea_handler, &NMEA_Handler::newDecodedDPT, this, &Interface::updateDataDPT);
    connect(nmea_handler, &NMEA_Handler::newDecodedMWD, this, &Interface::updateDataMWD);
    connect(nmea_handler, &NMEA_Handler::newDecodedMTW, this, &Interface::updateDataMTW);
    connect(nmea_handler, &NMEA_Handler::newDecodedMWV, this, &Interface::updateDataMWV);

    //Timers
    fileRecordingSizeTimer = new QTimer(this);
    connect(fileRecordingSizeTimer, &QTimer::timeout, this, &Interface::updateRecordingFileSize);

    //QML Map
    connect(this, SIGNAL(updateBoatPositionMap(QVariant,QVariant)), qmlMapObject, SLOT(updateBoatPosition(QVariant,QVariant)));
    connect(this, SIGNAL(updateBoatHeadingMap(QVariant)), qmlMapObject, SLOT(updateBoatHeading(QVariant)));
    connect(this, SIGNAL(updateBoatDepthMap(QVariant)), qmlMapObject, SLOT(updateBoatDepth(QVariant)));
    connect(this, SIGNAL(updateBoatSpeedMap(QVariant)), qmlMapObject, SLOT(updateBoatSpeed(QVariant)));
    connect(this, SIGNAL(updateBoatCourseMap(QVariant)), qmlMapObject, SLOT(updateBoatCourse(QVariant)));
    connect(this, SIGNAL(updateBoatWaterTemperatureMap(QVariant)), qmlMapObject, SLOT(updateBoatWaterTemperature(QVariant)));
    connect(this, SIGNAL(updateBoatDateMap(QVariant)), qmlMapObject, SLOT(updateBoatDate(QVariant)));
    connect(this, SIGNAL(updateBoatTimeMap(QVariant)), qmlMapObject, SLOT(updateBoatTime(QVariant)));
}



///////////
/// GUI ///
///////////
void Interface::initializeLists()
{
    checkboxOutputUDP = {
        ui->checkBox_udp_output_gga,
        ui->checkBox_udp_output_rmc,
        ui->checkBox_udp_output_gsv,
        ui->checkBox_udp_output_gll,
        ui->checkBox_udp_output_gsa,
        ui->checkBox_udp_output_vtg,
        ui->checkBox_udp_output_hdt,
        ui->checkBox_udp_output_dbt,
        ui->checkBox_udp_output_vhw,
        ui->checkBox_udp_output_zda,
        ui->checkBox_udp_output_dpt,
        ui->checkBox_udp_output_mwd,
        ui->checkBox_udp_output_mwv,
        ui->checkBox_udp_output_mtw,
        ui->checkBox_udp_output_others
    };

    checkboxOutputSerial = {
        ui->checkBox_serial_output_gga,
        ui->checkBox_serial_output_rmc,
        ui->checkBox_serial_output_gsv,
        ui->checkBox_serial_output_gll,
        ui->checkBox_serial_output_gsa,
        ui->checkBox_serial_output_vtg,
        ui->checkBox_serial_output_hdt,
        ui->checkBox_serial_output_dbt,
        ui->checkBox_serial_output_vhw,
        ui->checkBox_serial_output_zda,
        ui->checkBox_serial_output_dpt,
        ui->checkBox_serial_output_mwd,
        ui->checkBox_serial_output_mwv,
        ui->checkBox_serial_output_mtw,
        ui->checkBox_serial_output_others
    };

    nmeaSentenceMap = getSentenceMap();

}

void Interface::hideGUI()
{
    ui->horizontalFrame_udp_ip_address->hide();
}




////////////////////
/// Serial Input ///
////////////////////

// Connection
void Interface::closeInputSerial()
{
    if(serial_reader->isSerialOpen())
    {
        serial_reader->closeSerialDevice();
        ui->plainTextEdit_connection_status->setPlainText(serial_reader->getPortName()+" closed");
    }
    else
        ui->plainTextEdit_connection_status->setPlainText("Connection not opened");
}

void Interface::on_pushButton_connect_serial_input_clicked()
{
    //Update serial settings
    QString serial_input;
    if(ui->checkBox_serial_manual_input->isChecked())
        serial_input = ui->lineEdit_serial_manual_input->text();
    else
        serial_input = ui->comboBox_serial_input_port_list->currentText();

    serial_reader->setPortName(serial_input);
    serial_reader->setBaudRate((ui->comboBox_serial_input_port_baudrate->currentText()).toInt());

    //Try to connect
    QString result;
    if(serial_reader->openSerialDevice())
    {
        result =  "Connected to " + serial_reader->getPortName();
        ui->plainTextEdit_txt->clear();
        updateGuiAfterSerialConnection(true);
    }
    else
        result =  "Failed to open " + serial_reader->getPortName() + " : " + serial_reader->getErrorString();

    //Display connection status
    ui->plainTextEdit_connection_status->setPlainText(result);
}

void Interface::on_pushButton_disconnect_serial_input_clicked()
{
    closeInputSerial();
    updateGuiAfterSerialConnection(false);
}

void Interface::updateGuiAfterSerialConnection(bool connectSuccess)
{
    ui->horizontalFrame_serial_input_connection->setEnabled(!connectSuccess);
    ui->pushButton_connect_serial_input->setEnabled(!connectSuccess);
    ui->pushButton_disconnect_serial_input->setEnabled(connectSuccess);
    clearDecodedDataScreens();
}

void Interface::on_checkBox_serial_manual_input_stateChanged(int checked)
{
    ui->lineEdit_serial_manual_input->setEnabled(checked);
    ui->comboBox_serial_input_port_list->setEnabled(!checked);
    ui->pushButton_refresh_available_ports_list->setEnabled(!checked);
}


//COM ports
void Interface::listAvailablePorts(QComboBox *comboBox)
{
    comboBox->clear();
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &port : ports)
    {
        comboBox->addItem(port.portName());
    }
}

void Interface::on_pushButton_refresh_available_ports_list_clicked()
{
    listAvailablePorts(ui->comboBox_serial_input_port_list);
}



/////////////////
/// UDP Input ///
/////////////////
void Interface::on_pushButton_connect_udp_input_clicked()
{
    int udp_port_input = ui->spinBox_port_input_udp->value();
    int udp_port_output = ui->spinBox_update_udp_port_output->value();

    //Check if port already used by UDP output
    if (ui->pushButton_activate_udp_output->isChecked() && (udp_port_input == udp_port_output))
    {
        QMessageBox::warning(this, "UDP Port Error",
                             "Input UDP port conflicts with output UDP port.\nPlease choose a different port.");
        ui->spinBox_port_input_udp->setValue(ui->spinBox_port_input_udp->value() + 1);
        return;
    }

    udp_reader->updatePort(udp_port_input);
    QString result = udp_reader->connect();
    ui->plainTextEdit_connection_status_udp->setPlainText(result);

    if(udp_reader->isBounded())
        updateGuiAfterUdpConnection(true);
}

void Interface::on_pushButton_disconnect_udp_input_clicked()
{
    CloseInputUdp();
    updateGuiAfterUdpConnection(false);
}

void Interface::CloseInputUdp()
{
    QString result = udp_reader->disconnect();
    ui->plainTextEdit_connection_status_udp->setPlainText(result);
}

void Interface::updateGuiAfterUdpConnection(bool connectSuccess)
{
    ui->spinBox_port_input_udp->setEnabled(!connectSuccess);
    ui->pushButton_connect_udp_input->setEnabled(!connectSuccess);
    ui->pushButton_disconnect_udp_input->setEnabled(connectSuccess);
    ui->plainTextEdit_udp_sender_details->clear();
    clearDecodedDataScreens();
}

void Interface::updateUdpSenderDetails()
{
    ui->plainTextEdit_udp_sender_details->setPlainText(udp_reader->getSenderDetails());
}



///////////////////////////
/// Raw Nmea Sentences  ///
///////////////////////////

//Display On Screens
void Interface::displayNmeaSentence(const QString &type, const QString &nmeaText)
{
    //Do not display if freeze button is pressed
    bool isScreenTextFreezed = ui->pushButton_freeze_raw_sentences_screens->isChecked();
    if(isScreenTextFreezed)
        return;

    //Main Data Monitor
    displayRawSentences(nmeaText);

    //Sorted NMEA sentences tab
    if(nmeaSentenceMap.contains(type))
    {
        nmeaSentenceMap[type]->appendPlainText(nmeaText);
    }
}

void Interface::displayRawSentences(const QString &nmeaText)
{
    // Look for nmea ID in sentence
    QString nmeaType;
    int dollarIdx = nmeaText.indexOf('$');
    int commaIdx  = nmeaText.indexOf(',', dollarIdx);
    if (dollarIdx != -1 && commaIdx != -1 && commaIdx > dollarIdx)
    {
        nmeaType = nmeaText.mid(dollarIdx + 1, commaIdx - dollarIdx - 1).toUpper();
    }

    //Apply filter
    QString filter = ui->lineEdit_raw_data_filter->text().trimmed().toUpper();
    if (filter.isEmpty() || nmeaType.contains(filter))
    {
        QString timeStampedText = getTimeStamp() + nmeaText;
        ui->plainTextEdit_raw_data->appendPlainText(timeStampedText);
    }
}

QMap<QString, QPlainTextEdit*> Interface::getSentenceMap() const
{
    QList<QPlainTextEdit*> editors = getPlainTextEditors();
    QMap<QString, QPlainTextEdit*> map;

    for (int i = 0; i < acceptedNmeaList.size() && i < editors.size(); ++i)
        map.insert(acceptedNmeaList[i], editors[i]);

    return map;
}


//Clear Screens
void Interface::clearRawSortedSentencesScreens()
{
    const QList<QPlainTextEdit*> &editors = getPlainTextEditors();

    for (QPlainTextEdit* editor : editors)
        editor->clear();
}

void Interface::clearRawSentencesScreens()
{
    ui->plainTextEdit_raw_data->clear();
    clearRawSortedSentencesScreens();
}

void Interface::on_pushButton_clear_raw_sentences_screens_clicked()
{
    clearRawSentencesScreens();
}

void Interface::on_pushButton_clear_raw_sorted_sentences_screens_clicked()
{
    clearRawSentencesScreens();
}


//Sync Freeze Screens Buttons States
void Interface::on_pushButton_freeze_raw_sorted_sentences_screens_toggled(bool checked)
{
    QSignalBlocker blocker(ui->pushButton_freeze_raw_sentences_screens);
    ui->pushButton_freeze_raw_sentences_screens->setChecked(checked);
}

void Interface::on_pushButton_freeze_raw_sentences_screens_toggled(bool checked)
{
    QSignalBlocker blocker(ui->pushButton_freeze_raw_sorted_sentences_screens);
    ui->pushButton_freeze_raw_sorted_sentences_screens->setChecked(checked);
}


// Scroll down screens
void Interface::scrollDownPlainText(int index)
{
    //Scroll down all the screens when changing to the raw data tab
    if(index == ui->tabWidget->indexOf(ui->tab_raw_data_sorted))
    {
        const QList<QPlainTextEdit*> &editors = getPlainTextEditors();

        for (QPlainTextEdit* editor : editors)
            editor->verticalScrollBar()->setValue(editor->verticalScrollBar()->maximum());
    }

    else if(index == ui->tabWidget->indexOf(ui->tab_raw_data))
    {
        ui->plainTextEdit_raw_data->verticalScrollBar()->setValue(ui->plainTextEdit_raw_data->verticalScrollBar()->maximum());
    }
}


//Get list of plainTexts
QList<QPlainTextEdit*> Interface::getPlainTextEditors() const
{
    return
    {
        ui->plainTextEdit_dbt,
        ui->plainTextEdit_dpt,
        ui->plainTextEdit_gga,
        ui->plainTextEdit_gll,
        ui->plainTextEdit_gsa,
        ui->plainTextEdit_gsv,
        ui->plainTextEdit_hdt,
        ui->plainTextEdit_mtw,
        ui->plainTextEdit_mwd,
        ui->plainTextEdit_mwv,
        ui->plainTextEdit_others,
        ui->plainTextEdit_rmc,
        ui->plainTextEdit_txt,
        ui->plainTextEdit_vhw,
        ui->plainTextEdit_vtg,
        ui->plainTextEdit_zda
    };
}




//////////////////////////////////
/// Display Decoded NMEA data  ///
//////////////////////////////////

//Data
void Interface::updateDataGGA(QString time, double latitude, double longitude, int fixQuality, int numSatellites, double hdop, double altitude, double freqHz)
{
    ui->label_utcTime_gga->setText(time);
    ui->lcdNumber_latitude_gga->display(latitude);
    ui->lcdNumber_longitude_gga->display(longitude);
    ui->lcdNumber_fixQuality_gga->display(fixQuality);
    ui->lcdNumber_satellites_gga->display(numSatellites);
    ui->lcdNumber_hdop_gga->display(hdop);
    ui->lcdNumber_altitude_gga->display(altitude);
    ui->lcdNumber_frequency_gga->display(freqHz);

    emit updateBoatPositionMap(latitude, longitude);
    emit updateBoatTimeMap(time);
}

void Interface::updateDataGLL(QString utc, double latitude, double longitude, double freqHz)
{
    ui->label_utcTime_gll->setText(utc);
    ui->lcdNumber_latitude_gll->display(latitude);
    ui->lcdNumber_longitude_gll->display(longitude);
    ui->lcdNumber_frequency_gll->display(freqHz);

    emit updateBoatPositionMap(latitude, longitude);
    emit updateBoatTimeMap(utc);
}

void Interface::updateDataGSV(int satellitesInView, double frequency)
{
    ui->lcdNumber_satellites_gsv->display(satellitesInView);
    ui->lcdNumber_frequency_gsv->display(frequency);
}

void Interface::updateDataVTG(double track_true, double track_mag, double speed_knot, double speedKmh, double frequency)
{
    ui->lcdNumber_track_true_vtg->display(track_true);
    ui->lcdNumber_track_mag_vtg->display(track_mag);
    ui->lcdNumber_speed_knot_vtg->display(speed_knot);
    ui->lcdNumber_speed_kmh_vtg->display(speedKmh);
    ui->lcdNumber_frequency_vtg->display(frequency);

    emit updateBoatHeadingMap(track_true);
    emit updateBoatSpeedMap(speed_knot);
}

void Interface::updateDataGSA(double pdop, double hdop, double vdop, double freqHz)
{
    ui->lcdNumber_pdop_gsa->display(pdop);
    ui->lcdNumber_hdop_gsa->display(hdop);
    ui->lcdNumber_vdop_gsa->display(vdop);
    ui->lcdNumber_frequency_gsa->display(freqHz);
}

void Interface::updateDataRMC(QString utcDate, QString utcTime, double latitude, double longitude, double speedMps, double course, double magVar, double freqHz)
{
    ui->label_date_rmc->setText(utcDate);
    ui->label_utcTime_rmc->setText(utcTime);
    ui->lcdNumber_latitude_rmc->display(latitude);
    ui->lcdNumber_longitude_rmc->display(longitude);
    ui->lcdNumber_sog_rmc->display(speedMps);
    ui->lcdNumber_cog_rmc->display(course);
    ui->lcdNumber_magVar_rmc->display(magVar);
    ui->lcdNumber_frequency_rmc->display(freqHz);

    emit updateBoatPositionMap(latitude, longitude);
    emit updateBoatCourseMap(course);
    emit updateBoatDateMap(utcDate);
    emit updateBoatTimeMap(utcTime);
}

void Interface::updateDataHDT(double heading, double freqHz)
{
    ui->lcdNumber_heading_hdt->display(heading);
    ui->lcdNumber_frequency_hdt->display(freqHz);

    emit updateBoatHeadingMap(heading);
}

void Interface::updateDataDBT(double depthFeet, double depthMeters, double depthFathom, double freqHz)
{
    ui->lcdNumber_depth_feet_dbt->display(depthFeet);
    ui->lcdNumber_depth_meter_dbt->display(depthMeters);
    ui->lcdNumber_depth_fathom_dbt->display(depthFathom);
    ui->lcdNumber_frequency_dbt->display(freqHz);

    emit updateBoatDepthMap(depthMeters);
}

void Interface::updateDataVHW(double headingTrue, double headingMag, double speedKnots, double speedKmh, double freqHz)
{
    ui->lcdNumber_heading_true_vhw->display(headingTrue);
    ui->lcdNumber_heading_mag_vhw->display(headingMag);
    ui->lcdNumber_speed_knot_vhw->display(speedKnots);
    ui->lcdNumber_speed_kmh_vhw->display(speedKmh);
    ui->lcdNumber_frequency_vhw->display(freqHz);

    emit updateBoatHeadingMap(headingTrue);
    emit updateBoatSpeedMap(speedKnots);
}

void Interface::updateDataZDA(QString date, QString time, QString offsetTime, double freqHz)
{
    ui->label_date_zda->setText(date);
    ui->label_utcTime_zda->setText(time);
    ui->label_localZone_zda->setText(offsetTime);
    ui->lcdNumber_frequency_zda->display(freqHz);

    emit updateBoatDateMap(date);
    emit updateBoatTimeMap(time);
}

void Interface::updateDataDPT(double depth, double offset, double freqHz)
{
    ui->lcdNumber_depth_dpt->display(depth);
    ui->lcdNumber_depth_offset_dpt->display(offset);
    ui->lcdNumber_frequency_dpt->display(freqHz);

    emit updateBoatDepthMap(depth);
}

void Interface::updateDataMWD(double dir1, QString dir1Unit, double dir2, QString dir2Unit, double speed1, QString speed1Unit, double speed2, QString speed2Unit, double freqHz)
{
    ui->lcdNumber_windDirection_mwd->display(dir1);
    ui->label_windDirectionUnit_mwd->setText("Wind Direction (" + dir1Unit + ") :" );
    ui->lcdNumber_windDirection_mwd_2->display(dir2);
    ui->label_windDirectionUnit_mwd_2->setText("Wind Direction (" + dir2Unit + ") :" );
    ui->lcdNumber_windSpeed_mwd->display(speed1);
    ui->label_windSpeedUnit_mwd->setText("Wind Speed (" + speed1Unit + ") :" );
    ui->lcdNumber_windSpeed_mwd_2->display(speed2);
    ui->label_windSpeedUnit_mwd_2->setText("Wind Speed (" + speed2Unit + ") :" );
    ui->lcdNumber_frequency_mwd->display(freqHz);
}

void Interface::updateDataMTW(double temp, QString tempUnit, double freqHz)
{
    ui->lcdNumber_waterTemp_mtw->display(temp);
    ui->label_waterTempUnit_mtw->setText("Water Temp. (°" + tempUnit + ") :");
    ui->lcdNumber_frequency_mtw->display(freqHz);

    emit updateBoatWaterTemperatureMap(temp);
}

void Interface::updateDataMWV(double angle, QString ref, double speed, QString unit, double freqHz)
{
    ui->lcdNumber_windAngle_mwv->display(angle);
    ui->label_windAngleUnit_mwv->setText("Wing Angle (" + ref + ") :" );
    ui->lcdNumber_windSpeed_mwv->display(speed);
    ui->label_windSpeedUnit_mwv->setText("Wing Speed (" + unit + ") :" );
    ui->lcdNumber_frequency_mwv->display(freqHz);
}


//Clear
void Interface::clearDecodedDataScreens()
{
    //RMC
    ui->label_utcTime_rmc->setText("No Data");
    ui->label_date_rmc->setText("No Data");
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
    ui->label_utcTime_gll->setText("No Data");
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
    ui->label_utcTime_zda->setText("No Data");
    ui->label_date_zda->setText("No Data");
    ui->label_localZone_zda->setText("No Data");
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




//////////////////////////
/// Serial Output Data ///
//////////////////////////

//Settings
void Interface::closeOutputSerial()
{
    if(serial_writer->isSerialOpen())
    {
        serial_writer->closeSerialDevice();
        ui->plainTextEdit_connection_status_output_serial->setPlainText(serial_writer->getPortName()+" closed");
    }
    else
        ui->plainTextEdit_connection_status_output_serial->setPlainText("Connection not opened");
}

void Interface::on_pushButton_refresh_available_port_serial_output_clicked()
{
    listAvailablePorts(ui->comboBox_serial_output_port_list);

    //Remove input serial as output choice
    if(serial_reader->isSerialOpen())
    {
        int indexToRemove = ui->comboBox_serial_output_port_list->findText(ui->comboBox_serial_input_port_list->currentText());
        if (indexToRemove != -1)
            ui->comboBox_serial_output_port_list->removeItem(indexToRemove);
    }
}

void Interface::on_pushButton_connect_serial_output_clicked()
{
    //Update serial settings
    serial_writer->setPortName(ui->comboBox_serial_output_port_list->currentText());
    serial_writer->setBaudRate((ui->comboBox_serial_output_port_baudrate->currentText()).toInt());

    //Try to connect
    QString result;
    if(serial_writer->openSerialDevice())
        result =  "Connected to " + serial_writer->getPortName();
    else
        result =  "Failed to open " + serial_writer->getPortName() + " : " + serial_writer->getErrorString();

    //Display connection status
    ui->plainTextEdit_connection_status_output_serial->setPlainText(result);
}

void Interface::on_pushButton_disconnect_serial_output_clicked()
{
    closeOutputSerial();
}


//Outputs
void Interface::on_pushButton_activate_serial_output_toggled(bool checked)
{
    if (!checked)
    {
        serial_writer->updateSocketOutputActivated(false);
        return;
    }

    if (serial_writer->isSerialOpen())
    {
        serial_writer->updateSocketOutputActivated(true);
    }
    else
    {
        QMessageBox::warning(this, "Serial Output Not Available",
                             "No serial output port is currently opened.\n\n"
                             "Please select a valid port and click 'Connect' before enabling serial output.");
        ui->pushButton_activate_serial_output->setChecked(false);
    }
}

void Interface::on_checkBox_serial_output_gga_toggled(bool checked)
{
    serial_writer->updateOutputNMEA("GGA", checked);
}

void Interface::on_checkBox_serial_output_gsv_toggled(bool checked)
{
    serial_writer->updateOutputNMEA("GSV", checked);
}

void Interface::on_checkBox_serial_output_rmc_toggled(bool checked)
{
    serial_writer->updateOutputNMEA("RMC", checked);
}

void Interface::on_checkBox_serial_output_gsa_toggled(bool checked)
{
    serial_writer->updateOutputNMEA("GSA", checked);
}

void Interface::on_checkBox_serial_output_gll_toggled(bool checked)
{
    serial_writer->updateOutputNMEA("GLL", checked);
}

void Interface::on_checkBox_serial_output_vtg_toggled(bool checked)
{
    serial_writer->updateOutputNMEA("VTG", checked);
}

void Interface::on_checkBox_serial_output_hdt_toggled(bool checked)
{
    serial_writer->updateOutputNMEA("HDT", checked);
}

void Interface::on_checkBox_serial_output_dbt_toggled(bool checked)
{
    serial_writer->updateOutputNMEA("DBT", checked);
}

void Interface::on_checkBox_serial_output_vhw_toggled(bool checked)
{
    serial_writer->updateOutputNMEA("VHW", checked);
}

void Interface::on_checkBox_serial_output_zda_toggled(bool checked)
{
    serial_writer->updateOutputNMEA("ZDA", checked);
}

void Interface::on_checkBox_serial_output_dpt_toggled(bool checked)
{
    serial_writer->updateOutputNMEA("DPT", checked);
}

void Interface::on_checkBox_serial_output_mtw_toggled(bool checked)
{
    serial_writer->updateOutputNMEA("MTW", checked);
}

void Interface::on_checkBox_serial_output_mwv_toggled(bool checked)
{
    serial_writer->updateOutputNMEA("MWV", checked);
}

void Interface::on_checkBox_serial_output_mwd_toggled(bool checked)
{
    serial_writer->updateOutputNMEA("MWD", checked);
}

void Interface::on_checkBox_serial_output_others_toggled(bool checked)
{
    serial_writer->updateOutputNMEA("OTHER", checked);
}

void Interface::on_pushButton_check_all_serial_output_clicked()
{
    updateCheckBoxSerialOutput(true);
}

void Interface::on_pushButton_uncheck_all_serial_output_clicked()
{
    updateCheckBoxSerialOutput(false);
}

void Interface::updateCheckBoxSerialOutput(bool check)
{
    const auto& boxes = checkboxOutputSerial;
    for (QCheckBox* box : boxes)
        box->setChecked(check);
}



///////////////////////
/// UDP Output Data ///
///////////////////////

//UDP Settings
void Interface::on_spinBox_update_udp_port_output_valueChanged(int udp_port)
{
    if(checkUdpOutputPortIsFree())
        udp_writer->updateUdpPort(udp_port);
}

void Interface::on_comboBox_udp_host_address_currentTextChanged(const QString &udpMethod)
{
    if(udpMethod == "Broadcast")
    {
        udp_writer->updateUdpMethod(QHostAddress::Broadcast);
        ui->horizontalFrame_udp_ip_address->hide();
    }
    else if(udpMethod == "Unicast" || udpMethod == "Multicast")
    {
        emit ui->lineEdit_udp_ip_address->editingFinished();
        ui->horizontalFrame_udp_ip_address->show();
    }
}

void Interface::on_lineEdit_udp_ip_address_editingFinished()
{
    udp_writer->updateUdpMethod(QHostAddress(ui->lineEdit_udp_ip_address->text()));
}

bool Interface::checkUdpOutputPortIsFree()
{
    int udp_input_port = ui->spinBox_port_input_udp->value();
    int udp_output_port = ui->spinBox_update_udp_port_output->value();

    //Check if port already used by UDP input
    if (udp_reader->isBounded() && (udp_output_port == udp_input_port) )
    {
        QMessageBox::warning(this, "UDP Port Error", "Output UDP port conflicts with input UDP port.\nPlease choose a different port.");
        ui->pushButton_activate_udp_output->setChecked(false);
        return false;
    }
    else
        return true;
}


//Check data to outpout
void Interface::on_pushButton_activate_udp_output_toggled(bool checked)
{
    if(checked && checkUdpOutputPortIsFree())
        udp_writer->updateSocketOutputActivated(true);
    else if(!checked)
        udp_writer->updateSocketOutputActivated(false);
}

void Interface::on_checkBox_udp_output_gga_toggled(bool checked)
{
    udp_writer->updateOutputNMEA("GGA", checked);
}

void Interface::on_checkBox_udp_output_rmc_toggled(bool checked)
{
    udp_writer->updateOutputNMEA("RMC", checked);
}

void Interface::on_checkBox_udp_output_gsv_toggled(bool checked)
{
    udp_writer->updateOutputNMEA("GSV", checked);
}

void Interface::on_checkBox_udp_output_gll_toggled(bool checked)
{
    udp_writer->updateOutputNMEA("GLL", checked);
}

void Interface::on_checkBox_udp_output_gsa_toggled(bool checked)
{
    udp_writer->updateOutputNMEA("GSA", checked);
}

void Interface::on_checkBox_udp_output_vtg_toggled(bool checked)
{
    udp_writer->updateOutputNMEA("VTG", checked);
}

void Interface::on_checkBox_udp_output_hdt_toggled(bool checked)
{
    udp_writer->updateOutputNMEA("HDT", checked);
}

void Interface::on_checkBox_udp_output_dbt_toggled(bool checked)
{
    udp_writer->updateOutputNMEA("DBT", checked);
}

void Interface::on_checkBox_udp_output_vhw_toggled(bool checked)
{
    udp_writer->updateOutputNMEA("VHW", checked);
}

void Interface::on_checkBox_udp_output_zda_toggled(bool checked)
{
udp_writer->updateOutputNMEA("ZDA", checked);
}

void Interface::on_checkBox_udp_output_dpt_toggled(bool checked)
{
    udp_writer->updateOutputNMEA("DPT", checked);
}

void Interface::on_checkBox_udp_output_mtw_toggled(bool checked)
{
    udp_writer->updateOutputNMEA("MTW", checked);
}

void Interface::on_checkBox_udp_output_mwv_toggled(bool checked)
{
    udp_writer->updateOutputNMEA("MWV", checked);
}

void Interface::on_checkBox_udp_output_mwd_toggled(bool checked)
{
    udp_writer->updateOutputNMEA("MWD", checked);
}

void Interface::on_checkBox_udp_output_others_toggled(bool checked)
{
    udp_writer->updateOutputNMEA("OTHER", checked);
}

void Interface::on_pushButton_check_all_udp_output_clicked()
{
    updateCheckBoxUdpOutput(true);
}

void Interface::on_pushButton_uncheck_all_udp_output_clicked()
{
    updateCheckBoxUdpOutput(false);
}

void Interface::updateCheckBoxUdpOutput(bool check)
{
    const auto& boxes = checkboxOutputUDP;
    for (QCheckBox* box : boxes)
        box->setChecked(check);
}




/////////////////////
/// Save TXT File ///
/////////////////////
void Interface::on_pushButton_folder_path_documents_clicked()
{
    ui->plainTextEdit_txt_file_path->setPlainText(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
}

void Interface::on_pushButton_folder_path_downloads_clicked()
{
    ui->plainTextEdit_txt_file_path->setPlainText(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
}

void Interface::on_pushButton_browse_folder_path_clicked()
{
    QString dirPath = QFileDialog::getExistingDirectory(this, "Select Output Directory",
                                                        QStandardPaths::writableLocation(QStandardPaths::HomeLocation),  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);


    if (!dirPath.isEmpty())
        ui->plainTextEdit_txt_file_path->setPlainText(dirPath);
}

void Interface::on_pushButton_automatic_txt_file_name_clicked()
{
    QString automaticFileName = "Log_NMEA_" + QDateTime::currentDateTime().toString("yyyy_MM_dd_HH_mm_ss");
    ui->plainTextEdit_txt_file_name->setPlainText(automaticFileName);
}

void Interface::on_pushButton_save_txt_file_toggled(bool checked)
{
    bool isTimestampChecked = ui->checkBox_output_txt_file_add_timestamp->isChecked();
    text_file_writer->updateAddTimestamp(isTimestampChecked);

    if(checked)
    {
        QString dirPath = ui->plainTextEdit_txt_file_path->toPlainText().trimmed();
        QString fileName = ui->plainTextEdit_txt_file_name->toPlainText().trimmed();

        if (dirPath.isEmpty() || fileName.isEmpty()) {
            QMessageBox::warning(this, "Missing Information", "Please select an output folder and enter a file name before saving.");
            ui->pushButton_save_txt_file->setChecked(false);
            return;
        }

        int result = text_file_writer->createFile(getRecordingFilePath());
        if(!result)
            return;

        fileRecordingSizeTimer->start(1000);
        ui->pushButton_save_txt_file->setText(" Stop Recording");
    }
    else
    {
        fileRecordingSizeTimer->stop();
        ui->label_file_txt_size->setText("Not recording");
        text_file_writer->closeFile();
        ui->pushButton_save_txt_file->setText(" Record Data To File");
    }
}

void Interface::updateRecordingFileSize()
{
    QFile file(getRecordingFilePath());
    if (file.exists())
    {
        qint64 size = file.size();
        ui->label_file_txt_size->setText(QString("%1 Kb").arg(static_cast<int>(std::round(size / 1000.0))));
    }
    else
    {
        ui->label_file_txt_size->setText("File missing");
    }
}

QString Interface::getRecordingFilePath()
{
    QString dirPath = ui->plainTextEdit_txt_file_path->toPlainText().trimmed();
    QString fileName = ui->plainTextEdit_txt_file_name->toPlainText().trimmed();
    QString fileExtension = ui->comboBox_txt_file_extension->currentText();
    QString fullPath = QDir(dirPath).filePath(fileName + fileExtension);

    return fullPath;
}







