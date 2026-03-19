#pragma once

#include <QDialog>
#include <QTimer>
#include <QDateTime>
#include <QLineEdit>
#include <QCloseEvent>
#include <QCheckBox>

#include "nmea/utils.h"

namespace Ui {
class MenuBarSimData;
}

struct ManualInputRow
{
    QCheckBox* checkBox;
    QLineEdit* lineEdit;
};


class MenuBarSimData : public QDialog
{
    Q_OBJECT

    public:
        explicit MenuBarSimData(QWidget *parent = nullptr);
        ~MenuBarSimData();
        void retranslate();

    private slots:
        void on_pushButton_send_manual_input_clicked();
        void on_checkBox_automatic_send_stateChanged(int arg1);
        void on_doubleSpinBox_automatic_send_freq_valueChanged(double arg1);

        //Simu
        void on_doubleSpinBox_depth_valueChanged(double arg1);
        void on_spinBox_valueChanged(int arg1);
        void on_doubleSpinBox_transducerOffset_valueChanged(double arg1);
        void on_doubleSpinBox_latitude_valueChanged(double arg1);
        void on_doubleSpinBox_longitude_valueChanged(double arg1);
        void on_doubleSpinBox_simFreq_valueChanged(double arg1);
        void on_checkBox_simAutomaticSend_stateChanged(int arg1);
        void on_doubleSpinBox_trueHeading_valueChanged(double arg1);
        void on_doubleSpinBox_waterTemp_valueChanged(double arg1);
        void on_doubleSpinBox_magHeading_valueChanged(double arg1);
        void on_doubleSpinBox_magVariation_valueChanged(double arg1);
        void on_doubleSpinBox_speed_valueChanged(double arg1);
        void on_doubleSpinBox_cog_valueChanged(double arg1);
        void on_doubleSpinBox_windSpeed_valueChanged(double arg1);
        void on_doubleSpinBox_windRelativeAngle_valueChanged(double arg1);

        void on_pushButton_checkAll_clicked();
        void on_pushButton_uncheckAll_clicked();
        void on_pushButton_add_new_line_manual_input_clicked();
        void on_pushButton_delete_all_lines_clicked();

        void on_pushButton_enable_all_manual_inputs_clicked();

        void on_pushButton_disable_all_manual_inputs_clicked();

    signals:
        void dataReady(const QString &senderName, const QByteArray &data);
        void windowClosed();

    private:
        //Common
        QString formatLatitude(double lat);
        QString formatLongitude(double lon);
        QString getTime();
        void sendNmeaData(const QString &payload);

        //Manual Data Input
        void sendManualInputData();
        void enabledCheckboxManualInputLines(bool checked);

        //Simu
        void activateAllOutputs(bool check);
        void sendSimuData();
        void sendGLL();
        void sendDBT();
        void sendDPT();
        void sendGSV();
        void sendHDT();
        void sendMTW();
        void sendVTG();
        void sendRMC();
        void sendMWV();

    private:
        Ui::MenuBarSimData *ui;
        int defaultTimeMsTimer = 1000;
        QString sender = "Virtual";
        QTimer autoSendSimuDataTimer;
        QTimer autoSendManualDataTimer;
        QVector<ManualInputRow> manualInputsLineList;

        //Simu
        double depthMeter = 10.0;
        double transducerOffet = 1.5;
        int numberSatellites = 12;
        double latitude = 43.2778;
        double longitude = 5.3073;
        double trueHeading = 0.0;
        double magneticHeading = 0.0;
        double waterTemp = 15.0;
        double sog = 6;
        double cog = 0;
        double magneticVariation = 0.0;
        double windSpeed = 5.0;
        double windRelativeAngle = 0.0;

    protected:
        void closeEvent(QCloseEvent *event) override;
};
