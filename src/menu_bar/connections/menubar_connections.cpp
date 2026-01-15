#include "menubar_connections.h"
#include "ui_menubar_connections.h"


///////////////////
/// Class Setup ///
///////////////////
MenuBarConnections::MenuBarConnections(QWidget *parent) : QDialog(parent), ui(new Ui::MenuBarConnections)
{
    ui->setupUi(this);
    ui->tabWidget->setCurrentWidget(ui->tab_inputs);

    // Enable minimize, maximize, close buttons
    Qt::WindowFlags flags = Qt::Dialog
                            | Qt::WindowMaximizeButtonHint
                            | Qt::WindowCloseButtonHint
                            | Qt::WindowStaysOnTopHint;
    setWindowFlags(flags);
    setAttribute(Qt::WA_ShowWithoutActivating);

    //Hide widgets
    hideGUI();

    //Create lists (NMEA, UI elements, etc)
    initializeLists();

    //Settings
    loadSettings();

    //Connects
    connect(&udp_reader, &UdpReader::newSenderDetails, this, &MenuBarConnections::updateUdpSenderDetails);
    connect(&tcp_reader, &TcpReader::updateState, this, &MenuBarConnections::updateTcpState);

    //Emit new input data received -> interface -> nmea_handler
    connect(&serial_reader, &SerialReader::newLineReceived, this, &MenuBarConnections::newLineReceived);
    connect(&udp_reader, &UdpReader::newLineReceived, this, &MenuBarConnections::newLineReceived);
    connect(&tcp_reader, &TcpReader::newLineReceived, this, &MenuBarConnections::newLineReceived);
}

MenuBarConnections::~MenuBarConnections()
{
    saveSettings();
    delete ui;
}




/////////////////
/// Settings ///
/////////////////
void MenuBarConnections::loadSettings()
{
    //Init Settings
    initSettings();

    //Serial Input
    loadSerialInputSettings();

    //TCP Input
    loadTcpInputSettings();

    //UDP Input
    loadUdpInputSettings();
}

void MenuBarConnections::initSettings()
{
    //Create directory
    QString settingsFolderName = "/OpenSeaNav";
    configPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + settingsFolderName;
    QDir().mkpath(configPath);

    //Connections Settings
    QString settingsFileName = "/configConnection.ini";
    settingsConnections = new QSettings(configPath + settingsFileName, QSettings::IniFormat, this);
}

void MenuBarConnections::loadSerialInputSettings()
{
    //Baudrate
    int serialInputBaudrate = settingsConnections->value("serialInput/baudrate", ui->comboBox_serial_input_port_baudrate->currentIndex()).toInt();
    ui->comboBox_serial_input_port_baudrate->setCurrentIndex(serialInputBaudrate);

    //List input ports
    QString savedPort = settingsConnections->value("serialInput/listPort").toString();
    int index = ui->comboBox_serial_input_port_list->findText(savedPort, Qt::MatchExactly);
    if (index >= 0)
        ui->comboBox_serial_input_port_list->setCurrentIndex(index);

    //Manual input port
    bool serialInputSelectManual= settingsConnections->value("serialInput/selectManual", false).toBool();
    ui->checkBox_serial_manual_input->setChecked(serialInputSelectManual);

    QString serialInputPort = settingsConnections->value("serialInput/manualPort", "").toString();
    ui->lineEdit_serial_manual_input->setText(serialInputPort);

    //Autoconnect
    bool serialAutoConnect = settingsConnections->value("serialInput/autoConnect", false).toBool();
    ui->checkBox_serial_autoconnect->setChecked(serialAutoConnect);
    if(ui->checkBox_serial_autoconnect->isChecked())
        ui->pushButton_connect_serial_input->click();
}

void MenuBarConnections::loadTcpInputSettings()
{
    //IP address
    QString tcpInputIpAddress = settingsConnections->value("tcpInput/ipAddress", "").toString();
    ui->lineEdit_tcp_manual_input->setText(tcpInputIpAddress);

    //Port
    int tcpInputPort = settingsConnections->value("tcpInput/port", ui->spinBox_tcp_input_port->value()).toInt();
    ui->spinBox_tcp_input_port->setValue(tcpInputPort);

    //Autoconnect
    bool tcpAutoConnect = settingsConnections->value("tcpInput/autoConnect", false).toBool();
    ui->checkBox_tcp_autoconnect->setChecked(tcpAutoConnect);
    if(ui->checkBox_tcp_autoconnect->isChecked())
        ui->pushButton_connect_tcp_input->click();
}

void MenuBarConnections::loadUdpInputSettings()
{
    //Port
    int udpInputPort = settingsConnections->value("udpInput/port", ui->spinBox_port_input_udp->value()).toInt();
    ui->spinBox_port_input_udp->setValue(udpInputPort);

    //Autoconnect
    bool udpAutoConnect = settingsConnections->value("udpInput/autoConnect",false).toBool();
    ui->checkBox_udp_autoconnect->setChecked(udpAutoConnect);
    if(ui->checkBox_udp_autoconnect->isChecked())
        ui->pushButton_connect_udp_input->click();
}

void MenuBarConnections::saveSettings()
{
    //Serial Input
    settingsConnections->setValue("serialInput/selectManual", ui->checkBox_serial_manual_input->isChecked());
    settingsConnections->setValue("serialInput/baudrate", ui->comboBox_serial_input_port_baudrate->currentIndex());
    settingsConnections->setValue("serialInput/manualPort",  ui->lineEdit_serial_manual_input->text());
    settingsConnections->setValue("serialInput/autoConnect", ui->checkBox_serial_autoconnect->isChecked());
    settingsConnections->setValue("serialInput/listPort", ui->comboBox_serial_input_port_list->currentText());

    //TCP Input
    settingsConnections->setValue("tcpInput/ipAddress", ui->lineEdit_tcp_manual_input->text());
    settingsConnections->setValue("tcpInput/port", ui->spinBox_tcp_input_port->value());
    settingsConnections->setValue("tcpInput/autoConnect", ui->checkBox_tcp_autoconnect->isChecked());

    // UDP Input
    settingsConnections->setValue("udpInput/port", ui->spinBox_port_input_udp->value());
    settingsConnections->setValue("udpInput/autoConnect", ui->checkBox_udp_autoconnect->isChecked());
}



/////////////////
/// Functions ///
/////////////////
void MenuBarConnections::retranslate()
{
    ui->retranslateUi(this);
    setWindowTitle(tr("Connections IO"));
}

void MenuBarConnections::hideGUI()
{
    ui->horizontalFrame_udp_ip_address->hide(); //broadcast by default
}

void MenuBarConnections::initializeLists()
{
    //Update COM ports list
    listAvailableSerialPorts(ui->comboBox_serial_input_port_list);
    listAvailableSerialPorts(ui->comboBox_serial_output_port_list);

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

void MenuBarConnections::publishNMEA(const QString &senderName, const QString &type, const QString &nmeaText)
{
    Q_UNUSED(senderName);
    udp_writer.publishNMEA(type, nmeaText);
    serial_writer.publishNMEA(type, nmeaText);
}



////////////////////
/// Serial Input ///
////////////////////

// Connection
void MenuBarConnections::on_pushButton_connect_serial_input_clicked()
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
        result =  tr("Connected");
        updateGuiAfterSerialConnection(true);
    }
    else
    {
        result =  tr("Failed to connect") + " : " + serial_reader.getErrorString();
    }

    //Display connection status
    ui->plainTextEdit_connection_status_serial->setPlainText(result);
}

void MenuBarConnections::on_pushButton_disconnect_serial_input_clicked()
{
    closeInputSerial();
    updateGuiAfterSerialConnection(false);
}

void MenuBarConnections::closeInputSerial()
{
    if(serial_reader.isSerialOpen())
    {
        serial_reader.closeSerialDevice();
        ui->plainTextEdit_connection_status_serial->setPlainText(serial_reader.getPortName()+ tr(" closed"));
    }
    else
        ui->plainTextEdit_connection_status_serial->setPlainText(tr("Connection not opened"));
}

void MenuBarConnections::updateGuiAfterSerialConnection(bool connectSuccess)
{
    ui->horizontalFrame_serial_input_connection->setEnabled(!connectSuccess);
    ui->pushButton_connect_serial_input->setEnabled(!connectSuccess);
    ui->pushButton_disconnect_serial_input->setEnabled(connectSuccess);
}


//COM ports
void MenuBarConnections::listAvailableSerialPorts(QComboBox* comboBox)
{
    comboBox->clear();
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &port : ports)
    {
        comboBox->addItem(port.portName());
    }
}

void MenuBarConnections::on_pushButton_refresh_available_ports_list_clicked()
{
    listAvailableSerialPorts(ui->comboBox_serial_input_port_list);
}


//Settings
void MenuBarConnections::on_checkBox_serial_manual_input_stateChanged(int checked)
{
    ui->lineEdit_serial_manual_input->setEnabled(checked);
    ui->comboBox_serial_input_port_list->setEnabled(!checked);
    ui->pushButton_refresh_available_ports_list->setEnabled(!checked);
}



/////////////////
/// TCP Input ///
/////////////////
void MenuBarConnections::on_pushButton_connect_tcp_input_clicked()
{
    //Check if IP address is valid
    if(!isIpAddressValid(ui->lineEdit_tcp_manual_input->text()))
    {
        QMessageBox::warning(this,
            tr("Invalid IP address"),
            tr("Please enter a valid IP address in the format 0-255.0-255.0-255.0-255"));
        ui->lineEdit_tcp_manual_input->setFocus();
        return ;
    }

    ui->plainTextEdit_connection_status_tcp->clear();

    QString ip = ui->lineEdit_tcp_manual_input->text();
    int port = ui->spinBox_tcp_input_port->value();
    tcp_reader.connectToHost(ip, port);
}

void MenuBarConnections::on_pushButton_disconnect_tcp_input_clicked()
{
    ui->pushButton_connect_tcp_input->setEnabled(true);
    tcp_reader.abortConnection();
}

void MenuBarConnections::updateTcpState(QString state)
{
    ui->plainTextEdit_connection_status_tcp->setPlainText(state);
    updateGuiAfterTcpConnection(tcp_reader.isConnectedOrConnecting());
}

void MenuBarConnections::updateGuiAfterTcpConnection(bool connectSuccess)
{
    ui->lineEdit_tcp_manual_input->setEnabled(!connectSuccess);
    ui->pushButton_tcp_input_localhost->setEnabled(!connectSuccess);
    ui->spinBox_tcp_input_port->setEnabled(!connectSuccess);
    ui->pushButton_connect_tcp_input->setEnabled(!connectSuccess);
}

void MenuBarConnections::on_pushButton_tcp_input_localhost_clicked()
{
    ui->lineEdit_tcp_manual_input->setText("127.0.0.1");
}



/////////////////
/// UDP Input ///
/////////////////
void MenuBarConnections::on_pushButton_connect_udp_input_clicked()
{
    int udp_port_input  = ui->spinBox_port_input_udp->value();
    int udp_port_output = ui->spinBox_update_udp_port_output->value();

    if (ui->pushButton_activate_udp_output->isChecked() &&
        udp_port_input == udp_port_output)
    {
        QMessageBox::warning(this, tr("UDP Port Error"),
                             tr("Input UDP port conflicts with output UDP port.\nPlease choose a different port."));
        ui->spinBox_port_input_udp->setValue(udp_port_input + 1);
        return;
    }

    udp_reader.updatePort(udp_port_input);
    QString result = udp_reader.connect();
    ui->plainTextEdit_connection_status_udp->setPlainText(result);

    if (udp_reader.isBounded()) {
        updateGuiAfterUdpConnection(true);
    }
}

void MenuBarConnections::on_pushButton_disconnect_udp_input_clicked()
{
    closeInputUdp();
    updateGuiAfterUdpConnection(false);
}

void MenuBarConnections::closeInputUdp()
{
    QString result = udp_reader.disconnect();
    ui->plainTextEdit_connection_status_udp->setPlainText(result);
}

void MenuBarConnections::updateGuiAfterUdpConnection(bool connectSuccess)
{
    ui->spinBox_port_input_udp->setEnabled(!connectSuccess);
    ui->pushButton_connect_udp_input->setEnabled(!connectSuccess);
    ui->pushButton_disconnect_udp_input->setEnabled(connectSuccess);
    ui->plainTextEdit_udp_sender_details->clear();
}

void MenuBarConnections::updateUdpSenderDetails()
{
    ui->plainTextEdit_udp_sender_details->setPlainText(udp_reader.getSenderDetails());
}



//////////////////////////
/// Serial Output Data ///
//////////////////////////

//Settings
void MenuBarConnections::closeOutputSerial()
{
    if(serial_writer.isSerialOpen())
    {
        serial_writer.closeSerialDevice();
        ui->plainTextEdit_connection_status_output_serial->setPlainText(serial_writer.getPortName() + tr(" closed"));
    }
    else
        ui->plainTextEdit_connection_status_output_serial->setPlainText(tr("Connection not opened"));
}

void MenuBarConnections::on_pushButton_refresh_available_port_serial_output_clicked()
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

void MenuBarConnections::on_pushButton_connect_serial_output_clicked()
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
        result =  tr("Connected to ") + serial_writer.getPortName();
    else
        result =  tr("Failed to open ") + serial_writer.getPortName() + " : " + serial_writer.getErrorString();

    //Display connection status
    ui->plainTextEdit_connection_status_output_serial->setPlainText(result);
}

void MenuBarConnections::on_pushButton_disconnect_serial_output_clicked()
{
    closeOutputSerial();
}

void MenuBarConnections::on_checkBox_serial_manual_output_stateChanged(int checked)
{
    ui->lineEdit_serial_manual_output->setEnabled(checked);
    ui->comboBox_serial_output_port_list->setEnabled(!checked);
    ui->pushButton_refresh_available_port_serial_output->setEnabled(!checked);
}


//Outputs
void MenuBarConnections::on_pushButton_activate_serial_output_toggled(bool checked)
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
        QMessageBox::warning(this, tr("Serial Output Not Available"),
                             tr("No serial output port is currently opened.\n\n"
                                "Please select a valid port and click 'Connect' before enabling serial output."));
        ui->pushButton_activate_serial_output->setChecked(false);
    }
}

void MenuBarConnections::on_checkBox_serial_output_gga_toggled(bool checked)
{
    serial_writer.updateOutputNMEA("GGA", checked);
}

void MenuBarConnections::on_checkBox_serial_output_gsv_toggled(bool checked)
{
    serial_writer.updateOutputNMEA("GSV", checked);
}

void MenuBarConnections::on_checkBox_serial_output_rmc_toggled(bool checked)
{
    serial_writer.updateOutputNMEA("RMC", checked);
}

void MenuBarConnections::on_checkBox_serial_output_gsa_toggled(bool checked)
{
    serial_writer.updateOutputNMEA("GSA", checked);
}

void MenuBarConnections::on_checkBox_serial_output_gll_toggled(bool checked)
{
    serial_writer.updateOutputNMEA("GLL", checked);
}

void MenuBarConnections::on_checkBox_serial_output_vtg_toggled(bool checked)
{
    serial_writer.updateOutputNMEA("VTG", checked);
}

void MenuBarConnections::on_checkBox_serial_output_hdt_toggled(bool checked)
{
    serial_writer.updateOutputNMEA("HDT", checked);
}

void MenuBarConnections::on_checkBox_serial_output_dbt_toggled(bool checked)
{
    serial_writer.updateOutputNMEA("DBT", checked);
}

void MenuBarConnections::on_checkBox_serial_output_vhw_toggled(bool checked)
{
    serial_writer.updateOutputNMEA("VHW", checked);
}

void MenuBarConnections::on_checkBox_serial_output_zda_toggled(bool checked)
{
    serial_writer.updateOutputNMEA("ZDA", checked);
}

void MenuBarConnections::on_checkBox_serial_output_dpt_toggled(bool checked)
{
    serial_writer.updateOutputNMEA("DPT", checked);
}

void MenuBarConnections::on_checkBox_serial_output_mtw_toggled(bool checked)
{
    serial_writer.updateOutputNMEA("MTW", checked);
}

void MenuBarConnections::on_checkBox_serial_output_mwv_toggled(bool checked)
{
    serial_writer.updateOutputNMEA("MWV", checked);
}

void MenuBarConnections::on_checkBox_serial_output_mwd_toggled(bool checked)
{
    serial_writer.updateOutputNMEA("MWD", checked);
}

void MenuBarConnections::on_checkBox_serial_output_others_toggled(bool checked)
{
    serial_writer.updateOutputNMEA("OTHER", checked);
}

void MenuBarConnections::on_pushButton_check_all_serial_output_clicked()
{
    updateCheckBoxSerialOutput(true);
}

void MenuBarConnections::on_pushButton_uncheck_all_serial_output_clicked()
{
    updateCheckBoxSerialOutput(false);
}

void MenuBarConnections::updateCheckBoxSerialOutput(bool check)
{
    const auto& boxes = checkboxOutputSerial;
    for (QCheckBox* box : boxes)
        box->setChecked(check);
}



///////////////////////
/// UDP Output Data ///
///////////////////////

//UDP Settings
void MenuBarConnections::on_spinBox_update_udp_port_output_valueChanged(int udp_port)
{
    if(checkUdpOutputPortIsFree())
        udp_writer.updateUdpPort(udp_port);
}

void MenuBarConnections::on_comboBox_udp_host_address_currentTextChanged(const QString &udpMethod)
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

void MenuBarConnections::on_lineEdit_udp_ip_address_editingFinished()
{
    udp_writer.updateUdpMethod(QHostAddress(ui->lineEdit_udp_ip_address->text()));
}

bool MenuBarConnections::checkUdpOutputPortIsFree()
{
    int udp_input_port = ui->spinBox_port_input_udp->value();
    int udp_output_port = ui->spinBox_update_udp_port_output->value();

    //Check if port already used by UDP input
    if (udp_reader.isBounded() && (udp_output_port == udp_input_port) )
    {
        QMessageBox::warning(this, tr("UDP Port Error"), tr("Output UDP port conflicts with input UDP port.\nPlease choose a different port."));
        ui->pushButton_activate_udp_output->setChecked(false);
        return false;
    }
    else
        return true;
}


//Check data to outpout
void MenuBarConnections::on_pushButton_activate_udp_output_toggled(bool checked)
{
    if(checked && checkUdpOutputPortIsFree())
    {
        udp_writer.updateUdpPort(ui->spinBox_update_udp_port_output->value());
        //should also update QHostAddress
        udp_writer.updateSocketOutputActivated(true);
    }
    else if(!checked)
        udp_writer.updateSocketOutputActivated(false);
}

void MenuBarConnections::on_checkBox_udp_output_gga_toggled(bool checked)
{
    udp_writer.updateOutputNMEA("GGA", checked);
}

void MenuBarConnections::on_checkBox_udp_output_rmc_toggled(bool checked)
{
    udp_writer.updateOutputNMEA("RMC", checked);
}

void MenuBarConnections::on_checkBox_udp_output_gsv_toggled(bool checked)
{
    udp_writer.updateOutputNMEA("GSV", checked);
}

void MenuBarConnections::on_checkBox_udp_output_gll_toggled(bool checked)
{
    udp_writer.updateOutputNMEA("GLL", checked);
}

void MenuBarConnections::on_checkBox_udp_output_gsa_toggled(bool checked)
{
    udp_writer.updateOutputNMEA("GSA", checked);
}

void MenuBarConnections::on_checkBox_udp_output_vtg_toggled(bool checked)
{
    udp_writer.updateOutputNMEA("VTG", checked);
}

void MenuBarConnections::on_checkBox_udp_output_hdt_toggled(bool checked)
{
    udp_writer.updateOutputNMEA("HDT", checked);
}

void MenuBarConnections::on_checkBox_udp_output_dbt_toggled(bool checked)
{
    udp_writer.updateOutputNMEA("DBT", checked);
}

void MenuBarConnections::on_checkBox_udp_output_vhw_toggled(bool checked)
{
    udp_writer.updateOutputNMEA("VHW", checked);
}

void MenuBarConnections::on_checkBox_udp_output_zda_toggled(bool checked)
{
    udp_writer.updateOutputNMEA("ZDA", checked);
}

void MenuBarConnections::on_checkBox_udp_output_dpt_toggled(bool checked)
{
    udp_writer.updateOutputNMEA("DPT", checked);
}

void MenuBarConnections::on_checkBox_udp_output_mtw_toggled(bool checked)
{
    udp_writer.updateOutputNMEA("MTW", checked);
}

void MenuBarConnections::on_checkBox_udp_output_mwv_toggled(bool checked)
{
    udp_writer.updateOutputNMEA("MWV", checked);
}

void MenuBarConnections::on_checkBox_udp_output_mwd_toggled(bool checked)
{
    udp_writer.updateOutputNMEA("MWD", checked);
}

void MenuBarConnections::on_checkBox_udp_output_others_toggled(bool checked)
{
    udp_writer.updateOutputNMEA("OTHER", checked);
}

void MenuBarConnections::on_pushButton_check_all_udp_output_clicked()
{
    updateCheckBoxUdpOutput(true);
}

void MenuBarConnections::on_pushButton_uncheck_all_udp_output_clicked()
{
    updateCheckBoxUdpOutput(false);
}

void MenuBarConnections::updateCheckBoxUdpOutput(bool check)
{
    const auto& boxes = checkboxOutputUDP;
    for (QCheckBox* box : boxes)
        box->setChecked(check);
}











