#ifndef MAINTRAY_H
#define MAINTRAY_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <netcontroller.h>

class MainTray: public QSystemTrayIcon
{
    Q_OBJECT
public:
    MainTray(QByteArray username, QByteArray password, QObject *parent = Q_NULLPTR);

private:
    QMenu *menu;
    QAction *loginAction, *logoutAction, *autoLogin, *optionsAction, *quitAction;
    NetController *netctrl;
    bool isAutoLogin = true;
    void handleActivated(QSystemTrayIcon::ActivationReason reason);

    QByteArray user, passwd;
    int msgDur = 500;

signals:
    void exit();

private slots:

};

#endif // MAINTRAY_H
