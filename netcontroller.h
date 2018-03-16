#ifndef NETCONTROLLER_H
#define NETCONTROLLER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>

class NetController : public QObject
{
    Q_OBJECT
public:
    explicit NetController(QByteArray username, QByteArray password, QObject *parent = nullptr);
    void checkStatus();
    void sendCheckRequest();
    void sendLoginRequest();
    void sendLogoutRequest();

    enum Status {Offline, Online, Disconnected};

private:
    QString checkUrl = "http://ipgw.neu.edu.cn/include/auth_action.php";
    QString loginUrl = "http://ipgw.neu.edu.cn/srun_portal_pc.php?ac_id=1&";
    QString offlineString = "not_online";
    QByteArray checkParam, loginParam, logoutParam,  username, password;
    Status NEUStatus, flag;

    QNetworkAccessManager manager;
    QNetworkRequest request;
    QTimer *checkTimer;

signals:
    void getOnline();
    void getOffline();
    void getDisconnected();

public slots:
    void handleResponse(QNetworkReply *reply);
};

#endif // NETCONTROLLER_H
