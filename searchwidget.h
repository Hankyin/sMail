#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QWidget>

namespace Ui {
class SearchWidget;
}

class User;

class SearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SearchWidget(User *user, QString &keyword, QWidget *parent = 0);
    ~SearchWidget();

private:
    Ui::SearchWidget *ui;
    User *user;
    void search(QString &keyword);
};

#endif // SEARCHWIDGET_H
