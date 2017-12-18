#include "mainwindow.h"
#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFile theme(":/theme/write.qss");
    theme.open(QIODevice::ReadOnly);
    qApp->setStyleSheet(theme.readAll());
    MainWindow w;
    w.show();

    return a.exec();
}
