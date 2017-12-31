#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "user.h"
#include "smaildb.h"
#include <QMainWindow>
#include <QByteArray>
#include <QSqlDatabase>
#include <QShowEvent>
#include <QList>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTcpSocket>
#include <QUrl>
#include <QModelIndex>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
signals:
    void getMailCount(int mailCount);
    void getMailHead(QByteArray &head);
private slots:
    void slotWriteMail();
    void slotChangeDir(QModelIndex index);
    void slotReadMail(QModelIndex index);
    void slotRefreshMail();
    void slotAfterShow();
private:
    Ui::MainWindow *ui;
    QList<User*> userList;
    User *curUser;
    sMailDB *db;
};


#endif // MAINWINDOW_H
