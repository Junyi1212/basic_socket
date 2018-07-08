#ifndef __MSG_QUEUE_H__
#define __MSG_QUEUE_H__

#include <stdio.h>
#include <pthread.h>
#include <queue>
#include "SocketCommon.h"
#include "XSPrint.h"

#define MAX_QUEUE_NUM 16

///消息队列，不负责内存的申请和注销
using namespace std;

class CXSMsgQueue
{
public:
    CXSMsgQueue();
    ~CXSMsgQueue();

    /// @获取发送队列指针
    /// @para[in] newMsg 新消息
    /// @para[out] 0 成功 -1 失败
    int pushback(const CSocketPacket *newMsg);

    /// @取出队首消息
    /// @para[in] 无
    /// @para[out] 返回队首指针
    CSocketPacket *popFront(void);

    /// @取消队列
    /// @para[in]无
    /// @para[out]无
    void cancel(void);

private:
    /// @清除队列
    /// @para[in] 无
    /// @para[out] 0 成功 -1 失败   
    int clean(void);

private:
    std::queue<CSocketPacket*> m_msgQueue;
    pthread_mutex_t m_queueMutex;
    pthread_cond_t m_queueCond;
    bool m_isCancel;
    const int m_MaxQueueNum;
};

#endif

