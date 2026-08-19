#include "sensor_widget.h"
#include "ui_sensor_widget.h"
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>
#include <QRect>

#include "ap3216c.h"
#include <QtCharts>
#include <QProcess>

SensorWidget::SensorWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SensorWidget)
{
    ui->setupUi(this);

    chart_init();
    file.setFileName("/sys/devices/platform/leds/leds/sys-led/brightness");
    file1.setFileName("/sys/devices/platform/leds/leds/beep/brightness");

    if (!file.exists())
        ui->pushButton->setText("未获取到led设备！");
    if (!file1.exists())
        ui->pushButton_2->setText("未获取到beep设备！");

    /* 获取LED的状态 */
    getLedState();
    getBeepState();

    icm20608 = new Icm20608;
    connect(icm20608, SIGNAL(icm20608DataChanged()),
               this, SLOT(getIcm20608Data()));

    ap3216c = new Ap3216c;
    connect(ap3216c, SIGNAL(ap3216cDataChanged()),
            this, SLOT(getAp3216cData()));

    connect(ui->returnButton, &QPushButton::clicked, this, &SensorWidget::onReturnClicked);
}

SensorWidget::~SensorWidget()
{

    delete icm20608;
    delete ap3216c;
    delete m_chart;
    delete ui;
}

void SensorWidget::chart_init()
{
    // 创建横纵坐标轴并设置显示范围
    m_axisX = new QValueAxis();
    m_axisY = new QValueAxis();
    m_axisX->setLabelsColor(Qt::white);
    m_axisY->setLabelsColor(Qt::white);
    m_axisX->setLinePenColor(Qt::white);
    m_axisY->setLinePenColor(Qt::white);
    m_axisX->setTitleBrush(QBrush(Qt::white));
    m_axisY->setTitleBrush(QBrush(Qt::white));

    m_axisX->setTitleText("X-label");
    m_axisY->setTitleText("Y-label");
    m_axisX->setMin(0);
    m_axisY->setMax(0);
    m_axisX->setMax(AXIS_MAX_X);
    m_axisY->setMax(AXIS_MAX_Y);

    // m_lineSeries = new QLineSeries();                      // 创建曲线绘制对象
    splineSeries = new QSplineSeries();
    splineSeries->setPointsVisible(false);                     // 设置数据点可见
    splineSeries->setName("als数据");                         // 图例名称

    //创建ir曲线对象
    splineSeries1 = new QSplineSeries();
    splineSeries1->setPointsVisible(false);                    // 设置数据点可见
    splineSeries1->setName("ir数据");                         // 图例名称

    //创建ps曲线对象
    splineSeries2 = new QSplineSeries();
    splineSeries2->setPointsVisible(false);                   // 设置数据点可见
    splineSeries2->setName("ps数据");                         // 图例名称

    m_chart = new QChart();                                        // 创建图表对象
    m_chart->setBackgroundBrush(QBrush(Qt::black));
    m_chart->setTitleBrush(QBrush(Qt::white));
    m_chart->legend()->setBrush(QBrush(Qt::white));
    m_chart->legend()->setLabelColor(Qt::white);

    m_chart->addAxis(m_axisY, Qt::AlignLeft);                      // 将X轴添加到图表上
    m_chart->addAxis(m_axisX, Qt::AlignBottom);                    // 将Y轴添加到图表上
    m_chart->addSeries(splineSeries);                              // 将als曲线对象添加到图表上
    m_chart->addSeries(splineSeries1);                              // 将ir曲线对象添加到图表上
    m_chart->addSeries(splineSeries2);                              // 将ps曲线对象添加到图表上


    m_chart->setAnimationOptions(QChart::SeriesAnimations);        // 动画：能使曲线绘制显示的更平滑，过渡效果更好看

    splineSeries->attachAxis(m_axisX);                             // als曲线对象关联上X轴，此步骤必须在m_chart->addSeries之后
    splineSeries->attachAxis(m_axisY);                             // als曲线对象关联上Y轴，此步骤必须在m_chart->addSeries之后

    //ir曲线关联到X，Y轴
    splineSeries1->attachAxis(m_axisX);
    splineSeries1->attachAxis(m_axisY);

    //ps曲线关联到X，Y轴
    splineSeries2->attachAxis(m_axisX);
    splineSeries2->attachAxis(m_axisY);


    ui->graphicsView->setChart(m_chart);                         // 将图表对象设置到graphicsView上进行显示
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);       // 设置渲染：抗锯齿，如果不设置那么曲线就显得不平滑
}


void SensorWidget::setLedState()
{
    /* 在设置LED状态时先读取 */
    bool state = getLedState();

    /* 如果文件不存在，则返回 */
    if (!file.exists())
        return;

    if(!file.open(QIODevice::ReadWrite))
        qDebug()<<file.errorString();

    QByteArray buf1[2] = {"0", "1"};

    /* 写0或1 */
    if (state)
        file.write(buf1[0]);
    else
        file.write(buf1[1]);

    /* 关闭文件 */
    file.close();

    /*重新获取LED的状态 */
    getLedState();
}

bool SensorWidget::getLedState()
{
    /* 如果文件不存在，则返回 */
    if (!file.exists())
        return false;

    if(!file.open(QIODevice::ReadWrite))
        qDebug()<<file.errorString();

    QTextStream in(&file);

    /* 读取文件所有数据 */
    QString buf1 = in.readLine();

    /* 打印出读出的值 */
    qDebug()<<"buf: "<<buf1<<endl;
    file.close();
    if (buf1 == "1") {
        ui->pushButton->setText("LED点亮");
        return true;
    } else {
        ui->pushButton->setText("LED熄灭");
        return false;
    }
}

void SensorWidget::setBeepState()
{
    /* 在设置BEEP状态时先读取 */
    bool state = getBeepState();

    /* 如果文件不存在，则返回 */
    if (!file1.exists())
        return;

    if(!file1.open(QIODevice::ReadWrite))
        qDebug()<<file1.errorString();

    QByteArray buf2[2] = {"0", "1"};

    if (state)
        file1.write(buf2[0]);
    else
        file1.write(buf2[1]);

    file1.close();

    getBeepState();
}

bool SensorWidget::getBeepState()
{
    /* 如果文件不存在，则返回 */
    if (!file1.exists())
        return false;

    if(!file1.open(QIODevice::ReadWrite))
        qDebug()<<file1.errorString();

    QTextStream in(&file1);

    /* 读取文件所有数据 */
    QString buf2 = in.readLine();

    /* 打印出读出的值 */
    qDebug()<<"buf: "<<buf2<<endl;
    file1.close();
    if (buf2 == "1") {
        ui->pushButton_2->setText("BEEP开");
        return true;
    } else {
        ui->pushButton_2->setText("BEEP关");
        return false;
    }
}

void SensorWidget::on_pushButton_clicked()
{
    /* 设置LED的状态 */
    setLedState();
}

void SensorWidget::on_pushButton_2_clicked()
{
    /* 设置蜂鸣器的状态 */
    setBeepState();
}

void SensorWidget::on_pushButton_icm2060_off_clicked()
{
    icm20608->setCapture(false);
    ui->pushButton_icm2060_off->setText("已停止检测");
    ui->gxdataEdit->setText("无");
    ui->gydataEdit->setText("无");
    ui->gzdataEdit->setText("无");

    ui->axdataEdit->setText("无");
    ui->aydataEdit->setText("无");
    ui->azdataEdit->setText("无");

    ui->tempdataEdit->setText("无");
}

void SensorWidget::on_pushButton_icm2060_on_clicked()
{
    icm20608->setCapture(true);
    ui->pushButton_icm2060_on->setText("已启动检测");
}

void SensorWidget::getIcm20608Data()
{
    static QString gx = icm20608->gxData();
    if (gx != icm20608->gxData()) {
        gx = icm20608->gxData();
    }

    static QString gy = icm20608->gyData();
    if (gy != icm20608->gyData()) {
        gy = icm20608->gyData();
    }

    static QString gz = icm20608->gzData();
    if (gz != icm20608->gzData()) {
        gz = icm20608->gzData();
    }

    static QString ax = icm20608->axData();
    if (ax != icm20608->axData()) {
        ax = icm20608->axData();
    }

    static QString ay = icm20608->ayData();
    if (ay != icm20608->ayData()) {
        ay = icm20608->ayData();
    }

    static QString az = icm20608->azData();
    if (az != icm20608->azData()) {
        az = icm20608->azData();
    }

    static QString temp_data = icm20608->tempData();
    if (temp_data != icm20608->tempData()) {
        temp_data = icm20608->tempData();
    }


    ui->gxdataEdit->setText(gx);
    ui->gydataEdit->setText(gy);
    ui->gzdataEdit->setText(gz);

    ui->axdataEdit->setText(ax);
    ui->aydataEdit->setText(ay);
    ui->azdataEdit->setText(az);

    ui->tempdataEdit->setText(temp_data);

}

void SensorWidget::getAp3216cData()
{
    static QString als = ap3216c->alsData();
    if (als != ap3216c->alsData()) {
        als = ap3216c->alsData();
        //arcGraph[0]->setangleLength(als.toUInt() * 360 / 65535);
    }

    static QString ps = ap3216c->psData();
    if (ps != ap3216c->psData()) {
        ps = ap3216c->psData();
        //arcGraph[1]->setangleLength(ps.toUInt() * 360 / 1023);
    }

    static QString ir = ap3216c->irData();
    if (ir != ap3216c->irData()) {
        ir = ap3216c->irData();
        //arcGraph[2]->setangleLength(ir.toUInt() * 360 / 1023);
    }

    ui->alsEdit->setText(als);
    ui->irEdit->setText(ir);
    ui->psEdit->setText(ps);

    //画曲线代码
    if(pointCount > AXIS_MAX_X)
    {
        splineSeries->remove(0);

        m_chart->axisX()->setMin(pointCount - AXIS_MAX_X);
        m_chart->axisX()->setMax(pointCount);                    // 更新X轴范围
    }
    splineSeries->append(QPointF(pointCount, als.toInt()));  // 更新显示
    splineSeries1->append(QPointF(pointCount, ir.toInt()));  // 更新显示
    splineSeries2->append(QPointF(pointCount, ps.toInt()));  // 更新显示

    pointCount++;

    //glowText[0]->setTextData(als);
    //glowText[1]->setTextData(ps);
    //glowText[2]->setTextData(ir);

}

void SensorWidget::on_stop_clicked()
{
    ap3216c->setCapture(false);
    ui->stop->setText("停止检测");
    pointCount = 0;
}

void SensorWidget::on_start_clicked()
{
    ap3216c->setCapture(true);
    ui->start->setText("启动检测");
}

void SensorWidget::on_del_clicked()
{
    splineSeries->clear();
    splineSeries1->clear();
    splineSeries2->clear();
    m_chart->axisX()->setMin(0);
    m_chart->axisX()->setMax(AXIS_MAX_X);
    pointCount = 0;
}

void SensorWidget::onReturnClicked()
{
    emit returnRequested();
}