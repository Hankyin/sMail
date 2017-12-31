#ifndef USER_H
#define USER_H

#include "pop.h"
#include "smtp.h"
#include "mailmodel.h"
#include "maileditwidget.h"
#include <QObject>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QList>
#include <QMap>
#include <QWebView>

class UserInfo
{
public:
    UserInfo() {}
    //当前登录用户信息
    uint id = 0;
    QString name;
    QString mailAddr;
    QString mailPasswd;
    QString SMTPServer;
    int SMTPPort = 25;
    bool SMTPSSL = false;
    QString POPServer;
    int POPPort;
    bool POPSSL;
};

class MailEditWidget;

class User : public QObject
{
    Q_OBJECT
public:
    explicit User(QObject *parent = nullptr);
    User(UserInfo &userInfo,QSqlDatabase &db,QObject *parent = nullptr);
    User(User &user);
    ~User();
    UserInfo& getUserInfo() {return this->userInfo;}
    SMTP* getSMTP() {return this->smtp;}
    void setConnection();
    QSqlDatabase& getDatabase() {return this->database;}
    void setUserInfo(UserInfo &userInfo);
    void setDatabase(QSqlDatabase &db);
    QAbstractItemModel* getDirModel();
    QAbstractItemModel* getMailModel();
    void changeDir(const QString &dir);
    void readMail(const QModelIndex &index);
    void sendMail();
    void refreshMailList();
signals:
    void sendMailDown(bool err);
private slots:
    void slotRefreshMailList_stat(uint mailCount);
    void slotRefreshMailList_uidl(int mailIndex,QByteArray uid);
    void slotRefreshMailList_top(int mailIndex,QByteArray head);
    void slotRefreshMailList_down();
    void slotSendDown();
    void slotRetr(int mailIndex,QByteArray mail);
    void slotOpenExternalLink(const QUrl &url);
private:
    UserInfo userInfo;
    QSqlDatabase database;
    QSqlTableModel *dirModel;
    MailModel *mailModel;
    QMap<int,QString> mailUIDIndex;
    POP *pop;
    SMTP *smtp;
    QList<QByteArray> newUIDList;
    QWebView *webView;
    MailEditWidget *mailEditWidget;
    void POPLogin();
    void writeMail(const QString &mail, const QString &fileName);
};

#endif // USER_H
