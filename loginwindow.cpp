#include "loginwindow.h"
#include "ui_loginwindow.h"
#include "userlist.h"
#include <QTcpSocket>
#include <QDebug>

QTcpSocket *client = NULL;


LoginWindow::LoginWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginWindow)
{
    ui->setupUi(this);


    //登录按钮的槽函数
    connect(ui->loginButton,&QPushButton::clicked,[=](){
      QString inputUserName = ui->userName->text();
      if(inputUserName.isEmpty())
      {
          qDebug()<< "请输入用户名";
          return;
      }


      QString inputPassword = ui->password->text();
      if(inputPassword.isEmpty())
      {
          qDebug()<< "请输入密码";
          return;
      }

      client = new QTcpSocket;

        //设置服务器的信息
      client->connectToHost("47.98.58.0",7890);

      QString msg = "login:" + inputUserName +"\r\n\r\n" + inputPassword;

      client->write (msg.toUtf8().data());

      //等待服务器回复（等待3S）
      if(client->waitForReadyRead(3000))
      {
          QByteArray feedback = client->readAll();
          if(feedback == "true")
          {

            UserList *userList = new UserList(0,inputUserName);
            userList->show();
            this->close();
          }
          else if(feedback == "false")
          {
            qDebug() << "密码错误";
            client->disconnectFromHost();
          }
          else
          {
             qDebug() << "错误的反馈" << feedback;
             client->disconnectFromHost();
          }
      }
      else
      {
          qDebug() << "服务器无响应";
          client->disconnectFromHost();
      }

    });



    //退出按钮的槽函数
    connect(ui->quitButton,&QPushButton::clicked,[=](){
        this->close();
    });
}

LoginWindow::~LoginWindow()
{
    delete ui;
}
