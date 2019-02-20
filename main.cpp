#include "maintray.h"
#include <QApplication>
#include <QTranslator>
#include <QFont>
#include <QFontDatabase>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);
    a.setApplicationName(QObject::tr("NEU-Monitor"));
    a.setWindowIcon(QIcon(QObject::tr(":/icon/favicon.ico")));
//    QFontDatabase::addApplicationFont(QString(":/resource/SourceHanSansCN-Regular.otf"));
//    QFont font("SourceHanSourceHanSansCN", 9);
//    a.setFont(font);
//    a.setFont(QFont("Microsoft YaHei", 9));
    MainTray *tray = new MainTray();
    QObject::connect(tray, &MainTray::exit, &a, &QApplication::quit);
    tray->show();
    return a.exec();
}
