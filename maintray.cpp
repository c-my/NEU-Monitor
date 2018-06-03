#include "maintray.h"

MainTray::MainTray(QByteArray username, QByteArray password, QObject *parent): QSystemTrayIcon(parent),
    //初始化menu
    menu(new QMenu()),
    infoMenu(new QMenu()),
    settingsMenu(new QMenu()),
    //初始化settings
    settings(QSettings::IniFormat, QSettings::UserScope, "Cai.MY", "NEU-Monitor"),
    //初始化optionswindow
    opWindow(settings.value("id", "").toByteArray(), settings.value("password", "").toByteArray(), settings.value("total traffic", 60).toInt()),
    user(username),
    passwd(password)
{
    opWindow.hide();
    setIcon(QIcon(tr(":/icon/favicon.ico")));

    netctrl = new NetController(user,passwd,this);
    user = settings.value("id",0).toByteArray();
    passwd = settings.value("password", 0).toByteArray();
    totalTraffic = settings.value("total traffic", 60).toInt();
    netctrl->setUsername(user);
    netctrl->setPassword(passwd);
    netctrl->setTotalTraffic(totalTraffic);

    connect(&opWindow, OptionsWindow::saveSettings, this, updateUserInfo);
    //连接controller状态改变信号
    connect(netctrl, NetController::stateChanged, this, [this](NetController::State state){
        switch (state) {
        case NetController::Online:
            isForceLogin = false;
            setIcon(QIcon(tr(":/icon/favicon.ico")));
            if(!muteAction->isChecked())
                showMessage(tr("网络已连接"),tr("校园网登陆成功"), this->icon(), msgDur);
            loginAction->setEnabled(true);
            logoutAction->setEnabled(true);
            break;
        case NetController::Offline:
            if(!isForceLogin){
                setIcon(QIcon(tr(":/icon/offline.ico")));
                if(!muteAction->isChecked())
                    showMessage(tr("网络已断开"),tr("校园网已注销"), this->icon(), msgDur);
                loginAction->setEnabled(true);
                logoutAction->setEnabled(true);
            }
            break;
        case NetController::Disconnected:
            setIcon(QIcon(tr(":/icon/offline.ico")));
            if(!muteAction->isChecked())
                showMessage(tr("网络已断开"),tr("无法连接至校园网"), this->icon(), msgDur);
            loginAction->setDisabled(true);
            logoutAction->setDisabled(true);
            break;
        case NetController::WrongPass:
            setIcon(QIcon(tr(":/icon/offline.ico")));
            if(!muteAction->isChecked())
                showMessage(tr("登陆失败"),tr("密码错误"), this->icon(), msgDur);
            break;
        case NetController::Owed:
            setIcon(QIcon(tr(":/icon/offline.ico")));
            if(!muteAction->isChecked())
                showMessage(tr("登陆失败"),tr("已欠费"), this->icon(), msgDur);
            break;
        default:
            break;
        }
        currentState = state;
        showToolTip(state);
    });


    connect(this, QSystemTrayIcon::messageClicked, this, [this](){
        if(currentState == NetController::WrongPass)
            opWindow.show();
    });

    connect(netctrl, NetController::sendInfo, this, handleInfo);

    //菜单Action
    loginAction = new QAction(tr("连接网络"),this);
    logoutAction = new QAction(tr("断开网络"),this);
    autoLogin = new QAction(tr("自动重连"), this);
    optionsAction = new QAction(tr("选项"), this);
    bootAction = new QAction(tr("开机启动"), this);
    muteAction = new QAction(tr("勿扰模式"), this);
    aboutAction = new QAction(tr("关于"), this);
    quitAction = new QAction(tr("退出"), this);

    mbAction = new QAction(tr("已用流量: "),this);
    timeAction = new QAction(tr("已用时长: "),this);
    balanceAction = new QAction(tr("账户余额: "),this);
    ipAction = new QAction(tr("IP地址: "),this);

    autoLogin->setCheckable(true);
    bootAction->setCheckable(true);
    muteAction->setCheckable(true);

    muteAction->setToolTip(tr("勿扰模式下不会发出通知"));

    connect(loginAction, QAction::triggered, this,[this](){
        isForceLogin = true;
        netctrl->sendLogoutRequest();
        hasWarned = false;
        netctrl->sendLoginRequest();
        isForceLogout = false;
    });
    connect(logoutAction, QAction::triggered, this,[this](){
        netctrl->sendLogoutRequest();
        isForceLogout = true;
    });
    connect(autoLogin, QAction::toggled, this, [this](bool set){
        settings.setValue("isAutoLogin", set);
        isForceLogout = false;
        showToolTip(currentState);
    });
    connect(muteAction, QAction::toggled, this, [this](bool set){
        settings.setValue("isMute", set);
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
    muteAction->setChecked(settings.value("isMute", false).toBool());


    settingsMenu->setTitle(tr("设置"));
    settingsMenu->addAction(optionsAction);
    settingsMenu->addAction(muteAction);
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
    autoLoginTimer->setInterval(checkInterval);
    autoLoginTimer->start();
    connect(autoLoginTimer,QTimer::timeout, this, [this](){
        netctrl->checkState();
        //Offline状态下自动重连
        if(currentState == NetController::Offline && autoLogin->isChecked() && !isForceLogout){
            loginAction->trigger();
        }
    });

    showToolTip(currentState);
    if(settings.value("id").isNull()||settings.value("password").isNull())
    {
        opWindow.show();
    }
}

MainTray::~MainTray()
{
    delete menu;
    delete infoMenu;
}

void MainTray::showToolTip(NetController::State state)
{
    QString tooltipString;
    switch (state) {
    case NetController::Online:
        tooltipString += tr("当前状态：连接");
        break;
    case NetController::Offline:
        tooltipString += tr("当前状态：断开 （双击登陆）");
        break;
    case NetController::Disconnected:
        tooltipString += tr("当前状态：无法连接");
        break;
    case NetController::Unknown:
        tooltipString += tr("当前状态：正在识别");
        break;
    case NetController::Owed:
        tooltipString += tr("当前状态：已欠费");
        break;
    case NetController::WrongPass:
        tooltipString += tr("当前状态：密码错误");
        break;
    default:
        break;
    }
    tooltipString += tr("\n自动重连：");
    tooltipString += autoLogin->isChecked()? tr("开启"):tr("关闭");
    if(state == NetController::Online){
        switch (trafficstate) {
        case Over:
            tooltipString += tr("\n流量已超额");
            break;
        case Nearly:
            tooltipString += tr("\n剩余流量不足");
            break;
        default:
            break;
        }
    }
    setToolTip(tooltipString);
}

void MainTray::handleActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::DoubleClick:
        if(currentState == NetController::Offline)
            loginAction->trigger();
        break;
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
                             "<p>Based on Qt 5.11.0 (MinGW 5.3.0, 32bit)</p>"
                             "Source Code: <a href=\"https://github.com/c-my/NEU-Monitor\">https://github.com/c-my/NEU-Monitor</a><br/>"
                            "Email: <address>"
                            "<a href=\"mailto:cmy1113@yeah.net?subject=SerialAsst Feedback\">Cai.MY</a>"
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

void MainTray::updateUserInfo(QByteArray id, QByteArray pass, int traffic)
{
    netctrl->setUsername(id);
    netctrl->setPassword(pass);
    netctrl->setTotalTraffic(traffic);
    user = id;
    passwd = pass;
    totalTraffic = traffic;
    opWindow.hide();
    netctrl->sendLoginRequest();
    hasWarned = false;
    settings.setValue("id", id);
    settings.setValue("password", pass);
    settings.setValue("total traffic", traffic);
}

void MainTray::handleInfo(QString byte, QString sec, QString balance, QString ip)
{
    QString mbString = QString::number(byte.toDouble()/1000000.0, 'f', 2);
    QString gbString = QString::number(byte.toDouble()/1000000000.0, 'f', 2);
    QString leftoverString = QString::number(totalTraffic - byte.toDouble()/1000000000.0, 'f',2 );
    int totalSec = sec.toInt();
    int hour = (totalSec/3600);
    int min = ((totalSec-hour*3600)/60);
    QString second = QString::number(totalSec-hour*3600-min*60);
    mbAction->setText(tr("已用流量:\t") + mbString + tr(" M"));
    timeAction->setText(tr("已用时长:\t") + QString::number(hour) + ":" +QString::number(min) + ":" + second);
    balanceAction->setText(tr("账户余额:\t") + balance);
    ipAction->setText(tr("IP地址:\t") + ip);
    if(!hasWarned){
        hasWarned = true;
        if(byte.toDouble() / 1000000.0 > totalTraffic * 1024){//流量已超
            trafficstate = Over;
            if(!muteAction->isChecked())
                showMessage(tr("流量警告"), tr("本月流量已超"), QSystemTrayIcon::Warning);
        }
        else if(byte.toDouble() / 1000000.0 + 5000 > totalTraffic * 1024){//流量将超
            trafficstate = Nearly;
            if(!muteAction->isChecked())
                showMessage(tr("流量预警"), tr("剩余流量：") + leftoverString + tr("G"));
        }
        showToolTip(currentState);
    }
}

