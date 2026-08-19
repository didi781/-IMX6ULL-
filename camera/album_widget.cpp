#include "album_widget.h"
#include "ui_album_widget.h"
#include <QDebug>
#include <QMessageBox>
#include <QPixmap>

AlbumWidget::AlbumWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AlbumWidget)
    , currentIndex(0)
{
    ui->setupUi(this);

    ui->prevButton->setEnabled(false);
    ui->nextButton->setEnabled(false);
    ui->deleteButton->setEnabled(false);

    loadPhotos();
}

AlbumWidget::~AlbumWidget()
{
    delete ui;
}

void AlbumWidget::loadPhotos()
{
    photoList.clear();
    currentIndex = 0;

    QString photoDir = "/home/root/mypictures";
    QDir dir(photoDir);

    if (!dir.exists()) {
        ui->photoLabel->setText("相册目录不存在");
        return;
    }

    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp" << "*.gif";
    photoList = dir.entryList(filters, QDir::Files | QDir::Readable, QDir::Time);

    if (photoList.isEmpty()) {
        ui->photoLabel->setText("暂无照片");
        ui->prevButton->setEnabled(false);
        ui->nextButton->setEnabled(false);
        ui->deleteButton->setEnabled(false);
        return;
    }

    updatePhotoDisplay();
}

void AlbumWidget::updatePhotoDisplay()
{
    if (photoList.isEmpty()) {
        ui->photoLabel->setText("暂无照片");
        ui->prevButton->setEnabled(false);
        ui->nextButton->setEnabled(false);
        ui->deleteButton->setEnabled(false);
        return;
    }

    QString photoPath = QString("/home/root/mypictures/%1").arg(photoList[currentIndex]);
    QPixmap pixmap(photoPath);

    if (!pixmap.isNull()) {
        pixmap = pixmap.scaled(ui->photoLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->photoLabel->setPixmap(pixmap);
    } else {
        ui->photoLabel->setText("无法加载图片");
    }

    ui->prevButton->setEnabled(currentIndex > 0);
    ui->nextButton->setEnabled(currentIndex < photoList.size() - 1);
    ui->deleteButton->setEnabled(true);

    ui->label_count->setText(QString("%1 / %2").arg(currentIndex + 1).arg(photoList.size()));
}

void AlbumWidget::onPrevClicked()
{
    if (currentIndex > 0) {
        currentIndex--;
        updatePhotoDisplay();
    }
}

void AlbumWidget::onNextClicked()
{
    if (currentIndex < photoList.size() - 1) {
        currentIndex++;
        updatePhotoDisplay();
    }
}

void AlbumWidget::onDeleteClicked()
{
    if (currentIndex >= photoList.size() || currentIndex < 0) {
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认删除",
                                  "确定要删除这张照片吗？",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QString photoPath = QString("/home/root/mypictures/%1").arg(photoList[currentIndex]);
        QFile file(photoPath);

        if (file.remove()) {
            qDebug() << "照片已删除:" << photoPath;
            photoList.removeAt(currentIndex);

            if (photoList.isEmpty()) {
                currentIndex = 0;
                updatePhotoDisplay();
            } else if (currentIndex >= photoList.size()) {
                currentIndex = photoList.size() - 1;
                updatePhotoDisplay();
            } else {
                updatePhotoDisplay();
            }
        } else {
            QMessageBox::warning(this, "错误", "删除失败！");
            qDebug() << "删除失败:" << photoPath;
        }
    }
}

void AlbumWidget::onReturnClicked()
{
    emit returnRequested();
}