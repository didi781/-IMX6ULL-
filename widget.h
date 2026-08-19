#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QStackedWidget>
#include <QPixmap>
#include <QProcess>
#include <QTimer>

// 使用 Qt 资源系统加载图片（编译后图片内嵌到可执行文件中）
#define Main_Interface_PATH ":/images/Main-Interface.jpg"

// NES游戏外部可执行文件路径
#define NES_GAME_PATH "/home/root/Qt"  // TODO: 修改为你自己的NES游戏可执行文件路径

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class MqttWidget;
class SensorWidget;
class MusicWidget;
class VideoWidget;
class CameraWidget;
class AlbumWidget;
class BookWidget;

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    void setWidgetBackground();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onMqttButtonClicked();
    void onSensorButtonClicked();
    void onMusicButtonClicked();
    void onVideosButtonClicked();
    void onCameraButtonClicked();
    void onPhotosButtonClicked();
    void onBookButtonClicked();
    void onGamesButtonClicked();
    void onReturnToMain();
    void onGameFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void checkGameProcessState();

private:
    Ui::Widget *ui;
    QPixmap backgroundPixmap;

    QStackedWidget *stackedWidget;
    QWidget *mainPage;
    MqttWidget *mqttPage;
    SensorWidget *sensorPage;
    MusicWidget *musicPage;
    VideoWidget *videosPage;
    CameraWidget *cameraPage;
    AlbumWidget *albumPage;
    BookWidget *bookPage;

    QProcess *gameProcess;
    QTimer *checkTimer;
};
#endif // WIDGET_H