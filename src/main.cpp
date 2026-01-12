#include "interface/interface.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Interface gui;

    //Icon & Name
    QIcon appIcon(":/icons/mainIcon");
    app.setWindowIcon(appIcon);

    gui.show();
    return app.exec();
}
