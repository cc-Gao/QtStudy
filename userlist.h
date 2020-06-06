#ifndef USERLIST_H
#define USERLIST_H

#include <QWidget>
#include <QToolButton>
#include "widget.h"


typedef struct isopen
{
    QToolButton * p_Toolbtn;
    Widget *p_wight;
    QString userName;
    bool isShown;
}WindowOpenMessage;


namespace Ui {
class UserList;
}

class UserList : public QWidget
{
    Q_OBJECT

public:
    explicit UserList(QWidget *parent,QString userName);
    ~UserList();

    QVector <WindowOpenMessage *> widgetMessage;

private:
    Ui::UserList *ui;
};

#endif // USERLIST_H
