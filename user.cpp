#include "user.h"
#include "mime.h"
#include "searchwidget.h"
#include <QObject>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QFileDialog>
#include <QDir>

User::User(QObject *parent) : QObject(parent)
{
    setConnection();
}

User::User(UserInfo &userInfo, QSqlDatabase &db, QObject *parent) :QObject(parent)
{
    this->setUserInfo(userInfo);
    this->setDatabase(db);
    this->pop = new POP(this);
    this->smtp = new SMTP(this);
    setConnection();
}

User::User(User &user) : QObject(user.parent())
{
    this->setUserInfo(user.getUserInfo());
    this->setDatabase(user.getDatabase());
    setConnection();
}

User::~User()
{
}

void User::setConnection()
{
    connect(this->pop,SIGNAL(statFinish(uint,qint64)),this,SLOT(slotRefreshMailList_stat(uint)));
    connect(this->pop,SIGNAL(uidlFinish(int,QByteArray)),this,SLOT(slotRefreshMailList_uidl(int,QByteArray)));
    connect(this->pop,SIGNAL(topFinish(int,QByteArray)),this,SLOT(slotRefreshMailList_top(int,QByteArray)));
    connect(this->pop,SIGNAL(down(bool)),this,SLOT(slotRefreshMailList_down()));
    connect(this->pop,SIGNAL(retrFinish(int,QByteArray)),this,SLOT(slotRetr(int,QByteArray)));
}

void User::setUserInfo(UserInfo &userInfo)
{
    this->userInfo = userInfo;
}

void User::setDatabase(QSqlDatabase &db)
{
    this->database = db;
    this->dirModel = new QSqlTableModel(this,db);
    QString dirTableName = QString("Dir_%1").arg(QString::number(this->userInfo.id));
    this->dirModel->setTable(dirTableName);
    this->dirModel->select();
    this->mailModel = new MailModel(this,db);
    QString mailTableName = QString("Mail_%1").arg(QString::number(this->userInfo.id));
    this->mailModel->setTable(mailTableName);
    this->mailModel->setDir("收件箱");
    this->mailModel->select();
}

QAbstractItemModel *User::getDirModel()
{
    return this->dirModel;
}

QAbstractItemModel *User::getMailModel()
{
    return this->mailModel;
}

void User::refreshMailList()
{
    this->POPLogin();
    this->pop->stat();
}

QStringList User::getContactList()
{
    return this->mailModel->getContactList();
}

void User::search(QString &keyword)
{

}

void User::slotRefreshMailList_stat(uint mailCount)
{
    for(uint i = 1;i <= mailCount; i++)
    {
        this->pop->uidl(i);
    }
}

void User::slotRefreshMailList_uidl(int mailIndex, QByteArray uid)
{
    bool isExist = this->mailModel->mailExist(uid);
    if(isExist)
    {
        //如果该邮件已经存在于数据库中，更新index，以便于下载
        this->mailModel->updateId(uid,mailIndex);
    }
    else
    {
        //不在，保存uid和index键值对，然后在top中插入数据库，
        this->mailUIDIndex.insert(mailIndex,QString(uid));
        this->pop->top(mailIndex,0);
    }
}

void User::slotRefreshMailList_top(int mailIndex, QByteArray head)
{
    MailPraser praser(head);
    QString uid = this->mailUIDIndex.value(mailIndex);
    QString senderName = praser.getSenderName();
    QString senderMail = praser.getSenderMail();
    QDateTime sendDate = praser.getDateTime();
    QString subject = praser.getSubject();
    this->mailModel->insert(uid,mailIndex,sendDate.toTime_t(),senderName,senderMail,subject);
}

void User::slotRefreshMailList_down()
{
    this->mailModel->select();
}

void User::slotSendDown()
{
    QString mail = this->mailEditWidget->getMail();
    MailPraser praser(mail.toUtf8());
    QString uid = QString("Send") + QTime::currentTime().toString();
    int mailIndex = 0;//发送的邮件索引为0;
    QString senderName = this->userInfo.name;
    QString senderMail = this->userInfo.mailAddr;
    QDateTime sendDate = praser.getDateTime();
    QString subject = praser.getSubject();
    writeMail(mail,uid);
    this->mailModel->insert(uid,mailIndex,sendDate.toTime_t(),senderName,senderMail,subject,"发件箱");
    this->mailModel->select();
}

void User::slotRetr(int mailIndex, QByteArray mail)
{
    MailPraser praser(mail);
    MailShowWidget *mailShowWidget = new MailShowWidget();
    mailShowWidget->setPraser(praser);
    mailShowWidget->show();
    QString uid = this->mailModel->getUID(mailIndex);
    writeMail(mail,uid);
}

void User::changeDir(const QString &dir)
{
    this->mailModel->setDir(dir);
    this->mailModel->select();
}

void User::readMail(const QModelIndex &index)
{
    //先从本地读取，找不到再从网上下载
    QString uid = index.data(MailModel::UIDRole).toString();
    QString filePath = "sMailRepertory/" + this->userInfo.mailAddr + "/" + uid + ".eml";
    QFile file(filePath);
    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        QByteArray mail = file.readAll();
        file.close();
        MailPraser praser(mail);
        MailShowWidget *mailShowWidget = new MailShowWidget();
        mailShowWidget->setPraser(praser);
        mailShowWidget->show();
    }
    else
    {
        int mailIndex = index.data(MailModel::MIndexRole).toInt();
        POPLogin();
        this->pop->retr(mailIndex);
    }
}

void User::sendMail()
{
    this->mailEditWidget = new MailEditWidget(this);
    connect(this->mailEditWidget,SIGNAL(sendDown()),this,SLOT(slotSendDown()));
    mailEditWidget->show();
}

void User::POPLogin()
{
    this->pop->setDebugMode(true);
    this->pop->connectToServer(this->userInfo.POPServer,this->userInfo.POPPort);
    this->pop->user(this->userInfo.mailAddr);
    this->pop->pass(this->userInfo.mailPasswd);
}

void User::writeMail(const QString &mail, const QString &fileName)
{
    QString filePath = "sMailRepertory/" + this->userInfo.mailAddr + "/";
    QDir dir;
    dir.mkpath(filePath);
    QFile file(filePath + fileName + ".eml");
    file.open(QIODevice::WriteOnly);
    file.write(mail.toUtf8());
    file.close();
}
