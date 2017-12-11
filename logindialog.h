#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

class UserInfo
{
public:
    UserInfo() {id = 0;}//id 为0说明是新建的用户，数据库中读取的用户id从1开始
    //当前登录用户信息
    uint id;
    QString userName;
    QString userMailAddr;
    QString userMailPasswd;
    QString SMTPServerAddr;
    QString SMTPAccount;
    QString SMTPPasswd;
    int SMTPPort = 25;
    bool SMTPSSL = false;
    QString POPServerAddr;
    QString POPAccount;
    QString POPPasswd;
    int POPPort;
    bool POPSSL;

    int mailCount = 0;
};

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = 0);
    ~LoginDialog();
    UserInfo& getNewUser(){return this->newUser;}
private slots:
    void slotToSettingPage();
    void slotToUserInfoPage();
    void slotCreateAcount();
private:
    Ui::LoginDialog *ui;
    UserInfo newUser;
    void connectTest();

};

#endif // LOGINDIALOG_H
