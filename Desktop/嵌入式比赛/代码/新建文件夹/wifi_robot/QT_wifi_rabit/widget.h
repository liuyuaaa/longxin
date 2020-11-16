#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtNetwork/QTcpSocket>
#include <QString>
#include <QMutex>
#include <opencv2/core/core.hpp>
#include "opencv2/ml/ml.hpp"
#include <opencv2/core/utility.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/imgproc/types_c.h"
#include "opencv2/opencv.hpp"

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include <QImage>
//#include "IplImageToQImage.h"
#include <QTime>
#include <QImage>
#include <QPainter>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:


    void on_pushButton_clicked();

    void on_run_clicked();

    void on_back_clicked();

    void on_left_clicked();

    void on_right_clicked();

    void on_stop_car_clicked();

    void doSpeed();

    void read();

    void sendrequest();

    void on_buzzer_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

     void read_t();

     void on_right_3_clicked();

private:
    Ui::Widget *ui;
    QTcpSocket *client;
    QTcpSocket *motor_client;
    QByteArray block1;
    QMutex mutex;
};

#endif // WIDGET_H
