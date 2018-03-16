#ifndef MAINTRAY_H
#define MAINTRAY_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <netcontroller.h>
#include <optionswindow.h>
#include <QSettings>

class MainTray: public QSystemTrayIcon
{
    Q_OBJECT
public:
    MainTray(QByteArray username, QByteArray password, QObject *parent = Q_NULLPTR);

private:
    QMenu *menu;
    QAction *loginAction, *logoutAction, *autoLogin, *optionsAction, *quitAction;
    NetController *netctrl;
    QSettings settings;
    OptionsWindow opWindow;

    bool isAutoLogin = true;
    void handleActivated(QSystemTrayIcon::ActivationReason reason);
    void showOptions();

    QByteArray user, passwd;
    int msgDur = 500;

signals:
    void exit();

private slots:
    void updataUserInfo(QByteArray id, QByteArray pass);

};

#endif // MAINTRAY_H
