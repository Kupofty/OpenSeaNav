#pragma once

#include <QDialog>

namespace Ui {
class MenuBarFAQ;
}

class MenuBarFAQ : public QDialog
{
    Q_OBJECT

public:
    explicit MenuBarFAQ(QWidget *parent = nullptr);
    ~MenuBarFAQ();

private:
    Ui::MenuBarFAQ *ui;
};


