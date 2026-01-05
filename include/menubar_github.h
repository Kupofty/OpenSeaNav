#ifndef MENUBAR_GITHUB_H
#define MENUBAR_GITHUB_H

#include <QDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>

namespace Ui {
class MenuBarGithub;
}

class MenuBarGithub : public QDialog
{
    Q_OBJECT

    public:
        explicit MenuBarGithub(QWidget *parent = nullptr);
        ~MenuBarGithub();

    private:
        Ui::MenuBarGithub *ui;
};

#endif // MENUBAR_GITHUB_H
