#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QByteArray>
#include <QShowEvent>
#include <QList>
#include <QListWidget>
#include <QListWidgetItem>
#include "smtp.h"
#include "mime.h"
#include "pop.h"

class UserInfo
{
public:
    UserInfo() {}
    //当前登录用户信息
    int id;
    QString userName;
    QString userMailAddr;
    QString userMailPasswd;
    QString SMTPServerAddr;
    QString SMTPAccount;
    QString SMTPPasswd;
    int SMTPPort;
    bool SMTPSSL;
    QString POPServerAddr;
    QString POPAccount;
    QString POPPasswd;
    int POPPort;
    bool POPSSL;
};

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

private slots:
    void slotWriteMail();
    void slotRefresh();
    void slotReadMail(QListWidgetItem *item);
    void slotAfterShow();
private:
    Ui::MainWindow *ui;
    QList<UserInfo*> userInfoList;
    QByteArray downloadMail(UserInfo *user,int mailId);
};


#endif // MAINWINDOW_H
