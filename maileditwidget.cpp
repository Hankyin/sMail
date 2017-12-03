#include "maileditwidget.h"
#include "ui_maileditwidget.h"
#include "mime.h"
#include "smtp.h"
#include <QDateTime>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QImageReader>

MailEditWidget::MailEditWidget(UserInfo *user, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MailEditWidget)
{
    this->userInfo = user;
    ui->setupUi(this);
    ui->editTo->setPlaceholderText("收件人");
    ui->editSubject->setPlaceholderText("主题");
    connect(ui->btSend,SIGNAL(clicked(bool)),this,SLOT(slotSend()));
    connect(ui->btAttachment,SIGNAL(clicked(bool)),this,SLOT(slotAddAttachment()));
    connect(ui->btInsert,SIGNAL(clicked(bool)),this,SLOT(slotInsert()));
}

MailEditWidget::~MailEditWidget()
{
    delete ui;
}

void MailEditWidget::slotSend()
{
    ui->btSend->setEnabled(false);
    QString from = userInfo->userMailAddr;
    QString mailFrom = from;
    QString mimeFrom = MIME::HeadEncoding(userInfo->userName.toUtf8(),MIME::UTF8,false) + " <" + from + ">";
    QString to = ui->editTo->text();
    QList<QString> rcptTo = to.split(';',QString::SkipEmptyParts);
    QString mimeTo;
    for(int i = 0;i < rcptTo.size();i++)
    {
        QString t;
        t.append('<');
        t.append(rcptTo.at(i));
        t.append('>');
        mimeTo.append(t);
    }
    //To: loufand 163.com <loufand@163.com>, =?utf-8?B?6ZO2IOihjA==?=
    //<loufand@outlook.com>, 1391240791 qq.com <1391240791@qq.com>
    //这是群发邮件的头
    QString subject = ui->editSubject->text();
    QDateTime date = QDateTime::currentDateTime();
    QString htmlContent = ui->textEdit->toHtml();
    QString plainContent = ui->textEdit->toPlainText();

    MIMEMultipart mixed(MIME::multipart_mixed);
    mixed.addHead("From",mimeFrom.toUtf8());
    mixed.addHead("To",mimeTo.toUtf8());
    mixed.addHead("Subject",mixed.HeadEncoding(subject.toUtf8(),MIME::UTF8,false));
    mixed.addHead("Date",date.toString(Qt::RFC2822Date).toUtf8());
    mixed.addHead("Mime-Version","1.0");
    MIMEMultipart alternative(MIME::multipart_alternative);
    MIMEText msgText(plainContent.toUtf8(),MIME::text_plain,0);
    MIMEText msgHtml(htmlContent.toUtf8(),MIME::text_html,0);
    alternative.append(msgHtml);
    alternative.append(msgText);
    mixed.append(alternative);
    //插入附件
    for(int i = 0;i < this->attachmentList.size();i++)
    {
        mixed.append(*attachmentList.at(i));
    }

//    QFile file("test.txt");
//    if(file.open(QIODevice::WriteOnly))
//    {
//        file.write(mixed.getContent());
//        file.close();
//        return;
//    }
    //try
    {
    SMTP smtp;
    smtp.setDebugMode(true);
    smtp.connectServer(userInfo->SMTPServerAddr,userInfo->SMTPPort,userInfo->SMTPSSL);
    smtp.login(userInfo->SMTPAccount,userInfo->SMTPPasswd);
    smtp.sendMail(mailFrom,rcptTo,mixed.getContent());//getcontent有问题，他不能多次调用
    smtp.quit();
    }
    //attacmhentList中存储的是指针，用完后记得删除
    qDeleteAll(this->attachmentList);
    this->attachmentList.clear();
    QMessageBox::information(this,"邮件","邮件发送成功");
    ui->btSend->setEnabled(true);
    this->close();
}

void MailEditWidget::slotAddAttachment()
{
    QString path = QFileDialog::getOpenFileName(this,"添加附件");
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this,"错误","附件打开错误");
        return;
    }
    MIMEImage *attachment = new MIMEImage(path,MIME::image_jpeg,true);
    this->attachmentList.append(attachment);//append是深拷贝还是浅拷贝？？
}

void MailEditWidget::slotInsert()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Select an image"),
                                  ".", tr("Bitmap Files (*.bmp)\n"
                                    "JPEG (*.jpg *jpeg)\n"
                                    "GIF (*.gif)\n"
                                    "PNG (*.png)\n"));
    QUrl Uri ( QString ( "file://%1" ).arg ( file ) );
    QImage image = QImageReader ( file ).read();

    QTextDocument * textDocument = ui->textEdit->document();
    textDocument->addResource( QTextDocument::ImageResource, Uri, QVariant ( image ) );
    QTextCursor cursor = ui->textEdit->textCursor();
    QTextImageFormat imageFormat;
    imageFormat.setWidth( image.width() );
    imageFormat.setHeight( image.height() );
    imageFormat.setName( Uri.toString() );
    cursor.insertImage(imageFormat);
}
