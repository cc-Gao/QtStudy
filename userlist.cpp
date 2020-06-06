#include <QToolButton>
#include <QDebug>
#include <QTcpSocket>
#include "userlist.h"
#include "ui_userlist.h"
#include "widget.h"

extern QTcpSocket *client;
//在线用户列表
UserList::UserList(QWidget *parent,QString userName) :
    QWidget(parent),
    ui(new Ui::UserList)
{
    ui->setupUi(this);

    //设置标题（后续利用用户名来进行更改）
    setWindowTitle("MyQQ username:"+userName);
    //设置图标  目前暂无
    //setWindowIcon();


    //向服务器请求在线信息
    QString request = "request:"+ userName + "\r\n\r\n";

    client->write(request.toUtf8().data());
    //    分析服务器返回的在线信息
    //    等待服务器回复（等待3S）
    QByteArray feedback;

    if(client->waitForReadyRead(3000))
    {
        feedback = client->readAll();
    }
    else
    {
        qDebug() << "服务器无响应";
        client->disconnectFromHost();
        this->close();
    }


    QString onlineUserList(feedback);

    int endIndex = onlineUserList.indexOf("\r\n\r\n");
    int nowIndex = onlineUserList.indexOf("\r\n");
    int nextIndex = 0;
    //建立对应的toolputton
    while(nowIndex!= endIndex)
    {
        //添加新用户
        QToolButton *btn = new QToolButton;
        //设置名称
        nextIndex = onlineUserList.indexOf("\r\n",nowIndex+2);
        QString name = onlineUserList.mid(nowIndex+2,nextIndex-nowIndex-2);
        nowIndex = nextIndex;
        qDebug() << name;
        btn->setText(name);

        //设置图像（随机生成）
        //btn1->setIcon();
        //btn1->setIconSize();


        //设置按钮风格
        btn->setAutoRaise(true);
        btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        btn->setBaseSize(100,30);

        ui->vLayout->addWidget(btn);


        WindowOpenMessage * window = new WindowOpenMessage;
        window->userName = btn->text();
        window->isShown = false;
        window->p_Toolbtn = btn;
        window->p_wight = NULL;
        widgetMessage.push_back(window);

        //建立私聊窗口的槽函数
        connect(btn,&QToolButton::clicked,[=]()
        {

            for (int i =0;i<widgetMessage.size();i++)
            {
                if(widgetMessage.at(i)->userName == btn->text())
                {
                    if(widgetMessage.at(i)->isShown == false)
                    {
                        //构造聊天窗口时发送一个聊天请求？发送信息时再进行通知？
                        //获取窗口名：btn->text()
                        Widget *wight = new Widget(0,btn->text());
                        wight->setWindowTitle(btn->text());
                        widgetMessage.at(i)->isShown = true;
                        widgetMessage.at(i)->p_wight = wight;
                        wight->show();
                        connect(wight,&Widget::closeWidght,[=](){
                            widgetMessage.at(i)->isShown = false;
                        });
                        break;
                    }
                    else
                    {
                        qDebug() << widgetMessage.at(i)->userName << "已经被打开";
                        return;
                    }
                }
                else
                {
                    continue;
                }
            }

        });

    }

    //绑定client的读信号，调用相应的私聊窗口来接收信息
    connect(client,&QTcpSocket::readyRead,[=](){
        qDebug() << "接收到聊天信息" << widgetMessage.size();
        QByteArray msgPacket = client->readAll();
        QString msg_packet(msgPacket);
        int fromUserNameIndex = msg_packet.indexOf("\r\n\r\n");
        QString fromUserName = msg_packet.mid(5,fromUserNameIndex-5);
        QString msg = msg_packet.mid(fromUserNameIndex+4);
        for (int i =0;i<widgetMessage.size();i++)
        {
            if(widgetMessage.at(i)->userName == fromUserName)
            {
                if(widgetMessage.at(i)->p_wight!=NULL)
                {
                    widgetMessage.at(i)->p_wight->readMessage(msg.toUtf8());
                }
                else
                {
                    //
                    qDebug() << "对应聊天窗口未打开";
                }
            }

        }

    });

    //绑定刷新信号
    connect(ui->refleshButton,&QPushButton::clicked,[=](){
        this->close();
        disconnect(client,&QTcpSocket::readyRead,0,0);
        UserList *userList = new UserList(0,userName);
        userList->show();
    });

}

UserList::~UserList()
{
    delete ui;
}
