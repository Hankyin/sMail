#include "maileditwidget.h"
#include "ui_maileditwidget.h"
#include "mime.h"
#include "user.h"
#include <QDateTime>
#include <QFileDialog>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QDebug>
#include <QImageReader>
#include <QVBoxLayout>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QCompleter>
#include <QSqlQueryModel>

MailEditWidget::MailEditWidget(User *user, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MailEditWidget)
{
    ui->setupUi(this);
    this->user = user;
    this->smtp = user->getSMTP();
    ui->editTo->setPlaceholderText("收件人");
    QCompleter *completer = new QCompleter(user->getContactList());
    completer->setCompletionMode(QCompleter::PopupCompletion);
    ui->editTo->setCompleter(completer);
    ui->editSubject->setPlaceholderText("主题");
    this->mailWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(this->mailWidget);
    this->textEdit = new QTextEdit;
    this->textEdit->setFrameShape(QFrame::Panel);
    layout->addWidget(this->textEdit);
    layout->setMargin(0);
    ui->scrollArea->setWidget(this->mailWidget);
    ui->scrollArea->setBackgroundRole(QPalette::Light);
    ui->scrollArea->setFrameShape(QFrame::Panel);
    connect(ui->btSend,SIGNAL(clicked(bool)),this,SLOT(slotSend()));
    connect(ui->btAttachment,SIGNAL(clicked(bool)),this,SLOT(slotAddAttachment()));
    connect(ui->btBold,SIGNAL(clicked(bool)),this,SLOT(slotBold()));
    connect(ui->btItalic,SIGNAL(clicked(bool)),this,SLOT(slotItalic()));
    connect(this->smtp,SIGNAL(down(bool)),this,SLOT(slotSendMailDown(bool)));
    connect(this->textEdit,SIGNAL(textChanged()),this,SLOT(slotResetTextEditHeight()));
}

MailEditWidget::~MailEditWidget()
{
    delete ui;
}

void MailEditWidget::slotSend()
{
    if(ui->editTo->text().isEmpty())
        return;
    ui->btSend->setEnabled(false);
    UserInfo userInfo = this->user->getUserInfo();
    QString from = userInfo.mailAddr;
    QString mimeFrom = MIME::HeadEncoding(userInfo.name.toUtf8(),MIME::UTF8,false) + " <" + from + ">";
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
    QString htmlContent = this->textEdit->toHtml();
    QString plainContent = this->textEdit->toPlainText();

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
    for(auto a : this->attList)
    {
        if(a == nullptr)
            continue;
        QString fn = a->getFile();
        int fti = fn.lastIndexOf('.');
        QString ft = fn.mid(fti);
        if(ft.toLower() == "jpeg" || ft.toLower() == "jpg")
        {
            MIMEImage att(fn,MIME::image_jpeg,true);
            mixed.append(att);
        }
        else if(ft.toLower() == "png")
        {
            MIMEImage att(fn,MIME::image_png,true);
            mixed.append(att);
        }
        else if(ft.toLower() == "pdf")
        {
            MIMEApplication att(fn,MIME::application_pdf);
            mixed.append(att);
        }
        else if(ft.toLower() == "zip")
        {
            MIMEApplication att(fn,MIME::application_zip);
            mixed.append(att);
        }
        else
        {
            MIMEApplication att(fn,MIME::application_octet_stream);
            mixed.append(att);
        }
        delete a;
        a = nullptr;
    //attacmhentList中存储的是指针，用完后记得删
    }
    this->attList.clear();
    this->mail = mixed.getContent();
    smtp->setDebugMode(true);
    smtp->setServerAddr(userInfo.SMTPServer,userInfo.SMTPPort,userInfo.SMTPSSL);
    smtp->setLoginInfo(userInfo.mailAddr,userInfo.mailPasswd);
    smtp->sendMail(rcptTo,mail.toUtf8());
}

void MailEditWidget::slotAddAttachment()
{
    QString path = QFileDialog::getOpenFileName(this,"添加附件");
    if(path.isEmpty())
    {
        return;
    }
    int attId = this->attList.size();//attid是为了删除时确定它在layout中的位置
    AttachmentWidget *att = new AttachmentWidget(path,attId);
    this->attList.append(att);
    this->mailWidget->layout()->addWidget(att);
    connect(att,SIGNAL(remove(int)),this,SLOT(slotRemoveAttachment(int)));
}

void MailEditWidget::slotRemoveAttachment(int attId)
{
    QWidget *widget = this->attList.at(attId);
    QLayout *layout = this->mailWidget->layout();
    layout->removeWidget(widget);
    delete widget;
    widget = nullptr;
    this->attList.replace(attId,nullptr);
}

void MailEditWidget::slotInsert()
{
//    QString file = QFileDialog::getOpenFileName(this, tr("Select an image"),
//                                  ".", tr("Bitmap Files (*.bmp)\n"
//                                    "JPEG (*.jpg *jpeg)\n"
//                                    "GIF (*.gif)\n"
//                                    "PNG (*.png)\n"));
//    QUrl Uri ( QString ( "file://%1" ).arg ( file ) );
//    QImage image = QImageReader ( file ).read();

//    QTextDocument * textDocument = ui->textEdit->document();
//    textDocument->addResource( QTextDocument::ImageResource, Uri, QVariant ( image ) );
//    QTextCursor cursor = ui->textEdit->textCursor();
//    QTextImageFormat imageFormat;
//    imageFormat.setWidth( image.width() );
//    imageFormat.setHeight( image.height() );
//    imageFormat.setName( Uri.toString() );
    //    cursor.insertImage(imageFormat);
}

void MailEditWidget::slotBold()
{
    if(this->textEdit->currentCharFormat().fontWeight() == QFont::Bold)
    {
        this->textEdit->setFontWeight(QFont::Normal);
    }
    else
    {
        this->textEdit->setFontWeight(QFont::Bold);
    }
}

void MailEditWidget::slotItalic()
{
    if(this->textEdit->currentCharFormat().fontItalic())
    {
        this->textEdit->setFontItalic(false);
    }
    else
    {
        this->textEdit->setFontItalic(true);
    }
}

void MailEditWidget::slotSendMailDown(bool err)
{
    QMessageBox::information(this,"邮件","邮件发送成功");
    ui->btSend->setEnabled(true);
    emit sendDown();
    this->close();
}

void MailEditWidget::slotSendMailErr(int error)
{
    QString errorType;
    QString errorContent;
    switch (error)
    {
    case SMTP::RetCodeError:
        errorType = "返回码错误";
        break;
    case SMTP::TCPError:
        errorType = "网络错误";
        break;
    default:
        break;
    }
    QMessageBox::critical(this,errorType,errorContent,QMessageBox::Ok);
    this->close();
}

void MailEditWidget::slotResetTextEditHeight()
{
    QTextDocument *doc = this->textEdit->document();
    if(doc)
    {
        int docHeight = doc->size().height();
        this->textEdit->setMinimumHeight(docHeight + 10);
    }
}

void MailEditWidget::closeEvent(QCloseEvent *event)
{
    bool isSending = ui->btSend->isEnabled() ? false : true;
    if(isSending)
    {
        event->ignore();
        QMessageBox::information(this,"提示","正在发送邮件，发送成功后该窗口自动关闭");
    }
}



