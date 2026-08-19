#include "widget.h"

#include <QApplication>

extern "C"
{
#include <libavformat/avformat.h>
}

int main(int argc, char *argv[])
{
    av_register_all();

    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}