#include "widget.h"
#include "ui_widget.h"

#include <QDebug>
#include <QTcpSocket>


//私聊界面
extern QTcpSocket *client;

Widget::Widget(QWidget *parent,QString userName)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    qDebug() << userName;

    //发送信息槽函数
    connect(ui->sendButton,&QPushButton::clicked,[=](){

        //获取编辑框内容
        QString str = ui->msgEdit->toHtml();
        if(str.isEmpty())
        {
            qDebug() << "no msg";
            return;
        }
        //发送数据
        QString msg = "send:"+ userName+ "\r\n\r\n" +str;
        qDebug() << msg;
        client->write( msg.toUtf8().data());

        //可以对信息进行一些加工（如添加时间等。。）
        ui->msgBrowser->append(str);
        ui->msgEdit->clear();

    });


    connect(ui->quitButton,&QPushButton::clicked,[=](){
        this->close();
    });

}




void Widget::readMessage(QByteArray msg)
{
    //可以对信息进行一些加工（如添加时间等。。）
     ui->msgBrowser->append(msg);
}


void Widget::closeEvent(QCloseEvent *)
{
    emit this->closeWidght();
}

Widget::~Widget()
{
    delete ui;
}

