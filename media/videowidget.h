#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QSlider>
#include <QStringList>
#include <QThread>
#include <QElapsedTimer>

#include "videoplay.h"


class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);

signals:
    void returnRequested();

private:
    QPushButton *btn_back;
    QPushButton *btn_play;
    QPushButton *btn_next;
    QPushButton *btn_volume;
    QPushButton *btn_scale;     // 全屏或缩小

    QLabel *label_video;
    QLabel *label_curr_time;
    QLabel *label_total_time;
    QLabel *label_list;

    QListWidget *list_videos;

    QSlider *slider_video;      // 播放进度条
    QSlider *slider_volume;     // 音量条

    QStringList video_files;  // 字符串数组: 存储视频文件名
    int video_cnt;            // 视频总数
    int curr_indx;            // 当前索引

    QThread *thread = nullptr;
    VideoPlay *video_play = nullptr;

    bool is_cleaning_up = false;   // 正在清理资源的标志

    QElapsedTimer m_frameTimer;    // 帧率控制计时器

    void scanVideo();             // 扫描视频文件
    void create_play_thread();    // 创建播放线程
    void stop_play_thread();      // 停止播放线程
    void force_cleanup();         // 强制清理资源

private slots:
    void btnBackClicked();
    void btnPlayClicked();
    void btnNextClicked();
    void btnVolumeClicked();
    void btnScaleClicked();

    void listVideosCliked(QListWidgetItem*);    // 列表单击
    void sliderVolumeReleased();                // 音量进度条松开

    void setCurrTime(int curr_time);      // 获取当前播放时间
    void setTotalTime(int total_time);    // 获取总时长
    void playComplete();                  // 当前视频播放完成

    void displayFrame(QImage image);      // 显示视频帧
    void onThreadFinished();              // 线程结束后的异步清理
};

#endif // VIDEOWIDGET_H