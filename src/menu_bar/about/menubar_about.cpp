#include "menubar_about.h"
#include "ui_menubar_about.h"

MenuBarAbout::MenuBarAbout(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MenuBarAbout)
{
    ui->setupUi(this);
    setWindowTitle(tr("About software"));


    QString settingsFolderName = "/OpenSeaNav";
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + settingsFolderName;
    ui->label_config_files_path->setText(configPath);
}

MenuBarAbout::~MenuBarAbout()
{
    delete ui;
}
