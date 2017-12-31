#include "smaildb.h"
#include "logindialog.h"
#include <QSqlQuery>
#include <QSql>
#include <QSqlError>
#include <QDateTime>
#include <QVariant>

sMailDB::sMailDB(QObject *parent) : QObject(parent)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("sMail.db");
    if(!db.open())
    {
        this->lastErrorString = "can't open datebase";
        emit error();
        return ;
    }
    //sqlite会自动维护一个系统表sqlite_master，该表存储了我们所创建的各个table, view, trigger等等信息。
    QSqlQuery query(db);
    query.exec("SELECT count(*) "
               "FROM sqlite_master "
               "WHERE type = 'table' AND name = 'User' ;");
    //判断user是否存在，不存在则创建一个
    int hasUserTable;
    while (query.next())
    {
        hasUserTable = query.value(0).toInt();
    }
    if(!hasUserTable)
    {
        bool ok = query.exec("CREATE TABLE User"
                             "("
                             "  ID        INT     NOT NULL PRIMARY KEY,"
                             "  MailAddr  TEXT    NOT NULL,"
                             "  MailPass  TEXT    NOT NULL,"
                             "  Name      TEXT    NOT NULL,"
                             "  POPServer TEXT    NOT NULL,"
                             "  POPPort   INT     NOT NULL,"
                             "  POPSSL    INT     NOT NULL,"
                             "  SMTPServer TEXT   NOT NULL,"
                             "  SMTPPort  INT     NOT NULL,"
                             "  SMTPSSL   INT     NOT NULL"
                             ");");
        if(!ok)
        {
            this->lastErrorString = query.lastError().text();
            emit error();
            return;
        }
    }
}

QList<User*> sMailDB::getUserList()
{
    QSqlQuery query(db);
    QList<User*> userList;
    bool ok = query.exec("SELECT count(*)"
                         "FROM User");
    if(!ok)
    {
        this->lastErrorString = query.lastError().text();
        emit error();
        return QList<User*>();
    }
    bool hasUser = false;
    while (query.next())
    {
        hasUser = query.value(0).toBool();
    }
    if(!hasUser)
    {
        ok = addUser();
        if(!ok)
        {
            return QList<User*>();
        }
    }
    ok = query.exec("SELECT *"
                    "FROM User;");
    while (query.next())
    {
        UserInfo u;
        u.id = query.value(0).toUInt();
        u.mailAddr = query.value(1).toString();
        u.mailPasswd = query.value(2).toString();
        u.name = query.value(3).toString();
        u.POPServer = query.value(4).toString();
        u.POPPort = query.value(5).toInt();
        u.POPSSL = query.value(6).toBool();
        u.SMTPServer = query.value(7).toString();
        u.SMTPPort = query.value(8).toInt();
        u.SMTPSSL = query.value(9).toBool();
        User *nu = new User(u,this->db,this);
        userList.append(nu);
    }
    return userList;
}

bool sMailDB::addUser()
{
    LoginDialog login;
    if(!login.exec())
    {
        return false;
    }
    UserInfo newUser = login.getNewUser();
    newUser.id = QDateTime::currentDateTime().toTime_t();
    //在User表中插入新记录
    QString insert = "INSERT INTO User"
                     "("
                     "  ID,"
                     "  MailAddr,"
                     "  MailPass,"
                     "  Name,"
                     "  POPServer,"
                     "  POPPort,"
                     "  POPSSL,"
                     "  SMTPServer,"
                     "  SMTPPort,"
                     "  SMTPSSL"
                     ")"
                     "VALUES"
                     "(?,?,?,?,?,?,?,?,?,?);";
    QSqlQuery query(db);
    query.prepare(insert);
    query.addBindValue(newUser.id);
    query.addBindValue(newUser.mailAddr);
    query.addBindValue(newUser.mailPasswd);
    query.addBindValue(newUser.name);
    query.addBindValue(newUser.POPServer);
    query.addBindValue(newUser.POPPort);
    query.addBindValue(newUser.POPSSL);
    query.addBindValue(newUser.SMTPServer);
    query.addBindValue(newUser.SMTPPort);
    query.addBindValue(newUser.SMTPSSL);
    bool ok = query.exec();
    if(!ok)
    {
        this->lastErrorString = query.lastError().text();
        emit error();
        return false;
    }
    //创建该用户的邮件目录表
    QString creatDirTable = "CREATE TABLE Dir_%1"
                            "("
                            "   DirName TEXT NOT NULL PRIMARY KEY"
                            ");";
    ok = query.exec(creatDirTable.arg(QString::number(newUser.id)));
    if(!ok)
    {
        this->lastErrorString = query.lastError().text();
        emit error();
        return false;
    }
    //在该用户的邮件目录表中插入默认目录
    QString insertDirTalbe = "INSERT INTO Dir_%1"
                             "(DirName)"
                             "VALUES"
                             "(?);";
    query.prepare(insertDirTalbe.arg(QString::number(newUser.id)));
    QStringList defDirList;
    defDirList<<QStringLiteral("收件箱")<<QStringLiteral("发件箱")<<QStringLiteral("废件箱");
    for(auto d : defDirList)
    {
        query.addBindValue(d);
        ok = query.exec();
        if(!ok)
        { 
            this->lastErrorString = query.lastError().text();
            emit error();
            return false;
        }
    }
    //创建邮件表
    QString createMailTable;
    createMailTable = QString("CREATE TABLE Mail_%1"
                          "("
                          " UID          NTEXT    NOT NULL COLLATE NOCASE,"
                          " MIndex       INT     NOT NULL COLLATE NOCASE,"
                          " SendDate     INT     NOT NULL,"
                          " SenderName   NTEXT    NOT NULL,"
                          " SenderMail   NTEXT    NOT NULL,"
                          " Subject      NTEXT,"
                          " Dir          NTEXT    NOT NULL COLLATE NOCASE,"
                          " HasRead      INT     NOT NULL"
                          ")");
    ok = query.exec(createMailTable.arg(QString::number(newUser.id)));
    if(!ok)
    {
        this->lastErrorString = query.lastError().text();
        emit error();
        return false;
    }
    return ok;
}
