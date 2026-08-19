#include "book_widget.h"
#include "ui_book_widget.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QScrollBar>
#include <QApplication>

BookWidget::BookWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BookWidget)
    , currentPage(0)
{
    ui->setupUi(this);

    ui->stackedWidget->setCurrentIndex(0);

    ui->prevButton->setEnabled(false);
    ui->nextButton->setEnabled(false);
    ui->prevBookButton->setEnabled(false);
    ui->nextBookButton->setEnabled(false);
    ui->startReadButton->setEnabled(false);

    applyDisplayMode(NormalMode);

    loadBookList();
}

BookWidget::~BookWidget()
{
    delete ui;
}

void BookWidget::loadBookList()
{
    bookList.clear();
    ui->listWidget->clear();

    QString bookDir = "/home/root/mybooks";
    QDir dir(bookDir);

    if (!dir.exists()) {
        ui->listWidget->addItem("书籍目录不存在");
        ui->prevBookButton->setEnabled(false);
        ui->nextBookButton->setEnabled(false);
        ui->startReadButton->setEnabled(false);
        ui->label_book_count->setText("0 / 0");
        return;
    }

    QStringList filters;
    filters << "*.txt" << "*.text";
    QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files | QDir::Readable, QDir::Name);

    for (const QFileInfo &fileInfo : fileList) {
        bookList.append(fileInfo.absoluteFilePath());
        ui->listWidget->addItem(fileInfo.fileName());
    }

    if (bookList.isEmpty()) {
        ui->listWidget->addItem("暂无电子书");
        ui->prevBookButton->setEnabled(false);
        ui->nextBookButton->setEnabled(false);
        ui->startReadButton->setEnabled(false);
        ui->label_book_count->setText("0 / 0");
    } else {
        ui->listWidget->setCurrentRow(0);
        updateBookSelection();
    }
}

void BookWidget::onBookSelected(int row)
{
    if (row < 0 || row >= bookList.size()) {
        return;
    }

    updateBookSelection();

    currentBookPath = bookList[row];

    ui->textBrowser->setText("正在加载书籍，请稍候...");
    ui->stackedWidget->setCurrentIndex(1);
    QApplication::processEvents();

    loadBook(currentBookPath);
}

void BookWidget::loadBook(const QString &filePath)
{
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "错误", "无法打开文件！");
        ui->stackedWidget->setCurrentIndex(0);
        return;
    }

    qint64 fileSize = file.size();

    if (fileSize > 50 * 1024 * 1024) {
        QMessageBox::warning(this, "错误", "文件过大（超过50MB），无法打开！");
        file.close();
        ui->stackedWidget->setCurrentIndex(0);
        return;
    }

    QByteArray rawData = file.readAll();
    file.close();

    qDebug() << "开始加载文件:" << filePath << " 大小:" << fileSize / 1024 << "KB";

    QString encoding = detectFileEncoding(rawData);
    qDebug() << "检测到文件编码:" << encoding;

    QTextStream in(rawData);
    in.setCodec(encoding.toUtf8().constData());

    QString content = in.readAll();

    qDebug() << "文件读取完成，内容长度:" << content.length() << "字符";

    pages.clear();
    currentPage = 0;

    splitIntoPages(content);

    qDebug() << "分页完成，共:" << pages.size() << "页";

    if (pages.isEmpty()) {
        ui->textBrowser->setText("文件内容为空");
        ui->prevButton->setEnabled(false);
        ui->nextButton->setEnabled(false);
    } else {
        updateDisplay();
    }
}

QString BookWidget::detectFileEncoding(const QByteArray &rawData)
{
    if (rawData.isEmpty()) {
        return "UTF-8";
    }

    if (rawData.startsWith("\xEF\xBB\xBF")) {
        return "UTF-8";
    }

    if (rawData.startsWith("\xFF\xFE")) {
        return "UTF-16LE";
    }

    if (rawData.startsWith("\xFE\xFF")) {
        return "UTF-16BE";
    }

    bool isAscii = true;
    bool hasHighBit = false;
    int utf8ContinuationBytes = 0;
    int gbkValidSequences = 0;
    int totalBytes = qMin(rawData.size(), 8192);

    for (int i = 0; i < totalBytes; ++i) {
        unsigned char byte = rawData[i];

        if (byte >= 0x80) {
            isAscii = false;
            hasHighBit = true;

            if ((byte & 0xC0) == 0x80) {
                utf8ContinuationBytes++;
            }
        }
    }

    if (isAscii) {
        return "UTF-8";
    }

    if (!hasHighBit) {
        return "UTF-8";
    }

    bool likelyUtf8 = true;
    int i = 0;
    while (i < totalBytes) {
        unsigned char byte = rawData[i];

        if (byte < 0x80) {
            i++;
            continue;
        } else if ((byte & 0xE0) == 0xC0) {
            if (i + 1 >= totalBytes || (rawData[i + 1] & 0xC0) != 0x80) {
                likelyUtf8 = false;
                break;
            }
            i += 2;
        } else if ((byte & 0xF0) == 0xE0) {
            if (i + 2 >= totalBytes ||
                (rawData[i + 1] & 0xC0) != 0x80 ||
                (rawData[i + 2] & 0xC0) != 0x80) {
                likelyUtf8 = false;
                break;
            }
            i += 3;
        } else if ((byte & 0xF8) == 0xF0) {
            if (i + 3 >= totalBytes ||
                (rawData[i + 1] & 0xC0) != 0x80 ||
                (rawData[i + 2] & 0xC0) != 0x80 ||
                (rawData[i + 3] & 0xC0) != 0x80) {
                likelyUtf8 = false;
                break;
            }
            i += 4;
        } else {
            likelyUtf8 = false;
            break;
        }
    }

    if (likelyUtf8 && utf8ContinuationBytes > 0) {
        return "UTF-8";
    }

    for (i = 0; i < totalBytes - 1; ++i) {
        unsigned char byte1 = rawData[i];
        unsigned char byte2 = rawData[i + 1];

        if (byte1 >= 0x81 && byte1 <= 0xFE &&
            ((byte2 >= 0x40 && byte2 <= 0x7E) || (byte2 >= 0x80 && byte2 <= 0xFE))) {
            gbkValidSequences++;
            i++;
        }
    }

    if (gbkValidSequences > totalBytes / 10) {
        return "GBK";
    }

    return "GB18030";
}

void BookWidget::splitIntoPages(const QString &content)
{
    int maxCharsPerPage = 800;

    QString remainingContent = content;
    int totalLength = content.length();
    int processedLength = 0;

    while (remainingContent.length() > 0) {
        if (remainingContent.length() <= maxCharsPerPage) {
            pages.append(remainingContent);
            processedLength += remainingContent.length();
            break;
        } else {
            int splitPos = remainingContent.lastIndexOf('\n', maxCharsPerPage);

            if (splitPos <= 0) {
                splitPos = maxCharsPerPage;
            }

            pages.append(remainingContent.left(splitPos));
            processedLength += splitPos;
            remainingContent = remainingContent.mid(splitPos).trimmed();

            if (pages.size() % 10 == 0) {
                QApplication::processEvents();
            }
        }
    }
}

void BookWidget::updateDisplay()
{
    if (pages.isEmpty() || currentPage >= pages.size()) {
        ui->textBrowser->setText("");
        return;
    }

    ui->textBrowser->setText(pages[currentPage]);

    ui->label_page->setText(QString("%1 / %2").arg(currentPage + 1).arg(pages.size()));

    ui->prevButton->setEnabled(currentPage > 0);
    ui->nextButton->setEnabled(currentPage < pages.size() - 1);

    ui->textBrowser->verticalScrollBar()->setValue(0);
}

void BookWidget::onPrevPageClicked()
{
    if (currentPage > 0) {
        currentPage--;
        updateDisplay();
    }
}

void BookWidget::onNextPageClicked()
{
    if (currentPage < pages.size() - 1) {
        currentPage++;
        updateDisplay();
    }
}

void BookWidget::applyDisplayMode(DisplayMode mode)
{
    QString styleSheet;

    switch (mode) {
    case NormalMode:
        styleSheet = "background-color: #ffffff; color: #000000; font-size: 16px;";
        break;
    case NightMode:
        styleSheet = "background-color: #1a1a1a; color: #ffffff; font-size: 16px;";
        break;
    case EyeCareMode:
        styleSheet = "background-color: #c7edcc; color: #000000; font-size: 16px;";
        break;
    }

    ui->textBrowser->setStyleSheet(styleSheet);
}

void BookWidget::onNormalModeClicked()
{
    applyDisplayMode(NormalMode);
}

void BookWidget::onNightModeClicked()
{
    applyDisplayMode(NightMode);
}

void BookWidget::onEyeCareModeClicked()
{
    applyDisplayMode(EyeCareMode);
}

void BookWidget::onReturnClicked()
{
    ui->stackedWidget->setCurrentIndex(0);
    loadBookList();
    emit returnRequested();
}

void BookWidget::onReturnToListClicked()
{
    ui->stackedWidget->setCurrentIndex(0);
    loadBookList();
}

void BookWidget::onPrevBookClicked()
{
    int currentRow = ui->listWidget->currentRow();

    if (currentRow > 0) {
        ui->listWidget->setCurrentRow(currentRow - 1);
        updateBookSelection();
    }
}

void BookWidget::onNextBookClicked()
{
    int currentRow = ui->listWidget->currentRow();

    if (currentRow < bookList.size() - 1 && currentRow >= 0) {
        ui->listWidget->setCurrentRow(currentRow + 1);
        updateBookSelection();
    }
}

void BookWidget::onStartReadClicked()
{
    int currentRow = ui->listWidget->currentRow();

    if (currentRow >= 0 && currentRow < bookList.size()) {
        onBookSelected(currentRow);
    }
}

void BookWidget::updateBookSelection()
{
    int currentRow = ui->listWidget->currentRow();

    ui->label_book_count->setText(QString("%1 / %2").arg(currentRow + 1).arg(bookList.size()));

    ui->prevBookButton->setEnabled(currentRow > 0);
    ui->nextBookButton->setEnabled(currentRow < bookList.size() - 1);
    ui->startReadButton->setEnabled(currentRow >= 0 && currentRow < bookList.size());
}