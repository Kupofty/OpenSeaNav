#include "menubar_github.h"
#include "ui_menubar_github.h"


///////////////////
/// Class Setup ///
///////////////////
MenuBarGithub::MenuBarGithub(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MenuBarGithub)
{
    ui->setupUi(this);
}

MenuBarGithub::~MenuBarGithub()
{
    delete ui;
}




///////////
/// GUI ///
///////////

