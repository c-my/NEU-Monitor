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

    void setUsername(QByteArray newName) { username = newName; }
    void setPassword(QByteArray newPass) { password = newPass; }

    enum Status {Unknown, Offline, Online, Disconnected};

private:
    QString checkUrl = "http://ipgw.neu.edu.cn/include/auth_action.php";
    QString loginUrl = "http://ipgw.neu.edu.cn/srun_portal_pc.php?ac_id=1&";
    QString offlineString = "not_online";
    QByteArray checkParam, username, password;
    Status NEUStatus = Unknown, flag = Unknown;

    QNetworkAccessManager manager;
    QNetworkRequest request;
    QTimer *checkTimer;
    bool isForceLogout = false;
signals:
    void getOnline();
    void getOffline(bool isForce);
    void getDisconnected();

public slots:
    void handleResponse(QNetworkReply *reply);
};

#endif // NETCONTROLLER_H
