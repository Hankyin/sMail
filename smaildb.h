#ifndef SMAILDB_H
#define SMAILDB_H

#include "user.h"
#include <QObject>
#include <QSqlDatabase>
#include <QList>
#include <QByteArray>
#include <QSqlTableModel>

class User;

class sMailDB : public QObject
{
    Q_OBJECT
public:
   explicit sMailDB(QObject *parent = nullptr);
    QList<User*> getUserList();
    bool addUser();

    QString getLastErrorString() {return this->lastErrorString;}
signals:
    void error();
public slots:

private:
    QSqlDatabase db;
    QString lastErrorString;
};

#endif // SMAILDB_H
