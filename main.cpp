#include <QApplication>
#include "loginwindow.h"
#include "widget.h"
#include "userlist.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    LoginWindow login;
    login.show();

//    UserList *userList = new UserList(0,NULL);
//    userList->show();

    return a.exec();
}

