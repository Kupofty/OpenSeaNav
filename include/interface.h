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

#include "ui_interface.h"

#include "serial_reader.h"
#include "serial_writer.h"
#include "nmea_handler.h"
#include "udp_reader.h"
#include "udp_writer.h"
#include "text_file_writter.h"
#include "nmea_list.h"
#include "utils.h"

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
        SerialReader *serial_reader;
        SerialWriter *serial_writer;
        NMEA_Handler *nmea_handler;
        UdpReader *udp_reader;
        UdpWriter *udp_writer;
        TextFileWritter *text_file_writer;
        QTimer *fileRecordingSizeTimer;
        QMap<QString, QPlainTextEdit*> nmeaSentenceMap;
        QList<QCheckBox*> checkboxOutputSerial;
        QList<QCheckBox*> checkboxOutputUDP;
        QQuickItem *qmlMapObject;

    private:
        QMap<QString, QPlainTextEdit*> getSentenceMap() const;
        QList<QPlainTextEdit*> getPlainTextEditors() const;
        void clearDecodedDataScreens();
        void CloseInputUdp();
        void updateGuiAfterUdpConnection(bool connectSuccess);
        void closeInputSerial();
        void closeOutputSerial();
        void clearRawSortedSentencesScreens();
        void clearRawSentencesScreens();
        void listAvailablePorts(QComboBox *comboBox);
        void hideGUI();
        void connectSignalSlot();
        void updateGuiAfterSerialConnection(bool connectSuccess);
        void updateRecordingFileSize();
        QString getRecordingFilePath();
        bool checkUdpOutputPortIsFree();
        void updateCheckBoxSerialOutput(bool check);
        void updateCheckBoxUdpOutput(bool check);
        void initializeLists();

    private slots:
        void on_pushButton_clear_raw_sorted_sentences_screens_clicked();
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

    private slots:
        void scrollDownPlainText(int index);
        void updateUdpSenderDetails();

        void on_pushButton_clear_raw_sentences_screens_clicked();
        void on_pushButton_freeze_raw_sorted_sentences_screens_toggled(bool checked);
        void on_pushButton_freeze_raw_sentences_screens_toggled(bool checked);

        void on_checkBox_serial_manual_input_stateChanged(int arg1);

    public slots:
        void displayNmeaSentence(const QString &type, const QString &line);
        void displayRawSentences(const QString &nmeaText);
        void updateDataGSV(int totalSatellites, double freq);
        void updateDataGGA(QString time, double latitude, double longitude, int fixQuality, int numSatellites, double hdop, double altitude, double freqHz);
        void updateDataGLL(QString utc, double latitude, double longitude,  double freqHz);
        void updateDataVTG(double track_true, double track_mag, double speed_knot, double speedKmh, double frequency);
        void updateDataGSA(double pdop, double hdop, double vdop, double freqHz);
        void updateDataRMC(QString utcDate, QString utcTime, double latitude, double longitude, double speedMps, double course, double magVar, double freqHz);
        void updateDataHDT(double heading, double freqHz);
        void updateDataDBT(double depthFeet, double depthMeters, double depthFathom, double freqHz);
        void updateDataVHW(double headingTrue, double headingMag, double speedKnots, double speedKmh, double freqHz);
        void updateDataZDA(QString dateStr, QString timeStr, QString offsetStr, double freqHz);
        void updateDataDPT(double depth, double offset, double freqHz);
        void updateDataMWD(double dir1, QString dir1Unit, double dir2, QString dir2Unit, double speed1, QString speed1Unit, double speed2, QString speed2Unit, double freqHz);
        void updateDataMTW(double temp, QString tempUnit, double freqHz);
        void updateDataMWV(double angle, QString ref, double speed, QString unit, double freqHz);

    signals:
        void setAddTimestamp(bool checked);

        void updateBoatPositionMap(QVariant, QVariant);
        void updateBoatHeadingMap(QVariant);
        void updateBoatDepthMap(QVariant);
        void updateBoatSpeedMap(QVariant);
        void updateBoatCourseMap(QVariant);
        void updateBoatWaterTemperatureMap(QVariant);
        void updateBoatDateMap(QVariant);
        void updateBoatTimeMap(QVariant);
};
