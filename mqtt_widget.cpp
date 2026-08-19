#include "mqtt_widget.h"
#include <QProcess>
#include <QFile>
#include <QDebug>
#include <QPainter>
#include <QPaintEvent>

#define BROKER_ADDRESS "192.168.1.100"  // TODO: 修改为你自己的MQTT Broker IP地址
#define CLIENTID "your_client_id"      // TODO: 修改为你自己的MQTT客户端ID
#define USERNAME "your_username"       // TODO: 修改为你自己的MQTT用户名
#define PASSWORD "your_password"       // TODO: 修改为你自己的MQTT密码

#define LED_TOPIC "dt_mqtt/led"
#define TEMP_TOPIC "dt_mqtt/temperature"
#define MOTOR_TOPIC "dt_mqtt/montor"
#define STM32LED_TOPIC "dt_mqtt/stm32led"
#define STM32TEMP_TOPIC "dt_mqtt/32temp"

MqttWidget::MqttWidget(QWidget *parent)
    : QWidget(parent),
      m_client(nullptr),
      m_tempTimer(nullptr),
      m_speed(0),
      m_ledState(false),
      m_stm32LedState(false)
{
    setupUi();
    setWidgetBackground();

    m_client = new QMqttClient(this);
    m_client->setHostname(BROKER_ADDRESS);
    m_client->setPort(1883);  // TODO: 修改为你自己的MQTT Broker端口号
    m_client->setClientId(CLIENTID);
    m_client->setUsername(USERNAME);
    m_client->setPassword(PASSWORD);

    connect(m_client, &QMqttClient::connected, this, &MqttWidget::onConnected);
    connect(m_client, &QMqttClient::disconnected, this, &MqttWidget::onDisconnected);
    connect(m_client, &QMqttClient::messageReceived, this, [this](const QByteArray &msg, const QMqttTopicName &topic) {
        onMessageReceived(msg, topic.name());
    });

    m_tempTimer = new QTimer(this);
    m_tempTimer->setInterval(TEMP_PUBLISH_INTERVAL);
    connect(m_tempTimer, &QTimer::timeout, this, &MqttWidget::publishTemperature);
}

void MqttWidget::startConnection()
{
    if (m_client && m_client->state() == QMqttClient::Disconnected) {
        qDebug() << "开始连接MQTT服务器...";
        m_client->connectToHost();
    }
}

void MqttWidget::stopConnection()
{
    if (m_tempTimer) {
        m_tempTimer->stop();
    }

    if (m_client && m_client->state() == QMqttClient::Connected) {
        qDebug() << "断开MQTT连接...";
        m_client->disconnectFromHost();
    }
}

MqttWidget::~MqttWidget()
{
    if (m_tempTimer) {
        m_tempTimer->stop();
        delete m_tempTimer;
    }
}

void MqttWidget::setupUi()
{
    setWindowTitle("MQTT控制面板");
    resize(400, 300);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *lblTitle = new QLabel("速度控制", this);
    lblTitle->setAlignment(Qt::AlignCenter);
    QFont titleFont;
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    lblTitle->setFont(titleFont);
    mainLayout->addWidget(lblTitle);

    QHBoxLayout *speedLayout = new QHBoxLayout();

    m_btnSpeedDown = new QPushButton("速度 -", this);
    m_btnSpeedDown->setMinimumSize(80, 40);
    speedLayout->addWidget(m_btnSpeedDown);

    m_lblSpeed = new QLabel("0", this);
    m_lblSpeed->setAlignment(Qt::AlignCenter);
    m_lblSpeed->setMinimumSize(100, 40);
    m_lblSpeed->setFrameStyle(QFrame::Box | QFrame::Sunken);
    QFont speedFont;
    speedFont.setPointSize(16);
    speedFont.setBold(true);
    m_lblSpeed->setFont(speedFont);
    speedLayout->addWidget(m_lblSpeed);

    m_btnSpeedUp = new QPushButton("速度 +", this);
    m_btnSpeedUp->setMinimumSize(80, 40);
    speedLayout->addWidget(m_btnSpeedUp);

    mainLayout->addLayout(speedLayout);

    m_btnPublishSpeed = new QPushButton("发布速度", this);
    m_btnPublishSpeed->setMinimumSize(0, 45);
    QFont btnFont;
    btnFont.setPointSize(11);
    m_btnPublishSpeed->setFont(btnFont);
    mainLayout->addWidget(m_btnPublishSpeed);

    m_btnLedToggle = new QPushButton("LED控制: 关闭", this);
    m_btnLedToggle->setMinimumSize(0, 45);
    m_btnLedToggle->setFont(btnFont);
    mainLayout->addWidget(m_btnLedToggle);

    m_btnStm32LedToggle = new QPushButton("STM32 LED控制: 关闭", this);
    m_btnStm32LedToggle->setMinimumSize(0, 45);
    m_btnStm32LedToggle->setFont(btnFont);
    mainLayout->addWidget(m_btnStm32LedToggle);

    m_txtStm32Temp = new QLineEdit(this);
    m_txtStm32Temp->setReadOnly(true);
    m_txtStm32Temp->setPlaceholderText("等待STM32温度数据...");
    m_txtStm32Temp->setAlignment(Qt::AlignCenter);
    QFont tempFont;
    tempFont.setPointSize(14);
    m_txtStm32Temp->setFont(tempFont);
    mainLayout->addWidget(m_txtStm32Temp);

    QPushButton *btnReturn = new QPushButton("返回主界面", this);
    btnReturn->setMinimumSize(0, 50);
    QFont returnFont;
    returnFont.setPointSize(12);
    returnFont.setBold(true);
    btnReturn->setFont(returnFont);
    btnReturn->setStyleSheet("background-color: #ff6b6b; color: white; border: none; border-radius: 5px;");
    mainLayout->addWidget(btnReturn);

    mainLayout->addStretch();

    connect(m_btnSpeedUp, &QPushButton::clicked, this, &MqttWidget::onSpeedUp);
    connect(m_btnSpeedDown, &QPushButton::clicked, this, &MqttWidget::onSpeedDown);
    connect(m_btnPublishSpeed, &QPushButton::clicked, this, &MqttWidget::onPublishSpeed);
    connect(m_btnLedToggle, &QPushButton::clicked, this, &MqttWidget::onToggleLed);
    connect(m_btnStm32LedToggle, &QPushButton::clicked, this, &MqttWidget::onToggleStm32Led);
    connect(btnReturn, &QPushButton::clicked, this, &MqttWidget::returnRequested);
}

void MqttWidget::onConnected()
{
    qDebug() << "MQTT服务器连接成功";

    controlLed("echo none > /sys/class/leds/sys-led/trigger");
    controlLed("echo 0 > /sys/class/leds/sys-led/brightness");

    m_client->publish(QMqttTopicName("dt_mqtt/will"), "Online", 0, true);

    m_client->subscribe(QMqttTopicFilter(LED_TOPIC), 0);
    m_client->subscribe(QMqttTopicFilter(STM32TEMP_TOPIC), 0);

    publishTemperature();
    m_tempTimer->start();
}

void MqttWidget::onDisconnected()
{
    qDebug() << "与MQTT服务器断开连接";
    m_tempTimer->stop();
}

void MqttWidget::onMessageReceived(const QByteArray &message, const QString &topic)
{
    QString msgStr = QString::fromUtf8(message);

    qDebug() << "收到消息 - 主题:" << topic << "内容:" << msgStr;

    if (topic == LED_TOPIC) {
        if (msgStr == "2") {
            controlLed("echo heartbeat > /sys/class/leds/sys-led/trigger");
        } else if (msgStr == "1") {
            controlLed("echo none > /sys/class/leds/sys-led/trigger");
            controlLed("echo 1 > /sys/class/leds/sys-led/brightness");
        } else if (msgStr == "0") {
            controlLed("echo none > /sys/class/leds/sys-led/trigger");
            controlLed("echo 0 > /sys/class/leds/sys-led/brightness");
        } else {
            qDebug() << "未识别的LED指令:" << msgStr;
        }
    } else if (topic == STM32TEMP_TOPIC) {
        m_txtStm32Temp->setText(msgStr);
        qDebug() << "更新STM32温度显示:" << msgStr;
    }
}

void MqttWidget::publishTemperature()
{
    QFile file("/sys/class/thermal/thermal_zone0/temp");
    if (file.open(QIODevice::ReadOnly)) {
        QString tempStr = file.readLine().trimmed();
        file.close();
        m_client->publish(QMqttTopicName(TEMP_TOPIC), tempStr.toUtf8(), 0, true);
    } else {
        qWarning() << "无法读取温度文件";
    }
}

void MqttWidget::controlLed(const QString &cmd)
{
    QProcess::execute("sh", QStringList() << "-c" << cmd);
}

void MqttWidget::onSpeedUp()
{
    m_speed += SPEED_STEP;
    m_lblSpeed->setText(QString::number(m_speed));
}

void MqttWidget::onSpeedDown()
{
    m_speed -= SPEED_STEP;
    m_lblSpeed->setText(QString::number(m_speed));
}

void MqttWidget::onPublishSpeed()
{
    if (m_client->state() == QMqttClient::Connected) {
        QString speedStr = QString::number(m_speed);
        m_client->publish(QMqttTopicName(MOTOR_TOPIC), speedStr.toUtf8(), 0, false);
        qDebug() << "发布速度到" << MOTOR_TOPIC << ":" << speedStr;
    } else {
        qWarning() << "MQTT未连接，无法发布速度";
    }
}

void MqttWidget::onToggleLed()
{
    m_ledState = !m_ledState;

    if (m_client->state() == QMqttClient::Connected) {
        QString cmd = m_ledState ? "1" : "0";
        m_client->publish(QMqttTopicName(LED_TOPIC), cmd.toUtf8(), 0, false);
        qDebug() << "发布LED控制到" << LED_TOPIC << ":" << cmd;
    } else {
        qWarning() << "MQTT未连接，无法发布LED控制";
    }

    m_btnLedToggle->setText(m_ledState ? "LED控制: 开启" : "LED控制: 关闭");
}

void MqttWidget::onToggleStm32Led()
{
    m_stm32LedState = !m_stm32LedState;

    if (m_client->state() == QMqttClient::Connected) {
        QString cmd = m_stm32LedState ? "1" : "0";
        m_client->publish(QMqttTopicName(STM32LED_TOPIC), cmd.toUtf8(), 0, false);
        qDebug() << "发布STM32 LED控制到" << STM32LED_TOPIC << ":" << cmd;
    } else {
        qWarning() << "MQTT未连接，无法发布STM32 LED控制";
    }

    m_btnStm32LedToggle->setText(m_stm32LedState ? "STM32 LED控制: 开启" : "STM32 LED控制: 关闭");
}

void MqttWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    if (!backgroundPixmap.isNull()) {
        painter.drawPixmap(this->rect(), backgroundPixmap);
    }

    QWidget::paintEvent(event);
}

void MqttWidget::setWidgetBackground()
{
    QPixmap bgPix(SENSOR_BACK_PATH);

    if (bgPix.isNull()) {
        qDebug() << "MQTT背景图片加载失败，路径：" << SENSOR_BACK_PATH;
        return;
    }

    int windowWidth = this->width();
    int windowHeight = this->height();
    backgroundPixmap = bgPix.scaled(windowWidth, windowHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}