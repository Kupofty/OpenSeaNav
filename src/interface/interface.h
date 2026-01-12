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

#include "ui_interface.h"

#include "readers/serial_reader.h"
#include "readers/udp_reader.h"
#include "nmea/nmea_handler.h"
#include "writers/udp_writer.h"
#include "writers/serial_writer.h"
#include "writers/text_file_writter.h"
#include "nmea/utils.h"

#include "menu_bar/about/menubar_about.h"
#include "menu_bar/faq/menubar_faq.h"
#include "menu_bar/simu/menubar_simdata.h"
#include "menu_bar/data_monitor/menubar_datamonitor.h"
#include "menu_bar/decoded_nmea/menubar_decodednmea.h"

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
        SerialReader serial_reader;
        SerialWriter serial_writer;
        NMEA_Handler nmea_handler;
        UdpReader udp_reader;
        UdpWriter udp_writer;
        TextFileWritter text_file_writer;
        QTimer fileRecordingSizeTimer;
        QMap<QString, QPlainTextEdit*> nmeaSentenceMap;
        QList<QCheckBox*> checkboxOutputSerial;
        QList<QCheckBox*> checkboxOutputUDP;
        QQuickItem *qmlMapObject;
        MenuBarDataMonitor *data_monitor_window;
        MenuBarDecodedNmea *decoded_nmea_window;
        QString configPath;
        QSettings *settingsGUI;
        QSettings *settingsConnections;
        QTranslator translator;

    private:
        void initSettings();
        void saveSettings();
        void loadSettings();
        void loadUiSettings();
        void loadSerialInputSettings();
        void loadUdpInputSettings();

        void toggleFullscreen();
        void clearDecodedDataScreens();
        void closeInputUdp();
        void updateGuiAfterUdpConnection(bool connectSuccess);
        void closeInputSerial();
        void closeOutputSerial();
        void listAvailableSerialPorts(QComboBox *comboBox);
        void hideGUI();
        void connectSignalSlot();
        void updateGuiAfterSerialConnection(bool connectSuccess);
        void updateRecordingFileSize();
        QString getRecordingFilePath();
        bool checkUdpOutputPortIsFree();
        void updateCheckBoxSerialOutput(bool check);
        void updateCheckBoxUdpOutput(bool check);
        void initializeLists();
        void loadTranslation(QString translationPath);
        void removeTranslation();
        void updateTranslationGUI(QString language);

    private slots:
        void updateUdpSenderDetails();

    private slots:
        void on_pushButton_connect_serial_input_clicked();
        void on_pushButton_disconnect_serial_input_clicked();
        void on_pushButton_refresh_available_ports_list_clicked();
        void on_spinBox_update_udp_port_output_valueChanged(int port);
        void on_checkBox_udp_output_gga_toggled(bool checked);
        void on_checkBox_udp_output_rmc_toggled(bool checked);
        void on_checkBox_udp_output_gsv_toggled(bool checked);
        void on_checkBox_udp_output_gll_toggled(bool checked);
        void on_checkBox_udp_output_gsa_toggled(bool checked);
        void on_checkBox_udp_output_vtg_toggled(bool checked);
        void on_checkBox_udp_output_zda_toggled(bool checked);
        void on_checkBox_udp_output_dpt_toggled(bool checked);
        void on_checkBox_udp_output_mtw_toggled(bool checked);
        void on_checkBox_udp_output_mwv_toggled(bool checked);
        void on_checkBox_udp_output_mwd_toggled(bool checked);
        void on_checkBox_udp_output_hdt_toggled(bool checked);
        void on_checkBox_udp_output_dbt_toggled(bool checked);
        void on_checkBox_udp_output_vhw_toggled(bool checked);
        void on_pushButton_activate_udp_output_toggled(bool checked);
        void on_checkBox_udp_output_others_toggled(bool checked);
        void on_pushButton_check_all_udp_output_clicked();
        void on_pushButton_uncheck_all_udp_output_clicked();
        void on_comboBox_udp_host_address_currentTextChanged(const QString &hostAddress);
        void on_lineEdit_udp_ip_address_editingFinished();
        void on_pushButton_browse_folder_path_clicked();
        void on_pushButton_save_txt_file_toggled(bool checked);
        void on_pushButton_automatic_txt_file_name_clicked();
        void on_pushButton_folder_path_documents_clicked();
        void on_pushButton_folder_path_downloads_clicked();
        void on_pushButton_refresh_available_port_serial_output_clicked();
        void on_pushButton_connect_serial_output_clicked();
        void on_pushButton_disconnect_serial_output_clicked();
        void on_checkBox_serial_output_gga_toggled(bool checked);
        void on_pushButton_activate_serial_output_toggled(bool checked);
        void on_checkBox_serial_output_gsv_toggled(bool checked);
        void on_checkBox_serial_output_rmc_toggled(bool checked);
        void on_checkBox_serial_output_gsa_toggled(bool checked);
        void on_checkBox_serial_output_gll_toggled(bool checked);
        void on_checkBox_serial_output_vtg_toggled(bool checked);
        void on_checkBox_serial_output_others_toggled(bool checked);
        void on_checkBox_serial_output_zda_toggled(bool checked);
        void on_checkBox_serial_output_dpt_toggled(bool checked);
        void on_checkBox_serial_output_mtw_toggled(bool checked);
        void on_checkBox_serial_output_mwv_toggled(bool checked);
        void on_checkBox_serial_output_mwd_toggled(bool checked);
        void on_checkBox_serial_output_hdt_toggled(bool checked);
        void on_checkBox_serial_output_dbt_toggled(bool checked);
        void on_checkBox_serial_output_vhw_toggled(bool checked);
        void on_pushButton_check_all_serial_output_clicked();
        void on_pushButton_uncheck_all_serial_output_clicked();
        void on_pushButton_connect_udp_input_clicked();
        void on_pushButton_disconnect_udp_input_clicked();
        void on_checkBox_serial_manual_input_stateChanged(int arg1);
        void on_checkBox_serial_manual_output_stateChanged(int arg1);

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

    signals:
        void setAddTimestamp(bool checked);


};
