#ifndef MENUBAR_ABOUT_H
#define MENUBAR_ABOUT_H

#include <QDialog>

namespace Ui {
class MenuBarAbout;
}

class MenuBarAbout : public QDialog
{
    Q_OBJECT

    public:
        explicit MenuBarAbout(QWidget *parent = nullptr);
        ~MenuBarAbout();

    private:
        Ui::MenuBarAbout *ui;
};

#endif // MENUBAR_ABOUT_H
