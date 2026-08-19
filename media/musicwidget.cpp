#include "musicwidget.h"
#include "commonresource.h"
#include <QDir>
#include <QRandomGenerator>
#include <QDebug>


MusicWidget::MusicWidget(QWidget *parent) : QWidget(parent)
{
    this->resize(1024, 600);

    this->setStyleSheet("QWidget { background-image: url(:/images/background.png); background-repeat: no-repeat; }");

    btn_back = new QPushButton(this);
    btn_back->setText("返回");
    btn_back->setGeometry(20, 20, 100, 40);
    btn_back->setStyleSheet("QPushButton { background-color: rgba(255, 255, 255, 180); border-radius: 5px; } QPushButton:hover { background-color: rgba(255, 255, 255, 220); }");

    list_musics = new QListWidget(this);
    list_musics->setObjectName("list_musics");
    list_musics->setGeometry(56, 80, 400, 400);
    list_musics->setStyleSheet("QListWidget { background-color: rgba(0, 0, 0, 120); color: white; border-radius: 10px; } QListWidget::item { height: 50px; padding: 5px; } QListWidget::item:selected { background-color: rgba(170, 0, 0, 150); }");

    btn_prev = new QPushButton(this);
    btn_prev->setObjectName("btn_prev_music");
    btn_prev->setGeometry(50, 495, 80, 80);
    btn_prev->setIcon(QIcon(":/images/music_prev.png"));
    btn_prev->setIconSize(QSize(60, 60));
    btn_prev->setStyleSheet("QPushButton { border: none; background: transparent; }");

    btn_play = new QPushButton(this);
    btn_play->setCheckable(true);
    btn_play->setObjectName("btn_play_music");
    btn_play->setGeometry(216, 495, 80, 80);
    btn_play->setIcon(QIcon(":/images/music_play.png"));
    btn_play->setIconSize(QSize(70, 70));
    btn_play->setStyleSheet("QPushButton { border: none; background: transparent; }");

    btn_next = new QPushButton(this);
    btn_next->setObjectName("btn_next_music");
    btn_next->setGeometry(382, 495, 80, 80);
    btn_next->setIcon(QIcon(":/images/music_next.png"));
    btn_next->setIconSize(QSize(60, 60));
    btn_next->setStyleSheet("QPushButton { border: none; background: transparent; }");

    label_cd = new QLabel(this);
    label_cd->setObjectName("label_cd");
    label_cd->setGeometry(608, 80, 320, 320);
    label_cd->setPixmap(QPixmap(":/images/music_cd.png").scaled(320, 320, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    label_cd->setAlignment(Qt::AlignCenter);
    label_cd->setStyleSheet("QLabel { background: transparent; }");

    slider_play = new QSlider(Qt::Horizontal, this);
    slider_play->setObjectName("slider_music");
    slider_play->setGeometry(588, 420, 360, 20);
    slider_play->setStyleSheet("QSlider::groove:horizontal { border: 1px solid #999999; height: 8px; background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #333333, stop:1 #666666); margin: 2px 0px; border-radius: 4px; } QSlider::handle:horizontal { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #aaaaaa, stop:1 #ffffff); border: 1px solid #888888; width: 18px; margin: -6px 0px; border-radius: 9px; } QSlider::sub-page:horizontal { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #aa0000, stop:1 #ff0000); border-radius: 4px; }");

    label_curr_time = new QLabel(this);
    label_curr_time->setGeometry(588, 440, 40, 30);
    label_curr_time->setText("00:00");
    label_curr_time->setStyleSheet("QLabel { color: white; background: transparent; font-weight: bold; }");

    label_total_time = new QLabel(this);
    label_total_time->setGeometry(908, 440, 40, 30);
    label_total_time->setText("--:--");
    label_total_time->setAlignment(Qt::AlignRight);
    label_total_time->setStyleSheet("QLabel { color: white; background: transparent; font-weight: bold; }");

    QFont font;
    font.setPixelSize(12);
    label_curr_time->setFont(font);
    label_total_time->setFont(font);

    btn_like = new QPushButton(this);
    btn_like->setCheckable(true);
    btn_like->setObjectName("btn_like");
    btn_like->setGeometry(588, 505, 60, 60);
    btn_like->setIcon(QIcon(":/images/music_unlike.png"));
    btn_like->setIconSize(QSize(45, 45));
    btn_like->setStyleSheet("QPushButton { border: none; background: transparent; }");

    btn_mode = new QPushButton(this);
    btn_mode->setObjectName("btn_mode");
    btn_mode->setGeometry(688, 505, 60, 60);
    btn_mode->setIcon(QIcon(":/images/music_mode_list.png"));
    btn_mode->setIconSize(QSize(45, 45));
    btn_mode->setProperty("data-state", 0);
    btn_mode->setStyleSheet("QPushButton { border: none; background: transparent; }");

    btn_list = new QPushButton(this);
    btn_list->setObjectName("btn_list");
    btn_list->setGeometry(788, 505, 60, 60);
    btn_list->setIcon(QIcon(":/images/music_list.png"));
    btn_list->setIconSize(QSize(45, 45));
    btn_list->setStyleSheet("QPushButton { border: none; background: transparent; }");

    btn_volume = new QPushButton(this);
    btn_volume->setObjectName("btn_volume");
    btn_volume->setGeometry(888, 505, 60, 60);
    btn_volume->setIcon(QIcon(":/images/music_volume.png"));
    btn_volume->setIconSize(QSize(45, 45));
    btn_volume->setStyleSheet("QPushButton { border: none; background: transparent; }");

    slider_volume = new QSlider(Qt::Vertical, this);
    slider_volume->setObjectName("slider_volume");
    slider_volume->setRange(0, 100);
    slider_volume->setValue(50);
    slider_volume->setGeometry(960, 335, 20, 200);
    slider_volume->hide();

    mode_loop = MODE_LIST;
    is_slider_pressed = false;

    connect(btn_back, &QPushButton::clicked, this, &MusicWidget::btnBackClicked);
    connect(btn_play, &QPushButton::clicked, this, &MusicWidget::btnPlayClicked);
    connect(btn_play, &QPushButton::toggled, this, &MusicWidget::btnPlayToggled);
    connect(btn_prev, &QPushButton::clicked, this, &MusicWidget::btnPrevClicked);
    connect(btn_next, &QPushButton::clicked, this, &MusicWidget::btnNextClicked);
    connect(btn_like, &QPushButton::clicked, this, &MusicWidget::btnLikeClicked);
    connect(btn_mode, &QPushButton::clicked, this, &MusicWidget::btnModeClicked);
    connect(btn_list, &QPushButton::clicked, this, &MusicWidget::btnListClicked);
    connect(btn_volume, &QPushButton::clicked, this, &MusicWidget::btnVolumeClicked);

    connect(list_musics, &QListWidget::itemClicked, this, &MusicWidget::listWidgetCliked);

    connect(slider_play, &QSlider::sliderPressed, this, &MusicWidget::sliderPlayPressed);
    connect(slider_play, &QSlider::sliderReleased, this, &MusicWidget::sliderPlayReleased);

    connect(slider_volume, &QSlider::sliderReleased, this, &MusicWidget::sliderVolumeReleased);
    connect(slider_volume, &QSlider::valueChanged, this, &MusicWidget::sliderVolumeChanged);

    scanMusic();
}

void MusicWidget::btnBackClicked()
{
    stop_play_thread();

    btn_play->setChecked(false);
    btn_play->setIcon(QIcon(":/images/music_play.png"));

    emit returnRequested();
}

void MusicWidget::scanMusic()
{
    QDir dir(MUSIC_PATH);

    QStringList filters;
    filters << "*.mp3" << "*.wav" << "*.flag";

    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Name);
    for (int i = 0; i < files.count(); i++) {
        music_files.append(files.at(i).fileName());
        QString list_item = QString::fromUtf8(files.at(i).fileName().replace(".mp3", "").toUtf8().data());
        list_musics->addItem(list_item.split("-").at(0) + "\n" + "----" + list_item.split("-").at(1));
    }

    music_cnt = music_files.size();
    curr_indx = 0;
}

void MusicWidget::create_play_thread()
{
    if (!music_play) {
        thread = new QThread(this);
        music_play = new MusicPlay(music_files[curr_indx]);
        music_play->moveToThread(thread);

        connect(thread, &QThread::started, music_play, &MusicPlay::start);
        connect(music_play, &MusicPlay::play_complete, this, &MusicWidget::playComplete);
        connect(music_play, &MusicPlay::send_total_time, this, &MusicWidget::setTotalTime);
        connect(music_play, &MusicPlay::send_curr_time, this, &MusicWidget::setCurrTime);

        list_musics->setCurrentRow(curr_indx);
    }
}

void MusicWidget::stop_play_thread()
{
    if (music_play) {
        music_play->stop();

        thread->quit();
        thread->wait();

        delete music_play;
        music_play = nullptr;

        delete thread;
        thread = nullptr;
    }
}

void MusicWidget::btnPlayClicked()
{
    bool isChecked = btn_play->isChecked();

    if (isChecked) {
        if (!music_play) {
            create_play_thread();
        }
        if (!thread->isRunning()) {
            thread->start();
        }
        else {
            music_play->resume();
        }
    }
    else {
        if (music_play) {
            music_play->pause();
        }
    }
}

void MusicWidget::btnPlayToggled(bool checked)
{
    if (checked) {
        btn_play->setIcon(QIcon(":/images/music_pause.png"));
    }
    else {
        btn_play->setIcon(QIcon(":/images/music_play.png"));
    }
}

void MusicWidget::btnPrevClicked()
{
    stop_play_thread();

    curr_indx--;
    if (curr_indx < 0) {
        curr_indx = music_cnt - 1;
    }

    create_play_thread();
    if (btn_play->isChecked()) {
        thread->start();
    }
}

void MusicWidget::btnNextClicked()
{
    stop_play_thread();

    curr_indx++;
    if (curr_indx >= music_cnt) {
        curr_indx = 0;
    }

    create_play_thread();
    if (btn_play->isChecked()) {
        thread->start();
    }
}

void MusicWidget::setCurrTime(int curr_time)
{
    int minute = curr_time / 60;
    int second = curr_time % 60;

    QString music_position;
    music_position.clear();

    if (minute >= 10) {
        music_position = QString::number(minute, 10);
    } else {
        music_position = "0" + QString::number(minute, 10);
    }

    if (second >= 10) {
        music_position = music_position + ":" + QString::number(second, 10);
    } else {
        music_position = music_position + ":0" + QString::number(second, 10);
    }

    label_curr_time->setText(music_position);

    if (!is_slider_pressed) {
        slider_play->setValue(curr_time);
    }
}

void MusicWidget::setTotalTime(int total_time)
{
    int minute = total_time / 60;
    int second = total_time % 60;

    QString music_duration;
    music_duration.clear();

    if (minute >= 10) {
        music_duration = QString::number(minute, 10);
    } else {
        music_duration = "0" + QString::number(minute, 10);
    }

    if (second >= 10) {
        music_duration = music_duration + ":" + QString::number(second, 10);
    } else {
        music_duration = music_duration + ":0" + QString::number(second, 10);
    }

    label_total_time->setText(music_duration);

    slider_play->setRange(0, total_time);
}

void MusicWidget::playComplete()
{
    stop_play_thread();

    switch (mode_loop) {
    case MODE_LIST:
        curr_indx++;
        if (curr_indx >= music_cnt) {
            curr_indx = 0;
        }
        break;
    case MODE_SINGLE:
        break;
    case MODE_RANDOM:
        curr_indx = QRandomGenerator::global()->bounded(0, music_cnt);
        break;
    default:
        break;
    }

    create_play_thread();
    if (btn_play->isChecked()) {
        thread->start();
    }
}

void MusicWidget::listWidgetCliked(QListWidgetItem *item)
{
    stop_play_thread();
    curr_indx = list_musics->row(item);
    create_play_thread();
    if (btn_play->isChecked()) {
        thread->start();
    }
}

void MusicWidget::sliderPlayPressed()
{
    is_slider_pressed = true;
}

void MusicWidget::sliderPlayReleased()
{
    is_slider_pressed = false;
    int play_position = slider_play->value();

    music_play->set_seek(play_position);
}

void MusicWidget::btnLikeClicked()
{
    if (btn_like->isChecked()) {
        btn_like->setIcon(QIcon(":/images/music_like.png"));
    }
    else {
        btn_like->setIcon(QIcon(":/images/music_unlike.png"));
    }
}

void MusicWidget::btnModeClicked()
{
    int state = btn_mode->property("data-state").toInt();
    mode_loop = (state + 1) % 3;
    btn_mode->setProperty("data-state", mode_loop);

    switch (mode_loop) {
    case MODE_LIST:
        btn_mode->setIcon(QIcon(":/images/music_mode_list.png"));
        break;
    case MODE_SINGLE:
        btn_mode->setIcon(QIcon(":/images/music_mode_single.png"));
        break;
    case MODE_RANDOM:
        btn_mode->setIcon(QIcon(":/images/music_mode_random.png"));
        break;
    }

    btn_mode->style()->polish(btn_mode);
}

void MusicWidget::btnListClicked()
{
}

void MusicWidget::btnVolumeClicked()
{
    if (slider_volume->isVisible()) {
        slider_volume->setVisible(false);
    } else {
        slider_volume->setVisible(true);
    }
}

void MusicWidget::sliderVolumeReleased()
{
}

void MusicWidget::sliderVolumeChanged(int value)
{
    if (music_play) {
        music_play->set_volume(value);
        qDebug() << "set volume: " << value << endl;
    }
}