#ifndef ATTACHMENTWIDGET_H
#define ATTACHMENTWIDGET_H

#include <QWidget>
#include <QPaintEvent>

class AttachmentWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AttachmentWidget(QWidget *parent = nullptr);
    AttachmentWidget(const QString &path, int attachmentId, QWidget *parent = nullptr);
    void setMIME(const QString &path, int attachmentId);
    QString getFile() {return this->attachFile;}
signals:
    void remove(int id);
public slots:
    void slotClick();
protected:
    void paintEvent(QPaintEvent *event);
private:
    int id;
    QString attachFile;
    const int IconSize = 16;
    const int BtSize = 16;
    const int WidgetWidth = 200;
};

#endif // ATTACHMENTWIDGET_H
