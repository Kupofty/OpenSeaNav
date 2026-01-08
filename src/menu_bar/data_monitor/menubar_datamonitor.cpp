#include "menubar_datamonitor.h"
#include "ui_menubar_datamonitor.h"


///////////////////
/// Class Setup ///
///////////////////
MenuBarDataMonitor::MenuBarDataMonitor(QWidget *parent) : QDialog(parent), ui(new Ui::MenuBarDataMonitor)
{
    ui->setupUi(this);

    //Set title
    setWindowTitle("NMEA Data Monitor");

    // Enable minimize, maximize, close buttons
    Qt::WindowFlags flags = Qt::Dialog
                            | Qt::WindowMaximizeButtonHint
                            | Qt::WindowCloseButtonHint
                            | Qt::WindowStaysOnTopHint;
    setWindowFlags(flags);
    setAttribute(Qt::WA_ShowWithoutActivating);
}

MenuBarDataMonitor::~MenuBarDataMonitor()
{
    delete ui;
}



/////////////////
/// Functions ///
/////////////////
void MenuBarDataMonitor::displayNmeaSentence(const QString &type, const QString &nmeaText)
{
    Q_UNUSED(type);

    //Do not display if freeze button is pressed
    bool isDataMonitorFreezed = ui->pushButton_freeze_data_monitor->isChecked();
    if(isDataMonitorFreezed)
        return;

    //Main Data Monitor
    addToDataMonitor(nmeaText);
}

void MenuBarDataMonitor::addToDataMonitor(const QString &nmeaText)
{
    // Look for nmea ID in sentence
    QString nmeaType = getNmeaType(nmeaText);

    // Apply filter
    QString filter = ui->lineEdit_data_monitor_filter->text().trimmed().toUpper();
    if (!filter.isEmpty() && !nmeaType.contains(filter))
        return;

    // Build colored HTML line according to checksum validity
    bool valid = isNmeaChecksumValid(nmeaText);
    QString color = valid ? "green" : "red";
    QString htmlLine = QString("<span style=\"color:%1;\">%2</span>")
                           .arg(color, getTimeStamp() + nmeaText.toHtmlEscaped());

    // Append to QTextEdit
    ui->textEdit_data_monitor->append(htmlLine);
}

void MenuBarDataMonitor::scrollUpPlainText()
{
    QTextEdit* dataMonitor = ui->textEdit_data_monitor;
    dataMonitor->verticalScrollBar()->setValue(dataMonitor->verticalScrollBar()->minimum());
}

void MenuBarDataMonitor::scrollDownPlainText()
{
    QTextEdit* dataMonitor = ui->textEdit_data_monitor;
    dataMonitor->verticalScrollBar()->setValue(dataMonitor->verticalScrollBar()->maximum());
}

void MenuBarDataMonitor::updatePlainTextsSizeLimit(unsigned int sentenceLimit)
{
    ui->textEdit_data_monitor->document()->setMaximumBlockCount(sentenceLimit);
}



/////////////////
/// GUI Logic ///
/////////////////
void MenuBarDataMonitor::on_pushButton_clear_data_monitor_clicked()
{
    ui->textEdit_data_monitor->clear();
}

void MenuBarDataMonitor::on_spinBox_data_monitor_size_limit_editingFinished()
{
    unsigned int new_limit = ui->spinBox_data_monitor_size_limit->value();
    updatePlainTextsSizeLimit(new_limit);
}

void MenuBarDataMonitor::on_pushButton_scroll_down_clicked()
{
    scrollDownPlainText();
}

void MenuBarDataMonitor::on_pushButton_scroll_up_clicked()
{
    scrollUpPlainText();
}

