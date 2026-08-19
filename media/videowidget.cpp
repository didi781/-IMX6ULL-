#include "videowidget.h"
#include "commonresource.h"
#include <QDebug>
#include <QDir>
#include <QTimer>


VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent)
{
    this->resize(1024, 600);

    this->setStyleSheet("QWidget { background-image: url(:/images/background.png); background-repeat: no-repeat; }");

    btn_back = new QPushButton(this);
    btn_back->setText("返回");
    btn_back->setGeometry(20, 20, 100, 40);
    btn_back->setStyleSheet("QPushButton { background-color: rgba(255, 255, 255, 180); border-radius: 5px; } QPushButton:hover { background-color: rgba(255, 255, 255, 220); }");

    label_list = new QLabel(this);
    label_list->setGeometry(20, 80, 120, 40);
    label_list->setText(" 视频文件列表:");
    label_list->setObjectName("label_list");
    label_list->setStyleSheet("QLabel { color: white; font-weight: bold; }");

    list_videos = new QListWidget(this);
    list_videos->setObjectName("list_videos");
    list_videos->setGeometry(20, 120, 280, 360);
    list_videos->setStyleSheet("QListWidget { background-color: rgba(0, 0, 0, 120); color: white; border-radius: 10px; } QListWidget::item { height: 40px; padding: 5px; } QListWidget::item:selected { background-color: rgba(170, 0, 0, 150); }");

    label_video = new QLabel(this);
    label_video->setObjectName("label_video");
    label_video->setGeometry(340, 40, 600, 450);
    label_video->setStyleSheet("QLabel { background-color: black; }");
    label_video->setAlignment(Qt::AlignCenter);

    slider_video = new QSlider(Qt::Horizontal, this);
    slider_video->setObjectName("slider_video");
    slider_video->setGeometry(20, 500, 984, 20);

    btn_play = new QPushButton(this);
    btn_play->setObjectName("btn_play_video");
    btn_play->setGeometry(20, 540, 40, 40);
    btn_play->setCheckable(true);
    btn_play->setIcon(QIcon(":/images/music_play.png"));
    btn_play->setIconSize(QSize(30, 30));
    btn_play->setStyleSheet("QPushButton { border: none; background: transparent; }");

    btn_next = new QPushButton(this);
    btn_next->setObjectName("btn_next_video");
    btn_next->setGeometry(80, 540, 40, 40);
    btn_next->setIcon(QIcon(":/images/music_next.png"));
    btn_next->setIconSize(QSize(30, 30));
    btn_next->setStyleSheet("QPushButton { border: none; background: transparent; }");

    label_curr_time = new QLabel(this);
    label_curr_time->setGeometry(300, 540, 60, 40);
    label_curr_time->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    label_curr_time->setText("00:00");
    label_curr_time->setStyleSheet("QLabel { color: white; background: transparent; }");

    label_total_time = new QLabel(this);
    label_total_time->setGeometry(360, 540, 80, 40);
    label_total_time->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    label_total_time->setText(" / --:--");
    label_total_time->setStyleSheet("QLabel { color: white; background: transparent; }");

    QFont font;
    font.setPixelSize(20);
    label_curr_time->setFont(font);
    label_total_time->setFont(font);

    btn_volume = new QPushButton(this);
    btn_volume->setObjectName("btn_volume_video");
    btn_volume->setGeometry(804, 540, 40, 40);
    btn_volume->setIcon(QIcon(":/images/music_volume.png"));
    btn_volume->setIconSize(QSize(30, 30));
    btn_volume->setStyleSheet("QPushButton { border: none; background: transparent; }");

    slider_volume = new QSlider(this);
    slider_volume->setObjectName("slider_volume_video");
    slider_volume->setRange(0, 100);
    slider_volume->setValue(50);
    slider_volume->setGeometry(770, 360, 20, 200);
    slider_volume->hide();    // 默认隐藏

    btn_scale = new QPushButton(this);
    btn_scale->setObjectName("btn_scale");
    btn_scale->setGeometry(944, 540, 40, 40);
    btn_scale->setCheckable(true);
    btn_scale->setIcon(QIcon(":/images/video_big.png"));
    btn_scale->setIconSize(QSize(30, 30));
    btn_scale->setStyleSheet("QPushButton { border: none; background: transparent; }");

    connect(btn_back, &QPushButton::clicked, this, &VideoWidget::btnBackClicked);
    connect(btn_play, &QPushButton::clicked, this, &VideoWidget::btnPlayClicked);
    connect(btn_next, &QPushButton::clicked, this, &VideoWidget::btnNextClicked);
    connect(btn_volume, &QPushButton::clicked, this, &VideoWidget::btnVolumeClicked);
    connect(btn_scale, &QPushButton::clicked, this, &VideoWidget::btnScaleClicked);
    connect(list_videos, &QListWidget::itemClicked, this, &VideoWidget::listVideosCliked);
    connect(slider_volume, &QSlider::sliderReleased, this, &VideoWidget::sliderVolumeReleased);

    scanVideo();
}

void VideoWidget::btnBackClicked()
{
    stop_play_thread();

    btn_play->setChecked(false);

    label_video->clear();

    emit returnRequested();
}

void VideoWidget::scanVideo()
{
    QDir dir(VIDEO_PATH);

    QStringList filters;    // 定义过滤器
    filters << "*.mp4" << "*.MP4" << "*.m4v" << "*.M4V" << "*.mkv" << "*.MKV" << "*.avi" << "*.AVI";

    /* 获取目录下所有符合条件的文件 */
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Name);    // 按名字排序
    for (int i = 0; i < files.count(); i++) {
        video_files.append(files.at(i).fileName());
        QString list_item = QString::fromUtf8(files.at(i).fileName().toUtf8().data());
        list_videos->addItem(list_item);    // 添加到 listWidget
    }

    video_cnt = video_files.size();
    curr_indx = 0;
}

void VideoWidget::create_play_thread()
{
    if (is_cleaning_up) {
        qDebug() << "Previous cleanup not finished, forcing cleanup now";
        force_cleanup();
    }
    
    if (!video_play) {
        thread = new QThread(this);
        video_play = new VideoPlay(video_files[curr_indx]);
        video_play->moveToThread(thread);

        connect(thread, &QThread::started, video_play, &VideoPlay::start);
        connect(video_play, &VideoPlay::sig_total_time, this, &VideoWidget::setTotalTime);
        connect(video_play, &VideoPlay::sig_curr_time, this, &VideoWidget::setCurrTime);
        connect(video_play, &VideoPlay::sig_play_complete, this, &VideoWidget::playComplete);
        connect(video_play, &VideoPlay::sig_ready_frame, this, &VideoWidget::displayFrame);

        list_videos->setCurrentRow(curr_indx);    // 刷新列表 list_videos
    }
}

void VideoWidget::force_cleanup()
{
    if (thread && thread->isRunning()) {
        qWarning() << "Force terminating thread";
        thread->terminate();
        thread->wait(200);  // 给200ms让操作系统完成终止
    }
    
    if (video_play) {
        delete video_play;
        video_play = nullptr;
    }
    
    if (thread) {
        delete thread;
        thread = nullptr;
    }
    
    is_cleaning_up = false;
    qDebug() << "Force cleanup completed";
}

void VideoWidget::stop_play_thread()
{
    if (video_play) {
        disconnect(thread, nullptr, this, nullptr);
        disconnect(video_play, nullptr, this, nullptr);
        
        video_play->force_close();
        thread->quit();
        
        is_cleaning_up = true;
        
        // 延迟1秒后执行安全清理（给线程足够时间优雅退出）
        QTimer::singleShot(1000, this, [this]() {
            if (thread) {
                if (thread->isRunning()) {
                    qWarning() << "Thread still running after 1s, force cleanup";
                    force_cleanup();
                } else {
                    delete video_play;
                    video_play = nullptr;
                    delete thread;
                    thread = nullptr;
                    is_cleaning_up = false;
                    qDebug() << "Cleanup completed gracefully";
                }
            }
        });
        
        qDebug() << "Stop command sent, will cleanup later";
    }
}

void VideoWidget::onThreadFinished()
{
    qDebug() << "Play thread finished";
}

void VideoWidget::btnPlayClicked()
{
    bool isChecked = btn_play->isChecked();  // 获取当前状态

    if (isChecked) {
        if (!video_play) {
            create_play_thread();   // 创建线程
        }
        if (thread && !thread->isRunning()) {
            thread->start();        // 启动线程
        }
        else if (video_play) {
            video_play->resume();   // 恢复线程
        }
    }
    else {
        if (video_play) {
            video_play->pause();    // 暂停线程
        }
    }
}

void VideoWidget::btnNextClicked()
{
    stop_play_thread();
    
    curr_indx++;
    if (curr_indx >= video_cnt) {
        curr_indx = 0;
    }

    create_play_thread();

    if (btn_play->isChecked() && thread) {
        thread->start();
    }
}

void VideoWidget::btnScaleClicked()
{

}

void VideoWidget::btnVolumeClicked()
{
    if (slider_volume->isVisible()) {
        slider_volume->setVisible(false);
    } else {
        slider_volume->setVisible(true);
    }
}

void VideoWidget::sliderVolumeReleased()
{
    if (video_play) {
        video_play->set_volume(slider_volume->value());
        int volume = slider_volume->value();
        qDebug() << "set volume: " << volume << endl;
    }
}

void VideoWidget::listVideosCliked(QListWidgetItem *item)
{
    stop_play_thread();
    curr_indx = list_videos->row(item);
    create_play_thread();
    if (btn_play->isChecked() && thread) {
        thread->start();
    }
}

void VideoWidget::setCurrTime(int curr_time)
{
    /* 设置标签 */
    int minute = curr_time / 60;
    int second = curr_time % 60;

    QString video_position;
    video_position.clear();

    if (minute >= 10) {
        video_position = QString::number(minute, 10);
    } else {
        video_position = "0" + QString::number(minute, 10);
    }

    if (second >= 10) {
        video_position = video_position + ":" + QString::number(second, 10);
    } else {
        video_position = video_position + ":0" + QString::number(second, 10);
    }

    label_curr_time->setText(video_position);

    /* 刷新进度条 */
    if (!slider_video->isSliderDown()) {
        slider_video->setValue(curr_time);
    }
}

void VideoWidget::setTotalTime(int total_time)
{
    /* 设置标签 */
    int minute = total_time / 60;
    int second = total_time % 60;

    QString video_duration;
    video_duration.clear();

    if (minute >= 10) {
        video_duration = QString::number(minute, 10);  // 把整数转为字符串，以十进制形式
    } else {
        video_duration = "0" + QString::number(minute, 10);
    }

    if (second >= 10) {
        video_duration = " / " + video_duration + ":" + QString::number(second, 10);
    } else {
        video_duration = video_duration + ":0" + QString::number(second, 10);
    }

    label_total_time->setText(video_duration);

    /* 设置进度条范围 */
    slider_video->setRange(0, total_time);
}

void VideoWidget::playComplete()
{
    stop_play_thread();

    curr_indx++;
    if (curr_indx >= video_cnt) {
        curr_indx = 0;
    }

    create_play_thread();

    btn_play->setChecked(false);    // 停止状态
}

void VideoWidget::displayFrame(QImage image)
{
    static int frameCount = 0;
    static QElapsedTimer fpsTimer;
    static QElapsedTimer displayTimer;

    frameCount++;

    /* 性能监控：测量实际处理时间 */
    if (!displayTimer.isValid()) {
        displayTimer.start();
    }

    /* 帧率控制：限制最大帧率 */
    if (fpsTimer.isValid()) {
        qint64 elapsed = fpsTimer.elapsed();

        /* 动态计算目标帧间隔（30fps 作为平衡点） */
        qint64 targetInterval = 33;  // 33ms ≈ 30fps

        /* 如果距离上一帧时间太短，跳过此帧 */
        if (elapsed < targetInterval) {
            return;  // 跳过，避免事件队列堆积
        }
    }
    fpsTimer.start();  // 重置计时器

    if (!image.isNull()) {
        /* 直接使用 QImage，避免额外转换开销 */
        QPixmap pixmap = QPixmap::fromImage(image);

        /* 只在尺寸不匹配时才缩放 */
        if (pixmap.size() != label_video->size() && !label_video->size().isEmpty()) {
            pixmap = pixmap.scaled(label_video->size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
        }

        label_video->setPixmap(pixmap);

        /* 降低日志频率：每3秒输出一次（减少IO开销） */
        if (frameCount % 90 == 0) {  // 30fps × 3秒 = 90帧
            double actualFPS = frameCount / (displayTimer.elapsed() / 1000.0 + 0.01);
            qDebug() << "Frame" << frameCount << ", FPS:" << qRound(actualFPS)
                     << ", Process time:" << (fpsTimer.elapsed()) << "ms";
        }
    } else {
        static int nullWarningCount = 0;
        if (nullWarningCount++ < 5) {  // 只警告前5次
            qWarning() << "Frame" << frameCount << ": QImage is null!";
        }
    }
}