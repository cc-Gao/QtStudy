#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent,QString userName);
    ~Widget();

    //接收消息
    void readMessage(QByteArray msg);

    //关闭事件
    void closeEvent(QCloseEvent *);
private:
    Ui::Widget *ui;

signals:
    //关闭窗口发送自定义信号
    void closeWidght();



};
#endif // WIDGET_H
