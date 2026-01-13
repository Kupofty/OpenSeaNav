#ifndef MENUBAR_TXTLOGGER_H
#define MENUBAR_TXTLOGGER_H

#include <QDialog>
#include <QStandardPaths>
#include <QDir>
#include <QTimer>
#include <QFileDialog>
#include <QMessageBox>

#include "nmea/utils.h"
#include "writers/text_file_writter.h"

namespace Ui {
class MenuBarTxtLogger;
}

class MenuBarTxtLogger : public QDialog
{
    Q_OBJECT

    public:
        explicit MenuBarTxtLogger(QWidget *parent = nullptr);
        ~MenuBarTxtLogger();
        void retranslate();

    private:
        Ui::MenuBarTxtLogger *ui;
            TextFileWritter text_file_writer;
            QTimer fileRecordingSizeTimer;

    private:
        void updateRecordingFileSize();
        QString getRecordingFilePath();

    signals:
        void newRawSentences(const QString &type, const QString &nmeaText);

    public slots:
        void writeRawSentences(const QString &type, const QString &nmeaText);

    private slots:
        void on_pushButton_browse_folder_path_clicked();
        void on_pushButton_save_txt_file_toggled(bool checked);
        void on_pushButton_automatic_txt_file_name_clicked();
        void on_pushButton_folder_path_documents_clicked();
        void on_pushButton_folder_path_downloads_clicked();
};

#endif // MENUBAR_TXTLOGGER_H
