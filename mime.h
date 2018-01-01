#ifndef MAIL_H
#define MAIL_H
#include <QObject>
#include <QByteArray>
#include <QList>
#include <QMap>
#include <QStringList>
#include <QImage>
#include <QDate>

class MIME
{
public:
    enum MIMEType
    {
        //多部类型 0
        multipart_mixed = 0,
        multipart_related = 1,
        multipart_alternative = 2,
        //文本类型 1
        text_plain = 10,
        text_html = 11,
        //图片类型 2
        image_png = 20,
        image_jpeg = 21,
        image_gif = 22,
        image_bmp = 23,
        //应用类型 3
        application_pdf = 30,
        application_zip = 31,
        application_octet_stream = 32,
        //声音类型 4
        audio_mpeg = 40,
        //视频类型 5
        video_mpeg = 50
    };

    enum TextEncoding
    {
        UTF8 = 0,
        GB18030 = 1,
        BIT7 = 2,
        BIT8 = 3
    };

    enum TransferEncoding
    {
        Base64 = 0,
        Quoted_Printable = 1,
        bit7 = 2,
        bit8 = 3,
    };

    qint64 AttachmentMaxSize = 52428800;//50MB
    MIME() {}
    virtual QByteArray getContent();
    int getMIMEType();
    void addHead(const QByteArray fieldHead, const QByteArray fieldBody);

    static QByteArray HeadEncoding(const QByteArray head,int encoding,bool addQuotation = true);
    static QString HeadDecoding(QByteArray head);
    static QByteArray Quoted_printableDecoder(QByteArray encodingCode);
protected:
    QByteArray convertMIMEType(int mimeType);
    QByteArray content;
    QByteArray head;
    int type;
private:
    static QList<QList<QByteArray>> MIMETypeList;
    static QList<QByteArray> TextEncodingList;
};

class MIMEMultipart : public MIME
{
public:
    MIMEMultipart(int type);
    virtual QByteArray getContent();
    void append(MIME subMIME);
    static void getRandStr(QByteArray &str, int seed);
private:
    QByteArray boundary;
};

class MIMEText : public MIME
{
public:
    MIMEText(const QByteArray text, int type, int Encoding);
};

class MIMEImage : public MIME
{
public:
    MIMEImage(const QString imgPath,int type,bool isAttachment);
};

class MIMEApplication : public MIME
{
public:
    MIMEApplication(const QString filePath, int type, bool isAttachment = true);
};

class MIMEAudio : public MIME
{
public:
    MIMEAudio() {}
};

class MIMEVideo : public MIME
{
public:
    MIMEVideo() {}
};

class MailPraser
{
public:
    MailPraser() {}
    MailPraser(QByteArray mail);
    QString getFrom() {return this->from;}
    QString getSenderName() {return this->senderName;}
    QString getSenderMail() {return this->senderMail;}
    QString getTo() {return this->to;}
    QString getSubject() {return this->subject;}
    QDateTime getDateTime() {return this->datetime;}
    QString getPlain() {return this->plain;}
    QString getHtml() {return this->html;}
    int getAttCount() {return attContList.size();}
    const QByteArray& getAttCont(int index) {return this->attContList.at(index);}
    const QString& getAttName(int index) {return this->attNameList.at(index);}
private:
    void praser(QByteArray &mail);
    QMap<QByteArray,QByteArray> headPraser(QByteArray head);
    void mixPraser(QByteArray msg, QByteArray boundary);
    void relatedPraser(QByteArray msg,QByteArray boundary);
    void alternativePraser(QByteArray msg, QByteArray boundary);
    void textPraser(QByteArray txt, QMap<QByteArray, QByteArray> &headMap);
    void cutHeadAndContent(QByteArray msg,QByteArray &msgHead,QByteArray &msgContent);
    void cutMultipart(QByteArray msg, QByteArray boundary, QList<QByteArray> &subMsgList);
    QString from;
    QString senderName;
    QString senderMail;
    QString to;
    QString subject;
    QDateTime datetime;
    QString plain;
    QString html;
    QList<QByteArray> attContList;
    QList<QString> attNameList;
    QMap<QString,QString> cidTOpath;
};

#endif // MAIL_H
