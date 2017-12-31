#ifndef MAILMODEL_H
#define MAILMODEL_H

#include <QStandardItemModel>
#include <QStandardItem>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

class MailModel : public QStandardItemModel
{
    Q_OBJECT
public:
    enum HideRole
    {
        UIDRole = Qt::UserRole,
        MIndexRole,
    };
    explicit MailModel(QObject *parent = Q_NULLPTR, QSqlDatabase db = QSqlDatabase());
    void setTable(const QString &table);
    void setSort();
    void setFilter(const QString &filter);
    bool select();
    bool mailExist(const QString &uid);
    bool updateId(const QString &uid, int mailIndex);
    bool insert(const QString &UID, int MIndex, int SendDate, const QString &SenderName,
                const QString &SenderMail, const QString &Subject, const QString dir = QString("收件箱"));
    QString getUID(int mailIndex);
signals:

public slots:
    void setDir(const QString &dir);
private:
    QSqlDatabase database;
    QString tableName;
    QString filter;
};

#endif // MAILMODEL_H
