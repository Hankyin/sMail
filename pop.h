#ifndef POP_H
#define POP_H

#include <QObject>
#include <QTcpSocket>
#include <QStringList>
#include <QList>
#include <QByteArray>

class POP : public QObject
{
    Q_OBJECT
public:
    explicit POP(QObject *parent = nullptr);
    void setDebugMode(bool debug);
    bool connectToServer(const QString addr, int port = 110, bool ssl = false);
    bool login(const QString username, const QString password);
    bool list(int mailIndex, qint64 &mailSize);
    bool list(QList<qint64> &mailSizeList);
    bool stat(int& count, qint64 &totalSize);
    bool retr(int mailIndex, QByteArray &mail);
    bool dele(int mailIndex);
    bool uidl(int mailIndex,QByteArray &uid);
    bool rest();
    bool top(const int mailIndex, const int lines, QByteArray &summary);
    bool noop();
    bool quit();
    const QByteArray REQ_OK = "+OK";
signals:

public slots:

private:
    QTcpSocket tcpSocket;
    bool isDebugMode;
    bool POPWrite(const QByteArray block);
    QByteArray POPRead(const QByteArray endFlag = QByteArray("\r\n"));
};

#endif // POP_H
