#ifndef MAILEDITWIDGET_H
#define MAILEDITWIDGET_H

#include "mime.h"
#include "smtp.h"
#include "attachmentwidget.h"
#include <QWidget>
#include <QByteArray>
#include <QList>
#include <QTextEdit>
#include <QTextDocument>
#include <QCloseEvent>

class User;
class UserInfo;

namespace Ui {
class MailEditWidget;
}

class MailEditWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MailEditWidget(User *user,QWidget *parent = 0);
    ~MailEditWidget();
    QString& getMail() {return this->mail;}
signals:
    void sendDown();
private slots:
    void slotSend();
    void slotAddAttachment();
    void slotRemoveAttachment(int attId);
    void slotInsert();
    void slotBold();
    void slotItalic();
    void slotSendMailDown(bool err);
    void slotSendMailErr(int error);
    void slotResetTextEditHeight();
protected:
    void closeEvent(QCloseEvent *event);
private:
    Ui::MailEditWidget *ui;
    QList<AttachmentWidget*> attList;
    User *user;
    SMTP *smtp;
    QWidget *mailWidget;
    QTextEdit *textEdit;
    QString mail;
};

#endif // MAILEDITWIDGET_H
