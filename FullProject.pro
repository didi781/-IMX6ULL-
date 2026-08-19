QT       += core gui mqtt charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    mqtt_widget.cpp \
    widget.cpp \
    sensor/sensor_widget.cpp \
    sensor/ap3216c.cpp \
    sensor/icm20608.cpp \
    media/musicwidget.cpp \
    media/musicplay.cpp \
    media/videowidget.cpp \
    media/videoplay.cpp \
    media/videoplayaudio.cpp \
    media/videoplayvideo.cpp \
    media/videoqueue.cpp \
    camera/camera.cpp \
    camera/camera_widget.cpp \
    camera/album_widget.cpp \
    book/book_widget.cpp

HEADERS += \
    widget.h \
    mqtt_widget.h \
    sensor/sensor_widget.h \
    sensor/ap3216c.h \
    sensor/icm20608.h \
    media/musicwidget.h \
    media/musicplay.h \
    media/videowidget.h \
    media/videoplay.h \
    media/videoplayaudio.h \
    media/videoplayvideo.h \
    media/videoqueue.h \
    media/commonresource.h \
    camera/camera.h \
    camera/camera_widget.h \
    camera/album_widget.h \
    book/book_widget.h

FORMS += \
    game.ui \
    widget.ui \
    sensor/sensor_widget.ui \
    camera/camera_widget.ui \
    camera/album_widget.ui \
    book/book_widget.ui

INCLUDEPATH += $$PWD/sensor \
    $$PWD/media \
    $$PWD/camera \
    $$PWD/book

LIBS += -lavformat -lavcodec -lavutil -lswresample -lswscale -lavfilter -lavdevice -lasound \
    -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_imgcodecs -lopencv_videoio

RESOURCES += \
    resources.qrc

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target