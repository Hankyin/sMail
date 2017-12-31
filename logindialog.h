#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include "user.h"
#include <QDialog>

namespace Ui {
class LoginDialog;
}

class UserInfo;

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
