#ifndef SMTP_H

#define SMTP_H

#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QStringList>
#include <QDebug>
#include "mime.h"

class SMTP : public QObject
{
    Q_OBJECT
public:
    explicit SMTP(QObject *parent = nullptr);
    void setDebugMode(bool debug);
    bool connectServer(const QString &smtpServer, int port = 25, bool ssl = false);
    bool login(const QString &username,const QString &password);
    bool sendMail(const QString &sender, const QStringList &receivers, const QByteArray &msg);
    bool quit();
    QString errorInfo() const;
signals:
    void error();
public slots:

private:
    //smtp返回码
    const int SERVICE_READY = 220;
    const int SERVICE_CLOSE = 221;
    const int LOGIN_SUCCESS = 235;

    const int REQUEST_OK = 250;
    const int WAIT_INPUT = 334;
    const int START_MAIL_INPUT = 354;
    const int PARA_ERR = 501;
    const int COMMEND_NOT_FIND = 502;
    const int AUTH_FAILED = 535;

    bool debugMode;
    QTcpSocket tcpSocket;
    QString lastInfoFromServer;
    int SMTPRead();
    void SMTPWrite(const QByteArray block);

};

#endif // SMTP_H
