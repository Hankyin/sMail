#include "attachmentwidget.h"
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFontMetrics>
#include <QPixmap>
#include <QFileInfo>
#include <QPainter>

AttachmentWidget::AttachmentWidget(QWidget *parent) : QWidget(parent)
{

}

AttachmentWidget::AttachmentWidget(const QString &path, int attachmentId, QWidget *parent) : QWidget(parent)
{
    setMIME(path,attachmentId);
}

void AttachmentWidget::setMIME(const QString &path, int attachmentId)
{
    this->id = attachmentId;
    this->attachFile = path;
    //界面构建
    //图标构建
    QLabel *labIcon = new QLabel(this);
    labIcon->setScaledContents(true);
    if(path.endsWith(".jpg",Qt::CaseInsensitive) ||
            path.endsWith(".jpeg",Qt::CaseInsensitive) ||
            path.endsWith(".png",Qt::CaseInsensitive) ||
            path.endsWith(".bmp",Qt::CaseInsensitive))
    {
        labIcon->setPixmap(QPixmap(":/picture/img.png"));
    }
    else
    {
        labIcon->setPixmap(QPixmap(":/picture/file.png"));
    }
    labIcon->setMaximumSize(IconSize,IconSize);
    //文件名构建
    QLabel *labFileName = new QLabel(this);
    QFontMetrics elideFont(labFileName->font());
    QString fileName = QFileInfo(path).fileName();
    labFileName->setText(elideFont.elidedText(fileName,Qt::ElideMiddle,labFileName->width() - 10));
    //关闭按钮
    QPushButton *btClose = new QPushButton(this);
    btClose->setStyleSheet("QPushButton {"
                           "border-image :url(:/picture/close.png) 16;"
                           "}");
    btClose->setFixedSize(BtSize,BtSize);
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(labIcon);
    mainLayout->addWidget(labFileName);
    mainLayout->addWidget(btClose);

    this->setFixedWidth(WidgetWidth);

    connect(btClose,SIGNAL(clicked(bool)),this,SLOT(slotClick()));
}

void AttachmentWidget::slotClick()
{
    emit remove(this->id);
}

void AttachmentWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawRoundRect(this->rect()-QMargins(1,1,2,2),10,10);
}
