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
  void sendLogoutRequest(bool isAll = false);

  inline void setUsername(QByteArray newName) { username = newName; }
  inline void setPassword(QByteArray newPass) { password = newPass; }
  inline void setTotalTraffic(int newTraffic) { traffic = newTraffic; }
  inline void setMobile(bool isMobile) { this->isMobile = isMobile; }
  inline bool getMobile() { return this->isMobile; }

  enum State
  {
    Unknown,
    Offline,
    Online,
    Disconnected,
    Owed,
    WrongPass
  };

private:
  QString checkUrl = "http://ipgw.neu.edu.cn/include/auth_action.php";
  QString desktopLoginUrl = "http://ipgw.neu.edu.cn/srun_portal_pc.php?ac_id=1&";
  QString mobileLoginUrl = "http://ipgw.neu.edu.cn/srun_portal_phone.php?ac_id=1&";
  QString offlineString = "not_online";
  QByteArray checkParam, username, password;
  int traffic;
  bool isMobile;
  int onlineCount = 0, offlineCount = 0, disconnectCount = 0;

  QNetworkAccessManager manager;
  QNetworkRequest desktopRequest, mobileRequest;

signals:
  void sendState(State newState);
  void sendInfo(QString mb, QString sec, QString balance, QString ip);

public slots:
  void handleResponse(QNetworkReply *reply);
};

#endif // NETCONTROLLER_H
