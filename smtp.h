#ifndef SMTP_H
#define SMTP_H

#include <QObject>
#include <QTcpSocket>
#include <QSslSocket>
#include <QByteArray>
#include <QStringList>
#include <QList>
#include <QDebug>
#include "mime.h"

class SCommend
{
public:
    SCommend(){}
    SCommend(uint cmd, uint retCode, QByteArray para = QByteArray());
    uint cmdType;
    uint cmdRightReturnCode;
    QByteArray cmdContent;
};

class SMTP : public QObject
{
    Q_OBJECT
public:
    enum SMTPCMD
    {
        OFFLINE,
        CONNECT,
        HELO,
        AUTH_LOGIN,
        _USERNAME,
        _PASSWORD,
        MAIL_FROM,
        RCPT_TO,
        DATA,
        _MAIL,
        QUIT,
    };
    enum SMTPRetCode
    {
        ServiceReady = 220,
        ServiceClose = 221,
        AuthThrough = 235,
        ReqCompleted = 250,
        InputLine = 334,
        InputMulti = 354,
    };
    enum SMTPError
    {
        TCPError,
        RetCodeError,
    };

    explicit SMTP(QObject *parent = nullptr);
    void setDebugMode(bool debug) {this->debugMode = debug;}
    void setServerAddr(const QString &smtpServer, int port = 25, bool ssl = false);
    void setLoginInfo(const QString &userName,const QString &passwd);
    void sendMail(const QList<QString> receiveList, const QByteArray &mail);
    QString& getErrorString(){return errorString;}
    static QByteArray SMTPCMDConvert(int type);
signals:
    void error(SMTPError errorType);
    void down(bool err);
    void sendCmd();
public slots:
    void slotSend();
    void slotReceive();
    void slotConnedted();
    void slotError();
private:
    bool debugMode;
    bool isConnected = false;
    bool isProcessing = false;
    QString smtpServer;
    int port;
    bool ssl;
    QByteArray uName;
    QByteArray uPass;
    QList<SCommend> SMTPCMDList;
    SCommend curCMD;
    QByteArray receiveBuf;
    QTcpSocket *tcpSocket;
    QSslSocket *sslSocket;
    QString errorString;
};

#endif // SMTP_H
