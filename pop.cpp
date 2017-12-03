#include "pop.h"

POP::POP(QObject *parent) : QObject(parent)
{
    this->isDebugMode = false;
}

void POP::setDebugMode(bool debug)
{
    this->isDebugMode = debug;
}

bool POP::connectToServer(const QString addr, int port, bool ssl)
{
    bool ok = false;
    QByteArray retBytes;
    this->tcpSocket.connectToHost(addr,port);
    ok = this->tcpSocket.waitForConnected();
    if(!ok)
    {
        qDebug()<<"pop can not connect ot server"<<endl;
        return false;
    }
    retBytes = POPRead();
    if(retBytes.isEmpty())
    {
        return false;
    }
    return true;
}

bool POP::login(const QString username, const QString password)
{
    QByteArray retBytes;
    QByteArray request;
    request.append("user");
    request.append(" ");
    request.append(username);
    POPWrite(request);
    retBytes = POPRead();
    if(retBytes.isEmpty())
    {
        qDebug()<<"pop username err"<<endl;
        return false;
    }
    request.clear();
    request.append("pass");
    request.append(" ");
    request.append(password.toUtf8());
    POPWrite(request);
    retBytes.clear();
    retBytes = POPRead();
    if(retBytes.isEmpty())
    {
        qDebug()<<"pop password err"<<endl;
        return false;
    }
    return true;
}

bool POP::list(int mailIndex,qint64 &mailSize)
{
    QByteArray retBytes;
    QByteArray request;
    request.append("list");
    request.append(" ");
    request.append(QString::number(mailIndex));
    POPWrite(request);
    retBytes = POPRead();
    if(retBytes.isEmpty())
    {
        qDebug()<<"pop list num error"<<endl;
        return false;
    }
    QList<QByteArray> retList = retBytes.split(' ');
    mailSize = retList.at(2).toLong();
    return true;
}

bool POP::list(QList<qint64> &mailSizeList)
{
    QByteArray retBytes;
    POPWrite("list");
    retBytes = POPRead("\r\n.\r\n");
    if(retBytes.isEmpty())
    {
        qDebug()<<"popo list err"<<endl;
        return false;
    }
    mailSizeList.append(0);//邮件从1开始
    QByteArray n;
    for(int i = 0;i < retBytes.size() - 1;)
    {
        if(retBytes.at(i) != '\r' || retBytes.at(i+1) != '\n')
        {
            n.append(retBytes.at(i));
            i++;
        }
        else
        {
            if(n.at(0) >= '0' && n.at(0) <= '9')
            {
                mailSizeList.append(n.split(' ').at(1).toLongLong());
            }
            n.clear();
            i += 2;
        }
    }
    return true;
}

bool POP::stat(int &count, qint64 &totalSize)
{
    QByteArray retBytes;
    POPWrite("stat");
    retBytes = POPRead();
    if(retBytes.isEmpty())
    {
        qDebug()<<"pop stat err"<<endl;
    }
    count = retBytes.split(' ').at(1).toInt();
    totalSize = retBytes.split(' ').at(2).toLong();
    return true;
}

bool POP::retr(int mailIndex, QByteArray &mail)
{
    QByteArray retBytes;
    QByteArray request;
    request.append("retr");
    request.append(" ");
    request.append(QString::number(mailIndex));
    POPWrite(request);
    retBytes = POPRead("\r\n.\r\n");
    if(retBytes.isEmpty())
    {
        qDebug()<<"pop retr error"<<endl;
        return false;
    }
    mail = retBytes;
    //删除开头和结尾的标示
    mail.remove(0,mail.indexOf("\r\n") + 2);
    mail.remove(mail.lastIndexOf("\r\n"),3);
    return true;
}

bool POP::dele(int mailIndex)
{
    QByteArray retBytes;
    QByteArray request;
    request.append("dele");
    request.append(" ");
    request.append(QString::number(mailIndex));
    POPWrite(request);
    retBytes = POPRead();
    if(retBytes.isEmpty())
    {
        qDebug()<<"pop uidl error"<<endl;
        return false;
    }
    return true;
}

bool POP::uidl(int mailIndex, QByteArray &uid)
{
    QByteArray retBytes;
    QByteArray request;
    request.append("uidl");
    request.append(" ");
    request.append(QString::number(mailIndex));
    POPWrite(request);
    retBytes = POPRead();
    if(retBytes.isEmpty())
    {
        qDebug()<<"pop uidl error"<<endl;
        return false;
    }
    uid = retBytes.split(' ').at(1);
    return true;
}

bool POP::rest()
{
    QByteArray retBytes;
    POPWrite("rest");
    retBytes = POPRead();
    if(retBytes.isEmpty())
    {
        qDebug()<<"pop rest error"<<endl;
        return false;
    }
    return true;
}

bool POP::top(const int mailIndex, const int lines, QByteArray &summary)
{
    QByteArray retBytes;
    QByteArray request;
    request.append("top");
    request.append(" ");
    request.append(QString::number(mailIndex));
    request.append(" ");
    request.append(QString::number(lines));
    POPWrite(request);
    retBytes = POPRead("\r\n.\r\n");
    if(retBytes.isEmpty())
    {
        qDebug()<<"pop rest error"<<endl;
        return false;
    }
    summary = retBytes;
    //删除开头和结尾的标示
    summary.remove(0,summary.indexOf("\r\n") + 2);
    summary.remove(summary.lastIndexOf("\r\n"),3);
    return true;
}

bool POP::noop()
{
    QByteArray retBytes;
    POPWrite("noop");
    retBytes = POPRead();
    if(retBytes.isEmpty())
    {
        qDebug()<<"pop noop error"<<endl;
        return false;
    }
    return true;
}

bool POP::quit()
{
    QByteArray retBytes;
    POPWrite("quit");
    retBytes = POPRead();
    if(retBytes.isEmpty())
    {
        qDebug()<<"pop quit error"<<endl;
        return false;
    }
    return true;
}

bool POP::POPWrite(const QByteArray block)
{
    QByteArray sendBlock;
    sendBlock.append(block);
    sendBlock.append("\r\n");
    if(isDebugMode)
    {
        qDebug()<<sendBlock<<endl;
    }
    this->tcpSocket.write(sendBlock);
    return true;
}

QByteArray POP::POPRead(const QByteArray endFlag)
{
    bool ok = false;
    QByteArray receive;
    while (1)
    {
        QByteArray block;
        ok = tcpSocket.waitForReadyRead();
        if(!ok)
        {
            qDebug()<<"pop timeout"<<endl;
            break;
        }
        block.resize(tcpSocket.bytesAvailable());
        tcpSocket.read(block.data(),block.size());
        receive.append(block);
        if(receive.endsWith(endFlag))
        {
            break;
        }
    }
    if(isDebugMode)
    {
        qDebug()<<receive<<endl;
    }
    if(receive.split(' ').at(0) != REQ_OK)
    {
        qDebug()<<"pop return not ok"<<endl;
        return QByteArray();
    }
    return receive;
}
