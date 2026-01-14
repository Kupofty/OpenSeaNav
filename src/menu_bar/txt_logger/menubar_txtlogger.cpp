#include "menubar_txtlogger.h"
#include "ui_menubar_txtlogger.h"


///////////////////
/// Class Setup ///
///////////////////
MenuBarTxtLogger::MenuBarTxtLogger(QWidget *parent) : QDialog(parent), ui(new Ui::MenuBarTxtLogger)
{
    ui->setupUi(this);

    // Enable minimize, maximize, close buttons
    Qt::WindowFlags flags = Qt::Dialog
                            | Qt::WindowMaximizeButtonHint
                            | Qt::WindowCloseButtonHint
                            | Qt::WindowStaysOnTopHint;
    setWindowFlags(flags);
    setAttribute(Qt::WA_ShowWithoutActivating);

    connect(this, &MenuBarTxtLogger::newRawSentences, &text_file_writer, &TextFileWritter::writeRawSentences);
    connect(&fileRecordingSizeTimer, &QTimer::timeout, this, &MenuBarTxtLogger::updateRecordingFileSize);

}

MenuBarTxtLogger::~MenuBarTxtLogger()
{
    delete ui;
}



////////////////////
/// Public slots ///
////////////////////
void MenuBarTxtLogger::writeRawSentences(const QString &senderName, const QString &type, const QString &nmeaText)
{
    Q_UNUSED(senderName);
    emit newRawSentences(type, nmeaText); //bridge for TextFileWriter
}



/////////////////
/// Functions ///
/////////////////
void MenuBarTxtLogger::retranslate()
{
    ui->retranslateUi(this);
    setWindowTitle(tr("Data logger"));
}

void MenuBarTxtLogger::updateRecordingFileSize()
{
    QFile file(getRecordingFilePath());
    if (file.exists())
    {
        qint64 size = file.size();
        ui->label_file_txt_size->setText(QString("%1 Kb").arg(static_cast<int>(std::round(size / 1000.0))));
    }
    else
    {
        ui->label_file_txt_size->setText(tr("File missing"));
    }
}

QString MenuBarTxtLogger::getRecordingFilePath()
{
    QString dirPath = ui->plainTextEdit_txt_file_path->toPlainText().trimmed();
    QString fileName = ui->plainTextEdit_txt_file_name->toPlainText().trimmed();
    QString fileExtension = ui->comboBox_txt_file_extension->currentText();
    QString fullPath = QDir(dirPath).filePath(fileName + fileExtension);

    return fullPath;
}



/////////////////
/// GUI Logic ///
/////////////////
void MenuBarTxtLogger::on_pushButton_folder_path_documents_clicked()
{
    ui->plainTextEdit_txt_file_path->setPlainText(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
}

void MenuBarTxtLogger::on_pushButton_folder_path_downloads_clicked()
{
    ui->plainTextEdit_txt_file_path->setPlainText(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
}

void MenuBarTxtLogger::on_pushButton_browse_folder_path_clicked()
{
    QString dirPath = QFileDialog::getExistingDirectory(this, tr("Select output directory"),
                                                        QStandardPaths::writableLocation(QStandardPaths::HomeLocation),  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);


    if (!dirPath.isEmpty())
        ui->plainTextEdit_txt_file_path->setPlainText(dirPath);
}

void MenuBarTxtLogger::on_pushButton_automatic_txt_file_name_clicked()
{
    QString automaticFileName = "Log_NMEA_" + QDateTime::currentDateTime().toString("yyyy_MM_dd_HH_mm_ss");
    ui->plainTextEdit_txt_file_name->setPlainText(automaticFileName);
}

void MenuBarTxtLogger::on_pushButton_save_txt_file_toggled(bool checked)
{
    //Update add timestamp
    bool isTimestampChecked = ui->checkBox_output_txt_file_add_timestamp->isChecked();
    text_file_writer.updateAddTimestamp(isTimestampChecked);

    if(checked)
    {
        //Check for missing path/name
        QString dirPath = ui->plainTextEdit_txt_file_path->toPlainText().trimmed();
        QString fileName = ui->plainTextEdit_txt_file_name->toPlainText().trimmed();
        if (dirPath.isEmpty() || fileName.isEmpty())
        {
            QMessageBox::warning(this, tr("Missing information"),
                                 tr("Please select an output folder and enter a file name before saving."));
            ui->pushButton_save_txt_file->setChecked(false);
            return;
        }

        // Ask before overwriting previous recording
        QString fullPath = getRecordingFilePath();
        QFileInfo fileInfo(fullPath);
        if (fileInfo.exists())
        {
            auto reply = QMessageBox::question(this,
                                               tr("Overwrite file?"),
                                               tr("A file with the same name already exists in the selected location.\n"
                                                  "Do you want to replace it?"),
                                               QMessageBox::Yes | QMessageBox::No,
                                               QMessageBox::No
                                               );

            if (reply != QMessageBox::Yes)
            {
                ui->pushButton_save_txt_file->setChecked(false);
                return;
            }
        }

        //Create file if possible
        int result = text_file_writer.createFile(getRecordingFilePath());
        if (!result)
        {
            ui->pushButton_save_txt_file->setChecked(false);
            return;
        }

        //Update file size
        fileRecordingSizeTimer.start(1000);
        ui->pushButton_save_txt_file->setText(tr("Stop recording"));
    }
    else
    {
        fileRecordingSizeTimer.stop();
        ui->label_file_txt_size->setText(tr("Not recording"));
        text_file_writer.closeFile();
        ui->pushButton_save_txt_file->setText(tr(" Record Data To File"));
    }
}


