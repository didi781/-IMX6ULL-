#include "camera_widget.h"
#include "ui_camera_widget.h"
#include <QDebug>
#include <QDateTime>
#include <QMessageBox>
#include <QDir>

CameraWidget::CameraWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CameraWidget)
    , camera(nullptr)
{
    ui->setupUi(this);

    camera = new Camera(this);

    connect(camera, &Camera::readyImage, this, &CameraWidget::updateImage);

    scanCameraDevice();
}

CameraWidget::~CameraWidget()
{
    if (camera) {
        camera->cameraProcess(false);
    }
    delete ui;
}

void CameraWidget::scanCameraDevice()
{
#if win32
    ui->comboBox->addItem("windows摄像头0");
    connect(ui->comboBox,
            SIGNAL(currentIndexChanged(int)),
            this, SLOT(onComboBoxChanged(int)));
#else
    QFile file("/dev/video0");

    if (file.exists())
        ui->comboBox->addItem("video0");
    else {
        ui->displayLabel->setText("无摄像头设备");
        return;
    }

    file.setFileName("/dev/video1");

    if (file.exists()) {
        ui->comboBox->addItem("video1");
        ui->comboBox->setCurrentIndex(1);
    }

    file.setFileName("/dev/video2");

    if (file.exists())
        ui->comboBox->addItem("video2");

#if !__arm__
    ui->comboBox->setCurrentIndex(0);
#endif

    connect(ui->comboBox,
            SIGNAL(currentIndexChanged(int)),
            this, SLOT(onComboBoxChanged(int)));
#endif

    if (ui->comboBox->count() > 0) {
        camera->selectCameraDevice(ui->comboBox->currentIndex());
    }
}

void CameraWidget::onOpenCameraClicked()
{
    if (ui->comboBox->count() == 0) {
        QMessageBox::warning(this, "警告", "无可用摄像头设备！");
        return;
    }

    bool isOpened = camera->cameraProcess(true);

    if (isOpened) {
        ui->openButton->setEnabled(false);
        ui->closeButton->setEnabled(true);
        ui->captureButton->setEnabled(true);
        qDebug() << "摄像头已开启";
    } else {
        QMessageBox::critical(this, "错误", "无法打开摄像头设备！");
        qDebug() << "摄像头打开失败";
    }
}

void CameraWidget::onCloseCameraClicked()
{
    camera->cameraProcess(false);

    ui->openButton->setEnabled(true);
    ui->closeButton->setEnabled(false);
    ui->captureButton->setEnabled(false);

    qDebug() << "摄像头已关闭";
}

void CameraWidget::onCaptureClicked()
{
    const QPixmap *pixmapPtr = ui->displayLabel->pixmap();

    if (!pixmapPtr || pixmapPtr->isNull()) {
        QMessageBox::warning(this, "警告", "没有可保存的图像！");
        return;
    }

    QPixmap pixmap = *pixmapPtr;

    QString saveDir = "/home/root/mypictures";
    QDir dir(saveDir);
    if (!dir.exists()) {
        dir.mkpath(saveDir);
    }

    QString currentTime = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString fileName = QString("%1/photo_%2.png").arg(saveDir).arg(currentTime);

    if (pixmap.save(fileName)) {
        QMessageBox::information(this, "成功", QString("照片已保存至:\n%1").arg(fileName));
        qDebug() << "照片已保存:" << fileName;
    } else {
        QMessageBox::critical(this, "错误", "照片保存失败！\n请检查目录权限或磁盘空间");
        qDebug() << "照片保存失败:" << fileName;
    }
}

void CameraWidget::updateImage(const QImage &image)
{
    if (!image.isNull()) {
        QPixmap pixmap = QPixmap::fromImage(image);
        pixmap = pixmap.scaled(ui->displayLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->displayLabel->setPixmap(pixmap);
    }
}

void CameraWidget::onComboBoxChanged(int index)
{
    bool wasRunning = camera->cameraProcess(false);

    camera->selectCameraDevice(index);

    if (wasRunning) {
        camera->cameraProcess(true);
    }

    qDebug() << "切换到摄像头设备:" << index;
}

void CameraWidget::onReturnClicked()
{
    camera->cameraProcess(false);
    emit returnRequested();
}