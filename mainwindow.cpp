#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QByteArray>
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
#include <QSqlDatabase>
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
    connect(ui->btRefresh,SIGNAL(clicked(bool)),this,SLOT(slotRefresh()));
    connect(ui->btWriteMail,SIGNAL(clicked(bool)),this,SLOT(slotWriteMail()));
    connect(ui->listWidget,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(slotReadMail(QListWidgetItem*)));
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

void MainWindow::slotRefresh()
{
    MIME mail;
    POP pop;
    pop.setDebugMode(true);
    pop.connectToServer("pop.163.com");
    pop.login("loufand@163.com","fallredhate139");
    int count = 0;
    qint64 totalSize = 0;
    pop.stat(count,totalSize);
    for(int i = 1; i <10; i++)
    {

    }
    pop.quit();

}

void MainWindow::slotReadMail(QListWidgetItem *item)
{
    int userId = item->data(Qt::UserRole).toInt();
    int mailId = item->data(Qt::UserRole + 1).toInt();
    UserInfo *userInfo;
    for(auto u : this->userInfoList)
    {
        if(u->id == userId)
            userInfo = u;
    }
    QByteArray mail = downloadMail(userInfo,mailId);
    MailPraser mailPraser(mail);
    QFile file(mailPraser.getSubject() + ".eml");
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug()<<"write eml file"<<endl;
        qDebug()<<file.write(mail)<<endl;
        file.close();
    }
    QString html = mailPraser.getHtml();
    ui->webView->setHtml(html);
    this->update();
}

void MainWindow::slotAfterShow()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("sMail.db");
    if(!db.open())
    {
        qDebug()<<"db open error"<<endl;
        return;
    }
    //sqlite会自动维护一个系统表sqlite_master，该表存储了我们所创建的各个table, view, trigger等等信息。
    QSqlQuery query;
    query.exec("select count(*) from sqlite_master where type = 'table' and name = 'userList' ;");
    //判断userList是否存在，不存在则创建一个
    int hasUserTable = 999;
    while (query.next())
    {
        hasUserTable = query.value(0).toInt();
        qDebug()<<hasUserTable<<endl;
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
    //读取表中被选中的记录
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
    }
    if(this->userInfoList.isEmpty())
    {//如果查不到记录，就打开登录对话框，让用户输入一个
        LoginDialog login(this);
        if(!login.exec())
            this->close();
        else
        {
            UserInfo *userInfo = new UserInfo();
            userInfo->id = 1;
            userInfo->userName = login.userName;
            userInfo->userMailAddr = login.userMailAddr;
            userInfo->userMailPasswd = login.userMailPasswd;
            userInfo->POPServerAddr = login.POPServerAddr;
            userInfo->POPAccount = login.POPAccount;
            userInfo->POPPasswd = login.POPPasswd;
            userInfo->POPPort = login.POPPort;
            userInfo->POPSSL = login.POPSSL;
            userInfo->SMTPServerAddr = login.SMTPServerAddr;
            userInfo->SMTPAccount = login.SMTPAccount;
            userInfo->SMTPPasswd = login.SMTPPasswd;
            userInfo->SMTPPort = login.SMTPPort;
            userInfo->SMTPSSL = login.SMTPSSL;
            this->userInfoList.append(userInfo);
            QString SQLinsert = "insert into userList (userName,userMailAddr,userMailPasswd,"
                                "POPServerAddr,POPAccount,POPPasswd,POPPort,POPSSL,"
                                "SMTPServerAddr,SMTPAccount,SMTPPasswd,SMTPPort,SMTPSSL)"
                                "values(?,?,?,?,?,?,?,?,?,?,?,?,?);";
            query.prepare(SQLinsert);
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
        }
    }
    for(auto u : this->userInfoList)
    {
        //测试用户信息正确性
        //TODO 用try catch检测
        SMTP smtp;
        smtp.connectServer(u->SMTPServerAddr,u->SMTPPort,u->SMTPSSL);
        smtp.login(u->SMTPAccount,u->SMTPPasswd);
        smtp.quit();

        POP pop;
        pop.connectToServer(u->POPServerAddr,u->POPPort,u->POPSSL);
        pop.login(u->POPAccount,u->POPPasswd);
        int count;
        qint64 totalSize;
        pop.stat(count,totalSize);
        for(int i = 1; i <= 10;i++)
        {
            QByteArray head;
            pop.top(i,0,head);
            MailPraser mailPraser(head);
            QString subject = mailPraser.getSubject();
            QString from = mailPraser.getFrom();
            QDate date = mailPraser.getDate();
            QListWidgetItem *item = new  QListWidgetItem(subject,ui->listWidget);
            item->setData(Qt::UserRole,u->id);//用户id
            item->setData(Qt::UserRole + 1,i);//邮件id
        }    
        QTreeWidgetItem *tItem = new QTreeWidgetItem(ui->treeWidget,QStringList(u->userName));
        new QTreeWidgetItem(tItem,QStringList("收件箱"));
        new QTreeWidgetItem(tItem,QStringList("发件箱"));
        new QTreeWidgetItem(tItem,QStringList("废件箱"));
    }
    db.close();
}

QByteArray MainWindow::downloadMail(UserInfo *user, int mailId)
{
    POP pop;
    pop.setDebugMode(true);
    pop.connectToServer(user->POPServerAddr,user->POPPort,user->POPSSL);
    pop.login(user->POPAccount,user->POPPasswd);
    QByteArray mail;
    pop.retr(mailId,mail);
    pop.quit();
    return mail;
}
