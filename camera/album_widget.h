#ifndef ALBUM_WIDGET_H
#define ALBUM_WIDGET_H

#include <QWidget>
#include <QDir>
#include <QFileInfoList>

QT_BEGIN_NAMESPACE
namespace Ui { class AlbumWidget; }
QT_END_NAMESPACE

class AlbumWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AlbumWidget(QWidget *parent = nullptr);
    ~AlbumWidget();

    void loadPhotos();

private slots:
    void onReturnClicked();
    void onPrevClicked();
    void onNextClicked();
    void onDeleteClicked();

signals:
    void returnRequested();

private:
    Ui::AlbumWidget *ui;
    QStringList photoList;
    int currentIndex;

    void updatePhotoDisplay();
};

#endif // ALBUM_WIDGET_H