#ifndef MENUBAR_CONNECTIONS_H
#define MENUBAR_CONNECTIONS_H

#include <QDialog>
#include <QComboBox>
#include <QCheckBox>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QRegularExpression>

#include "readers/tcp_reader.h"
#include "readers/serial_reader.h"
#include "readers/udp_reader.h"
#include "writers/udp_writer.h"
#include "writers/serial_writer.h"
#include "nmea/utils.h"

namespace Ui {
class MenuBarConnections;
}

class MenuBarConnections : public QDialog
{
    Q_OBJECT

    public:
        explicit MenuBarConnections(QWidget *parent = nullptr);
        ~MenuBarConnections();
        void retranslate();

    private:
        Ui::MenuBarConnections *ui;
        UdpReader udp_reader;
        UdpWriter udp_writer;
        SerialReader serial_reader;
        SerialWriter serial_writer;
        TcpReader tcp_reader;
        QList<QCheckBox*> checkboxOutputSerial;
        QList<QCheckBox*> checkboxOutputUDP;
        QString configPath;
        QSettings *settingsConnections;

    private:
        void initSettings();
        void saveSettings();
        void loadSettings();
        void loadSerialInputSettings();
        void loadTcpInputSettings();
        void loadUdpInputSettings();

        void closeInputUdp();
        void updateGuiAfterUdpConnection(bool connectSuccess);
        void updateGuiAfterTcpConnection(bool connectSuccess);
        void closeInputSerial();
        void closeOutputSerial();
        void listAvailableSerialPorts(QComboBox *comboBox);
        void hideGUI();
        void updateGuiAfterSerialConnection(bool connectSuccess);
        bool checkUdpOutputPortIsFree();
        void updateCheckBoxSerialOutput(bool check);
        void updateCheckBoxUdpOutput(bool check);
        void initializeLists();

    signals:
        void newLineReceived(QString senderName, const QByteArray &line);

    public slots:
        void publishNMEA(const QString &senderName, const QString &type, const QString &nmeaText);

    private slots:
        void updateUdpSenderDetails();
        void updateTcpState(QString state);

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
        void on_pushButton_connect_tcp_input_clicked();
        void on_pushButton_disconnect_tcp_input_clicked();
        void on_pushButton_tcp_input_localhost_clicked();
};

#endif // MENUBAR_CONNECTIONS_H
