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
    void checkState();
    void sendCheckRequest();
    void sendLoginRequest();
    void sendLogoutRequest();

    void setUsername(QByteArray newName) { username = newName; }
    void setPassword(QByteArray newPass) { password = newPass; }
    void setTotalTraffic(int newTraffic) { traffic = newTraffic; }

    enum State {Unknown, Offline, Online, Disconnected, Owed, WrongPass};

private:
    QString checkUrl = "http://ipgw.neu.edu.cn/include/auth_action.php";
    QString loginUrl = "http://ipgw.neu.edu.cn/srun_portal_pc.php?ac_id=1&";
    QString offlineString = "not_online";
    QByteArray checkParam, username, password;
    int traffic;
//    State NEUState = Unknown, lastState = Unknown;
    int onlineCount = 0, offlineCount = 0, disconnectCount = 0;

    QNetworkAccessManager manager;
    QNetworkRequest request;

signals:
    void sendState(State newState);
    void sendInfo(QString mb, QString sec, QString balance, QString ip);
    void sendLog(QString content, bool timeStamp = true);

public slots:
    void handleResponse(QNetworkReply *reply);
};

#endif // NETCONTROLLER_H
