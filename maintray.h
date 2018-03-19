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
#include <QApplication>

class MainTray: public QSystemTrayIcon
{
    Q_OBJECT
public:
    MainTray(QByteArray username, QByteArray password, QObject *parent = Q_NULLPTR);
    ~MainTray();
    enum Status {Unknown, Offline, Online, Disconnected};

private:
    QMenu *menu, *infoMenu, *settingsMenu;
    QAction *loginAction, *logoutAction, *autoLogin, *optionsAction, *aboutAction, *quitAction;
    QAction *mbAction, *timeAction, *balanceAction, *ipAction, *bootAction;
    NetController *netctrl;
    QSettings settings;
    OptionsWindow opWindow;
    QTimer *autoLoginTimer; //自动重连定时器

    bool isForceLogout = false; //自动重连flag
    bool isAutoLogin;    //手动注销flag
    Status currentState = Unknown;  //当前状态
    void handleActivated(QSystemTrayIcon::ActivationReason reason);//处理左键单击
    void showOptions(); //显示选项窗口
    void showAbout();   //显示关于窗口
    void setAutoStart(bool set);

    QByteArray user, passwd;
    int msgDur = 1000;   //通知持续时间


signals:
    void exit();

private slots:
    void updataUserInfo(QByteArray id, QByteArray pass);
    void handleInfo(QString mb, QString sec, QString balance, QString ip);

};

#endif // MAINTRAY_H
