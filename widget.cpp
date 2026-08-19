#include "widget.h"
#include "ui_widget.h"
#include "mqtt_widget.h"
#include "sensor/sensor_widget.h"
#include "media/musicwidget.h"
#include "media/videowidget.h"
#include "camera/camera_widget.h"
#include "camera/album_widget.h"
#include "book/book_widget.h"
#include <QDebug>
#include <QPainter>
#include <QPaintEvent>
#include <QScreen>
#include <QApplication>
#include <QVBoxLayout>
#include <QProcess>
#include <QFileInfo>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , stackedWidget(nullptr)
    , mainPage(nullptr)
    , mqttPage(nullptr)
    , sensorPage(nullptr)
    , musicPage(nullptr)
    , videosPage(nullptr)
    , cameraPage(nullptr)
    , albumPage(nullptr)
    , bookPage(nullptr)
    , gameProcess(nullptr)
    , checkTimer(nullptr)
{
    QScreen *screen = QApplication::primaryScreen();
    int screenWidth = 1024;
    int screenHeight = 600;

    if (screen) {
        QRect screenGeometry = screen->geometry();
        screenWidth = screenGeometry.width();
        screenHeight = screenGeometry.height();
        qDebug() << "屏幕尺寸:" << screenWidth << "x" << screenHeight;
    } else {
        qDebug() << "无法获取屏幕信息，使用默认尺寸: 1024x600";
    }

    setFixedSize(screenWidth, screenHeight);
    setAttribute(Qt::WA_StaticContents);

    stackedWidget = new QStackedWidget(this);
    stackedWidget->setFixedSize(screenWidth, screenHeight);

    mainPage = new QWidget();
    ui->setupUi(mainPage);
    setWidgetBackground();

    mqttPage = new MqttWidget();

    sensorPage = new SensorWidget();

    musicPage = new MusicWidget();

    videosPage = new VideoWidget();

    cameraPage = new CameraWidget();

    albumPage = new AlbumWidget();

    bookPage = new BookWidget();

    stackedWidget->addWidget(mainPage);   // 索引 0: 主界面
    stackedWidget->addWidget(mqttPage);   // 索引 1: MQTT界面
    stackedWidget->addWidget(sensorPage); // 索引 2: 传感器界面
    stackedWidget->addWidget(musicPage);  // 索引 3: 音乐播放器界面
    stackedWidget->addWidget(videosPage); // 索引 4: 视频播放器界面
    stackedWidget->addWidget(cameraPage); // 索引 5: 相机界面
    stackedWidget->addWidget(albumPage);  // 索引 6: 相册界面
    stackedWidget->addWidget(bookPage);   // 索引 7: 电子书界面
    stackedWidget->setCurrentIndex(0);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(stackedWidget);

    connect(ui->mqtt, &QPushButton::clicked, this, &Widget::onMqttButtonClicked);
    connect(ui->sensor, &QPushButton::clicked, this, &Widget::onSensorButtonClicked);
    connect(ui->music, &QPushButton::clicked, this, &Widget::onMusicButtonClicked);
    connect(ui->videos, &QPushButton::clicked, this, &Widget::onVideosButtonClicked);
    connect(ui->camera, &QPushButton::clicked, this, &Widget::onCameraButtonClicked);
    connect(ui->photos, &QPushButton::clicked, this, &Widget::onPhotosButtonClicked);
    connect(ui->book, &QPushButton::clicked, this, &Widget::onBookButtonClicked);
    connect(ui->games, &QPushButton::clicked, this, &Widget::onGamesButtonClicked);
    connect(mqttPage, &MqttWidget::returnRequested, this, &Widget::onReturnToMain);
    connect(sensorPage, &SensorWidget::returnRequested, this, &Widget::onReturnToMain);
    connect(musicPage, &MusicWidget::returnRequested, this, &Widget::onReturnToMain);
    connect(videosPage, &VideoWidget::returnRequested, this, &Widget::onReturnToMain);
    connect(cameraPage, &CameraWidget::returnRequested, this, &Widget::onReturnToMain);
    connect(albumPage, &AlbumWidget::returnRequested, this, &Widget::onReturnToMain);
    connect(bookPage, &BookWidget::returnRequested, this, &Widget::onReturnToMain);

    checkTimer = new QTimer(this);
    checkTimer->setInterval(1000);
    connect(checkTimer, &QTimer::timeout, this, &Widget::checkGameProcessState);
}

Widget::~Widget()
{
    if (checkTimer) {
        checkTimer->stop();
    }
    if (gameProcess) {
        gameProcess->disconnect();
        if (gameProcess->state() != QProcess::NotRunning) {
            gameProcess->terminate();
            if (!gameProcess->waitForFinished(3000)) {
                gameProcess->kill();
            }
        }
        delete gameProcess;
        gameProcess = nullptr;
    }
    delete ui;
}

void Widget::paintEvent(QPaintEvent *event)
{
    if (stackedWidget && stackedWidget->currentIndex() == 0) {
        QPainter painter(this);

        if (!backgroundPixmap.isNull()) {
            painter.drawPixmap(this->rect(), backgroundPixmap);
        }
    }

    QWidget::paintEvent(event);
}

void Widget::setWidgetBackground()
{
    QPixmap bgPix(Main_Interface_PATH);

    if (bgPix.isNull()) {
        qDebug() << "背景图片加载失败，路径：" << Main_Interface_PATH;
        return;
    }

    int windowWidth = this->width();
    int windowHeight = this->height();
    backgroundPixmap = bgPix.scaled(windowWidth, windowHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

void Widget::onMqttButtonClicked()
{
    stackedWidget->setCurrentIndex(1);
    mqttPage->startConnection();
    qDebug() << "切换到MQTT控制界面并启动连接";
}

void Widget::onSensorButtonClicked()
{
    stackedWidget->setCurrentIndex(2);
    qDebug() << "切换到传感器界面";
}

void Widget::onMusicButtonClicked()
{
    stackedWidget->setCurrentIndex(3);
    qDebug() << "切换到音乐播放器界面";
}

void Widget::onVideosButtonClicked()
{
    stackedWidget->setCurrentIndex(4);
    qDebug() << "切换到视频播放器界面";
}

void Widget::onCameraButtonClicked()
{
    stackedWidget->setCurrentIndex(5);
    qDebug() << "切换到相机界面";
}

void Widget::onPhotosButtonClicked()
{
    albumPage->loadPhotos();
    stackedWidget->setCurrentIndex(6);
    qDebug() << "切换到相册界面";
}

void Widget::onBookButtonClicked()
{
    bookPage->loadBookList();
    stackedWidget->setCurrentIndex(7);
    qDebug() << "切换到电子书界面";
}

void Widget::onGamesButtonClicked()
{
    qDebug() << "启动NES游戏外部程序:" << NES_GAME_PATH;

    if (gameProcess && gameProcess->state() != QProcess::NotRunning) {
        qDebug() << "游戏程序已在运行，忽略重复启动";
        return;
    }

    QFileInfo gameFile(NES_GAME_PATH);
    if (!gameFile.exists() || !gameFile.isExecutable()) {
        qDebug() << "游戏可执行文件不存在或不可执行:" << NES_GAME_PATH;
        return;
    }

    if (!gameProcess) {
        gameProcess = new QProcess(this);

        connect(gameProcess,
                QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this,
                [this](int exitCode, QProcess::ExitStatus exitStatus) {
            qDebug() << "[finished信号] 进程退出 exitCode=" << exitCode
                     << "exitStatus=" << exitStatus;
            onGameFinished(exitCode, exitStatus);
        });

        connect(gameProcess, &QProcess::stateChanged,
                this, [](QProcess::ProcessState newState) {
            qDebug() << "[stateChanged] 新状态:" << newState
                     << "(0=未运行,1=启动中,2=运行中)";
        });

        connect(gameProcess, &QProcess::errorOccurred,
                this, [this](QProcess::ProcessError error) {
            qDebug() << "[errorOccurred] 错误:" << error
                     << gameProcess->errorString();
        });
    }

    gameProcess->start(NES_GAME_PATH, QStringList());

    if (!gameProcess->waitForStarted(3000)) {
        qDebug() << "游戏程序启动失败:" << gameProcess->errorString();
        return;
    }

    qDebug() << "游戏程序已启动 PID=" << gameProcess->processId()
             << "，隐藏主界面，启动状态轮询定时器";

    this->hide();
    checkTimer->start();
}

void Widget::onGameFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "游戏程序已退出 exitCode=" << exitCode
             << "exitStatus=" << exitStatus;

    if (checkTimer) {
        checkTimer->stop();
    }

    this->show();
    this->raise();
    this->activateWindow();
    this->setFocus();

    qDebug() << "已返回主界面";
}

void Widget::checkGameProcessState()
{
    if (!gameProcess) {
        return;
    }

    QProcess::ProcessState state = gameProcess->state();
    qint64 pid = gameProcess->processId();

    if (state == QProcess::NotRunning) {
        qDebug() << "[轮询] 检测到游戏进程已不在运行 PID=" << pid
                 << "，触发返回主界面";
        checkTimer->stop();
        onGameFinished(-1, QProcess::NormalExit);
    }
}

void Widget::onReturnToMain()
{
    int currentIndex = stackedWidget->currentIndex();

    if (currentIndex == 1) {
        mqttPage->stopConnection();
        stackedWidget->setCurrentIndex(0);
        qDebug() << "断开MQTT连接并返回主界面";
    } else if (currentIndex == 2) {
        stackedWidget->setCurrentIndex(0);
        qDebug() << "从传感器界面返回主界面";
    } else if (currentIndex == 3) {
        stackedWidget->setCurrentIndex(0);
        qDebug() << "从音乐播放器界面返回主界面";
    } else if (currentIndex == 4) {
        stackedWidget->setCurrentIndex(0);
        qDebug() << "从视频播放器界面返回主界面";
    } else if (currentIndex == 5) {
        stackedWidget->setCurrentIndex(0);
        qDebug() << "从相机界面返回主界面";
    } else if (currentIndex == 6) {
        stackedWidget->setCurrentIndex(0);
        qDebug() << "从相册界面返回主界面";
    } else if (currentIndex == 7) {
        stackedWidget->setCurrentIndex(0);
        qDebug() << "从电子书界面返回主界面";
    }

    this->show();
    this->raise();
    this->activateWindow();

    qDebug() << "返回主界面完成";
}