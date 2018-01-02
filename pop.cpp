#include "pop.h"
#include <QSslError>
#include <QSslConfiguration>

PCommend::PCommend(int cmd, QList<QByteArray> para)
{
    this->cmdType = cmd;
    this->cmdParaList = para;
    this->cmdContent.append(POP::POPCMDConvert(cmd));
    for(auto p : para)
    {
        this->cmdContent.append(" ");
        this->cmdContent.append(p);
    }
}

PCommend::PCommend(const PCommend &cmd)
{
    this->cmdContent = cmd.cmdContent;
    this->cmdType = cmd.cmdType;
    this->cmdParaList = cmd.cmdParaList;
}


POP::POP(QObject *parent) : QObject(parent)
{
    this->tcpSocket = new QTcpSocket();
    connect(this,SIGNAL(sendCmd()),this,SLOT(slotSend()));
    connect(this->tcpSocket,SIGNAL(readyRead()),this,SLOT(slotReceive()));
    connect(this->tcpSocket,SIGNAL(connected()),this,SLOT(slotConnedted()));
    connect(this->tcpSocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(slotError()));
    this->sslSocket = new QSslSocket(this);
    QSslConfiguration config;
    config.setPeerVerifyMode(QSslSocket::VerifyNone);
    config.setProtocol(QSsl::TlsV1_0);
    this->sslSocket->setSslConfiguration(config);
    connect(this->sslSocket, SIGNAL(encrypted()), this, SLOT(slotConnedted()));
    connect(this->sslSocket,SIGNAL(readyRead()),this,SLOT(slotReceive()));
    connect(this->sslSocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(slotError()));
    connect(this->sslSocket,SIGNAL(sslErrors(QList<QSslError>)),this,SLOT(slotError()));
}

POP::~POP()
{
}

void POP::setDebugMode(bool debug)
{
    this->isDebugMode = debug;
}

void POP::connectToServer(const QString addr, uint port, bool ssl)
{
    this->popAddr = addr;
    this->port = port;
    this->ssl = ssl;
    QList<QByteArray> para;
    PCommend cmd(POP::CONNECT,para);
    this->POPCMDList.append(cmd);
    emit sendCmd();
}

void POP::user(const QString userName)
{
    QList<QByteArray> para;
    para.append(userName.toUtf8());
    PCommend cmd(POP::USER,para);
    this->POPCMDList.append(cmd);
    emit sendCmd();
}

void POP::pass(const QString passwd)
{
    QList<QByteArray> para;
    para.append(passwd.toUtf8());
    PCommend cmd(POP::PASS,para);
    this->POPCMDList.append(cmd);
    emit sendCmd();
}

void POP::list(int mailIndex)
{
    QList<QByteArray> para;
    if(mailIndex != 0)
    {
        para.append(QString::number(mailIndex).toUtf8());
    }
    PCommend cmd(POP::LIST,para);
    this->POPCMDList.append(cmd);
    emit sendCmd();
}

void POP::stat()
{
    QList<QByteArray> para;
    PCommend cmd(POP::STAT,para);
    this->POPCMDList.append(cmd);
    emit sendCmd();
}

void POP::retr(int mailIndex)
{
    if(mailIndex < 1)
        return;
    QList<QByteArray> para;
    para.append(QByteArray::number(mailIndex));
    PCommend cmd(POP::RETR,para);
    this->POPCMDList.append(cmd);
    emit sendCmd();
}

void POP::dele(int mailIndex)
{
    QList<QByteArray> para;
    para.append(QByteArray::number(mailIndex));
    PCommend cmd(POP::DELE,para);
    this->POPCMDList.append(cmd);
    emit sendCmd();
}

void POP::uidl(int mailIndex)
{
    QList<QByteArray> para;
    para.append(QByteArray::number(mailIndex));
    PCommend cmd(POP::UIDL,para);
    this->POPCMDList.append(cmd);
    emit sendCmd();
}

void POP::rest()
{
    QList<QByteArray> para;
    PCommend cmd(POP::REST,para);
    this->POPCMDList.append(cmd);
    emit sendCmd();
}

void POP::top(int mailIndex, int lines)
{
    QList<QByteArray> para;
    para.append(QByteArray::number(mailIndex));
    para.append(QByteArray::number(lines));
    PCommend cmd(POP::TOP,para);
    this->POPCMDList.append(cmd);
    emit sendCmd();
}

void POP::noop()
{
    QList<QByteArray> para;
    PCommend cmd(POP::NOOP,para);
    this->POPCMDList.append(cmd);
    emit sendCmd();
}

void POP::quit()
{
    QList<QByteArray> para;
    PCommend cmd(POP::QUIT,para);
    this->POPCMDList.append(cmd);
    emit sendCmd();
}

void POP::slotSend()
{
    if(this->processing)
    {
        //正在处理命令,直接返回
        return;
    }
    this->processing = true;
    bool hasSendCmd = false;
    while(!hasSendCmd)
    {
        if(this->POPCMDList.isEmpty())
        {
            this->processing = false;
            emit down(false);
            return;
        }
        this->curCmd = this->POPCMDList.first();
        this->POPCMDList.pop_front();
        if(curState == POPState::Offline)
        {
            //离线状态下可以执行的命令
            int cmd = this->curCmd.cmdType;
            if(cmd == POPCMD::CONNECT)
            {
                if(this->ssl)
                {
                    if(!this->sslSocket->isEncrypted())
                    {
                        this->sslSocket->connectToHostEncrypted(this->popAddr,this->port);
                        hasSendCmd = true;
                    }
                }
                else
                {
                    if(this->tcpSocket->state() == QAbstractSocket::UnconnectedState)
                    {
                        this->tcpSocket->connectToHost(this->popAddr,this->port);
                        hasSendCmd = true;
                    }
                }
            }
        }
        else if(curState == POPState::Connected)
        {
            //连接状态下可以执行的命令
            int cmd = this->curCmd.cmdType;
            if(cmd == POPCMD::USER || cmd == POPCMD::PASS)
            {
                POPWrite(curCmd.cmdContent);
                hasSendCmd = true;
            }
        }
        else
        {
            //登录状态下可以执行的命令
            int cmd = this->curCmd.cmdType;
            if(cmd != POPCMD::CONNECT && cmd != POPCMD::USER && cmd != POPCMD::PASS)
            {
                POPWrite(curCmd.cmdContent);
                hasSendCmd = true;
            }
        }
    }
}

void POP::slotReceive()
{
    QByteArray receive;
    if(this->ssl)
    {
        receive.resize(this->sslSocket->bytesAvailable());
        this->sslSocket->read(receive.data(),receive.size());
    }
    else
    {
        receive= this->tcpSocket->readAll();
    }
    this->receiveBuf.append(receive);
    QByteArray endFlag;
    if(curCmd.cmdType < POP::RETR)
    {
        //返回值以回车结尾的命令
        endFlag = "\r\n";
    }
    else
    {
        //返回值以.结尾的命令
        endFlag = "\r\n.\r\n";
    }
    if(!this->receiveBuf.endsWith(endFlag))
    {
        //读取尚未完成
        return;
    }
    if(isDebugMode)
    {
        qDebug()<<this->receiveBuf<<endl;
    }
    //结果分析
    if(!this->receiveBuf.startsWith(REQ_OK))
    {
        //返回值错误
        this->errorString = this->returnValue;
        this->POPCMDList.clear();
        emit error(POPError::RetError);
    }
    //修饰服务器返回值
    this->returnValue = this->receiveBuf;
    if(curCmd.cmdType < POP::RETR)
    {
        //返回值以回车结尾的命令
        this->returnValue.remove(0,4);
        this->returnValue.remove(this->returnValue.size()-2,2);
    }
    else
    {
        //返回值以.结尾的命令
        this->returnValue.remove(0,returnValue.indexOf("\r\n") + 2);
        this->returnValue.remove(this->returnValue.size()-3,3);
    }
    switch (this->curCmd.cmdType) {
    case TOP:
        emit topFinish(this->curCmd.cmdParaList.at(0).toInt(),this->returnValue);
        break;
    case UIDL:
    {
        uint index = this->returnValue.left(returnValue.indexOf(' ')).toUInt();
        QByteArray uid = this->returnValue.right(returnValue.size() - returnValue.indexOf(' ') - 1);
        emit uidlFinish(index,uid);
        break;
    }
    case RETR:
        emit retrFinish(this->curCmd.cmdParaList.at(0).toInt(),this->returnValue);
        break;
    case STAT:
    {
        uint index = this->returnValue.left(returnValue.indexOf(' ')).toUInt();
        qint64 totalSize = this->returnValue.right(returnValue.size() - returnValue.indexOf(' ') - 1).toLong();
        emit statFinish(index,totalSize);
        break;
    }
    case PASS:
        this->curState = POPState::Logged;
        break;
    case QUIT:
        this->curState = POPState::Offline;
        break;
    default:
        break;
    }
    this->receiveBuf.clear();
    this->processing = false;
    emit sendCmd();
}

void POP::slotConnedted()
{
    if(isDebugMode)
        qDebug()<<"connected to pop"<<endl;
    this->curState = POPState::Connected;
    emit sendCmd();
}

void POP::slotError()
{
    if(this->curState == POPState::Offline)
    {
        //离线状态不报错
        return;
    }
    this->POPCMDList.clear();
    if(this->ssl)
        this->errorString = this->sslSocket->errorString();
    else
        this->errorString = tcpSocket->errorString();
    qDebug()<<this->sslSocket->sslErrors().at(0).errorString()<<endl;
    this->curState = Offline;
    emit error(POPError::TCPError);
}

void POP::POPWrite(QByteArray block)
{
    QByteArray sendBlock;
    sendBlock.append(block);
    sendBlock.append("\r\n");
    if(isDebugMode)
    {
        qDebug()<<sendBlock<<endl;
    }
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
        this->POPCMDList.clear();
        emit error(POPError::TCPError);
    }
}

QByteArray POP::POPCMDConvert(int type)
{
    QByteArray cmdByte;
    switch (type) {
    case POP::USER:
        cmdByte = "user";
        break;
    case POP::PASS:
        cmdByte = "pass";
        break;
    case POP::LIST:
        cmdByte = "list";
        break;
    case POP::STAT:
        cmdByte = "stat";
        break;
    case POP::RETR:
        cmdByte = "retr";
        break;
    case POP::DELE:
        cmdByte = "dele";
        break;
    case POP::UIDL:
        cmdByte = "uidl";
        break;
    case POP::REST:
        cmdByte = "rest";
        break;
    case POP::TOP:
        cmdByte = "top";
        break;
    case POP::NOOP:
        cmdByte = "noop";
        break;
    case POP::QUIT:
        cmdByte = "quit";
        break;
    default:
        break;
    }
    return cmdByte;
}
