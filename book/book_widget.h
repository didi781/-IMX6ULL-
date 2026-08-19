#ifndef BOOK_WIDGET_H
#define BOOK_WIDGET_H

#include <QWidget>
#include <QFile>
#include <QStringList>
#include <QTextStream>

QT_BEGIN_NAMESPACE
namespace Ui { class BookWidget; }
QT_END_NAMESPACE

class BookWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BookWidget(QWidget *parent = nullptr);
    ~BookWidget();

    void loadBookList();
    enum DisplayMode {
        NormalMode,
        NightMode,
        EyeCareMode
    };

private slots:
    void onReturnClicked();
    void onReturnToListClicked();
    void onBookSelected(int row);
    void onPrevPageClicked();
    void onNextPageClicked();
    void onNormalModeClicked();
    void onNightModeClicked();
    void onEyeCareModeClicked();
    void onPrevBookClicked();
    void onNextBookClicked();
    void onStartReadClicked();

signals:
    void returnRequested();

private:
    Ui::BookWidget *ui;
    QStringList bookList;
    QString currentBookPath;
    QStringList pages;
    int currentPage;

    void loadBook(const QString &filePath);
    void splitIntoPages(const QString &content);
    void updateDisplay();
    void applyDisplayMode(DisplayMode mode);
    void updateBookSelection();
    QString detectFileEncoding(const QByteArray &rawData);
};

#endif // BOOK_WIDGET_H