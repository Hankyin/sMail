#ifndef MAILEDITWIDGET_H
#define MAILEDITWIDGET_H

#include <QWidget>
#include <QByteArray>
#include <QList>
#include "mime.h"
#include "mainwindow.h"
#include "smtp.h"

namespace Ui {
class MailEditWidget;
}

class MailEditWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MailEditWidget(UserInfo * user,QWidget *parent = 0);
    ~MailEditWidget();

private slots:
    void slotSend();
    void slotAddAttachment();
    void slotInsert();
    void slotSendMailDown(bool err);
private:
    Ui::MailEditWidget *ui;
    QList<MIME*> attachmentList;
    UserInfo *userInfo;
    SMTP smtp;
};

#endif // MAILEDITWIDGET_H
