#include "smtp.h"

SMTP::SMTP(QObject *parent)
    : QObject(parent)
{
    connect(this,SIGNAL(sendCmd()),this,SLOT(slotSend()));
    this->tcpSocket = new QTcpSocket(this);
    connect(this->tcpSocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(slotError()));
    connect(this->tcpSocket,SIGNAL(connected()),this,SLOT(slotConnedted()));
    connect(this->tcpSocket,SIGNAL(readyRead()),this,SLOT(slotReceive()));
    this->sslSocket = new QSslSocket(this);
    connect(this->sslSocket,SIGNAL(encrypted()),this,SLOT(slotConnedted()));
    connect(this->sslSocket,SIGNAL(readyRead()),this,SLOT(slotReceive()));
    connect(this->sslSocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(slotError()));
}

void SMTP::setServerAddr(const QString &smtpServer, int port, bool ssl)
{
    this->smtpServer = smtpServer;
    this->port = port;
    this->ssl = ssl;
}

void SMTP::setLoginInfo(const QString &userName, const QString &passwd)
{
    this->uName = userName.toUtf8();
    this->uPass = passwd.toUtf8();
}

void SMTP::sendMail(const QList<QString> receiveList, const QByteArray &mail)
{
    this->SMTPCMDList.append(SCommend(SMTPCMD::CONNECT,SMTPRetCode::ServiceReady,"connect"));
    this->SMTPCMDList.append(SCommend(SMTPCMD::HELO,SMTPRetCode::ReqCompleted,"localhost"));
    this->SMTPCMDList.append(SCommend(SMTPCMD::AUTH_LOGIN,SMTPRetCode::InputLine));
    this->SMTPCMDList.append(SCommend(SMTPCMD::_USERNAME,SMTPRetCode::InputLine,this->uName.toBase64()));
    this->SMTPCMDList.append(SCommend(SMTPCMD::_PASSWORD,SMTPRetCode::AuthThrough,this->uPass.toBase64()));
    for(auto u : receiveList)
    {
        this->SMTPCMDList.append(SCommend(SMTPCMD::MAIL_FROM,SMTPRetCode::ReqCompleted,this->uName));
        this->SMTPCMDList.append(SCommend(SMTPCMD::RCPT_TO,SMTPRetCode::ReqCompleted,u.toUtf8()));
        this->SMTPCMDList.append(SCommend(SMTPCMD::DATA,SMTPRetCode::InputMulti));
        QByteArray m = mail + "\r\n.";
        this->SMTPCMDList.append(SCommend(SMTPCMD::_MAIL,SMTPRetCode::ReqCompleted,m));
    }
    this->SMTPCMDList.append(SCommend(SMTPCMD::QUIT,SMTPRetCode::ServiceClose));
    emit sendCmd();
}

QByteArray SMTP::SMTPCMDConvert(int type)
{
    QByteArray retByteArray;
    switch(type)
    {
    case HELO:
        retByteArray = "HELO ";
        break;
    case AUTH_LOGIN:
        retByteArray = "AUTH LOGIN ";
        break;
    case _USERNAME:
        break;
    case _PASSWORD:
        break;
    case MAIL_FROM:
        retByteArray = "MAIL FROM: ";
        break;
    case RCPT_TO:
        retByteArray = "RCPT TO: ";
        break;
    case DATA:
        retByteArray = "DATA";
        break;
    case _MAIL:
        break;
    case QUIT:
        retByteArray = "QUIT";
        break;
    default:
        break;
    }
    return retByteArray;
}

void SMTP::slotSend()
{
    if(isProcessing)
    {
        return;
    }
    if(this->SMTPCMDList.isEmpty())
    {
        emit down(false);
        return;
    }
    this->isProcessing = true;
    this->curCMD = this->SMTPCMDList.at(0);
    this->SMTPCMDList.pop_front();
    QByteArray sendBlock;
    sendBlock.append(this->curCMD.cmdContent);
    sendBlock.append("\r\n");
    if(this->debugMode)
    {
        qDebug()<<sendBlock<<endl;
    }
    if(curCMD.cmdType == SMTPCMD::CONNECT)
    {
        if(this->ssl)
        {
            this->sslSocket->connectToHostEncrypted(this->smtpServer,this->port);
        }
        else
        {
            this->tcpSocket->connectToHost(this->smtpServer,this->port);
        }
    }
    else
    {
        qint64 byteNum;
        if(this->ssl)
        {
            byteNum = this->sslSocket->write(sendBlock);
        }
        else
        {
            byteNum = this->tcpSocket->write(sendBlock);
        }
        if(!byteNum)
        {
            this->errorString = "can't write tcp";
            this->SMTPCMDList.clear();
            this->receiveBuf.clear();
            emit error(SMTPError::TCPError);
        }
    }
}

void SMTP::slotReceive()
{
    QByteArray buf;
    if(this->ssl)
    {
        buf = this->sslSocket->readAll();
    }
    else
    {
        buf = this->tcpSocket->readAll();
    }
    this->receiveBuf.append(buf);
    if(!receiveBuf.endsWith("\r\n"))
    {
        //不以回车换行结尾的说明没接受完毕
        return;
    }
    if(debugMode)
    {
        qDebug()<<this->receiveBuf<<endl;
    }
    bool ok = false;
    uint retCode = receiveBuf.left(3).toUInt(&ok);
    if(!ok || retCode != curCMD.cmdRightReturnCode)
    {
        //发信错误，注意群发邮件时的处理
        this->errorString = receiveBuf;
        emit error(SMTPError::RetCodeError);
        if(curCMD.cmdType != SMTPCMD::_MAIL)
        {
            this->SMTPCMDList.clear();
            this->receiveBuf.clear();
            return;
        }
    }
    this->receiveBuf.clear();
    this->isProcessing = false;
    emit sendCmd();
}

void SMTP::slotConnedted()
{
    if(debugMode)
    {
        qDebug()<<"connected to smtp"<<endl;
    }
    this->isConnected = true;
    emit sendCmd();
}

void SMTP::slotError()
{
    this->isConnected = false;
    this->SMTPCMDList.clear();
    this->receiveBuf.clear();
    if(this->ssl)
        this->errorString = this->sslSocket->errorString();
    else
        this->errorString = tcpSocket->errorString();
    emit error(SMTPError::TCPError);
}

SCommend::SCommend(uint cmd, uint retCode, QByteArray para)
{
    this->cmdType = cmd;
    this->cmdRightReturnCode = retCode;
    this->cmdContent.append(SMTP::SMTPCMDConvert(cmd));
    if(cmd == SMTP::MAIL_FROM || cmd == SMTP::RCPT_TO)
    {
        QByteArray p;
        p.append('<');
        p.append(para);
        p.append('>');
        this->cmdContent.append(p);
    }
    else
    {
        this->cmdContent.append(para);
    }

}
