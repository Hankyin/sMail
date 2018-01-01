#include "mailshowwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QDir>
#include <QWebView>
#include <QWebPage>
#include <QDesktopServices>
#include <QPushButton>

MailShowWidget::MailShowWidget(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    webView = new QWebView(this);
    mainLayout->addWidget(webView);
    mainLayout->setMargin(0);
    webView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(this->webView,SIGNAL(linkClicked(QUrl)),this,SLOT(slotOpenExternalLink(QUrl)));
}

void MailShowWidget::setPraser(MailPraser &praser)
{
    this->praser = praser;
    this->webView->setHtml(praser.getHtml());
    int attnum = praser.getAttCount();
    if(attnum)
    {
        QString t = "下载%1个附件";
        QPushButton * btDownload = new QPushButton(t.arg(QString::number(attnum)),this);
        QHBoxLayout *btLayout = new QHBoxLayout;
        btLayout->setMargin(0);
        btLayout->addStretch();
        btLayout->addWidget(btDownload);
        qobject_cast<QBoxLayout*>(this->layout())->addLayout(btLayout);
        connect(btDownload,SIGNAL(clicked(bool)),this,SLOT(slotDownloadAttachment()));
    }
    webView->setHtml(praser.getHtml());
}

void MailShowWidget::slotOpenExternalLink(const QUrl &url)
{
    QDesktopServices::openUrl(url);
}

void MailShowWidget::slotDownloadAttachment()
{
    int attnum = praser.getAttCount();
    for(int i = 0;i < attnum;i++)
    {
        QString fileName = praser.getAttName(i);
        QByteArray fileContent = praser.getAttCont(i);
        QString filePath = QFileDialog::getSaveFileName(this,"保存附件",QDir::currentPath() + '/' + fileName);
        QFile file(filePath);
        if(file.open(QIODevice::WriteOnly))
        {
            file.write(fileContent);
            file.close();
        }
    }
}
