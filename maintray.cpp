#include "maintray.h"

MainTray::MainTray(QByteArray username, QByteArray password, QObject *parent): QSystemTrayIcon(parent),
    menu(new QMenu()),infoMenu(new QMenu()), settingsMenu(new QMenu()), settings("TurnMeOn", "NEU-Dectect"),
    opWindow(settings.value("id",0).toByteArray(), settings.value("password", 0).toByteArray()),
    user(username),passwd(password)
{

    opWindow.hide();
    setIcon(QIcon(tr(":/icon/favicon.ico")));

    netctrl = new NetController(user,passwd,this);
    netctrl->setUsername(settings.value("id",0).toByteArray());
    netctrl->setPassword(settings.value("password", 0).toByteArray());

    connect(&opWindow, OptionsWindow::saveSettings, this, updataUserInfo);
    //连接controller状态改变信号
    connect(netctrl, NetController::getOnline, this, [this](){
        showMessage(tr("网络已连接"),tr("校园网登陆成功"), this->icon(), msgDur);
        currentState = Online;
        setToolTip(tr("当前状态：连接"/*\n自动重连：") + (isAutoLogin? tr("开启"):tr("关闭")*/));
    });
    connect(netctrl, NetController::getOffline, this, [this](){
        showMessage(tr("网络已断开"),tr("校园网已注销"), this->icon(), msgDur);
        currentState = Offline;
        setToolTip(tr("当前状态：断开"/*\n自动重连：") + (isAutoLogin? tr("开启"):tr("关闭")*/));
        }
    );
    connect(netctrl, NetController::getDisconnected, this, [this](){
        showMessage(tr("网络已断开"),tr("无法连接至校园网"), this->icon(), msgDur);
        currentState = Disconnected;
        setToolTip(tr("当前状态：无法连接"/*\n自动重连：") + (isAutoLogin? tr("开启"):tr("关闭")*/));});
    connect(netctrl, NetController::sendInfo, this, handleInfo);

    //菜单Action
    loginAction = new QAction(tr("连接网络"),this);
    logoutAction = new QAction(tr("断开网络"),this);
    autoLogin = new QAction(tr("自动重连"), this);
    optionsAction = new QAction(tr("选项"), this);
    bootAction = new QAction(tr("开机启动"), this);
    aboutAction = new QAction(tr("关于"), this);
    quitAction = new QAction(tr("退出"), this);

    mbAction = new QAction(tr("已用流量: "),this);
    timeAction = new QAction(tr("已用时长: "),this);
    balanceAction = new QAction(tr("账户余额: "),this);
    ipAction = new QAction(tr("IP地址: "),this);

    autoLogin->setCheckable(true);
    bootAction->setCheckable(true);

    connect(loginAction, QAction::triggered, this,[this](){
        netctrl->sendLoginRequest();
        isForceLogout = false;
    });
    connect(logoutAction, QAction::triggered, this,[this](){
        netctrl->sendLogoutRequest();
        isForceLogout = true;
    });
    connect(autoLogin, QAction::toggled, this, [this](bool set){
        isAutoLogin = set;
        settings.setValue("isAutoLogin", set);
    });
    connect(bootAction, QAction::toggled, this, [this](bool set){
        settings.setValue("isOnBoot", set);
        setAutoStart(set);
    });
    connect(optionsAction, QAction::triggered, this, [this](){showOptions();});
    connect(aboutAction, QAction::triggered, this, showAbout);
    connect(quitAction, QAction::triggered, this,[this](){emit exit();});

    autoLogin->setChecked(settings.value("isAutoLogin", true).toBool());
    bootAction->setChecked(settings.value("isOnBoot", false).toBool());

    settingsMenu->setTitle(tr("设置"));
    settingsMenu->addAction(optionsAction);
    settingsMenu->addAction(bootAction);

    infoMenu->setTitle(tr("账户信息"));
    infoMenu->addAction(mbAction);
    infoMenu->addAction(timeAction);
    infoMenu->addAction(balanceAction);
    infoMenu->addAction(ipAction);

    menu->addAction(loginAction);
    menu->addAction(logoutAction);
    menu->addAction(autoLogin);
    menu->addMenu(infoMenu);
    menu->addMenu(settingsMenu);
    menu->addAction(aboutAction);
    menu->addAction(quitAction);

    setContextMenu(menu);

    connect(this, activated, this, handleActivated);
    connect(this, exit, this, [this](){setVisible(false);});

    autoLoginTimer = new QTimer(this);
    autoLoginTimer->setInterval(1000);
    autoLoginTimer->start();
    connect(autoLoginTimer,QTimer::timeout, this, [this](){
//        netctrl->checkStatus();   //可只使用一个定时器
        //Offline状态下自动重连
        if(currentState == Offline && isAutoLogin && !isForceLogout)
            netctrl->sendLoginRequest();
    });

}

MainTray::~MainTray()
{
    delete menu;
    delete infoMenu;
}

void MainTray::handleActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
        contextMenu()->popup(this->geometry().topRight());
        break;
    default:
        break;
    }
}

void MainTray::showOptions()
{
    opWindow.show();
}

void MainTray::showAbout()
{
    QMessageBox *aboutWindow = new QMessageBox();
    aboutWindow->setStandardButtons(QMessageBox::Ok);
    aboutWindow->setText(tr("<h1>NEU-Monitor</h1>"
                             "<p>Based on Qt 5.10.0 (MinGW 5.3.0, 32bit)</p>"
                             "Source Code: <a href=\"https://github.com/TurnMeOn/NEU-Dectect\">https://github.com/TurnMeOn/NEU-Dectect</a><br/>"
                            "Email: <address>"
                            "<a href=\"mailto:cmy1113@yeah.net?subject=SerialAsst Feedback\">TurnMeOn</a>"
                            "</address>"));
    aboutWindow->show();
}

void MainTray::setAutoStart(bool set)
{
    QString application_name = QApplication::applicationName();
    QSettings *settings = new QSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    if(set)
    {
        QString application_path = QApplication::applicationFilePath();
        settings->setValue(application_name, application_path.replace("/", "\\"));
    }
    else
    {
        settings->remove(application_name);
    }
    delete settings;
}

void MainTray::updataUserInfo(QByteArray id, QByteArray pass)
{
    netctrl->setUsername(id);
    netctrl->setPassword(pass);
    opWindow.hide();
    netctrl->sendLoginRequest();
    settings.setValue("id", id);
    settings.setValue("password", pass);
}

void MainTray::handleInfo(QString mb, QString sec, QString balance, QString ip)
{
    QString mbString = QString::number(mb.toDouble()/1000000.0, 'f', 2);
    int totalSec = sec.toInt();
    int hour = (totalSec/3600);
    int min = ((totalSec-hour*3600)/60);
    QString second = QString::number(totalSec-hour*3600-min*60);
    mbAction->setText(tr("已用流量:\t") + mbString + tr(" M"));
    timeAction->setText(tr("已用时长:\t") + QString::number(hour) + ":" +QString::number(min) + ":" + second);
    balanceAction->setText(tr("账户余额:\t") + balance);
    ipAction->setText(tr("IP地址:\t") + ip);
}

