#ifndef POP_H
#define POP_H

#include <QObject>
#include <QTcpSocket>
#include <QSslSocket>
#include <QStringList>
#include <QList>
#include <QByteArray>

class PCommend
{
public:
    PCommend() {}
    PCommend(int cmd,QList<QByteArray> para = QList<QByteArray>());
    PCommend(const PCommend &cmd);
    int cmdType;
    QList<QByteArray> cmdParaList;
    QByteArray cmdContent;
};

class POP : public QObject
{
    Q_OBJECT
public:
    enum POPCMD
    {
        CONNECT,
        USER,
        PASS,
        STAT,
        DELE,
        UIDL,
        REST,
        NOOP,
        QUIT,
        //下面这两个的返回值以.结尾，上面的以回车结尾
        RETR,
        TOP,
        LIST,
    };
    enum POPError
    {
        TCPError,
        RetError,
    };
    enum POPState
    {
        Offline,
        Connected,
        Logged,
    };

    explicit POP(QObject *parent = nullptr);
    ~POP();
    void setDebugMode(bool debug);
    void connectToServer(const QString addr, uint port = 110, bool ssl = false);
    void user(const QString userName);
    void pass(const QString passwd);
    void list(int mailIndex = 0);
    void stat();
    void retr(int mailIndex);
    void dele(int mailIndex);
    void uidl(int mailIndex);
    void rest();
    void top(int mailIndex,int lines);
    void noop();
    void quit();
    QString& getErrorString(){return errorString;}
    const QByteArray REQ_OK = "+OK";

    static QByteArray POPCMDConvert(int type);
signals:
    void sendCmd();
    void down(bool err);
    //这个地方传引用可能导致问题，如果用引用，传出的retValue就是这个对象的returnValue，
    //假设网速很快或其他原因，外部对象还没有处理这个信号发出的retValue，returnValue就被新的返回值覆盖，这样就会出错。
    //是这样么
    void topFinish(int mailId,QByteArray head);
    void uidlFinish(int mailId,QByteArray uid);
    void retrFinish(int mailId,QByteArray mail);
    void statFinish(uint count,qint64 totalSize);
    void error(POPError errorType);
public slots:
    void slotSend();
    void slotReceive();
    void slotConnedted();
    void slotError();
private:
    QTcpSocket *tcpSocket;
    QSslSocket *sslSocket;
    bool isDebugMode = false;
    bool processing = false;
    QString popAddr;
    uint port;
    bool ssl = false;
    QList<PCommend> POPCMDList;
    PCommend curCmd;
    int curState = Offline;
    QByteArray receiveBuf;
    QByteArray returnValue;
    QString errorString;
    void POPWrite(QByteArray block);

};

#endif // POP_H
