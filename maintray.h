#ifndef MAINTRAY_H
#define MAINTRAY_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <netcontroller.h>
#include <optionswindow.h>
#include <QSettings>
#include <QMessageBox>
#include <QTimer>

class MainTray: public QSystemTrayIcon
{
    Q_OBJECT
public:
    MainTray(QByteArray username, QByteArray password, QObject *parent = Q_NULLPTR);
    enum Status {Unknown, Offline, Online, Disconnected};

private:
    QMenu *menu, *infoMenu;
    QAction *loginAction, *logoutAction, *autoLogin, *optionsAction, *aboutAction, *quitAction;
    QAction *mbAction, *timeAction, *balanceAction, *ipAction;
    NetController *netctrl;
    QSettings settings;
    OptionsWindow opWindow;
    QTimer *autoLoginTimer;
    bool isForceLogout = false;

    bool isAutoLogin = true;
    Status currentState = Unknown;
    void handleActivated(QSystemTrayIcon::ActivationReason reason);
    void showOptions();
    void showAbout();

    QByteArray user, passwd;
    int msgDur = 500;


signals:
    void exit();

private slots:
    void updataUserInfo(QByteArray id, QByteArray pass);
    void handleInfo(QString mb, QString sec, QString balance, QString ip);

};

#endif // MAINTRAY_H
