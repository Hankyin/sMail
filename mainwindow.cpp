#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "maileditwidget.h"
#include "logindialog.h"
#include "smaildb.h"
#include <QByteArray>
#include <QDebug>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QHBoxLayout>
#include <QFile>
#include <QFileDialog>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QWebView>
#include <QMessageBox>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //构造函数中只处理窗体UI方面的东西，数据处理放在slotAfterShow中。应为无法在构造函数中退出整个程序
    ui->setupUi(this);
    ui->splitter->setStretchFactor(1,1);

    connect(ui->dirView,SIGNAL(clicked(QModelIndex)),this,SLOT(slotChangeDir(QModelIndex)));
    connect(ui->mailView,SIGNAL(clicked(QModelIndex)),this,SLOT(slotReadMail(QModelIndex)));
    connect(ui->btWriteMail,SIGNAL(clicked(bool)),this,SLOT(slotWriteMail()));
    connect(ui->btRefresh,SIGNAL(clicked(bool)),this,SLOT(slotRefreshMail()));
    QTimer::singleShot(500,this,SLOT(slotAfterShow()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slotWriteMail()
{
    this->curUser->sendMail();
}

void MainWindow::slotChangeDir(QModelIndex index)
{
    QString dirName = index.data().toString();
    this->curUser->changeDir(dirName);
}

void MainWindow::slotReadMail(QModelIndex index)
{
    this->curUser->readMail(index);
}

void MainWindow::slotRefreshMail()
{
    this->curUser->refreshMailList();
}

void MainWindow::slotAfterShow()
{
    this->db = new sMailDB(this);
    this->userList = this->db->getUserList();
    if(this->userList.isEmpty())
    {
        this->close();
    }
    this->curUser = this->userList.at(0);

    ui->dirView->setModel(this->curUser->getDirModel());
    ui->mailView->setModel(this->curUser->getMailModel());



    ui->mailView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->mailView->setSelectionMode ( QAbstractItemView::SingleSelection);
    ui->mailView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->mailView->setAlternatingRowColors(true);
    ui->mailView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->mailView->horizontalHeader()->setStretchLastSection(true);

    ui->dirView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->dirView->setSelectionMode ( QAbstractItemView::SingleSelection);

    slotRefreshMail();
}
