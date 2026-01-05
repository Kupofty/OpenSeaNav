#pragma once

#include <QDialog>
#include <QTimer>

#include "utils.h"

namespace Ui {
class MenuBarSimData;
}

class MenuBarSimData : public QDialog
{
    Q_OBJECT

    public:
        explicit MenuBarSimData(QWidget *parent = nullptr);
        ~MenuBarSimData();

    private slots:
        void on_pushButton_send_manual_input_clicked();
        void on_lineEdit_manual_input_textChanged(const QString &arg1);
        void on_checkBox_automatic_send_stateChanged(int arg1);
        void on_doubleSpinBox_automatic_send_freq_valueChanged(double arg1);

    signals:
        void dataReady(const QByteArray &data);

    private:
        void sendData();

    private:
        Ui::MenuBarSimData *ui;
        QTimer *autoSendTimer;
        int defaultTimeMsTimer = 1000;
};
