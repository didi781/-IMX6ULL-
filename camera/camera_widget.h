#ifndef CAMERA_WIDGET_H
#define CAMERA_WIDGET_H

#include <QWidget>
#include <QFile>
#include <QImage>
#include "camera.h"

QT_BEGIN_NAMESPACE
namespace Ui { class CameraWidget; }
QT_END_NAMESPACE

class CameraWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CameraWidget(QWidget *parent = nullptr);
    ~CameraWidget();

    void scanCameraDevice();

private slots:
    void onReturnClicked();
    void onOpenCameraClicked();
    void onCloseCameraClicked();
    void onCaptureClicked();
    void updateImage(const QImage &image);
    void onComboBoxChanged(int index);

signals:
    void returnRequested();

private:
    Ui::CameraWidget *ui;
    Camera *camera;
};

#endif // CAMERA_WIDGET_H