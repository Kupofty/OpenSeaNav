#include "interface/interface.h"

///////////////////
/// Class Setup ///
///////////////////
Interface::Interface(QWidget *parent) : QMainWindow(parent), ui(new Ui::Interface)
{
    //Setup UI
    ui->setupUi(this);
    ui->tabWidget->setCurrentWidget(ui->tab_connections);
    this->showMaximized();

    //QML Map
    ui->quickWidget_map->setSource(QUrl(QStringLiteral("qrc:/mainMap.qml")));
    ui->quickWidget_map->show();
    qmlMapObject = ui->quickWidget_map->rootObject();

    //Hide widget
    hideGUI();

    //Serial COM ports
    listAvailableSerialPorts(ui->comboBox_serial_input_port_list);
    listAvailableSerialPorts(ui->comboBox_serial_output_port_list);

    //Qt connects
    connectSignalSlot();

    //Create lists (NMEA, UI elements, etc)
    initializeLists();
}

Interface::~Interface()
{
    delete ui;
}

void Interface::connectSignalSlot()
{
    //Inputs
    connect(&serial_reader, &SerialReader::newLineReceived, &nmea_handler, &NMEA_Handler::handleRawSentences);
    connect(&udp_reader, &UdpReader::newLineReceived, &nmea_handler, &NMEA_Handler::handleRawSentences);

    //Outputs
    connect(&nmea_handler, &NMEA_Handler::newNMEASentence, &udp_writer, &UdpWriter::publishNMEA);
    connect(&nmea_handler, &NMEA_Handler::newNMEASentence, &serial_writer, &SerialWriter::publishNMEA);
    connect(&nmea_handler, &NMEA_Handler::newNMEASentence, &text_file_writer, &TextFileWritter::writeRawSentences);

    //General Display Settings
    connect(&nmea_handler, &NMEA_Handler::newNMEASentence, &data_monitor_window, &MenuBarDataMonitor::displayNmeaSentence);
    connect(&udp_reader, &UdpReader::newSenderDetails, this, &Interface::updateUdpSenderDetails);

    //Display decoded NMEA data
    connect(&nmea_handler, &NMEA_Handler::newDecodedGGA, &decoded_nmea_window, &MenuBarDecodedNmea::updateDataGGA);
    connect(&nmea_handler, &NMEA_Handler::newDecodedGLL, &decoded_nmea_window, &MenuBarDecodedNmea::updateDataGLL);
    connect(&nmea_handler, &NMEA_Handler::newDecodedGSV, &decoded_nmea_window, &MenuBarDecodedNmea::updateDataGSV);
    connect(&nmea_handler, &NMEA_Handler::newDecodedVTG, &decoded_nmea_window, &MenuBarDecodedNmea::updateDataVTG);
    connect(&nmea_handler, &NMEA_Handler::newDecodedGSA, &decoded_nmea_window, &MenuBarDecodedNmea::updateDataGSA);
    connect(&nmea_handler, &NMEA_Handler::newDecodedRMC, &decoded_nmea_window, &MenuBarDecodedNmea::updateDataRMC);
    connect(&nmea_handler, &NMEA_Handler::newDecodedHDT, &decoded_nmea_window, &MenuBarDecodedNmea::updateDataHDT);
    connect(&nmea_handler, &NMEA_Handler::newDecodedDBT, &decoded_nmea_window, &MenuBarDecodedNmea::updateDataDBT);
    connect(&nmea_handler, &NMEA_Handler::newDecodedVHW, &decoded_nmea_window, &MenuBarDecodedNmea::updateDataVHW);
    connect(&nmea_handler, &NMEA_Handler::newDecodedZDA, &decoded_nmea_window, &MenuBarDecodedNmea::updateDataZDA);
    connect(&nmea_handler, &NMEA_Handler::newDecodedDPT, &decoded_nmea_window, &MenuBarDecodedNmea::updateDataDPT);
    connect(&nmea_handler, &NMEA_Handler::newDecodedMWD, &decoded_nmea_window, &MenuBarDecodedNmea::updateDataMWD);
    connect(&nmea_handler, &NMEA_Handler::newDecodedMTW, &decoded_nmea_window, &MenuBarDecodedNmea::updateDataMTW);
    connect(&nmea_handler, &NMEA_Handler::newDecodedMWV, &decoded_nmea_window, &MenuBarDecodedNmea::updateDataMWV);

    //Timers
    connect(&fileRecordingSizeTimer, &QTimer::timeout, this, &Interface::updateRecordingFileSize);

    //QML Map
    connect(&nmea_handler, SIGNAL(updateBoatPositionMap(QVariant,QVariant)), qmlMapObject, SLOT(updateBoatPosition(QVariant,QVariant)));
    connect(&nmea_handler, SIGNAL(updateBoatHeadingMap(QVariant)), qmlMapObject, SLOT(updateBoatHeading(QVariant)));
    connect(&nmea_handler, SIGNAL(updateBoatDepthMap(QVariant)), qmlMapObject, SLOT(updateBoatDepth(QVariant)));
    connect(&nmea_handler, SIGNAL(updateBoatSpeedMap(QVariant)), qmlMapObject, SLOT(updateBoatSpeed(QVariant)));
    connect(&nmea_handler, SIGNAL(updateBoatCourseMap(QVariant)), qmlMapObject, SLOT(updateBoatCourse(QVariant)));
    connect(&nmea_handler, SIGNAL(updateBoatWaterTemperatureMap(QVariant)), qmlMapObject, SLOT(updateBoatWaterTemperature(QVariant)));
    connect(&nmea_handler, SIGNAL(updateBoatDateMap(QVariant)), qmlMapObject, SLOT(updateBoatDate(QVariant)));
    connect(&nmea_handler, SIGNAL(updateBoatTimeMap(QVariant)), qmlMapObject, SLOT(updateBoatTime(QVariant)));
}



////////////////
/// Menu Bar ///
////////////////

//Menu
void Interface::on_actionExit_triggered()
{
    close();
}


//View
void Interface::on_actionFullscreen_triggered()
{
    toggleFullscreen();
}


//Tools
void Interface::on_actionManual_Data_Input_triggered()
{
    MenuBarSimData *dlg = new MenuBarSimData(this);
    connect(dlg, &MenuBarSimData::dataReady, &nmea_handler, &NMEA_Handler::handleRawSentences);
    dlg->show();
}

void Interface::on_actionData_Monitor_triggered()
{
    if (data_monitor_window.isVisible())
    {
        data_monitor_window.hide();
    }
    else
    {
        data_monitor_window.show();
        data_monitor_window.scrollDownPlainText();
    }
}

void Interface::on_actionDecoded_NMEA_triggered()
{
    if (decoded_nmea_window.isVisible())
        decoded_nmea_window.hide();
    else
        decoded_nmea_window.show();
}


//Help
void Interface::on_actionAbout_triggered()
{
    MenuBarAbout dlg(this);
    dlg.exec();
}

void Interface::on_actionFAQ_triggered()
{
    MenuBarFAQ dlg(this);
    dlg.exec();
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
}

void Interface::hideGUI()
{
    ui->horizontalFrame_udp_ip_address->hide(); //broadcast by default
}

void Interface::toggleFullscreen()
{
    if (isFullScreen())
        showMaximized();
    else
        showFullScreen();
}



////////////////////
/// Serial Input ///
////////////////////

// Connection
void Interface::closeInputSerial()
{
    if(serial_reader.isSerialOpen())
    {
        serial_reader.closeSerialDevice();
        ui->plainTextEdit_connection_status->setPlainText(serial_reader.getPortName()+" closed");
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

    serial_reader.setPortName(serial_input);
    serial_reader.setBaudRate((ui->comboBox_serial_input_port_baudrate->currentText()).toInt());

    //Try to connect
    QString result;
    if(serial_reader.openSerialDevice())
    {
        result =  "Connected to " + serial_reader.getPortName();
        updateGuiAfterSerialConnection(true);
    }
    else
    {
        result =  "Failed to open " + serial_reader.getPortName() + " : " + serial_reader.getErrorString();
    }

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
    decoded_nmea_window.clearDecodedDataScreens();
}

void Interface::on_checkBox_serial_manual_input_stateChanged(int checked)
{
    ui->lineEdit_serial_manual_input->setEnabled(checked);
    ui->comboBox_serial_input_port_list->setEnabled(!checked);
    ui->pushButton_refresh_available_ports_list->setEnabled(!checked);
}


//COM ports
void Interface::listAvailableSerialPorts(QComboBox* comboBox)
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
    listAvailableSerialPorts(ui->comboBox_serial_input_port_list);
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

    udp_reader.updatePort(udp_port_input);
    QString result = udp_reader.connect();
    ui->plainTextEdit_connection_status_udp->setPlainText(result);

    if(udp_reader.isBounded())
        updateGuiAfterUdpConnection(true);
}

void Interface::on_pushButton_disconnect_udp_input_clicked()
{
    closeInputUdp();
    updateGuiAfterUdpConnection(false);
}

void Interface::closeInputUdp()
{
    QString result = udp_reader.disconnect();
    ui->plainTextEdit_connection_status_udp->setPlainText(result);
}

void Interface::updateGuiAfterUdpConnection(bool connectSuccess)
{
    ui->spinBox_port_input_udp->setEnabled(!connectSuccess);
    ui->pushButton_connect_udp_input->setEnabled(!connectSuccess);
    ui->pushButton_disconnect_udp_input->setEnabled(connectSuccess);
    ui->plainTextEdit_udp_sender_details->clear();
    decoded_nmea_window.clearDecodedDataScreens();
}

void Interface::updateUdpSenderDetails()
{
    ui->plainTextEdit_udp_sender_details->setPlainText(udp_reader.getSenderDetails());
}




//////////////////////////
/// Serial Output Data ///
//////////////////////////

//Settings
void Interface::closeOutputSerial()
{
    if(serial_writer.isSerialOpen())
    {
        serial_writer.closeSerialDevice();
        ui->plainTextEdit_connection_status_output_serial->setPlainText(serial_writer.getPortName()+" closed");
    }
    else
        ui->plainTextEdit_connection_status_output_serial->setPlainText("Connection not opened");
}

void Interface::on_pushButton_refresh_available_port_serial_output_clicked()
{
    listAvailableSerialPorts(ui->comboBox_serial_output_port_list);

    //Remove input serial as output choice
    if(serial_reader.isSerialOpen())
    {
        int indexToRemove = ui->comboBox_serial_output_port_list->findText(ui->comboBox_serial_input_port_list->currentText());
        if (indexToRemove != -1)
            ui->comboBox_serial_output_port_list->removeItem(indexToRemove);
    }
}

void Interface::on_pushButton_connect_serial_output_clicked()
{
    QString serial_output;
    if(ui->checkBox_serial_manual_output->isChecked())
        serial_output = ui->lineEdit_serial_manual_output->text();
    else
        serial_output = ui->comboBox_serial_output_port_list->currentText();

    //Update serial settings
    serial_writer.setPortName(serial_output);
    serial_writer.setBaudRate((ui->comboBox_serial_output_port_baudrate->currentText()).toInt());

    //Try to connect
    QString result;
    if(serial_writer.openSerialDevice())
        result =  "Connected to " + serial_writer.getPortName();
    else
        result =  "Failed to open " + serial_writer.getPortName() + " : " + serial_writer.getErrorString();

    //Display connection status
    ui->plainTextEdit_connection_status_output_serial->setPlainText(result);
}

void Interface::on_pushButton_disconnect_serial_output_clicked()
{
    closeOutputSerial();
}

void Interface::on_checkBox_serial_manual_output_stateChanged(int checked)
{
    ui->lineEdit_serial_manual_output->setEnabled(checked);
    ui->comboBox_serial_output_port_list->setEnabled(!checked);
    ui->pushButton_refresh_available_port_serial_output->setEnabled(!checked);
}


//Outputs
void Interface::on_pushButton_activate_serial_output_toggled(bool checked)
{
    if (!checked)
    {
        serial_writer.updateSocketOutputActivated(false);
        return;
    }

    if (serial_writer.isSerialOpen())
    {
        serial_writer.updateSocketOutputActivated(true);
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
    serial_writer.updateOutputNMEA("GGA", checked);
}

void Interface::on_checkBox_serial_output_gsv_toggled(bool checked)
{
    serial_writer.updateOutputNMEA("GSV", checked);
}

void Interface::on_checkBox_serial_output_rmc_toggled(bool checked)
{
    serial_writer.updateOutputNMEA("RMC", checked);
}

void Interface::on_checkBox_serial_output_gsa_toggled(bool checked)
{
    serial_writer.updateOutputNMEA("GSA", checked);
}

void Interface::on_checkBox_serial_output_gll_toggled(bool checked)
{
    serial_writer.updateOutputNMEA("GLL", checked);
}

void Interface::on_checkBox_serial_output_vtg_toggled(bool checked)
{
    serial_writer.updateOutputNMEA("VTG", checked);
}

void Interface::on_checkBox_serial_output_hdt_toggled(bool checked)
{
    serial_writer.updateOutputNMEA("HDT", checked);
}

void Interface::on_checkBox_serial_output_dbt_toggled(bool checked)
{
    serial_writer.updateOutputNMEA("DBT", checked);
}

void Interface::on_checkBox_serial_output_vhw_toggled(bool checked)
{
    serial_writer.updateOutputNMEA("VHW", checked);
}

void Interface::on_checkBox_serial_output_zda_toggled(bool checked)
{
    serial_writer.updateOutputNMEA("ZDA", checked);
}

void Interface::on_checkBox_serial_output_dpt_toggled(bool checked)
{
    serial_writer.updateOutputNMEA("DPT", checked);
}

void Interface::on_checkBox_serial_output_mtw_toggled(bool checked)
{
    serial_writer.updateOutputNMEA("MTW", checked);
}

void Interface::on_checkBox_serial_output_mwv_toggled(bool checked)
{
    serial_writer.updateOutputNMEA("MWV", checked);
}

void Interface::on_checkBox_serial_output_mwd_toggled(bool checked)
{
    serial_writer.updateOutputNMEA("MWD", checked);
}

void Interface::on_checkBox_serial_output_others_toggled(bool checked)
{
    serial_writer.updateOutputNMEA("OTHER", checked);
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
        udp_writer.updateUdpPort(udp_port);
}

void Interface::on_comboBox_udp_host_address_currentTextChanged(const QString &udpMethod)
{
    if(udpMethod == "Broadcast")
    {
        udp_writer.updateUdpMethod(QHostAddress::Broadcast);
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
    udp_writer.updateUdpMethod(QHostAddress(ui->lineEdit_udp_ip_address->text()));
}

bool Interface::checkUdpOutputPortIsFree()
{
    int udp_input_port = ui->spinBox_port_input_udp->value();
    int udp_output_port = ui->spinBox_update_udp_port_output->value();

    //Check if port already used by UDP input
    if (udp_reader.isBounded() && (udp_output_port == udp_input_port) )
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
        udp_writer.updateSocketOutputActivated(true);
    else if(!checked)
        udp_writer.updateSocketOutputActivated(false);
}

void Interface::on_checkBox_udp_output_gga_toggled(bool checked)
{
    udp_writer.updateOutputNMEA("GGA", checked);
}

void Interface::on_checkBox_udp_output_rmc_toggled(bool checked)
{
    udp_writer.updateOutputNMEA("RMC", checked);
}

void Interface::on_checkBox_udp_output_gsv_toggled(bool checked)
{
    udp_writer.updateOutputNMEA("GSV", checked);
}

void Interface::on_checkBox_udp_output_gll_toggled(bool checked)
{
    udp_writer.updateOutputNMEA("GLL", checked);
}

void Interface::on_checkBox_udp_output_gsa_toggled(bool checked)
{
    udp_writer.updateOutputNMEA("GSA", checked);
}

void Interface::on_checkBox_udp_output_vtg_toggled(bool checked)
{
    udp_writer.updateOutputNMEA("VTG", checked);
}

void Interface::on_checkBox_udp_output_hdt_toggled(bool checked)
{
    udp_writer.updateOutputNMEA("HDT", checked);
}

void Interface::on_checkBox_udp_output_dbt_toggled(bool checked)
{
    udp_writer.updateOutputNMEA("DBT", checked);
}

void Interface::on_checkBox_udp_output_vhw_toggled(bool checked)
{
    udp_writer.updateOutputNMEA("VHW", checked);
}

void Interface::on_checkBox_udp_output_zda_toggled(bool checked)
{
udp_writer.updateOutputNMEA("ZDA", checked);
}

void Interface::on_checkBox_udp_output_dpt_toggled(bool checked)
{
    udp_writer.updateOutputNMEA("DPT", checked);
}

void Interface::on_checkBox_udp_output_mtw_toggled(bool checked)
{
    udp_writer.updateOutputNMEA("MTW", checked);
}

void Interface::on_checkBox_udp_output_mwv_toggled(bool checked)
{
    udp_writer.updateOutputNMEA("MWV", checked);
}

void Interface::on_checkBox_udp_output_mwd_toggled(bool checked)
{
    udp_writer.updateOutputNMEA("MWD", checked);
}

void Interface::on_checkBox_udp_output_others_toggled(bool checked)
{
    udp_writer.updateOutputNMEA("OTHER", checked);
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
    //Update add timestamp
    bool isTimestampChecked = ui->checkBox_output_txt_file_add_timestamp->isChecked();
    text_file_writer.updateAddTimestamp(isTimestampChecked);

    if(checked)
    {
        //Check for missing path/name
        QString dirPath = ui->plainTextEdit_txt_file_path->toPlainText().trimmed();
        QString fileName = ui->plainTextEdit_txt_file_name->toPlainText().trimmed();
        if (dirPath.isEmpty() || fileName.isEmpty())
        {
            QMessageBox::warning(this, "Missing Information",
                                       "Please select an output folder and enter a file name before saving.");
            ui->pushButton_save_txt_file->setChecked(false);
            return;
        }

        // Ask before overwriting previous recording
        QString fullPath = getRecordingFilePath();
        QFileInfo fileInfo(fullPath);
        if (fileInfo.exists())
        {
            auto reply = QMessageBox::question(this,
                "Overwrite file?",
                "A file with the same name already exists in the selected location.\n"
                "Do you want to replace it?",
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No
                );

            if (reply != QMessageBox::Yes)
            {
                ui->pushButton_save_txt_file->setChecked(false);
                return;
            }
        }

        //Create file if possible
        int result = text_file_writer.createFile(getRecordingFilePath());
        if (!result)
        {
            ui->pushButton_save_txt_file->setChecked(false);
            return;
        }

        //Update file size
        fileRecordingSizeTimer.start(1000);
        ui->pushButton_save_txt_file->setText(" Stop Recording");
    }
    else
    {
        fileRecordingSizeTimer.stop();
        ui->label_file_txt_size->setText("Not recording");
        text_file_writer.closeFile();
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










