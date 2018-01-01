#include "mime.h"
#include <QTime>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QTextCodec>
#include <QFileInfo>

QList<QList<QByteArray>> MIME::MIMETypeList = {{"multipart/mixed", "multipart/related","multipart/alternative"},
                                             {"text/plain","text/html"},
                                             {"image/png","image/jpeg","image/gif","image/bmp"},
                                             {"application/pdf","application/zip","application/octet-stream"},
                                             {"audio/mpeg"},
                                             {"video/mpeg"}};
QList<QByteArray> MIME::TextEncodingList = {"utf-8","gb18030","7bit","8bit"};

QByteArray MIME::getContent()
{
    QByteArray ret;
    ret.append(head);
    ret.append("\r\n");
    ret.append(content);
    ret.append("\r\n");
    return ret;
}

int MIME::getMIMEType()
{
    return type;
}

void MIME::addHead(const QByteArray fieldHead, const QByteArray fieldBody)
{
    QByteArray newHead;
    newHead.append(fieldHead);
    newHead.append(":");
    newHead.append(fieldBody);
    newHead.append("\r\n");
    this->head.append(newHead);
}

QByteArray MIME::HeadEncoding(const QByteArray head, int encoding, bool addQuotation)
{
    QByteArray encodedHead;
    QString  e = TextEncodingList[encoding];
    //编码后的头部格式"=?gb18030?B?xxxxxxxxxx?="
    if(addQuotation)
    {
        encodedHead.append("\"");
    }
    encodedHead.append("=?");
    encodedHead.append(e);
    encodedHead.append("?b?");
    encodedHead.append(head.toBase64());
    encodedHead.append("?=");
    if(addQuotation)
    {
        encodedHead.append("\" ");
    }
    return encodedHead;
}

QString MIME::HeadDecoding(QByteArray head)
{
    QByteArray rawContent;
    QList<QByteArray> splitList = head.split('?');
    if(splitList.size() < 5)
        return head;
    //判断head的格式是否正确
    if(!splitList.at(0).endsWith('=') || !splitList.at(4).startsWith('='))
    {
        qDebug()<<"mime not encoded head"<<endl;
        return head;
    }
    //传输编码转换
    if(splitList.at(2) == "b" || splitList.at(2) == "B")
    {
        rawContent = QByteArray::fromBase64(splitList.at(3));
    }
    else if(splitList.at(2) == "q" || splitList.at(2) == "Q")
    {
        rawContent = MIME::Quoted_printableDecoder(splitList.at(3));
    }
    else
    {
        qDebug()<<"mime transferEncoding not supprot"<<endl;
        return head;
    }
    //字符编码转换
    QTextCodec *codec = QTextCodec::codecForName(splitList.at(1));
    if(codec == nullptr)
    {
        qDebug()<<"mime headdecoding encoding not support"<<endl;
        return head;
    }
    return codec->toUnicode(rawContent);
}

QByteArray MIME::Quoted_printableDecoder(QByteArray encodingCode)
{
    QByteArray decodeCode;
    for(int i = 0;i < encodingCode.size();)
    {
        char c = encodingCode.at(i);
        if(c == '=')
        {
            if(encodingCode.size() < i + 3)
            {
                qDebug()<<"mime q_p format error"<<endl;
                return encodingCode;
            }
            QByteArray ch;
            decodeCode.append(QByteArray::fromHex(encodingCode.mid(i+1,2)));
            i += 3;
        }
        else
        {
            decodeCode.append(c);
            i++;
        }
    }
    return decodeCode;
}

QByteArray MIME::convertMIMEType(int mimeType)
{
    return MIMETypeList.at(mimeType / 10).at(mimeType % 10);
}

MIMEMultipart::MIMEMultipart(int type)
{
    if(0 != type/10)
    {
        qDebug()<<"mimemultipart type err"<<endl;
        return;
    }
    QByteArray randstr;
    randstr.resize(15);
    getRandStr(randstr,type);//生成boundary的随机部分
    this->boundary = "part_";
    this->boundary.append(randstr);
    QByteArray attr;
    attr.append("boundary=\"");
    attr.append(this->boundary);
    attr.append("\"");
    addHead("Content-Type",convertMIMEType(type) + "; " + attr);
}

QByteArray MIMEMultipart::getContent()
{
    //在最后添加结束标志
    this->content.append("--");
    this->content.append(this->boundary);
    this->content.append("--");
    this->content.append("\r\n");
    return MIME::getContent();
}

void MIMEMultipart::append(MIME subMIME)
{
    this->content.append("--");
    this->content.append(boundary);
    this->content.append("\r\n");
    this->content.append(subMIME.getContent());
}

void MIMEMultipart::getRandStr(QByteArray &str,int seed)
{
    QString alphabet = "0123456789abcdefghijklmnopqrstuvwxyz";
    QTime t = QTime::currentTime();
    qsrand(t.msec() + t.second() + seed);
    for(int i = 0; i < str.size();i++)
    {
        QByteArray rep;
        rep.append(alphabet.at(qrand() % alphabet.size()));
        str.replace(i,1,rep);
    }
}

MIMEText::MIMEText(const QByteArray text, int type, int Encoding)
{
//    Content-Type: text/plain; charset=utf-8
//    Content-Transfer-Encoding: quoted-printable
    if(1 != type/10)
    {
        qDebug()<<"mimetext type err"<<endl;
        return;
    }
    addHead("Content-Type",convertMIMEType(type) + "; " + "charset=utf-8");
    addHead("Content-Transfer-Encoding","base64");
    QByteArray rawCont = text.toBase64();
    for(int i = 0;i < rawCont.size();i ++)
    {
        this->content.append(rawCont.at(i));
        if(i == 0)
            continue;
        if((i % 76) == 0)
        {
            this->content.append("\r\n");
        }
    }
    this->content.append("\r\n");
}

MIMEImage::MIMEImage(const QString imgPath, int type, bool isAttachment)
{
//    Content-Type: application/pdf; name="=?utf-8?B?Q0VU5YWt57qn5Y6G5bm055yf6aKYMjAxNS4xMuKAlDIwMTYucGRm?="
//    Content-Transfer-Encoding: base64
//    Content-Disposition: attachment; filename="=?utf-8?B?Q0VU5YWt57qn5Y6G5bm055yf6aKYMjAxNS4xMuKAlDIwMTYucGRm?="
    if(2 != type/10)
    {
        qDebug()<<"mimeimage type err"<<endl;
        return;
    }
    QFile imgFile(imgPath);
    if(!imgFile.open(QIODevice::ReadOnly))
    {
        qDebug()<<"mimeimage load error"<<endl;
        return;
    }
    QByteArray imgByte = imgFile.readAll();
    QString imgName = imgPath.section('/',-1,-1,QString::SectionSkipEmpty);
    QByteArray attr;
    attr.append("name=\"");
    attr.append(HeadEncoding(imgName.toUtf8(),MIME::UTF8,false));
    attr.append("\"");
    addHead("Content-Type",convertMIMEType(type) + "; " + attr);
    addHead("Content-Transfer-Encoding","base64");
    if(isAttachment)
    {
        attr.insert(0,"file");
        addHead("Content-Disposition",QByteArray("attachment;") + attr);
    }
    QByteArray rawCont = imgByte.toBase64();
    for(int i = 0;i < rawCont.size();i ++)
    {
        this->content.append(rawCont.at(i));
        if((i % 76) == 0)
        {
            this->content.append("\r\n");
        }
    }
    this->content.append("\r\n");
}

MailPraser::MailPraser(QByteArray mail)
{
    praser(mail);
}

void MailPraser::praser(QByteArray &mail)
{
    //分割邮件
    QByteArray head;
    QByteArray content,rawContent;
    cutHeadAndContent(mail,head,rawContent);
    //头部处理，获得邮件体的类型。
    QMap<QByteArray,QByteArray> headMap = headPraser(head);
    QByteArray mType = headMap.value("content-type");
    QByteArray mTEncoding = headMap.value("content-transfer-encoding");//传输编码
    this->from = headMap.value("from");
    int b = this->from.indexOf('<');
    this->senderName = MIME::HeadDecoding(from.left(b).toUtf8());
    this->senderMail = from.mid(b);
    if(senderMail.startsWith('<') && senderMail.endsWith('>'))
    {
        //去掉<>
        this->senderMail = senderMail.mid(1,senderMail.size()-2);
    }
    if(this->senderName.isEmpty())
        this->senderName = this->senderMail;
    this->to = headMap.value("to");
    this->subject = MIME::HeadDecoding(headMap.value("subject"));
    this->datetime = QDateTime::fromString(headMap.value("date"),Qt::RFC2822Date);
    if(mTEncoding == "quoted-printable")
    {
        content = MIME::Quoted_printableDecoder(rawContent);
    }
    else if(mTEncoding == "base64")
    {
        content = QByteArray::fromBase64(rawContent);
    }
    else
    {
        content = rawContent;
    }
    //类型检测
    if(mType.split('/').at(0) == "text")//如果邮件类型为文本类型，
    {
        QString str;
        QTextCodec *codec = QTextCodec::codecForName(headMap.value("charset"));
        if(codec == nullptr)
            str = content;
        else
        {
            str = codec->toUnicode(content);
        }
        this->plain = str;
        this->html = str;
    }
    else if(mType == "multipart/mixed")
    {
        mixPraser(content,headMap.value("boundary"));
    }
    else if(mType == "multipart/related")
    {
        relatedPraser(content,headMap.value("boundary"));
    }
    else if(mType == "multipart/alternative")
    {
        alternativePraser(content,headMap.value("boundary"));
    }
    else//其他情况以ascii码对待
    {
        this->plain = rawContent;
        this->html = rawContent;
    }
    if(!this->html.isEmpty())
    {
        //转换cid
        QMap<QString,QString>::const_iterator i = this->cidTOpath.constBegin();
        while (i != this->cidTOpath.constEnd())
        {
            QString cid = QString("cid:") + i.key();
            this->html.replace(cid,i.value());
        }
    }
}

//头部解析器将解析结果保存在一个QMap中，content—type的属性另外存储
//头部域的名字的每个单词都转换为小写
QMap<QByteArray, QByteArray> MailPraser::headPraser(QByteArray head)
{
    QMap<QByteArray,QByteArray> headMap;
    QByteArray tHead;
    for(int i = 0;i < head.size();)
    {
        //如果遇到回车换行
        if(head.at(i) == '\r' && head.at(i+1) == '\n')
        {
            if(i + 2 == head.size() || (head.at(i + 2) != ' ' && head.at(i + 2) != '\t'))
            {
                int colonIndex = tHead.indexOf(":");
                QByteArray field = tHead.left(colonIndex).toLower().trimmed();//头部域的名字的每个单词都转换为小写
                QByteArray value = tHead.right(tHead.size() - colonIndex - 1).trimmed();
                if(field == "content-type" || field == "content-disposition")
                {
                    int semicolonIndex = value.indexOf(";");
                    QByteArray val = value.left(semicolonIndex).trimmed();
                    QByteArray attr = value.right(value.size() - semicolonIndex - 1).trimmed();
                    headMap.insert(field,val);
                    int equalIndex = attr.indexOf("=");
                    QByteArray attrType = attr.left(equalIndex).trimmed();
                    QByteArray attrValue = attr.mid(equalIndex + 1,attr.indexOf(';') - equalIndex).trimmed();
                    if(attrValue.startsWith('\"'))
                    {
                        attrValue.remove(0,1);
                        attrValue.remove(attrValue.size() - 1,1);
                    }
                    headMap.insert(attrType,attrValue);
                }
                else
                {
                    headMap.insert(field,value);
                }
                tHead.clear();
            }
            i += 2;
        }
        else
        {
            tHead.append(head.at(i));
            i++;
        }
    }
    return headMap;
}

void MailPraser::mixPraser(QByteArray msg, QByteArray boundary)
{
    QList<QByteArray> subMsgList;
    cutMultipart(msg,boundary,subMsgList);
    for(int i = 0;i < subMsgList.size();i++)
    {
        QByteArray head,content;
        cutHeadAndContent(subMsgList.at(i),head,content);
        QMap<QByteArray,QByteArray> headMap = headPraser(head);
        if(headMap.value("content-type") == "multipart/mixed")
        {
            mixPraser(content,headMap.value("boundary"));
        }
        else if(headMap.value("content-type") == "multipart/alternative")
        {
            alternativePraser(content,headMap.value("boundary"));
        }
        else if(headMap.value("content-type") == "multipart/related")
        {
            relatedPraser(content,headMap.value("boundary"));
        }
        else if(headMap.value("content-type").startsWith("text"))
        {
            textPraser(content,headMap);
        }
        else
        {
            //其余的检查attachment属性，有视为附件，没有直接抛弃
            if(headMap.value("content-disposition").startsWith("attachment"))
            {
                QByteArray attCont = content;
                if(headMap.value("content-transfer-encoding") == "quoted-printable")
                {
                     attCont = MIME::Quoted_printableDecoder(content);
                }
                else if(headMap.value("content-transfer-encoding") == "base64")
                {
                    attCont = QByteArray::fromBase64(content);
                }
                this->attContList.append(attCont);
                QString fileName = MIME::HeadDecoding(headMap.value("filename"));
                if(fileName.isEmpty())
                    fileName = "未命名_" + QTime::currentTime().toString();
                this->attNameList.append(fileName);
            }
        }
    }
}

void MailPraser::relatedPraser(QByteArray msg, QByteArray boundary)
{
    QList<QByteArray> subMsgList;
    cutMultipart(msg,boundary,subMsgList);
    for(int i = 0;i < subMsgList.size();i++)
    {
        QByteArray head,content;
        cutHeadAndContent(subMsgList.at(i),head,content);
        QMap<QByteArray,QByteArray> headMap = headPraser(head);
        if(headMap.value("content-type") == "multipart/related")
        {
            relatedPraser(content,headMap.value("boundary"));
        }
        else if(headMap.value("content-type") == "multipart/alternative")
        {
            alternativePraser(content,headMap.value("boundary"));
        }
        else if(headMap.value("content-type").startsWith("text"))
        {
            textPraser(content,headMap);
        }
        else
        {
            //其余的视为内嵌文件，保存到缓存目录中，然后更新html中的cid为filepath
            //file:///home/yin/Doc
            //<img src="file://D:/我的下载/安装包/2.png"  width="112" height="112">
            if(headMap.value("content-disposition").startsWith("inline"))
            {
                QByteArray inlineCont = content;
                if(headMap.value("content-transfer-encoding") == "quoted-printable")
                {
                     inlineCont = MIME::Quoted_printableDecoder(content);
                }
                else if(headMap.value("content-transfer-encoding") == "base64")
                {
                    inlineCont = QByteArray::fromBase64(content);
                }
                QString cid = headMap.value("content-id");
                if(cid.startsWith('<') && cid.endsWith('>'));
                {
                    cid = cid.mid(1,cid.size()-2);
                }
                QString fileName = MIME::HeadDecoding(headMap.value("filename"));
                if(fileName.isEmpty())
                    fileName = "未命名_" + QTime::currentTime().toString();
                QString filePath = "cache/"+fileName;
                QDir dir;
                dir.mkpath(filePath);
                QFile file(filePath);
                file.open(QIODevice::WriteOnly);
                file.write(inlineCont);
                file.close();
                this->cidTOpath.insert(cid,filePath);
            }
        }
    }
}

void MailPraser::alternativePraser(QByteArray msg, QByteArray boundary)
{
    QList<QByteArray> subMsgList;
    cutMultipart(msg,boundary,subMsgList);
    for(int i = 0;i < subMsgList.size();i++)
    {
        QByteArray head,content;
        cutHeadAndContent(subMsgList.at(i),head,content);
        QMap<QByteArray,QByteArray> headMap = headPraser(head);
        if(headMap.value("content-type") == "multipart/alternative")
        {
            alternativePraser(content,headMap.value("boundary"));
        }
        else if(headMap.value("content-type").startsWith("text"))
        {
            textPraser(content,headMap);
        }
    }
}

void MailPraser::textPraser(QByteArray txt, QMap<QByteArray,QByteArray> &headMap)
{
    QByteArray content;
    if(headMap.value("content-transfer-encoding") == "quoted-printable")
    {
        content = MIME::Quoted_printableDecoder(txt);
    }
    else if(headMap.value("content-transfer-encoding") == "base64")
    {
        content = QByteArray::fromBase64(txt);
    }
    else
    {
        content = txt;
    }

    if(headMap.value("content-type") == "text/plain")
    {
        QTextCodec *codec = QTextCodec::codecForName(headMap.value("charset"));
        if(codec == nullptr)
            this->plain = content;
        this->plain = codec->toUnicode(content);
    }
    else if(headMap.value("content-type") == "text/html")
    {
        QTextCodec *codec = QTextCodec::codecForName(headMap.value("charset"));
        if(codec == nullptr)
            this->html = content;
        this->html = codec->toUnicode(content);
    }
}

void MailPraser::cutHeadAndContent(QByteArray msg,QByteArray &msgHead,QByteArray &msgContent)
{
    int splitIndex = msg.indexOf("\r\n\r\n");//搜索第一个空行
    msgHead = msg.left(splitIndex + 2);//要包含上回车换行
    msgContent = msg.mid(splitIndex + 4);//要去掉开头的空格
}

void MailPraser::cutMultipart(QByteArray msg, QByteArray boundary, QList<QByteArray> &subMsgList)
{
    QByteArray startBoundary;
    startBoundary.append("--");
    startBoundary.append(boundary);
    int boundaryIndex = msg.indexOf(startBoundary);
    int nextBoundaryIndex;
    for(;;)
    {
        nextBoundaryIndex = msg.indexOf(startBoundary,boundaryIndex + startBoundary.size());
        if(nextBoundaryIndex == -1)
        {
            break;
        }
        int subMsgLen = nextBoundaryIndex - boundaryIndex - startBoundary.size();
        QByteArray subMsg = msg.mid(boundaryIndex + startBoundary.size(),subMsgLen);
        subMsgList.append(subMsg.trimmed());
        if(msg.at(nextBoundaryIndex + startBoundary.size() +1) == '-' &&
                msg.at(nextBoundaryIndex + startBoundary.size() +1) == '-')
        {
            break;
        }
        boundaryIndex = nextBoundaryIndex;
    }
}

MIMEApplication::MIMEApplication(const QString filePath, int type, bool isAttachment)
{
    if(3 != type/10)
    {
        qDebug()<<"mimeapp type err"<<endl;
        return;
    }
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug()<<"mimeapp load error"<<endl;
        return;
    }
    QFileInfo fileInfo(filePath);
    QByteArray attr;
    attr.append("name=\"");
    attr.append(HeadEncoding(fileInfo.fileName().toUtf8(),MIME::UTF8,false));
    attr.append("\"");
    addHead("Content-Type",convertMIMEType(type) + "; " + attr);
    addHead("Content-Transfer-Encoding","base64");
    if(isAttachment)
    {
        attr.insert(0,"file");
        addHead("Content-Disposition",QByteArray("attachment;") + attr);
    }
    QByteArray rawCont = file.readAll().toBase64();
    for(int i = 0;i < rawCont.size();i ++)
    {
        this->content.append(rawCont.at(i));
        if((i % 76) == 0)
        {
            this->content.append("\r\n");
        }
    }
    this->content.append("\r\n");
}
