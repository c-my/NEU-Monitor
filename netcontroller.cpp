#include "netcontroller.h"

NetController::NetController(QByteArray id, QByteArray passwd, QObject *parent) : QObject(parent),
            username(id), password(passwd), NEUStatus(Unknown), flag(Unknown),
            manager(this), checkTimer(new QTimer(this))
{    
    checkParam.append("action=get_online_info");

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    connect(&manager, QNetworkAccessManager::finished, this, handleResponse);

    checkTimer->setInterval(1000);
    connect(checkTimer, QTimer::timeout, this, checkStatus);
    checkTimer->start();
}

void NetController::checkStatus()
{
    sendCheckRequest();
    if(NEUStatus==Offline&&flag!=Offline)
    {
        flag = Offline;
        emit getOffline();
    }
    else if(NEUStatus == Online && flag != Online)
    {
        flag = Online;
        emit getOnline();
    }
    else if(NEUStatus == Disconnected && flag != Disconnected)
    {
        flag = Disconnected;
        emit getDisconnected();
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
                NEUStatus = Offline;
            }
            else
            {
                NEUStatus = Online;
                QStringList infoList = status.split(',');
                emit sendInfo(infoList[0], infoList[1], infoList[2], infoList[5]);
            }
        }
        else
        {
            NEUStatus = Disconnected;
        }
    }
    else if(url==loginUrl)
    {

    }
}
