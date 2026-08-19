#include "videoqueue.h"
#include <QDebug>

VideoQueue::VideoQueue()
{

}

void VideoQueue::queue_push(AVPacket *pkt)
{
    AVPacketList *pktl = (AVPacketList *)av_malloc(sizeof(AVPacketList));
    pktl->pkt = *pkt;
    pktl->next = nullptr;

    QMutexLocker locker(&mutex);

    if (aborted) {
        av_free(pktl);
        return;
    }

    /* 检查队列是否已满（防止内存无限增长） */
    while ((nb_packets >= MAX_QUEUE_SIZE || nb_bytes >= MAX_QUEUE_BYTES) && !aborted) {
        /* 只在首次等待时输出日志，避免刷屏 */
        static int logCounter = 0;
        if (logCounter++ % 100 == 0) {  // 每100次才输出一次
            qDebug() << "Queue full, waiting... (packets:" << nb_packets << ", bytes:" << nb_bytes << ")";
        }
        cond.wait(&mutex);  // 等待队列有空间
    }

    if (aborted) {
        av_free(pktl);
        return;
    }

    if (!last_pkt) {        // 队列为空
        first_pkt = pktl;
    } else {
        last_pkt->next = pktl;    // 加入队尾
    }
    last_pkt = pktl;    // 更新队尾指针
    nb_packets++;
    nb_bytes += pkt->size;
    
    cond.wakeAll();     // 唤醒所有等待线程
}

int VideoQueue::queue_pop(AVPacket *pkt)
{
    AVPacketList *pktl = nullptr;
    int ret = 0;

    QMutexLocker locker(&mutex);

    while (1) {
        if (aborted) {
            return 0;
        }
        pktl = first_pkt;
        if (pktl) {
            first_pkt = pktl->next;     // 更新头指针
            if (!first_pkt) {           // 队列变为空
                last_pkt = nullptr;
            }
            nb_packets--;
            nb_bytes -= pktl->pkt.size;  // 更新字节数
            *pkt = pktl->pkt;           // 复制数据包到输出参数
            av_free(pktl);              // 释放资源
            ret = 1;
            
            cond.wakeAll();  // 唤醒可能等待的入队操作
            break;               // 退出
        } else {
            cond.wait(&mutex);   // 释放锁并等待条件变量
        }
    }

    return ret;
}

void VideoQueue::queue_flush()
{
    QMutexLocker locker(&mutex);

    AVPacketList *pktl = first_pkt;
    while (pktl != nullptr) {
        AVPacketList *next = pktl->next;  // 保存下一个节点指针
        av_packet_unref(&pktl->pkt);      // 释放AVPacket内部资源
        av_free(pktl);                    // 释放节点内存
        pktl = next;
    }
    // 重置队列状态
    first_pkt = nullptr;
    last_pkt = nullptr;
    nb_packets = 0;
    nb_bytes = 0;  // 重置字节数
    
    cond.wakeAll();  // 唤醒可能等待的线程
}

void VideoQueue::queue_abort()
{
    QMutexLocker locker(&mutex);
    aborted = true;
    cond.wakeAll();
}