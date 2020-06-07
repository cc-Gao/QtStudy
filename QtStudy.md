# Qt中事件，信号与槽函数



## 一、事件

### 1. 事件循环

```c++
//一个简单的Qt窗口程序如下：
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}
```

​		该窗口可以一直处于打开状态，是因为在`QCoreApplication/QGuiApplication/QApplication` 对象中维护了一个`QEventLoop`，这个循环被称为**主事件循环**。执行`a.exec()`其实是执行`QEventLoop::exec`方法。这个事件循环可以获取**windowsystem**的事件，将其转换成`QEvent`对象，并将其转发到对应的`QObject`上。`QObject`通过`QObject::event(QEvent *e)`方法来处理事件。

​		事件会被存储在一个**事件队列**中，同时Qt提供`QEventLoop::processEvents`方法可以让我们在某一事件处理过程中进行其他事件的处理（如果当前的事件处理时间较长会导致GUI卡死，在当前需要较长时间处理的事件处理函数中适当的调用该方法可以避免GUI卡死）。

```c++
//示例
Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QEventLoop* a = new QEventLoop;
	//定义按钮a
    QPushButton *btn1 = new QPushButton;
    btn1->setParent(this);
    btn1->setText("a");
	//连接槽函数
    connect(btn1,&QPushButton::clicked,[=](){
        for(int i = 0;i<100;i++)
        {
            Sleep(50);
            //使用该方法防止GUI卡死
            a->processEvents();
        }
    });

    //定义按钮b
    QPushButton *btn2 = new QPushButton;
    btn2->setParent(this);
    btn2->setText("b");
    btn2->move(100,200);
    connect(btn2,&QPushButton::clicked,[=](){
            qDebug()<<"press B";
    });
}
```



可以看到使用该方法后，即使在处理点击a按钮的事件时，按下依然会被处理，不会造成GUI的卡死。



同时，Qt还提供了`QCoreApplication::sendEvent()` 和 `QCoreApplication::postEvent()`方法，来进行手动事件的发送以及手动事件的入列。其中`QCoreApplication`是`QApplication`的父类`QGuiApplication`的父类。其中`sendEvent()`函数在事件处理结束后才返回。



### 2. QObject::event方法与QEvent类

​	以下为`QObject::event`的help page:

![image-20200605231323251](https://cdn.jsdelivr.net/gh/cc-Gao/personal-image-hosting-web-site@master/2020/06/07/b525efb3efd56942a8a529fef834d6e7.png)

​		该方法没有什么难以理解的部分，help page中展示的示例基本上可以体现用法。

​		对于`QEvent`类，有以下几个函数需要注意：

```C++
//接收事件
void accept();
//忽略事件
void ignore();
//用于获取accept flag
bool isAccepted() const;
//用于设定accept flag
void setAccepted(bool accepted);
bool spontaneous() const;
```

​	以下为Qt help page中相关函数的介绍：



![image-20200607215628646](https://cdn.jsdelivr.net/gh/cc-Gao/personal-image-hosting-web-site@master/2020/06/07/38e5743a89209d64e1e94c73a043522c.png)



![image-20200607215824749](https://cdn.jsdelivr.net/gh/cc-Gao/personal-image-hosting-web-site@master/2020/06/07/26c57798c97c7696ca22bc78ee9e811f.png)

​		最后一个函数可用于判断事件是否是手动发送的，即是否是通过上述的`sendEvent()`和`PostEvent()`发送的。

![image-20200607220210062](https://cdn.jsdelivr.net/gh/cc-Gao/personal-image-hosting-web-site@master/2020/06/07/5d3f8e270f7437f3abfe74cf78840ff4.png)



​		自定义事件的设定：

```c++
//在Qt中也可以定义自己的事件，定义大致流程如下：

//通过QEvent::registerEventType()函数返回一个可用的事件值。
const QEvent::Type MyType = (QEvent::Type)QEvent::registerEventType();

//自定义事件，在该事件中添加了一个字符串成员，用于传递字符串
//所有的事件都是QEvent的子类，自定义事件时也需要定义为QEvent的子类
class MyEvent : public QEvent
{
public:
    MyEvent(QEvent::Type type, QString param) : QEvent(type)
    {
        this->param = param;
    }
    QString param;
};

//该信号的发送需要使用sendEvent()方法或者postEvent()方法

//重写接受到该信号的event函数
bool Widget::event(QEvent *event)
{
    if(event->type() == MyType)
    {
        MyEvent *e = static_cast<MyEvent*>(event);
        qDebug() << e->param;
        return true;
    }
    return QWidget::event(event);
}


```







### 3. 事件过滤器(EventFilter)

​		提及事件，不免得提及一下`QObject::eventFilter(QObject* watched,QEnvent* event)`  ,该方法可用于过滤一些事件，不进行处理，或者进行一些自定义处理。

​	以下为**事件过滤器**的两个相关方法help page中的description：

![image-20200605232135902](https://cdn.jsdelivr.net/gh/cc-Gao/personal-image-hosting-web-site@master/2020/06/07/02cf277c1edc7134875badf2e2c6bd7c.png)



![image-20200605233201044](https://cdn.jsdelivr.net/gh/cc-Gao/personal-image-hosting-web-site@master/2020/06/07/9939de065945864af693522509561088.png)



为什么需要事件过滤器：

​		比如在一个登录界面，你填完其中某一项时，你希望一个快捷键来跳转到下一项，此时可以定义 `QLineEdit`的一个子类，并重写其`keyPressEvent`函数来进行处理。

```c++
class MyQLineEdit ： public QLineEdit
{
    //...
}

//重写keyPressEvent函数
void MyQLineEdit::keyPressEvent(QKeyEvent *event)
{
    //假设快捷键为Key_Space
    if (event->key() == Qt::Key_Space)
    {
        jumpNextEdit();
    }
    else
    {
        QLineEdit::keyPressEvent(event);
    }
}
```

​	但如果各项使用的不是相同的控件，而是比如comboBox等等，此时需要对每一种控件进行继承，再重写函数，会十分复杂。

​	此时我们可以通过重写登录界面对象中的`EventFilter`，来进行处理。

例：

```c++
class UserRegisterWindow ：public QObject
{
    Q_OBJECT
    //...
    
    protected:
    	bool eventFilter(QObject *obj,QEvent *event) override;
}

//重写函数
bool UserRegisterWindow::eventFilter(QObject *obj,QEvent *event)
{
    //此处也可以根据obj来进行处理，用来进一步的保证事件的正确处理
    if(event->type()==QEvent::KeyPress/*过滤的信号类型*/)
    {
        //对应的一些处理
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    	if (keyEvent->key() == Qt::Key_Space)
    	{
        	jumpNextEdit();
            return true;
    	}
    }
    else
    {
        return QObject::eventFilter(obj,event);
    }
}


UserRegisterWindow::UserRegisterWindow(QWidget *parent = nullptr)
{
    //...
    //注册EventFilter
    usernameEdit->installEventFilter(this);
    ageBox->installEventFilter(this);
    //...
}



```

​		此时，发送到usernameEdit和ageBox等的事件会先在`UserRegisterWindow::eventFilter`中进行处理。通过该对象来监听控件的事件。



​		事件过滤器是在一个对象中定义事件过滤器，再将这个对象注册到需要被监测的对象中(使用`installEventFilter`方法)，在本例中，事件过滤器是定义在父窗口中，被注册到了对应需要的子窗口。从事件派发转发来看，可能会被理解为事件是先被发送给了父窗口，再转发给子窗口，这可能与Qt中事件是向上转发违和，导致有些难以理解。通过下文的实现机理可以很好的理解事件过滤器实现过程中，事件的派发转发流程。



实现机理：

> 在所有Qt对象的基类: QObject中有一个类型为QObjectList的成员变量,名字为eventFilters,当某个QObject (qobjA)给另一个QObject (qobjB)安装了事件过滤器之后, qobjB会把qobjA的指针保存在eventFilters中. 在qobjB处理事件之前,会先去检查eventFilters列表, 如果非空, 就先调用列表中对象的eventFilter()函数. 一个对象可以给多个对象安装过滤器. 同样, 一个对象能同时被安装多个过滤器, 在事件到达之后, 这些过滤器以安装次序的反序被调用. 事件过滤器函数( eventFilter() ) 返回值是bool型, 如果返回true, 则表示该事件已经被处理完毕, Qt将直接返回, 进行下一事件的处理; 如果返回false, 事件将接着被送往剩下的事件过滤器或是目标对象进行处理.

（Qt5.15中未在help page中找到该成员变量，故该机理存疑，但该机理可以很好的用来理解事件过滤器的实现）









## 二、信号与槽函数

### 	1. 信号与槽函数的优势

​		信号与槽函数其实都是函数，有别于常见的回调，使用信号和槽函数可以保证类型安全，槽函数的参数个数必须小于等于信号函数的参数并且类型得相同，同时，信号和槽函数相比于回调函数来说耦合程度 更低，回调处理方法中处理函数必须明确知道哪个函数被回调。

​		个人看来，信号与槽函数的实现可以理解为使用回调加一个映射表。



### 	信号和槽函数的定义

```c++
class MyClass：public QOblect
{fun_
    Q_OBJECT
    
public:
    //...
private:
    //...
signals:
    //定义信号
    void signal_a(/*params*/);
public slots:
 	//定义槽函数
 	void slot_a(/*params*/);

}

```

​		信号是可以自定义的，只需要加上关键字`signals:`即可。

​		信号的发送可以使用关键字 `emit` ，也可以直接调用信号函数。



### 	2. 信号和槽函数的连接

1. 使用connect函数（connect函数也可以实现信号和信号的连接），有时可以使用lambda表达式来设定匿名函数为槽函数（如果该槽函数只需连接该信号时会显得比较简便）。
2. （个人感觉本质上信号和槽函数都只是一个public方法，没啥区别~所以信号也可以连接信号），不过注意信号函数的实现是由Qt自己完成的。
3. 一个信号连接多个槽函数时，按照connect的顺序依次调用。





## 三、小结与补充

- ​	系统获取到用户的操作后，向Qt程序发送信息，在Qt中，先检查是否有事件过滤器install在`QApplication`对象（`QApplication`也是一个`QObject`的子类）上，如果有，则进行处理，然后由`QApplication::notify()` 进行分析，处理信号，将其封装成QEvent对象，发送给对应的`QObjest`对象。

- ​	对于一个普通的`QObjest`对象而言，也是先检查是否install事件过滤器，若无或者事件未被过滤器处理，则通过`event()`函数进行处理，调用对应的事件处理函数，部分对应的事件处理函数会发送一些对应的信号。

- ​    Qt中和事件相关的函数一般分为两类：

  第一类：`QApplication::notify(), QObject::eventFilter(), QObject::event()` 这类通过返回值来表示信号是否已经处理，对于未处理的事件，这个事件将会向上转发给它的`parent`。

  第二类 `QEvent::ignore() 或 QEvent::accept()` 这两个函数一般用于忽略事件或者阻断事件。  

- ​	不同事件有不同的处理方式，部分事件会发送信号，此时我们需要设定槽函数来对这种信号来处理，相当于说捕获该信号。

- ​    事件和信号均可以自定义，事件继承 `QEvent`类，信号只需要在对应的`QObject`中声明即可。