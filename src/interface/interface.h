#pragma once

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTimer>
#include <QScrollBar>
#include <QVariant>
#include <QtGui>
#include <QtQuick>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <QShortcut>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QTranslator>
#include <QLocale>
#include <QCoreApplication>
#include <QPlainTextEdit>

#include "ui_interface.h"

#include "nmea/nmea_handler.h"
#include "nmea/utils.h"

#include "menu_bar/about/menubar_about.h"
#include "menu_bar/faq/menubar_faq.h"
#include "menu_bar/simu/menubar_simdata.h"
#include "menu_bar/data_monitor/menubar_datamonitor.h"
#include "menu_bar/decoded_nmea/menubar_decodednmea.h"
#include "menu_bar/txt_logger/menubar_txtlogger.h"
#include "menu_bar/connections/menubar_connections.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class Interface;
}
QT_END_NAMESPACE

class Interface : public QMainWindow
{
    Q_OBJECT

    public:
        Interface(QWidget *parent = nullptr);
        ~Interface();

    private:
        Ui::Interface *ui;
        QQuickItem *qmlMapObject;
        NMEA_Handler nmea_handler;
        MenuBarDataMonitor *data_monitor_window;
        MenuBarDecodedNmea *decoded_nmea_window;
        MenuBarTxtLogger *txt_logger_window;
        MenuBarConnections *connections_window;

        QString configPath;
        QSettings *settingsGUI;
        QTranslator translator;

    private:
        void initSettings();
        void saveSettings();
        void loadSettings();
        void loadUiSettings();

        void toggleFullscreen();
        void connectSignalSlot();
        void loadTranslation(QString translationPath);
        void updateTranslationMenuBarGUI(QString language);


    private slots:
        void on_actionExit_triggered();
        void on_actionAbout_triggered();
        void on_actionManual_Data_Input_triggered();
        void on_actionFAQ_triggered();
        void on_actionFullscreen_triggered();
        void on_actionData_Monitor_triggered();
        void on_actionDecoded_NMEA_triggered();
        void on_actionEnglish_triggered();
        void on_actionFrench_triggered();
        void on_actionStartFullscreen_toggled(bool arg1);
        void on_actionRestore_Last_Window_toggled(bool arg1);
        void on_actionStartMaximized_toggled(bool arg1);
        void on_actionData_Logger_triggered();
        void on_actionConnections_triggered();
};
