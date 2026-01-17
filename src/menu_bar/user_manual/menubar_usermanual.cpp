#include "menubar_usermanual.h"
#include "ui_menubar_usermanual.h"

MenuBarUserManual::MenuBarUserManual(QWidget *parent) : QDialog(parent), ui(new Ui::MenuBarUserManual)
{
    ui->setupUi(this);

    // Enable minimize, maximize, close buttons
    Qt::WindowFlags flags = Qt::Dialog
                            | Qt::WindowMaximizeButtonHint
                            | Qt::WindowCloseButtonHint
                            | Qt::WindowStaysOnTopHint;
    setWindowFlags(flags);
    setAttribute(Qt::WA_ShowWithoutActivating);

    //Init
    loadFile();

}

MenuBarUserManual::~MenuBarUserManual()
{
    delete ui;
}


void MenuBarUserManual::loadFile()
{
    QFile file(":/docs/UserManual.md");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString markdown = in.readAll();
        file.close();

        ui->textBrowser->setMarkdown(markdown);
    }
    else
    {
        ui->textBrowser->setText(tr("Failed to load manual."));
    }
}

void MenuBarUserManual::retranslate()
{
    ui->retranslateUi(this);
    setWindowTitle(tr("User Manual"));
}
