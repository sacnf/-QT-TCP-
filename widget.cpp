#include "widget.h"
#include "qsound.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    QString backgroundImagePath = ":/images/bei.jpeg";
    this->setStyleSheet("background-image: url(" + backgroundImagePath + "); background-repeat: no-repeat; background-position: center;");

    ui->listenBtn->setStyleSheet("background-color: rgb(255,165,0)");


    ui->sendBtn->setStyleSheet("background-color: rgb(255,165,0)");


    ui->leport->setStyleSheet("color:rgb(0,128,0)");

    ui->ipBox->setStyleSheet("color:rgb(0,128,0)");
    ui->ipBox->setStyleSheet("background-color: rgb(255,255,192)");


    //ui->textSend->setStyleSheet("border:red");
    ui->textSend->setStyleSheet("border:2px solid rgb(0,128,0)");

    ui->label_2->setStyleSheet("color:rgb(0,128,0)");
    ui->label_3->setStyleSheet("color:rgb(0,128,0)");
    ui->listWidget->setStyleSheet("border:2px solid rgb(255,255,192)");     //改变边框颜色
    //ui->textReceive->setStyleSheet("border:2px groove gray;border-radius:10px;padding:2px 4px");
    //this->setStyleSheet("QWidget{border-top-left-radius:15px;border-top-right-radius:5px;}");

    this->setWindowIcon(QIcon(":/images/dog.png"));

    connectSound = new QSound(":/new/prefix1/sounds/keke.wav", this);
    messSound = new QSound(":/new/prefix1/sounds/iphone.wav", this);
    severe = new QTcpServer(this);      //创建服务端
    this->enumAllIp();   //初始化ip地址
    connect(severe, &QTcpServer::newConnection, this, &Widget::newConnectSlot);     //有新连接时传入信号
    connect(ui->pushButton_clear, &QPushButton::clicked, this, &Widget::on_pushbutton_clear);
    connect(ui->pushButton_out, &QPushButton::clicked, this, &::Widget::on_pushButton_out);
    connect(ui->pushButton_num, &QPushButton::clicked, this, &::Widget::on_pushButton_num);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::enumAllIp()    //初始化本机所有可用的ipv4的地址并添加地址到列表框中
{
    QList <QHostAddress> addressList = QNetworkInterface::allAddresses();     //接收本机所有Ip地址
    QStringList addressList_str;
    for(int i = 0; i < addressList.size(); i++)
    {
        if(addressList.at(i).isNull()) continue;   //地址为空则跳过
        if(addressList.at(i).protocol() != QAbstractSocket::IPv4Protocol) continue;   //协议族不是ipv4的则跳过
        addressList_str.append(addressList.at(i).toString());      //符合条件的地址添加到列表中
    }

    ui->ipBox->addItems(addressList_str);   //将地址列表添加到地址列表框中
}

void Widget::newConnectSlot()    //新的客户端连接
{
    connectSound->play();
    clintSock = severe->nextPendingConnection();      //获取当前连接的客户端套接字
    clintList_sock.push_back(clintSock);           //将连接的客户端放入客户端列表中
    QString str = clintSock->peerAddress().toString() + ": " + QString::number(clintSock->peerPort()) + "已经连接";
    ui->listWidget->addItem(str);  //将连接信息放在列表窗口中

    connect(clintSock, &QTcpSocket::readyRead, this, &Widget::readMessage);  //准备接受信息
    connect(clintSock, &QTcpSocket::disconnected, this, &Widget::disConnectSlot);   //客户端的断开连接信息
}

void Widget::readMessage()      //读取客户端信息及给其他客户端发送信息
{
    QTcpSocket*  currentClint;
    QByteArray arr;
    QString str;
    if(!clintList_sock.isEmpty())      //有客户端存在
    {
        for(int i = 0; i < clintList_sock.count(); i++)     //服务端接收信息
        {
            arr = clintList_sock.at(i)->readAll();      //接收客户端发送的字节信息
            if(arr.isNull())
                continue;   //空代表不是该客户端发送
            messSound->play();
            currentClint = clintList_sock.at(i);
            str = QDateTime::currentDateTime().toString("dddd.yyyy.MM.dd HH:mm:ss") + '\n' + arr.data();
            break;
        }
        ui->textReceive->append(str);     //显示信息
        for(int i = 0; i < clintList_sock.count(); i++)     //给其它客户端发送信息
        {
            QTcpSocket *temp = clintList_sock.at(i);
            if(currentClint == temp)
                continue;      //遇到自己就跳过
            temp->write(str.toUtf8());   //发送信息
        }
    }

}


void Widget::disConnectSlot()       //客户端断开连接时服务端显示断开信息
{
    connectSound->play();
    QString closeStr = QDateTime::currentDateTime().toString("dddd.yyyy.MM.dd HH:mm:ss") + ' ' + "clint close";
    ui->listWidget->addItem(closeStr);
}

void Widget::on_listenBtn_clicked() // 服务端开始监听
{
    QString currentIp = ui->ipBox->currentText();     // 当前ip列表的Ip
    QString portText = ui->leport->text();           // 当前文本框显示的端口
    quint16 currentPort;
    bool ok;

    // 尝试将端口文本转换为整数，如果为空或转换失败，则设置为0表示监听所有端口
    currentPort = portText.toInt(&ok);
    if (!ok || portText.isEmpty())
    {
        currentPort = 0;
    }

    QHostAddress currentHostIP = QHostAddress(currentIp);   // 地址字符串转换成主机Ip
    if (severe->isListening())
    {     // 服务端正在监听
        severe->close();             // 关闭服务端
        ui->listenBtn->setText("监听");
    }
    else
    {    // 服务端没有在监听
        if (severe->listen(currentHostIP, currentPort))
        {
            // 监听成功返回true
            ui->listenBtn->setText("关闭监听");
        }
        else
        {
            // 监听失败
            QMessageBox::warning(this, "监听错误", severe->errorString());   // 出现错误提示消息框
        }
    }
}
void Widget::on_sendBtn_clicked()     //给客户端发送信息
{
    QString sendStr = ui->textSend->toPlainText();
    for(int i = 0; i < clintList_sock.count(); i++)
    {
        clintList_sock.at(i)->write(sendStr.toUtf8());
    }
    ui->textSend->clear();
    QString showStr = QDateTime::currentDateTime().toString("dddd.yyyy.MM.dd HH:mm:ss") + '\n' + sendStr;
    ui->textReceive->append(showStr);    //在接收信息框显示自己发送的信息
}


void Widget::on_pushbutton_clear()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("清空确认"),
                                  tr("您确定要清空所有文本信息吗？此操作不可逆。"),
                                  QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        ui->textSend->clear();
        ui->textReceive->clear();
    }
}
void Widget::on_pushButton_out() // 断开所有客户端
{
    for (QTcpSocket* clientSock : clintList_sock) {
        if (clientSock && clientSock->isOpen()) {
            clientSock->disconnectFromHost(); // 请求断开连接
            clientSock->deleteLater(); // 计划删除套接字对象（在事件循环中处理）
        }
    }
    clintList_sock.clear(); // 清空客户端列表
    ui->listWidget->clear(); // 清空列表窗口中的连接信息
}

void Widget::on_pushButton_num()//发送连接人数
{
    // clintList_sock.count();//连接人数(断开的客户端也会算入)
    int connectedClients = 0;
    QList<QTcpSocket*> connectedSockets;
    // 遍历客户端列表，检查每个套接字是否仍然打开
    for (QTcpSocket* clientSocket : clintList_sock)
    {
        if (clientSocket->isOpen())
        {
            ++connectedClients;
            connectedSockets.append(clientSocket); //存储已连接的套接字以便后续发送消息
        }
    }
    QString message = QString("当前有 %1 个客户端已连接").arg(connectedClients);
    QDateTime currentTime = QDateTime::currentDateTime();
    // message = currentTime.toString("dddd.yyyy.MM.dd HH:mm:ss") + '\n' + message; //时间戳
    // 向已连接的客户端发送消息
    for (QTcpSocket* clientSocket : connectedSockets)
    {
        clientSocket->write(message.toUtf8());
    }
    // 在接收框显示自己发送的信息
    ui->textReceive->append(message);
}
