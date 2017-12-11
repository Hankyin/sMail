#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QByteArray>
#include <QDebug>
#include <QDateTime>
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QFile>
#include <QFileDialog>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QWebView>
#include <QMessageBox>
#include <QTimer>
#include <QSql>
#include <QSqlQuery>
#include <QSqlError>

#include "maileditwidget.h"
#include "logindialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //构造函数中只处理窗体UI方面的东西，数据处理放在slotAfterShow中。应为无法在构造函数中退出整个程序
    ui->setupUi(this);
    ui->splitter->setStretchFactor(1,1);
    ui->splitter_2->setStretchFactor(1,1);
    ui->webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(ui->btRefresh,SIGNAL(clicked(bool)),this,SLOT(slotRefreshStart()));
    connect(&(this->pop),SIGNAL(statFinish(uint,qint64)),this,SLOT(slotRefreshStat(uint)));
    connect(&(this->pop),SIGNAL(topFinish(int,QByteArray)),this,SLOT(slotRefreshTop(int,QByteArray)));
    connect(&(this->pop),SIGNAL(uidlFinish(int,QByteArray)),this,SLOT(slotRefreshUidl(int,QByteArray)));
    connect(&(this->pop),SIGNAL(down(bool)),this,SLOT(slotPOPDown(bool)));
    connect(&(this->pop),SIGNAL(retrFinish(int,QByteArray)),this,SLOT(slotRetr(int,QByteArray)));
    connect(ui->btWriteMail,SIGNAL(clicked(bool)),this,SLOT(slotWriteMail()));
    connect(ui->listWidget,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(slotReadMail(QListWidgetItem*)));
    connect(ui->webView,SIGNAL(linkClicked(QUrl)),this,SLOT(slotOpenExternalLink(QUrl)));

    QTimer::singleShot(500,this,SLOT(slotAfterShow()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slotWriteMail()
{
    MailEditWidget *mailEdit = new MailEditWidget(this->userInfoList.at(0));
    mailEdit->show();
}

void MainWindow::slotRefreshStat(uint mailCount)
{
    for(int i = 1;i <= mailCount; i++)
    {
        pop.uidl(i);
    }
}

void MainWindow::slotRefreshTop(int mailId,QByteArray head)
{
    MailPraser praser(head);
    QString subject = praser.getSubject();
    QString from = praser.getFrom();
    QDateTime date = praser.getDateTime();
    QSqlQuery query(db);
    query.exec(QString("update inBox%1 set datetime = %2,sender = '%3',subject = '%4' "
                       "where id = %5;").arg(QString::number(this->userInfoList.at(0)->id),
                                             QString::number(date.toTime_t()),from,subject,
                                             QString::number(mailId)));
}

void MainWindow::slotRefreshUidl(int mailId, QByteArray uid)
{
    QSqlQuery query(db);
    query.exec(QString("select count(*) from inBox%1 where uid == '%2'").arg(QString::number(this->userInfoList.at(0)->id),QString(uid)));//这里有歧义？
    while (query.next())
    {
        QSqlQuery updateQuery;
        if(query.value(0) == 1)
        {
            //如果该邮件已经存在于数据库中，更新其id，以便于下载
            updateQuery.exec(QString("update inBox%1 set id = %2 where uid == '%3';").arg(QString::number(this->userInfoList.at(0)->id),
                                                                                             QString::number(mailId),uid));
        }
        else if(query.value(0).toUInt() == 0)
        {
            //数据库中不存在，则插入，并下载头部
            updateQuery.exec(QString("insert into inBox%1 (uid,id,read) values ('%2',%3,0);").arg(QString::number(this->userInfoList.at(0)->id),
                                                                                                      uid,QString::number(mailId)));
            pop.top(mailId,0);
        }
        else
        {
            //其他情况说明邮件多次插入
            qDebug()<<"邮件重复错误"<<endl;
        }
    }
}

void MainWindow::slotRefreshStart()
{
    //由于是异步执行，所以刷新邮件列表要分多步。不知道还有没有神么好办法,这样好蠢啊
    //刷新流程：slotRefreshStart()调用pop.stat()，
    //slotRefreshStat()收到mailClout，调用pop.top, pop emit topFinish（）
    //slotRefreshTop()收到数据，更新界面并保存到数据库
    //与服务器的tcp连接在slotRefreshStart开始，在down信号发射后断开，
    ui->statusBar->showMessage("邮件更新中");
    popLogin();
    pop.stat();
}

void MainWindow::slotReadMail(QListWidgetItem *item)
{
    //先从本地读取，找不到再从网上下载
    UserInfo *u = this->userInfoList.at(0);
    QString uid = item->data(Qt::UserRole).toString();
    //filename = sMailRepertory/yin/aeidssev+de.eml
    QString fileName = "sMailRepertory/" + u->userName +"/" + uid + ".eml";
    QFile file(fileName);
    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        QByteArray mail = file.readAll();
        MailPraser praser(mail);
        ui->webView->setHtml(praser.getHtml());
    }
    else
    {
        uint id = 0;
        QSqlQuery query(this->db);
        query.exec(QString("select id from inBox%1 where uid == '%2';").arg(QString::number(u->id),uid));
        qDebug()<<query.lastError().text();
        while (query.next())
        {
            id = query.value(0).toUInt();
            popLogin();
            pop.retr(id);
        }
    }
}

void MainWindow::slotRetr(int mailId, QByteArray mail)
{
    //把ui和数据处理放在一起似乎是不好的，但该如何修改呢？？
    MailPraser praser(mail);
    ui->webView->setHtml(praser.getHtml());
    UserInfo *u = this->userInfoList.at(0);
    QSqlQuery query(this->db);
    query.exec(QString("select uid from inBox%1 where id == %2;").arg(QString(u->id),QString::number(mailId)));
    while (query.next())
    {
        QString uid = query.value(0).toString();
        QString fileName = "sMailRepertory/" + u->userName +"/" + uid + ".eml";
        QFile file(fileName);
        file.open(QIODevice::WriteOnly);
        file.write(mail);
        file.close();
    }

}

void MainWindow::slotAfterShow()
{
    //打开数据库,什么时候关呢？
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("sMail.db");
    if(!db.open())
    {
        qDebug()<<"db open error"<<endl;
        return ;
    }
    //sqlite会自动维护一个系统表sqlite_master，该表存储了我们所创建的各个table, view, trigger等等信息。
    QSqlQuery query(db);
    query.exec("select count(*) from sqlite_master where type = 'table' and name = 'userList' ;");
    //判断userList是否存在，不存在则创建一个
    int hasUserTable = 999;
    while (query.next())
    {
        hasUserTable = query.value(0).toInt();
    }
    if(!hasUserTable)
    {
        bool ok = query.exec("create table userList ("
                   "id INTEGER PRIMARY KEY ,"
                   "userName text,"
                   "userMailAddr text,"
                   "userMailPasswd text,"
                   "POPServerAddr text,"
                   "POPAccount text,"
                   "POPPasswd text,"
                   "POPPort int,"
                   "POPSSL int,"
                   "SMTPServerAddr text,"
                   "SMTPAccount text,"
                   "SMTPPasswd text,"
                   "SMTPPort int,"
                   "SMTPSSL int);");
    }
    //读取用户信息
    query.exec("select * from userList");
    while (query.next())
    {
        UserInfo *userInfo = new UserInfo();
        userInfo->id = query.value(0).toInt();
        userInfo->userName = query.value(1).toString();
        userInfo->userMailAddr = query.value(2).toString();
        userInfo->userMailPasswd = query.value(3).toString();
        userInfo->POPServerAddr = query.value(4).toString();
        userInfo->POPAccount = query.value(5).toString();
        userInfo->POPPasswd = query.value(6).toString();
        userInfo->POPPort = query.value(7).toInt();
        userInfo->POPSSL = query.value(8).toBool();
        userInfo->SMTPServerAddr = query.value(9).toString();
        userInfo->SMTPAccount = query.value(10).toString();
        userInfo->SMTPPasswd = query.value(11).toString();
        userInfo->SMTPPort = query.value(12).toInt();
        userInfo->SMTPSSL = query.value(13).toBool();
        this->userInfoList.append(userInfo);
        //若要支持多用户这样似乎不行,应当只更新当前用户邮件列表
        //读取保存在本地的邮件列表
        updateListWidget(*userInfo);
    }

    if(this->userInfoList.isEmpty())
    {
        createUser();
    }
    //刷新用户邮件列表
    slotRefreshStart();
}

void MainWindow::slotOpenExternalLink(const QUrl &url)
{
    QDesktopServices::openUrl(url);
}

void MainWindow::slotPOPDown(bool err)
{
    if(pop.getConnectionStatu())
        pop.quit();
    updateListWidget(*(this->userInfoList.at(0)));
    ui->statusBar->showMessage("更新完成",3000);
}

bool MainWindow::createUser()
{
    QSqlQuery query(db);
    LoginDialog login(this);
    UserInfo *userInfo = new UserInfo();
    if(!login.exec())
        return false;
    else
    {
        userInfo->id = QDateTime::currentDateTime().toTime_t();//用时间戳来表示用户id
        userInfo->userName = login.getNewUser().userName;
        userInfo->userMailAddr = login.getNewUser().userMailAddr;
        userInfo->userMailPasswd = login.getNewUser().userMailPasswd;
        userInfo->POPServerAddr = login.getNewUser().POPServerAddr;
        userInfo->POPAccount = login.getNewUser().POPAccount;
        userInfo->POPPasswd = login.getNewUser().POPPasswd;
        userInfo->POPPort = login.getNewUser().POPPort;
        userInfo->POPSSL = login.getNewUser().POPSSL;
        userInfo->SMTPServerAddr = login.getNewUser().SMTPServerAddr;
        userInfo->SMTPAccount = login.getNewUser().SMTPAccount;
        userInfo->SMTPPasswd = login.getNewUser().SMTPPasswd;
        userInfo->SMTPPort = login.getNewUser().SMTPPort;
        userInfo->SMTPSSL = login.getNewUser().SMTPSSL;
        this->userInfoList.append(userInfo);
        QString insertUser = "insert into userList (id,userName,userMailAddr,userMailPasswd,"
                            "POPServerAddr,POPAccount,POPPasswd,POPPort,POPSSL,"
                            "SMTPServerAddr,SMTPAccount,SMTPPasswd,SMTPPort,SMTPSSL)"
                            "values(?,?,?,?,?,?,?,?,?,?,?,?,?,?);";
        query.prepare(insertUser);
        query.addBindValue(userInfo->id);
        query.addBindValue(userInfo->userName);
        query.addBindValue(userInfo->userMailAddr);
        query.addBindValue(userInfo->userMailPasswd);
        query.addBindValue(userInfo->POPServerAddr);
        query.addBindValue(userInfo->POPAccount);
        query.addBindValue(userInfo->POPPasswd);
        query.addBindValue(userInfo->POPPort);
        query.addBindValue(userInfo->POPSSL);
        query.addBindValue(userInfo->SMTPServerAddr);
        query.addBindValue(userInfo->SMTPAccount);
        query.addBindValue(userInfo->SMTPPasswd);
        query.addBindValue(userInfo->SMTPPort);
        query.addBindValue(userInfo->SMTPSSL);
        bool ok = query.exec();
        QString createUserMailListTable;
        createUserMailListTable = QString("create table inBox%1 "
                                          "("
                                          "uid text primary key,"
                                          "id int,"
                                          "datetime int,"
                                          "sender text,"
                                          "subject text,"
                                          "read int"
                                          ");").arg(QString::number(userInfo->id));
        ok = query.exec(createUserMailListTable);
//        createUserMailListTable = QString("create table user_%1_sender("
//                                          "uid integer,"
//                                          "id integer,"
//                                          "date integer,"
//                                          "from text,"
//                                          "subject text,"
//                                          "read integer);").arg(newId);
//        ok = query.exec(createUserMailListTable);
//        createUserMailListTable = QString("create table user_%1_recycle("
//                                          "uid integer,"
//                                          "id integer,"
//                                          "date integer,"
//                                          "from text,"
//                                          "subject text,"
//                                          "read integer);").arg(newId);
//        ok = query.exec(createUserMailListTable);
    }
    return true;
}

void MainWindow::popLogin()
{

    qDebug()<< " enter"<<pop.getConnectionStatu()<<endl;
    if(!pop.getConnectionStatu())
    {
        qDebug()<<"login"<<endl;
        UserInfo *u = this->userInfoList.at(0);
        pop.setDebugMode(true);
        pop.connectToServer(u->POPServerAddr,u->POPPort,u->POPSSL);
        pop.user(u->POPAccount);
        pop.pass(u->POPPasswd);
    }
}

void MainWindow::updateListWidget(UserInfo &u)
{
    ui->listWidget->clear();
    QSqlQuery mailQuery(db);
    bool ok = mailQuery.exec(QString("select * from inBox%1 order by datetime desc").arg(QString::number(u.id)));
    if(!ok)
    {
        qDebug()<<"update listwidget err"<<endl;
    }
    while (mailQuery.next())
    {
        QString subject = mailQuery.value(4).toString();
        QString uid = mailQuery.value(0).toString();
        QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
        item->setData(Qt::DisplayRole,subject);
        item->setData(Qt::UserRole,uid);
    }
}
