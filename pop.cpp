#include "pop.h"

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
}

POP::~POP()
{
    delete this->tcpSocket;
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
    if(this->POPCMDList.isEmpty())
    {
        emit down(false);
        return;
    }
    this->processing = true;
    this->curCmd = this->POPCMDList.first();
    this->POPCMDList.pop_front();
    if(curCmd.cmdType == POPCMD::CONNECT)
    {
        this->tcpSocket->connectToHost(this->popAddr,this->port);
    }
    else
    {
        QByteArray sendBlock;
        sendBlock.append(this->curCmd.cmdContent);
        sendBlock.append("\r\n");
        if(isDebugMode)
        {
            qDebug()<<sendBlock<<endl;
        }
        qint64 byteNum = this->tcpSocket->write(sendBlock);
        if(!byteNum)
        {
            this->errorString = "can't write tcp";
            this->POPCMDList.clear();
            emit error(POPError::TCPError);
        }
    }
}

void POP::slotReceive()
{
    QByteArray receive = this->tcpSocket->readAll();
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
        emit uidlFinish(this->returnValue.split(' ').at(0).toUInt(),this->returnValue.split(' ').at(1));
        break;
    case RETR:
        emit retrFinish(this->curCmd.cmdParaList.at(0).toInt(),this->returnValue);
        break;
    case STAT:
        emit statFinish(this->returnValue.split(' ').at(0).toUInt(),this->returnValue.split(' ').at(1).toLong());
        break;
    case QUIT:
        this->isConnected = false;
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
    this->isConnected = true;
}

void POP::slotError()
{
    this->POPCMDList.clear();
    this->errorString = this->tcpSocket->errorString();
    this->isConnected = false;
    emit error(POPError::TCPError);
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
