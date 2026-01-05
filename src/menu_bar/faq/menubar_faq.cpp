#include "menubar_faq.h"
#include "ui_menubar_faq.h"

MenuBarFAQ::MenuBarFAQ(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MenuBarFAQ)
{
    ui->setupUi(this);
}

MenuBarFAQ::~MenuBarFAQ()
{
    delete ui;
}
