#include "searchwidget.h"
#include "ui_searchwidget.h"

SearchWidget::SearchWidget(User *user,QString &keyword, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SearchWidget)
{
    ui->setupUi(this);
    this->user = user;
    search(keyword);
}

SearchWidget::~SearchWidget()
{
    delete ui;
}

void SearchWidget::search(QString &keyword)
{

}
