#include "widget.h"
#include "ui_widget.h"
#include <cstring>
#include <cstdbool>
#include <QHostAddress>

using namespace std;
using namespace cv;

uchar* qImageBuffer;
CvVideoWriter* videowr;
bool record;

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    /* 设置调节速度的滚动条 */
    ui->speed->setRange(1,100);
    ui->speed->setValue(100);
    /* 用于视频传输 */
    client=new QTcpSocket;
    /* 用于空中小车的方向 */
    motor_client=new QTcpSocket;
    //qImageBuffer=(uchar *)(malloc(320 * 256 * 4 *sizeof(uchar)));

    //ui->Image->setFixedSize(1000,1000);

    connect(client,SIGNAL(connected()),this,SLOT(sendrequest()));
    connect(client,SIGNAL(readyRead()),this,SLOT(read()));
    connect(ui->speed, SIGNAL(valueChanged(int)), this, SLOT(doSpeed()));
    connect(motor_client,SIGNAL(readyRead()),this,SLOT(read_t()));
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_pushButton_clicked()
{
    ui->label->setText(tr("connecting..."));
    QString ip = "192.168.43.100";
    /* 用于视频传输 */
    client->connectToHost(QHostAddress(ip),8080);
    /* 用于空中小车的方向 */  
    motor_client->connectToHost(QHostAddress(ip),8888);
}

void Widget::doSpeed()
{
    int pos = ui->speed->value();

    qDebug("speed:%d\n", pos);

    QString str = "speed:";
    str.append(qPrintable(tr("%1").arg(pos)));

    qDebug("test:%s\n", qPrintable(str));

    QByteArray block(qPrintable(str));
    motor_client->write(block);
}

void Widget::on_run_clicked()
{
    qDebug("run...\n");

    QByteArray block("direction:run");
    motor_client->write(block);
}

void Widget::on_back_clicked()
{
    qDebug("back...\n");

    QByteArray block("direction:back");
    motor_client->write(block);
}

void Widget::on_left_clicked()
{
    qDebug("left...\n");

    QByteArray block("direction:left");

    motor_client->write(block);
}

void Widget::on_right_clicked()
{
    qDebug("right...\n");

    QByteArray block("direction:right");
    motor_client->write(block);
}

void Widget::on_stop_car_clicked()
{
    qDebug("stop...\n");

    QByteArray block("direction:stop");
    motor_client->write(block);
}

void Widget::sendrequest()
{
    QByteArray block("GET /?action=stream\n\n");
    client->write(block);

    ui->label->setText(tr("connecting..., success"));
}

void Widget::read()
{
    if(client->bytesAvailable()<3000)
        return;

    QByteArray tmpBlock;
    static unsigned const char SOIData[]={0xff,0xd8};
    static unsigned const char EOIData[]={0xff,0xd9};

    QByteArray SOIstr=QByteArray::fromRawData((char *)SOIData,sizeof(SOIData));
    QByteArray EOIstr=QByteArray::fromRawData((char *)EOIData,sizeof(EOIData));

    int SOIPos=0;
    int EOIPos=0;

    mutex.lock();
    tmpBlock=client->readAll();
    block1.append(tmpBlock);//保存实际读到的数据

    if((SOIPos=block1.indexOf(SOIstr))!= -1)
    {
        if((EOIPos=block1.indexOf(EOIstr))!= -1)
        {
            EOIPos += 2;
            if(EOIPos>SOIPos)
            {
                QByteArray ba;
                ba=block1.mid(SOIPos,EOIPos-SOIPos);

                QImage image;
                image.loadFromData(ba);

                int width = image.width(), height = image.height();

                //QImage* qImage=new QImage(qImageBuffer, width, height, QImage::Format_RGB32);

                QPixmap pix;
                pix= pix.fromImage(image);
                //ui->Image->setPixmap(pix.scaled(ui->label->size()));
                ui->Image->setPixmap(pix);
                //delete qImage;

                ba.clear();
            }
            block1.remove(0,EOIPos+1);
        }
    }
    mutex.unlock();
}

void Widget::on_buzzer_clicked()
{
    qDebug("buzzer...\n");
    QByteArray block("buzzer:on");
    motor_client->write(block);
}

void Widget::on_pushButton_3_clicked()
{
    qDebug("rate_l...\n");

    QByteArray block("direction:rate_l");
    motor_client->write(block);
}


void Widget::on_pushButton_2_clicked()
{
    qDebug("rate_r...\n");

    QByteArray block("direction:rate_r");
    motor_client->write(block);
}

void Widget::read_t()
{
    QByteArray temp= motor_client->readAll();

    ui->label_3->setText(temp);

}


void Widget::on_right_3_clicked()
{
    QByteArray block("temp:");
    motor_client->write(block);
}
