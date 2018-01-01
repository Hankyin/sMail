#ifndef MAILSHOWWIDGET_H
#define MAILSHOWWIDGET_H

#include "mime.h"
#include <QWidget>
#include <QWebView>

class MailShowWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MailShowWidget(QWidget *parent = nullptr);
    void setPraser(MailPraser &praser);
signals:

public slots:
    void slotOpenExternalLink(const QUrl &url);
    void slotDownloadAttachment();
private:
    QWebView *webView;
    MailPraser praser;
};

#endif // MAILSHOWWIDGET_H
