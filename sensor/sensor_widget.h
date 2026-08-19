#ifndef SENSORWIDGET_H
#define SENSORWIDGET_H

#include <QWidget>
#include <QFile>
#include <QTimer>
#include <QChartView>
#include <QValueAxis>
#include <QLineSeries>
#include <QSplineSeries>
#include "ap3216c.h"
#include <icm20608.h>

class Icm20608;
class Ap3216c;
/* 光添加头文件QChartView还不够，还需要引入QChart的命名空间 */
QT_CHARTS_USE_NAMESPACE

QT_BEGIN_NAMESPACE
namespace Ui { class SensorWidget; }
QT_END_NAMESPACE

class SensorWidget : public QWidget
{
    Q_OBJECT

public:
    SensorWidget(QWidget *parent = nullptr);
    ~SensorWidget();

    void chart_init();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_icm2060_off_clicked();

    void on_pushButton_icm2060_on_clicked();

    void getIcm20608Data();

    /* 获取ap3216传感器数据 */
    void getAp3216cData();


    void on_stop_clicked();

    void on_start_clicked();

    void on_del_clicked();

    void onReturnClicked();

signals:
    void returnRequested();

private:
    Ui::SensorWidget *ui;

    QFile file;
    QFile file1;

    /* ii2传感器类 */
    Ap3216c *ap3216c;
    Icm20608 *icm20608;

    /* 用于模拟生成实时数据的定时器 */
    QTimer* m_timer;

    /* 图表对象 */
    QChart* m_chart;



    /* 横纵坐标轴对象 */
    QValueAxis *m_axisX, *m_axisY;

    /* 曲线图对象 */
    QLineSeries* m_lineSeries;
    QSplineSeries *splineSeries;        //als曲线
    QSplineSeries *splineSeries1;       //ir曲线
    QSplineSeries *splineSeries2;       //ps曲线

    /* 横纵坐标最大显示范围 */
    const int AXIS_MAX_X = 40, AXIS_MAX_Y = 40;

    /* 用来记录数据点数 */
    int pointCount = 0;


    /* 设置lED的状态 */
    void setLedState();

    /* 获取lED的状态 */
    bool getLedState();

    /* 设置BEEP的状态 */
    void setBeepState();

    /* 获取BEEP的状态 */
    bool getBeepState();
};
#endif // SENSORWIDGET_H