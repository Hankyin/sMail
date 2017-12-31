#include "mailmodel.h"
#include <QDateTime>

MailModel::MailModel(QObject *parent, QSqlDatabase db)
    : QStandardItemModel(parent)
{
    this->database = db;
}

void MailModel::setTable(const QString &table)
{
    this->tableName = table;
}

void MailModel::setSort()
{

}

void MailModel::setFilter(const QString &filter)
{
    this->filter = filter;
}

bool MailModel::select()
{
    QSqlQuery query(this->database);
    QString sql = QString("SELECT * "
                          "FROM %1 ").arg(this->tableName);
    if(!this->filter.isEmpty())
    {
        sql.append(QString("WHERE %1").arg(this->filter));
    }
    bool ok = query.exec(sql);
    //定义模型
    int colCount = 4;
    this->clear();
    this->setColumnCount(colCount);
    this->setHorizontalHeaderLabels(QStringList()<<"已读"<<"主题"<<"发送者"<<"日期");
    while(query.next())
    {
        QList<QStandardItem*> newRow;
        for(int i = 0;i < colCount;i++)
        {
            QStandardItem *item = new QStandardItem;
            item->setData(query.value("UID"),UIDRole);
            item->setData(query.value("MIndex"),MIndexRole);
            newRow.append(item);
        }
        bool hasRead = query.value("HasRead").toBool();
        if(hasRead)
        {
            newRow.at(0)->setText("已读");
        }
        else
        {
            newRow.at(0)->setText("未读");
        }
        QString subject = query.value("Subject").toString();
        newRow.at(1)->setText(subject);
        QString senderName = query.value("SenderName").toString();
        newRow.at(2)->setText(senderName);
        QDateTime sendDate = QDateTime::fromTime_t(query.value("SendDate").toUInt());
        newRow.at(3)->setText(sendDate.toString());
        this->appendRow(newRow);
    }
    return ok;
}

bool MailModel::mailExist(const QString &uid)
{
    QSqlQuery query(this->database);
    QString select = "SELECT *"
                     "FROM %1 "
                     "WHERE UID like '%2'";
    bool ok = query.exec(select.arg(this->tableName,uid));
    QString err = query.lastError().text();
    int isexist = 0;
    while(query.next())
    {
        isexist ++;
    }
    bool ret = isexist;
    return ret;
}

bool MailModel::updateId(const QString &uid,int mailIndex)
{
    QSqlQuery query(this->database);
    QString update = "UPDATE %1 "
                     "SET MIndex = %2 "
                     "WHERE UID like '%3' ";
    bool ok = query.exec(update.arg(this->tableName,QString::number(mailIndex),uid));
    return ok;
}

bool MailModel::insert(const QString &UID, int MIndex, int SendDate, const QString &SenderName,
                       const QString &SenderMail, const QString &Subject,const QString dir)
{
    QSqlQuery query(this->database);
    QString insert = "INSERT INTO %1 "
                     "("
                     "  UID,"
                     "  MIndex,"
                     "  SendDate,"
                     "  SenderName,"
                     "  SenderMail,"
                     "  Subject,"
                     "  Dir,"
                     "  HasRead"
                     ") "
                     "VALUES"
                     "(?,?,?,?,?,?,?,?);";
    bool ok = query.prepare(insert.arg(this->tableName));
    query.addBindValue(UID);
    query.addBindValue(MIndex);
    query.addBindValue(SendDate);
    query.addBindValue(SenderName);
    query.addBindValue(SenderMail);
    query.addBindValue(Subject);
    query.addBindValue(dir);
    query.addBindValue(false);
    ok = query.exec();
    return ok;
}

QString MailModel::getUID(int mailIndex)
{
    QSqlQuery query(this->database);
    QString select = "SELECT UID "
                     "FROM %1 "
                     "WHERE MIndex = %2";
    bool ok = query.exec(select.arg(this->tableName,QString::number(mailIndex)));
    QString uid;
    while (query.next())
    {
        uid = query.value(0).toString();
    }
    return uid;
}

void MailModel::setDir(const QString &dir)
{
    if(dir.isEmpty())
        return;
    QString f = "Dir like '";
    f.append(dir);
    f.append("'");
    setFilter(f);
}
