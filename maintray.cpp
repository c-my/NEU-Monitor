#include "maintray.h"

MainTray::MainTray(QObject *parent) : QSystemTrayIcon(parent),
                                      //初始化menu
                                      menu(new QMenu()),
                                      infoMenu(new QMenu()),
                                      settingsMenu(new QMenu()),
                                      //初始化settings
                                      settings(QSettings::IniFormat, QSettings::UserScope, "Cai.MY", "NEU-Monitor"),
                                      user(settings.value("id", "").toByteArray()),
                                      passwd(QByteArray::fromBase64(settings.value("password", "").toByteArray())),
                                      totalTraffic(settings.value("total traffic", 0).toInt()),
                                      isMobile(settings.value("isMobile", false).toBool()),
                                      forceLogin(settings.value("forceLogin", false).toBool()),
                                      opWindow(user, passwd, totalTraffic),
                                      logFileName("NEU_Monitor.log"),
                                      logFile(logFileName, this)
{
    openLogFile();

    opWindow.hide();
    setIcon(QIcon(olIconPath));

    netctrl = new NetController(user, passwd, this); //passwd为解密后密码
    connect(netctrl, &NetController::sendLog, this, &MainTray::writeLog);
    netctrl->setUsername(user);
    netctrl->setPassword(passwd);
    netctrl->setTotalTraffic(totalTraffic);
    netctrl->setMobile(isMobile);

    writeLog(tr("Load username[") + user + tr("] traffic[") + QString::number(totalTraffic) + tr("]."));

    connect(&opWindow, &OptionsWindow::saveSettings, this, &MainTray::updateUserInfo);
    //连接controller状态改变信号
    connect(netctrl, &NetController::sendState, this, &MainTray::handleState);

    connect(this, &QSystemTrayIcon::messageClicked, this, [this]() {
        if (currentState == NetController::WrongPass)
            opWindow.show();
    });

    connect(netctrl, &NetController::sendInfo, this, &MainTray::handleInfo);

    //菜单Action
    loginAction = new QAction(tr("连接网络"), this);
    logoutAction = new QAction(tr("断开网络"), this);
    autoLogin = new QAction(tr("自动登陆"), this);
    optionsAction = new QAction(tr("选项"), this);
    bootAction = new QAction(tr("开机启动"), this);
    muteAction = new QAction(tr("勿扰模式"), this);
    aboutAction = new QAction(tr("关于"), this);
    quitAction = new QAction(tr("退出"), this);

    mbAction = new QAction(tr("已用流量: "), this);
    timeAction = new QAction(tr("已用时长: "), this);
    balanceAction = new QAction(tr("账户余额: "), this);
    ipAction = new QAction(tr("IP地址: "), this);

    mobileAction = new QAction(tr("移动端登陆"), this);
    mobileAction->setCheckable(true);
    mobileAction->setToolTip(tr("假装自己是个手机"));

    forceLoginAction = new QAction(tr("强制登陆"), this);
    forceLoginAction->setCheckable(true);
    forceLoginAction->setToolTip(tr("先断开已有的所有连接再登陆"));

    connect(ipAction, &QAction::triggered, this, [this]() {
        QStringList ipList = ipAction->text().split('\t');
        if (ipList.size() > 1)
        {
            QClipboard *clipBoard = QApplication::clipboard();
            clipBoard->setText(ipList.at(1));
        }
        if (!muteAction->isChecked())
        {
            showMessage(tr("复制成功"), tr("IP地址已复制到剪切板"));
        }
    });

    autoLogin->setCheckable(true);
    bootAction->setCheckable(true);
    muteAction->setCheckable(true);
    muteAction->setToolTip(tr("勿扰模式下不会发出通知"));

    connect(loginAction, &QAction::triggered, this, [this]() {
        writeLog(tr("Login triggered."));
        isForceLogin = true;
        //        netctrl->sendLogoutRequest(true);
        if (forceLogin) {
            logoutAction->trigger();
        }
        hasWarned = false;
        netctrl->sendLoginRequest();
        isForceLogout = false;
    });
    connect(logoutAction, &QAction::triggered, this, [this]() {
        writeLog(tr("Logout triggered."));
        netctrl->sendLogoutRequest(currentState != NetController::Online); //若不在线则断开全部链接
        isForceLogout = true;
    });
    connect(autoLogin, &QAction::toggled, this, [this](bool set) {
        writeLog(tr("Autologin turn ") + (set ? tr("[on].") : tr("[off].")));
        settings.setValue("isAutoLogin", set);
        isForceLogout = false;
        showToolTip(currentState);
    });
    connect(muteAction, &QAction::toggled, this, [this](bool set) {
        writeLog(tr("Mute mode turn ") + (set ? tr("[on].") : tr("[off].")));
        settings.setValue("isMute", set);
    });
    connect(bootAction, &QAction::toggled, this, [this](bool set) {
        writeLog(tr("Boot with system turn ") + (set ? tr("[on].") : tr("[off].")));
        settings.setValue("isOnBoot", set);
        setAutoStart(set);
    });
    connect(optionsAction, &QAction::triggered, this, [this]() { showOptions(); });
    connect(aboutAction, &QAction::triggered, this, &MainTray::showAbout);
    connect(quitAction, &QAction::triggered, this, [this]() {
        writeLog(tr("Exit.\n\n"));
        emit exit();
    });

    connect(mobileAction, &QAction::triggered, this, [this](bool set) {
        writeLog(tr("Mobile model turn ") + tr(set? "[on].": "[off]."));
        settings.setValue("isMobile", set);
        isMobile = set;
        netctrl->setMobile(set);
    });

    connect(forceLoginAction, &QAction::triggered, this, [this](bool set) {
        writeLog(tr("Force Login turn ") + tr(set? "[on].": "[off]."));
        settings.setValue("forceLogin", set);
        forceLogin = set;
    });

    autoLogin->setChecked(settings.value("isAutoLogin", true).toBool());
    bootAction->setChecked(settings.value("isOnBoot", false).toBool());
    muteAction->setChecked(settings.value("isMute", false).toBool());
    mobileAction->setChecked(settings.value("isMobile", false).toBool());

    settingsMenu->setTitle(tr("设置"));
    settingsMenu->addAction(optionsAction);
    settingsMenu->addAction(muteAction);
    settingsMenu->addAction(bootAction);
    settingsMenu->addAction(mobileAction);
    settingsMenu->addAction(forceLoginAction);

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

    connect(this, &MainTray::activated, this, &MainTray::handleActivated);
    connect(this, &MainTray::exit, this, [this]() { setVisible(false); });

    autoLoginTimer = new QTimer(this);
    autoLoginTimer->setInterval(checkInterval);
    autoLoginTimer->start();
    connect(autoLoginTimer, &QTimer::timeout, this, [this]() {
        netctrl->checkState();
    });

    showToolTip(currentState);
    if (settings.value("id").isNull() || settings.value("password").isNull())
    {
        opWindow.show();
    }
}

MainTray::~MainTray()
{
    delete menu;
    delete infoMenu;
    delete autoLoginTimer;
    logFile.close();
}

void MainTray::showToolTip(NetController::State state)
{
    QString tooltipString;
    switch (state)
    {
    case NetController::Online:
        tooltipString += tr("当前状态：在线");
        break;
    case NetController::Offline:
        tooltipString += tr("当前状态：断开");
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
    }
    tooltipString += tr("(") + (netctrl->getMobile() ? tr("Mobile") : tr("PC")) + tr(")");
    tooltipString += tr("\n自动登陆：");
    tooltipString += autoLogin->isChecked() ? tr("开启") : tr("关闭");
    if (state == NetController::Online)
    {
        switch (trafficstate)
        {
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
    switch (reason)
    {
    case QSystemTrayIcon::DoubleClick:
    case QSystemTrayIcon::MiddleClick:
        writeLog(tr("Double click TrayIcon."));
        if (currentState == NetController::Offline)
        {
            loginAction->trigger();
        }
        else if (currentState == NetController::Online)
        {
            logoutAction->trigger();
        }
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
    writeLog(tr("Open options window."));
    opWindow.show();
}

void MainTray::showAbout()
{
    writeLog(tr("Open about window."));
    QMessageBox *aboutWindow = new QMessageBox();
    aboutWindow->setStandardButtons(QMessageBox::Ok);
    aboutWindow->setText(tr("<h1>NEU-Monitor</h1>"
                            "<h3>Version: 1.4.1</h3>"
                            "<p>Based on Qt 5.11.1 (MinGW 5.3.0, 32bit)</p>"
                            "Source Code: <a href=\"https://github.com/c-my/NEU-Monitor\">https://github.com/c-my/NEU-Monitor</a><br/>"
                            "Email: <address>"
                            "<a href=\"mailto:cmy1113@yeah.net?subject=Neu-Monitor-v1.4.1 Feedback\">Cai.MY</a>"
                            "</address>"));
    aboutWindow->setAttribute(Qt::WA_DeleteOnClose);
    aboutWindow->setModal(false);
    aboutWindow->show();
}

void MainTray::setAutoStart(bool set)
{
#ifdef Q_OS_WIN32
    QString application_name = QApplication::applicationName();
    QSettings *settings = new QSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    if (set)
    {
        QString application_path = QApplication::applicationFilePath();
        settings->setValue(application_name, application_path.replace("/", "\\"));
    }
    else
    {
        settings->remove(application_name);
    }
    delete settings;
#endif

#ifdef Q_OS_MACOS
#endif

#ifdef Q_OS_LINUX
    QString path = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.config/autostart";
    QFile file(path + "/ipgw.desktop");
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        if (!muteAction->isChecked())
        {
            showMessage(tr("设置失败"), tr("读写/home/user/.config/autostart/失败"), this->icon(), msgDur);
            return;
        }
    }
    if (set)
    {
        QTextStream in(&file);
        in << "[Desktop Entry]\n"
           << "Type=Application\n"
           << "Version=1.0\n"
           << "Name=NEU-Monitor\n"
           << "Comment=NEU-Monitor startup script\n"
           << "Exec=" << qApp->applicationFilePath() << " --hidden\n"
           << "StartupNotify=false\n"
           << "Terminal=false\n";
        file.close();
    }
    else
    {
        file.remove();
    }
#endif
}

void MainTray::openLogFile()
{
    if (!logFile.open(QIODevice::Append | QIODevice::Text))
    {
        if (!muteAction->isChecked())
        {
            showMessage(tr("警告"), tr("日志文件打开失败"));
        }
        //        return;
    }
    writeLog(tr("\n=================================================="), false);
    writeLog(tr("Open logfile [") + logFile.fileName() + tr("] successfully."));
    logFile.close();
}

void MainTray::updateUserInfo(QByteArray id, QByteArray pass, int traffic) //pass为未加密的密码
{
    writeLog(tr("Set username[") + id + tr("]; Traffic: [") + QString::number(traffic) + tr("]."));
    logoutAction->trigger(); //注销当前via old info
    netctrl->setUsername(id);
    netctrl->setPassword(pass);
    netctrl->setTotalTraffic(traffic);
    netctrl->setMobile(isMobile);
    user = id;
    passwd = pass;
    totalTraffic = traffic;
    opWindow.hide();
    loginAction->trigger();
    hasWarned = false;
    settings.setValue("id", id);
    settings.setValue("password", pass.toBase64());
    settings.setValue("total traffic", traffic);
}

void MainTray::handleState(NetController::State state)
{
    if (state != currentState)
    {
        switch (state)
        {
        case NetController::Online:
            writeLog(tr("Old state: [") + QString::number(currentState) + tr("]; New state: [") + QString::number(state) + tr("]."));
            isForceLogin = false;
            setIcon(QIcon(olIconPath));
            if (!muteAction->isChecked())
                showMessage(tr("网络已连接"), tr("校园网登陆成功"), this->icon(), msgDur);
            currentState = state;
            break;
        case NetController::Offline:
            if (currentState == NetController::WrongPass || currentState == NetController::Owed)
                return;
            writeLog(tr("Old state: [") + QString::number(currentState) + tr("]; New state: [") + QString::number(state) + tr("]."));
            setIcon(QIcon(offIconPath));
            if (!muteAction->isChecked())
                showMessage(tr("网络已断开"), tr("校园网已注销"), this->icon(), msgDur);
            //自动登陆
            if (autoLogin->isChecked() && !isForceLogout)
            {
                QTimer::singleShot(1000, loginAction, SLOT(trigger()));
                //                loginAction->trigger();
            }
            currentState = state;
            break;
        case NetController::Disconnected:
            writeLog(tr("Old state: [") + QString::number(currentState) + tr("]; New state: [") + QString::number(state) + tr("]."));
            setIcon(QIcon(offIconPath));
            if (!muteAction->isChecked())
                showMessage(tr("网络已断开"), tr("无法连接至校园网"), this->icon(), msgDur);
            currentState = state;
            break;
        case NetController::WrongPass:
            writeLog(tr("Old state: [") + QString::number(currentState) + tr("]; New state: [") + QString::number(state) + tr("]."));
            setIcon(QIcon(offIconPath));
            if (!muteAction->isChecked())
                showMessage(tr("登陆失败"), tr("密码错误"), this->icon(), msgDur);
            currentState = state;
            break;
        case NetController::Owed:
            setIcon(QIcon(offIconPath));
            writeLog(tr("Old state: [") + QString::number(currentState) + tr("]; New state: [") + QString::number(state) + tr("]."));
            if (!muteAction->isChecked())
                showMessage(tr("登陆失败"), tr("已欠费"), this->icon(), msgDur);
            currentState = state;
            break;
        default:
            break;
        }
        writeLog(tr("State change to [") + QString::number(currentState) + tr("]."));
        showToolTip(state);
    }
}

void MainTray::handleInfo(QString byte, QString sec, QString balance, QString ip)
{
    QString mbString = QString::number(byte.toDouble() / 1000000.0, 'f', 2);
    QString gbString = QString::number(byte.toDouble() / 1000000000.0, 'f', 2);
    QString leftoverString = QString::number(totalTraffic - byte.toDouble() / 1000000000.0, 'f', 2);
    double totalSec = sec.toDouble();
    double hour = (totalSec / 3600);
    int min = int((totalSec - hour * 3600) / 60);
    QString second = QString::number(totalSec - hour * 3600 - min * 60);
    mbAction->setText(tr("已用流量:\t") + mbString + tr(" M"));
    timeAction->setText(tr("已用时长:\t") + QString::number(hour, 'f', 2) + tr(" 小时") /*+ ":" +QString::number(min) + ":" + second*/);
    balanceAction->setText(tr("账户余额:\t") + balance + tr(" 元"));
    ipAction->setText(tr("IP地址:\t") + ip);
    if (!hasWarned && totalTraffic > 0)
    {
        hasWarned = true;
        if (byte.toDouble() / 1000000.0 > totalTraffic * 1024)
        { //流量已超
            trafficstate = Over;
            if (!muteAction->isChecked())
                QTimer::singleShot(5000, this, [this] {
                    showMessage(tr("流量警告"), tr("本月流量已超"), QSystemTrayIcon::Warning);
                });
        }
        else if (byte.toDouble() / 1000000.0 + 5000 > totalTraffic * 1024)
        { //流量将超
            trafficstate = Nearly;
            if (!muteAction->isChecked())
                QTimer::singleShot(5000, this, [this, leftoverString] {
                    showMessage(tr("流量预警"), tr("剩余流量：") + leftoverString + tr("G"));
                });
        }
        showToolTip(currentState);
    }
}

void MainTray::writeLog(QString content, bool timeStamp)
{
    if (!logFile.isOpen())
        logFile.open(QIODevice::Append | QIODevice::Text);
    if (timeStamp)
        logFile.write(QDateTime::currentDateTime().toString("[yyyy.MM.dd hh:mm:ss] ").toUtf8());
    logFile.write(content.toUtf8());
    logFile.write(tr("\n").toUtf8());
    //        qDebug()<<"Write log"<<content;
    logFile.close();
}
