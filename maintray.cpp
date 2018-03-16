#include "maintray.h"

MainTray::MainTray(QByteArray username, QByteArray password, QObject *parent): QSystemTrayIcon(parent), menu(new QMenu()),
    user(username),passwd(password)
{
    setIcon(QIcon(tr(":/icon/favicon.ico")));

    netctrl = new NetController(user,passwd,this);

    connect(netctrl, NetController::getOnline, this, [this](){
        showMessage(tr("网络已连接"),tr("校园网登陆成功"), this->icon(), msgDur);});
    connect(netctrl, NetController::getOffline, this, [this](){
        showMessage(tr("网络已断开"),tr("校园网已注销"), this->icon(), msgDur);
        if(isAutoLogin){
            qDebug()<<isAutoLogin;
            netctrl->sendLoginRequest();
        }
    });
    connect(netctrl, NetController::getDisconnected, this, [this](){
        showMessage(tr("无法连接至校园网"),tr("校园网失去连接"), this->icon(), msgDur);});

    loginAction = new QAction(tr("连接网络"),this);
    logoutAction = new QAction(tr("断开网络"),this);
    autoLogin = new QAction(tr("自动重连"), this);
    optionsAction = new QAction(tr("设置"), this);
    quitAction = new QAction(tr("退出"), this);

    autoLogin->setCheckable(true);

    connect(loginAction, QAction::triggered, this,[this](){netctrl->sendLoginRequest();});
    connect(logoutAction, QAction::triggered, this,[this](){netctrl->sendLogoutRequest();});
    connect(autoLogin, QAction::toggled, this, [this](bool set){
        isAutoLogin = set;
        netctrl->sendLoginRequest();
    });
//    connect(optionsAction, QAction::triggered, this, [this]());
    connect(quitAction, QAction::triggered, this,[this](){emit exit();});

    autoLogin->setChecked(true);

    menu->addAction(loginAction);
    menu->addAction(logoutAction);
    menu->addAction(autoLogin);
    menu->addAction(quitAction);

    setContextMenu(menu);

    connect(this, activated, this, handleActivated);

}

void MainTray::handleActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
        contextMenu()->show();
        break;
    default:
        break;
    }
}

