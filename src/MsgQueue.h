#ifndef __MSG_QUEUE_H__
#define __MSG_QUEUE_H__

#include <stdio.h>
#include <pthread.h>
#include <queue>
#include "SocketCommon.h"
#include "XSPrint.h"

#define MAX_QUEUE_NUM 16

///��Ϣ���У��������ڴ�������ע��
using namespace std;

class CXSMsgQueue
{
public:
    CXSMsgQueue();
    ~CXSMsgQueue();

    /// @��ȡ���Ͷ���ָ��
    /// @para[in] newMsg ����Ϣ
    /// @para[out] 0 �ɹ� -1 ʧ��
    int pushback(const CSocketPacket *newMsg);

    /// @ȡ��������Ϣ
    /// @para[in] ��
    /// @para[out] ���ض���ָ��
    CSocketPacket *popFront(void);

    /// @ȡ������
    /// @para[in]��
    /// @para[out]��
    void cancel(void);

private:
    /// @�������
    /// @para[in] ��
    /// @para[out] 0 �ɹ� -1 ʧ��   
    int clean(void);

private:
    std::queue<CSocketPacket*> m_msgQueue;
    pthread_mutex_t m_queueMutex;
    pthread_cond_t m_queueCond;
    bool m_isCancel;
    const int m_MaxQueueNum;
};

#endif

