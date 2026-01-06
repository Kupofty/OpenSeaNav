#include "menubar_about.h"
#include "ui_menubar_about.h"

MenuBarAbout::MenuBarAbout(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MenuBarAbout)
{
    ui->setupUi(this);
    setWindowTitle("About Software");
}

MenuBarAbout::~MenuBarAbout()
{
    delete ui;
}
