#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QByteArray>
#include <QSqlDatabase>
#include <QShowEvent>
#include <QList>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTcpSocket>
#include <QUrl>
#include "smtp.h"
#include "mime.h"
#include "pop.h"
#include "logindialog.h"

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
    void slotRefreshStart();
    void slotRefreshStat(uint mailCount);
    void slotRefreshTop(int mailId, QByteArray head);
    void slotRefreshUidl(int mailId,QByteArray uid);
    void slotReadMail(QListWidgetItem *item);
    void slotRetr(int mailId,QByteArray mail);
    void slotAfterShow();
    void slotOpenExternalLink(const QUrl &url);
    void slotPOPDown(bool err);
private:
    Ui::MainWindow *ui;
    QList<UserInfo*> userInfoList;
    POP pop;
//    SMTP smtp;
    QSqlDatabase db;
    bool createUser();
    void popLogin();
    void updateListWidget(UserInfo &u);
};


#endif // MAINWINDOW_H
