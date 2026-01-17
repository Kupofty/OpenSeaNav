#pragma once

#include <QDialog>

#include <QFile>
#include <QTextStream>
#include <QMessageBox>

namespace Ui {
class MenuBarUserManual;
}

class MenuBarUserManual : public QDialog
{
    Q_OBJECT

    public:
        explicit MenuBarUserManual(QWidget *parent = nullptr);
        ~MenuBarUserManual();
        void retranslate();

    private:
        Ui::MenuBarUserManual *ui;

    private:
        void loadFile();
};

