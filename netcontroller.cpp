#include "netcontroller.h"

NetController::NetController(QByteArray id, QByteArray passwd, QObject *parent) : QObject(parent),
                                                                                  username(id), password(passwd), manager(this)
{
    checkParam.append("action=get_online_info");

    desktopRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    desktopRequest.setHeader(QNetworkRequest::ContentLengthHeader, "100");
    desktopRequest.setRawHeader("Host", "ipgw.neu.edu.cn");
    desktopRequest.setRawHeader("Cache-Control", "max-age=0");
    desktopRequest.setRawHeader("Origin", "http://ipgw.neu.edu.cn");
    desktopRequest.setRawHeader("Upgrade-Insecure-Requests", "1");
#ifdef Q_OS_LINUX
    desktopRequest.setRawHeader("User-Agent", "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/69.0.3497.100 Safari/537.36");
#endif

#ifdef Q_OS_WIN32
    desktopRequest.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/65.0.3325.162 Safari/537.36");
#endif
    desktopRequest.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8");
    desktopRequest.setRawHeader("Referer", "http://ipgw.neu.edu.cn/srun_portal_pc.php?ac_id=1&");
    desktopRequest.setRawHeader("Accept-Encoding", "gzip, deflate");
    desktopRequest.setRawHeader("Accept-Language", "zh-CN,zh;q=0.9");

    mobileRequest.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8");
    mobileRequest.setRawHeader("Accept-Encoding", "gzip, deflate");
    mobileRequest.setRawHeader("Accept-Language", "en-US,en;q=0.9,zh-CN;q=0.8,zh;q=0.7");
    mobileRequest.setRawHeader("Cache-Control", "max-age=0");
    //    mobileRequest.setHeader(QNetworkRequest::ContentLengthHeader, "85"); //XXX:这行一定不能有
    mobileRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    mobileRequest.setRawHeader("Host", "ipgw.neu.edu.cn");
    mobileRequest.setRawHeader("Origin", "http://ipgw.neu.edu.cn");
    mobileRequest.setRawHeader("Proxy-Connection", "keep-alive");
    mobileRequest.setRawHeader("Referer", "http://ipgw.neu.edu.cn/srun_portal_phone.php?ac_id=1&");
    mobileRequest.setRawHeader("Upgrade-Insecure-Requests", "1");
    mobileRequest.setRawHeader("User-Agent", "Mozilla/5.0 (Linux; Android 6.0; Nexus 5 Build/MRA58N) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/67.0.3396.99 Mobile Safari/537.36");

    connect(&manager, &QNetworkAccessManager::finished, this, &NetController::handleResponse);
}

void NetController::checkState()
{
    sendCheckRequest();
}

void NetController::sendCheckRequest()
{
    desktopRequest.setUrl(QUrl(checkUrl));
    manager.post(desktopRequest, checkParam);
}

void NetController::sendLoginRequest()
{
    QByteArray loginParam;
    loginParam.append("action=login&");
    loginParam.append("nas_ip=&");
    loginParam.append("password=");
    loginParam.append(password);
    loginParam.append("&save_me=0&");
    loginParam.append("url=&");
    loginParam.append("user_ip=&");
    loginParam.append("user_mac=&");
    loginParam.append("username=");
    loginParam.append(username);

    if (!isMobile)
    {
        desktopRequest.setUrl(QUrl(desktopLoginUrl));
        manager.post(desktopRequest, loginParam);
    }
    else
    {
        mobileRequest.setUrl(QUrl(mobileLoginUrl));
        manager.post(mobileRequest, loginParam);
    }
}

void NetController::sendLogoutRequest(bool isAll)
{
    QByteArray logoutParam;
    logoutParam.append("action=logout&");
    logoutParam.append("ajax=1&");
    if (isAll)
    {
        logoutParam.append("password="); //有password是全部断开
        logoutParam.append(password);
        logoutParam.append("&");
    }
    logoutParam.append("username=");
    logoutParam.append(username);
    if (!isMobile)
    {
        desktopRequest.setUrl(QUrl(desktopLoginUrl));
        manager.post(desktopRequest, logoutParam);
    }
    else
    {
        mobileRequest.setUrl(QUrl(mobileLoginUrl));
        manager.post(mobileRequest, logoutParam);
    }
}

void NetController::handleResponse(QNetworkReply *reply)
{
    QString url = reply->url().toString();
    reply->deleteLater();
    if (url == checkUrl)
    {
        if (reply->error() == QNetworkReply::NoError)
        {
            QString status = reply->readAll();
            if (status == offlineString)
            {
                emit sendState(Offline);
            }
            else
            {
                emit sendState(Online);
                QStringList infoList = status.split(',');
                emit sendInfo(infoList[0], infoList[1], infoList[2], infoList[5]);
            }
        }
        else
        {
            emit sendState(Disconnected);
        }
    }
    else if (url == desktopLoginUrl)
    {
        QString loginPage(reply->readAll());
        if (loginPage.contains(QString("已欠费")))
        { //欠费
            emit sendState(Owed);
        }
        else if (loginPage.contains("Password is error"))
        { //密码错误
            emit sendState(WrongPass);
        }
        else if (loginPage.contains("网络已连接"))
        { //登陆成功
            emit sendState(Online);
        }
    }
    //    else if (url == mobileLoginUrl)
    //    {
    //        qDebug()<<reply->readAll().toStdString();
    //    }
}
