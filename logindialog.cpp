#include "logindialog.h"
#include "ui_logindialog.h"
#include <QRegExp>
#include <QRegExpValidator>
#include <QMessageBox>

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    slotToUserInfoPage();
    this->ui->editMailAddr->setPlaceholderText("邮箱地址");
    this->ui->editMailPasswd->setPlaceholderText("邮箱登录密码");
    this->ui->editUserName->setPlaceholderText("用户名");
    this->ui->editPOPServerAddr->setPlaceholderText("POP服务器地址");
    this->ui->editPOPAccount->setPlaceholderText("POP账号");
    this->ui->editPOPPasswd->setPlaceholderText("POP登录密码");
    this->ui->editSMTPServerAddr->setPlaceholderText("SMTP服务器地址");
    this->ui->editSMTPAccount->setPlaceholderText("SMTP账号");
    this->ui->editSMTPPasswd->setPlaceholderText("SMTP登录密码");
    QRegExp regexp("^[a-zA-Z0-9_]+@[a-zA-Z0-9]+\\.[a-zA-Z0-9]{1,3}");
    QRegExpValidator *validator = new QRegExpValidator(regexp);
    this->ui->editMailAddr->setValidator(validator);
    this->ui->editMailPasswd->setEchoMode(QLineEdit::Password);
    this->ui->editSMTPPasswd->setEchoMode(QLineEdit::Password);
    this->ui->editPOPPasswd->setEchoMode(QLineEdit::Password);

    connect(ui->btToMailSetting,SIGNAL(clicked(bool)),this,SLOT(slotToSettingPage()));
    connect(ui->btToUserInfo,SIGNAL(clicked(bool)),this,SLOT(slotToUserInfoPage()));
    connect(ui->btCreateAccount,SIGNAL(clicked(bool)),this,SLOT(slotCreateAcount()));
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::slotToSettingPage()
{
    if(ui->editUserName->text().isEmpty() ||
            ui->editMailAddr->text().isEmpty() ||
            ui->editMailPasswd->text().isEmpty())
    {
        QMessageBox::critical(this,"错误","请填写完整的信息");
        return;
    }
    QString mailAddr = ui->editMailAddr->text();
    QString mailPasswd = ui->editMailPasswd->text();
    QString domain = mailAddr.split('@').at(1);

    ui->editPOPAccount->setText(mailAddr);
    ui->editPOPPasswd->setText(mailPasswd);
    ui->editPOPServerAddr->setText("pop." + domain);
    ui->editPOPPort->setText(QString::number(110));
    ui->editSMTPAccount->setText(mailAddr);
    ui->editSMTPPasswd->setText(mailPasswd);
    ui->editSMTPServerAddr->setText("smtp." + domain);
    ui->editSMTPPort->setText(QString::number(25));
    ui->stackedWidget->setCurrentIndex(1);
}

void LoginDialog::slotToUserInfoPage()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void LoginDialog::slotCreateAcount()
{
    if(ui->editSMTPAccount->text().isEmpty()||
            ui->editSMTPServerAddr->text().isEmpty()||
            ui->editSMTPPasswd->text().isEmpty()||
            ui->editPOPAccount->text().isEmpty()||
            ui->editPOPServerAddr->text().isEmpty()||
            ui->editPOPPasswd->text().isEmpty())
    {
        QMessageBox::critical(this,"错误","请填写完整信息");
        return;
    }
    this->userName = ui->editUserName->text();
    this->userMailAddr = ui->editMailAddr->text();
    this->userMailPasswd = ui->editMailPasswd->text();
    this->SMTPAccount = ui->editSMTPAccount->text();
    this->SMTPServerAddr = ui->editSMTPServerAddr->text();
    this->SMTPPasswd = ui->editSMTPPasswd->text();
    this->SMTPPort = ui->editSMTPPort->text().toInt();
    this->SMTPSSL = ui->checkSMTPSSL->isChecked();
    this->POPAccount = ui->editPOPAccount->text();
    this->POPServerAddr = ui->editPOPServerAddr->text();
    this->POPPasswd = ui->editPOPPasswd->text();
    this->POPPort = ui->editPOPPort->text().toInt();
    this->POPSSL = ui->checkPOPSSL->isChecked();
    this->accept();
}
