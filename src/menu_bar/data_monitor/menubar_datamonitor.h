#pragma once

#include <QDialog>
#include <QScrollBar>

#include "nmea/utils.h"

namespace Ui {
class MenuBarDataMonitor;
}

class MenuBarDataMonitor : public QDialog
{
    Q_OBJECT

    public:
        explicit MenuBarDataMonitor(QWidget *parent = nullptr);
        ~MenuBarDataMonitor();
        void scrollDownPlainText();
        void scrollUpPlainText();

    public slots:
        void displayNmeaSentence(const QString &type, const QString &nmeaText);

    private slots:
        void on_pushButton_clear_data_monitor_clicked();
        void on_spinBox_data_monitor_size_limit_editingFinished();
        void on_pushButton_scroll_down_clicked();
        void on_pushButton_scroll_up_clicked();

    private:
        Ui::MenuBarDataMonitor *ui;

    private:
        void updatePlainTextsSizeLimit(unsigned int sentenceLimit);
        void addToDataMonitor(const QString &nmeaText);

};
