#include "menubar_simdata.h"
#include "ui_menubar_simdata.h"

///////////////////
/// Class Setup ///
///////////////////
MenuBarSimData::MenuBarSimData(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MenuBarSimData)
{
    ui->setupUi(this);

    autoSendTimer = new QTimer(this);
    autoSendTimer->setInterval(defaultTimeMsTimer);
    connect(autoSendTimer, &QTimer::timeout, this, &MenuBarSimData::sendData);

    setWindowTitle(tr("Manual Data Input"));
    setAttribute(Qt::WA_DeleteOnClose);
}

MenuBarSimData::~MenuBarSimData()
{
    delete ui;
}



/////////////////
/// Functions ///
/////////////////
void MenuBarSimData::sendData()
{
    QString payload = ui->lineEdit_manual_input->text().trimmed();
    if (payload.isEmpty())
        return;

    QString checksumText = ui->label_checksum->text();
    if (checksumText.isEmpty())
        return;

    QString nmea = QString("$%1%2")
                       .arg(payload, checksumText)
                       .toUpper();

    emit dataReady(nmea.toUtf8());
}



/////////////////
/// GUI Logic ///
/////////////////
void MenuBarSimData::on_pushButton_send_manual_input_clicked()
{
    sendData();
}

void MenuBarSimData::on_lineEdit_manual_input_textChanged(const QString &payload)
{
    quint8 checksum = calculateChecksum(payload);
    QString checksumStr = QString("*%1").arg(checksum, 2, 16, QLatin1Char('0')).toUpper();
    ui->label_checksum->setText(checksumStr);
}

void MenuBarSimData::on_checkBox_automatic_send_stateChanged(int state)
{
    if (state == Qt::Checked)
        autoSendTimer->start();
    else
        autoSendTimer->stop();
}

void MenuBarSimData::on_doubleSpinBox_automatic_send_freq_valueChanged(double value)
{
    int intervalMs = 1000 / value;
    autoSendTimer->setInterval(intervalMs);
}

