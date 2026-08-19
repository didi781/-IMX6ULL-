#ifndef VIDEOQUEUE_H
#define VIDEOQUEUE_H

#include <QMutex>
#include <QWaitCondition>

extern "C"
{
#include <libavutil/time.h>
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
}


class VideoQueue
{
public:
    VideoQueue();

    void queue_push(AVPacket *pkt);    // 入队
    int queue_pop(AVPacket *pkt);      // 出队，无数据时阻塞
    void queue_flush();                // 清空队列，释放内存
    void queue_abort();                // 中止队列，唤醒所有等待线程

private:
    AVPacketList *first_pkt = nullptr;  // 队首指针
    AVPacketList *last_pkt = nullptr;   // 队尾指针
    int nb_packets = 0;                 // 包数量
    int nb_bytes = 0;                   // 字节数（用于监控）
    bool aborted = false;               // 中止标志
    
    static const int MAX_QUEUE_SIZE = 30;   // 队列最大包数限制（降低以减少延迟）
    static const int MAX_QUEUE_BYTES = 5 * 1024 * 1024;  // 队列最大 5MB（减少内存占用）
    
    QMutex mutex;                       // 互斥锁
    QWaitCondition cond;                // 条件变量（用于阻塞唤醒）
};

#endif // VIDEOQUEUE_H