#include "smtp.h"

SMTP::SMTP(QObject *parent)
    : QObject(parent)
{
    debugMode = false;
    connect(&tcpSocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SIGNAL(error()));
}

//如果Debug模式开启，则向控制台输出与服务器之间交互的全部信息。
void SMTP::setDebugMode(bool debug)
{
    this->debugMode = debug;
}

bool SMTP::connectServer(const QString &smtpServer, int port, bool ssl)
{
    bool ok = false;
    tcpSocket.connectToHost(smtpServer,port);
    ok = tcpSocket.waitForConnected();
    if(!ok)
    {
        qDebug()<<"connect error"<<endl;
        return false;
    }
    int retCode = SMTPRead();
    if(retCode != SERVICE_READY)
    {
       qDebug()<<"service open error"<<endl;
       return false;
    }
    return true;
}

bool SMTP::login(const QString &username,const QString &password)
{
    int retCode = 0;
    SMTPWrite("helo localhost");
    retCode = SMTPRead();
    if(retCode != REQUEST_OK)
    {
        qDebug()<<"helo error"<<endl;
        return false;
    }
    retCode = 0;
    SMTPWrite("auth login");
    retCode = SMTPRead();
    if(retCode != WAIT_INPUT)
    {
        qDebug()<<"input error"<<endl;
        return false;
    }
    retCode = 0;
    SMTPWrite(username.toUtf8().toBase64());
    retCode = SMTPRead();
    if(retCode != WAIT_INPUT)
    {
        qDebug()<<"input error"<<endl;
        return false;
    }
    retCode = 0;
    SMTPWrite(password.toUtf8().toBase64());
    retCode = SMTPRead();
    if(retCode != LOGIN_SUCCESS)
    {
        qDebug()<<"login error"<<endl;
        return false;
    }
    return true;
}

bool SMTP::sendMail(const QString &sender, const QStringList &receivers, const QByteArray &msg)
{
    QByteArray block;
    int retCode = 0;

    //注意添加< >
    block.append("mail from: <");
    block.append(sender.toUtf8());
    block.append(">");
    SMTPWrite(block);
    retCode = SMTPRead();
    if(retCode != REQUEST_OK)
    {
        qDebug()<<"send mail error 1"<<endl;
        return false;
    }
    block.clear();
    retCode = 0;

    block.append("rcpt to: ");
    for(auto re : receivers)
    {
        block.append("<");
        block.append(re + "> ");//每个接受者后面都加一个空格
    }
    SMTPWrite(block);
    retCode = SMTPRead();
    if(retCode != REQUEST_OK)
    {
        qDebug()<<"send mail error 2"<<endl;
        return false;
    }
    block.clear();
    retCode = 0;

    block.append("data");
    SMTPWrite(block);
    retCode = SMTPRead();
    if(retCode != START_MAIL_INPUT)
    {
        qDebug()<<"send mail error 3"<<endl;
        return false;
    }
    block.clear();
    retCode = 0;

    block = msg;
    SMTPWrite(msg);
    SMTPWrite(".");//发送正文结束标志
    retCode = SMTPRead();
    if(retCode != REQUEST_OK)
    {
        qDebug()<<"send mail error 4"<<endl;
        return false;
    }
    return true;
}

bool SMTP::quit()
{
    SMTPWrite("quit");
    int retCode = SMTPRead();
    if(retCode != SERVICE_CLOSE)
    {
       qDebug()<<"server close error"<<endl;
       return false;
    }
    return true;
}

QString SMTP::errorInfo() const
{
    return tcpSocket.errorString();
}


int SMTP::SMTPRead()
{
    bool ok = false;
    QByteArray block;
    ok = tcpSocket.waitForReadyRead();
    if(!ok)
    {
        qDebug()<<"read error"<<endl;
        return -1;
    }
    block.resize(tcpSocket.bytesAvailable());
    tcpSocket.read(block.data(),block.size());
    if(debugMode)
    {
        qDebug()<<block<<endl;
    }
    int retCode = block.left(3).toInt(&ok);
    if(!ok)
    {
        qDebug()<<"return code read error"<<endl;
        return -1;
    }
    return retCode;
}

void SMTP::SMTPWrite(const QByteArray block)
{
    QByteArray sendData = block + "\r\n";
    if(debugMode)
    {
        qDebug()<<sendData<<endl;
    }
    tcpSocket.write(sendData);
}

