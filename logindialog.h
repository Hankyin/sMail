#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = 0);
    ~LoginDialog();

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
private slots:
    void slotToSettingPage();
    void slotToUserInfoPage();
    void slotCreateAcount();
private:
    Ui::LoginDialog *ui;
};

#endif // LOGINDIALOG_H
