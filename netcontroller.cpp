#include "netcontroller.h"

NetController::NetController(QByteArray id, QByteArray passwd, QObject *parent) : QObject(parent),
            username(id), password(passwd), NEUState(Unknown), lastState(Unknown),
            manager(this)
{    
    checkParam.append("action=get_online_info");

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setHeader(QNetworkRequest::ContentLengthHeader, "100");
    request.setRawHeader("Host", "ipgw.neu.edu.cn");
    request.setRawHeader("Cache-Control", "max-age=0");
    request.setRawHeader("Origin", "http://ipgw.neu.edu.cn");
    request.setRawHeader("Upgrade-Insecure-Requests", "1");
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/65.0.3325.162 Safari/537.36");
    request.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8");
    request.setRawHeader("Referer", "http://ipgw.neu.edu.cn/srun_portal_pc.php?ac_id=1&");
    request.setRawHeader("Accept-Encoding", "gzip, deflate");
    request.setRawHeader("Accept-Language", "zh-CN,zh;q=0.9");

    connect(&manager, QNetworkAccessManager::finished, this, handleResponse);
}

void NetController::checkState()
{
    sendCheckRequest();
    if(NEUState == Offline && lastState != Offline/* && offlineCount++ > 1*/)
    {
        lastState = Offline;
        emit stateChanged(NEUState);
    }
    else if(NEUState == Online && lastState != Online)
    {
        lastState = Online;
        emit stateChanged(NEUState);
    }
    else if(NEUState == Disconnected && lastState != Disconnected)
    {
        lastState = Disconnected;
        emit stateChanged(NEUState);
    }
    else if(NEUState == WrongPass && lastState != WrongPass)
    {
        lastState = WrongPass;
        emit stateChanged(NEUState);
    }
    else if(NEUState == Owed && lastState != Owed)
    {
        lastState = Owed;
        emit stateChanged(NEUState);
    }
}

void NetController::sendCheckRequest()
{
    request.setUrl(QUrl(checkUrl));
    manager.post(request, checkParam);
}

void NetController::sendLoginRequest()
{
    request.setUrl(QUrl(loginUrl));

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

    manager.post(request, loginParam);
}

void NetController::sendLogoutRequest()
{
    request.setUrl(QUrl(loginUrl));

    QByteArray logoutParam;
    logoutParam.append("action=logout&");
    logoutParam.append("ajax=1&");
    logoutParam.append("password=");
    logoutParam.append(password);
    logoutParam.append("&username=");
    logoutParam.append(username);

    manager.post(request, logoutParam);
}

void NetController::handleResponse(QNetworkReply *reply)
{
    //    qDebug()<<reply->url();
    QString url = reply->url().toString();
    reply->deleteLater();
    if(url==checkUrl)
    {
        if(reply->error()==QNetworkReply::NoError)
        {
            QString status = reply->readAll();
            if(status==offlineString)
            {
                if(NEUState!=WrongPass && NEUState!=Owed && offlineCount++ > 1)
                {
                    offlineCount = 0;
                    lastState = NEUState;
                    NEUState = Offline;
                }
            }
            else
            {
                offlineCount = 0;
                lastState = NEUState;
                NEUState = Online;
                QStringList infoList = status.split(',');
                emit sendInfo(infoList[0], infoList[1], infoList[2], infoList[5]);
            }
        }
        else
        {
            offlineCount = 0;
            lastState = NEUState;
            NEUState = Disconnected;
        }
    }
    else if(url==loginUrl)
    {
        QString loginPage(reply->readAll());
        if(loginPage.contains(QString("已欠费"))){//欠费
            lastState = NEUState;
            NEUState = Owed;
        }
        else if(loginPage.contains("Password is error")){//密码错误
            lastState = NEUState;
            NEUState = WrongPass;
        }
        else if(loginPage.contains("网络已连接")){//登陆成功
            lastState = NEUState;
            NEUState = Online;
        }
    }
}
