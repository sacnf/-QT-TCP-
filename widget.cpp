#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    // ui->connectBtn->setStyleSheet("background-color: rgb(6,163,220)");
    // ui->sendBtn->setStyleSheet("background-color: rgb(6,163,220)");
    // ui->leport->setStyleSheet("color:blue");
    // ui->leipAddress->setStyleSheet("color:blue");

    // ui->listWidget->setStyleSheet("border:2px solid blue");

    // 设置整个界面背景为深灰色
    this->setStyleSheet("background-color: #333333;");

    // 设置字体颜色为白色
    ui->label->setStyleSheet("color:white");
    ui->label_2->setStyleSheet("color:white");

    // 设置按钮样式：黑色背景，白色边框，白色字体
    ui->connectBtn->setStyleSheet("background-color: black; border: 2px solid white; color: white;");
    ui->sendBtn->setStyleSheet("background-color: black; border: 2px solid white; color: white;");

    // 设置标签样式：黑色背景，白色边框，白色字体
    ui->leport->setStyleSheet("background-color: black; border: 2px solid white; color: white;");
    ui->leipAddress->setStyleSheet("background-color: black; border: 2px solid white; color: white;");

    // 设置列表框样式：黑色背景，白色边框，白色字体
    ui->listWidget->setStyleSheet("background-color: black; border: 2px solid white; color: white;");

    // 设置文本框样式：黑色背景，白色边框，白色字体
    ui->textSend->setStyleSheet("background-color: black; border: 2px solid white; color: white;");
    ui->textReceive->setStyleSheet("background-color: black; border: 2px solid white; color: white;");

    this->setWindowIcon(QIcon(":/image/cat.png"));

    socket = new QTcpSocket(this);
    connectState = false;     //未连接状态

    connectSound = new QSound(":/new/prefix1/sounds/keke.wav", this);
    // QSoundEffect *connectSound = new QSoundEffect(this);
    // connectSound->setSource(QUrl::fromLocalFile(":/new/prefix1/sounds/keke.wav"));

    messageSound = new QSound(":/new/prefix1/sounds/iphone.wav", this);
    // QSoundEffect *messSound = new QSoundEffect(this);
    // messSound->setSource(QUrl::fromLocalFile(":/new/prefix1/sounds/iphone.wav"));

    connect(socket, &QTcpSocket::readyRead, this, &Widget::readMessage);    //接收信息
    connect(socket, &QTcpSocket::disconnected, this, &Widget::disconnectSlot);   //打印断开连接信息
}

Widget::~Widget()
{
    delete ui;
}


void Widget::readMessage()    //接收信息
{
    messageSound->play();
    QByteArray arr = socket->readAll();
    QString str;
    //str = QDateTime::currentDateTime().toString("dddd yyyy.MM.dd hh:mm:ss") + '\n' + arr.data();
    str = arr.data();
    ui->textReceive->append(str);     //显示信息
}


void Widget::disconnectSlot()    //打印连接断开信息
{
    ui->listWidget->addItem("clint disconnected");
}


void Widget::on_connectBtn_clicked()      //与客户端连接或者断开
{
    QString ipStr = ui->leipAddress->text();    //界面显示的地址
    quint16 currentPort = ui->leport->text().toInt();   //界面显示的当前端口
    if(!connectState)    //客户端还未连接服务端
    {
        socket->connectToHost(ipStr, currentPort);   //连接服务端
        if(socket->waitForConnected())   //等待连接成功
        {
            ui->listWidget->addItem("连接成功");
            ui->connectBtn->setText("关闭连接");
            connectSound->play();
            connectState = true;
        }

        else     //连接失败
        {
            QMessageBox::warning(this, "连接失败", socket->errorString());   //连接错误信息提醒
        }
    }

    else   //客户端已经连接
    {
        socket->close();   //关闭套接字，此时会发送disconnected信号
        connectSound->play();
        ui->connectBtn->setText("连接");
    }
}


void Widget::on_sendBtn_clicked()    //给服务端发送信息
{
    QString str = ui->textSend->toPlainText();
    if(socket->isOpen() && socket->isValid())
    {
        socket->write(str.toUtf8());    //给服务端发送信息
        ui->textSend->clear();
    }

    QString showStr = QDateTime::currentDateTime().toString("dddd yyyy.MM.dd hh:mm:ss") + '\n' + str;
    ui->textReceive->append(showStr);     //显示自己发送的信息
}








