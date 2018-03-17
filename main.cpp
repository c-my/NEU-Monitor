#include "maintray.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);
    MainTray *tray = new MainTray("","");
    QObject::connect(tray, MainTray::exit, &a, QApplication::quit);
    tray->show();
    return a.exec();
}
