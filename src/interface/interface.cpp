#include "interface/interface.h"


///////////////////
/// Class Setup ///
///////////////////
Interface::Interface(QWidget *parent) : QMainWindow(parent), ui(new Ui::Interface)
{
    //Setup UI
    ui->setupUi(this);

    //Instantiate child classes
    data_monitor_window = new MenuBarDataMonitor(this);
    decoded_nmea_window = new MenuBarDecodedNmea(this);
    txt_logger_window = new MenuBarTxtLogger(this);
    connections_window = new MenuBarConnections(this);



    //Settings
    loadSettings();

    //QML Map
    ui->quickWidget_map->setSource(QUrl(QStringLiteral("qrc:/mainMap.qml")));
    ui->quickWidget_map->show();
    qmlMapObject = ui->quickWidget_map->rootObject();

    //Qt connects
    connectSignalSlot();
}

Interface::~Interface()
{
    saveSettings();
    delete ui;
}

void Interface::connectSignalSlot()
{
    //Inputs
    connect(connections_window, &MenuBarConnections::newLineReceived, &nmea_handler, &NMEA_Handler::handleRawSentences);

    //Outputs
    connect(&nmea_handler, &NMEA_Handler::newNMEASentence, connections_window, &MenuBarConnections::publishNMEA);
    connect(&nmea_handler, &NMEA_Handler::newNMEASentence, txt_logger_window, &MenuBarTxtLogger::writeRawSentences);

    //General Display Settings
    connect(&nmea_handler, &NMEA_Handler::newNMEASentence, data_monitor_window, &MenuBarDataMonitor::displayNmeaSentence);

    //Display decoded NMEA data
    connect(&nmea_handler, &NMEA_Handler::newDecodedGGA, decoded_nmea_window, &MenuBarDecodedNmea::updateDataGGA);
    connect(&nmea_handler, &NMEA_Handler::newDecodedGLL, decoded_nmea_window, &MenuBarDecodedNmea::updateDataGLL);
    connect(&nmea_handler, &NMEA_Handler::newDecodedGSV, decoded_nmea_window, &MenuBarDecodedNmea::updateDataGSV);
    connect(&nmea_handler, &NMEA_Handler::newDecodedVTG, decoded_nmea_window, &MenuBarDecodedNmea::updateDataVTG);
    connect(&nmea_handler, &NMEA_Handler::newDecodedGSA, decoded_nmea_window, &MenuBarDecodedNmea::updateDataGSA);
    connect(&nmea_handler, &NMEA_Handler::newDecodedRMC, decoded_nmea_window, &MenuBarDecodedNmea::updateDataRMC);
    connect(&nmea_handler, &NMEA_Handler::newDecodedHDT, decoded_nmea_window, &MenuBarDecodedNmea::updateDataHDT);
    connect(&nmea_handler, &NMEA_Handler::newDecodedDBT, decoded_nmea_window, &MenuBarDecodedNmea::updateDataDBT);
    connect(&nmea_handler, &NMEA_Handler::newDecodedVHW, decoded_nmea_window, &MenuBarDecodedNmea::updateDataVHW);
    connect(&nmea_handler, &NMEA_Handler::newDecodedZDA, decoded_nmea_window, &MenuBarDecodedNmea::updateDataZDA);
    connect(&nmea_handler, &NMEA_Handler::newDecodedDPT, decoded_nmea_window, &MenuBarDecodedNmea::updateDataDPT);
    connect(&nmea_handler, &NMEA_Handler::newDecodedMWD, decoded_nmea_window, &MenuBarDecodedNmea::updateDataMWD);
    connect(&nmea_handler, &NMEA_Handler::newDecodedMTW, decoded_nmea_window, &MenuBarDecodedNmea::updateDataMTW);
    connect(&nmea_handler, &NMEA_Handler::newDecodedMWV, decoded_nmea_window, &MenuBarDecodedNmea::updateDataMWV);

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
/// Settings ///
////////////////
void Interface::loadSettings()
{
    //Init Settings
    initSettings();

    //UI
    loadUiSettings();
}

void Interface::initSettings()
{
    //Create directory
    QString settingsFolderName = "/OpenSeaNav";
    configPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + settingsFolderName;
    QDir().mkpath(configPath);

    //GUI Settings
    QString settingsFileName = "/configUI.ini";
    settingsGUI = new QSettings(configPath + settingsFileName, QSettings::IniFormat, this);
}

void Interface::loadUiSettings()
{
    QString language = settingsGUI->value("translation/language", "").toString();
    loadTranslation(language);

    int w, h, x, y;

    //Main Window
    bool showLastSize =  settingsGUI->value("mainWindow/showLastSize", false).toBool();
    bool showFullscreen =  settingsGUI->value("mainWindow/showFullScreen", false).toBool();

    if(showLastSize)
    {
        w = settingsGUI->value("mainWindow/width", 1200).toInt();
        h = settingsGUI->value("mainWindow/height", 800).toInt();
        this->resize(w, h);

        x = settingsGUI->value("mainWindow/x", 100).toInt();
        y = settingsGUI->value("mainWindow/y", 100).toInt();
        this->move(x, y);

        ui->actionRestore_Last_Window->setChecked(true);
    }
    else if(showFullscreen)
    {
        ui->actionStartFullscreen->setChecked(true);
        showFullScreen();
    }
    else // Maximized by default
    {
        ui->actionStartMaximized->setChecked(true);
        this->showMaximized();
    }

    //Data Monitor Window
    w = settingsGUI->value("dataMonitorWindow/width", 600).toInt();
    h = settingsGUI->value("dataMonitorWindow/height", 600).toInt();
    data_monitor_window->resize(w, h);

    x = settingsGUI->value("dataMonitorWindow/x", 100).toInt();
    y = settingsGUI->value("dataMonitorWindow/y", 100).toInt();
    data_monitor_window->move(x, y);

    //Decoded NMEA Monitor
    w = settingsGUI->value("decodedNmeaWindow/width", 600).toInt();
    h = settingsGUI->value("decodedNmeaWindow/height", 600).toInt();
    decoded_nmea_window->resize(w, h);

    x = settingsGUI->value("decodedNmeaWindow/x", 100).toInt();
    y = settingsGUI->value("decodedNmeaWindow/y", 100).toInt();
    decoded_nmea_window->move(x, y);
}

void Interface::saveSettings()
{
    //Translation
    settingsGUI->setValue("translation/language", translator.filePath());

    //Main Window
    settingsGUI->setValue("mainWindow/showLastSize", ui->actionRestore_Last_Window->isChecked());
    settingsGUI->setValue("mainWindow/showFullScreen", ui->actionStartFullscreen->isChecked());
    settingsGUI->setValue("mainWindow/width", this->width());
    settingsGUI->setValue("mainWindow/height", this->height());
    settingsGUI->setValue("mainWindow/x", this->x());
    settingsGUI->setValue("mainWindow/y", this->y());

    //Data Monitor
    settingsGUI->setValue("dataMonitorWindow/width", data_monitor_window->width());
    settingsGUI->setValue("dataMonitorWindow/height", data_monitor_window->height());
    settingsGUI->setValue("dataMonitorWindow/x", data_monitor_window->x());
    settingsGUI->setValue("dataMonitorWindow/y", data_monitor_window->y());

    //Decoded NMEA Monitor
    settingsGUI->setValue("decodedNmeaWindow/width", decoded_nmea_window->width());
    settingsGUI->setValue("decodedNmeaWindow/height", decoded_nmea_window->height());
    settingsGUI->setValue("decodedNmeaWindow/x", decoded_nmea_window->x());
    settingsGUI->setValue("decodedNmeaWindow/y", decoded_nmea_window->y());
}



////////////////////
/// Translations ///
////////////////////
void Interface::loadTranslation(QString translationPath)
{
    //Back to default (english)
    if(translationPath == "default")
        qApp->removeTranslator(&translator);

    //Install new translation
    else if (translator.load(translationPath))
        qApp->installTranslator(&translator);

    //Update UI
    ui->retranslateUi(this);
    setWindowTitle(tr("OpenSeaNav - Navigation software"));
    updateTranslationMenuBarGUI(translationPath);

    data_monitor_window->retranslate();
    decoded_nmea_window->retranslate();
    txt_logger_window->retranslate();
    connections_window->retranslate();
}

void Interface::updateTranslationMenuBarGUI(QString language)
{
    if(language == ":/translations/french.qm")
    {
        ui->actionEnglish->setChecked(false);
        ui->actionEnglish->setEnabled(true);

        ui->actionFrench->setChecked(true);
        ui->actionFrench->setEnabled(false);
    }
    else //english by default
    {
        ui->actionEnglish->setChecked(true);
        ui->actionEnglish->setEnabled(false);

        ui->actionFrench->setChecked(false);
        ui->actionFrench->setEnabled(true);
    }
}



////////////////
/// Menu Bar ///
////////////////

//Menu
void Interface::on_actionConnections_triggered()
{
    if (connections_window->isVisible())
    {
        connections_window->hide();
    }
    else
    {
        connections_window->show();
    }
}

void Interface::on_actionEnglish_triggered()
{
    loadTranslation("default");
}

void Interface::on_actionFrench_triggered()
{
    loadTranslation(":/translations/french.qm");
}

void Interface::on_actionExit_triggered()
{
    close();
}


//View
void Interface::on_actionFullscreen_triggered()
{
    toggleFullscreen();
}

void Interface::on_actionStartFullscreen_toggled(bool checked)
{
    if(checked)
    {
        ui->actionStartMaximized->setChecked(false);
        ui->actionRestore_Last_Window->setChecked(false);
    }
}

void Interface::on_actionRestore_Last_Window_toggled(bool checked)
{
    if(checked)
    {
        ui->actionStartFullscreen->setChecked(false);
        ui->actionStartMaximized->setChecked(false);
    }
}

void Interface::on_actionStartMaximized_toggled(bool checked)
{
    if(checked)
    {
        ui->actionStartFullscreen->setChecked(false);
        ui->actionRestore_Last_Window->setChecked(false);
    }
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
    if (data_monitor_window->isVisible())
    {
        data_monitor_window->hide();
    }
    else
    {
        data_monitor_window->show();
        data_monitor_window->scrollDownPlainText();
    }
}

void Interface::on_actionDecoded_NMEA_triggered()
{
    if (decoded_nmea_window->isVisible())
        decoded_nmea_window->hide();
    else
        decoded_nmea_window->show();
}

void Interface::on_actionData_Logger_triggered()
{
    if (txt_logger_window->isVisible())
    {
        txt_logger_window->hide();
    }
    else
    {
        txt_logger_window->show();
    }
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
void Interface::toggleFullscreen()
{
    if (isFullScreen())
        showMaximized();
    else
        showFullScreen();
}





