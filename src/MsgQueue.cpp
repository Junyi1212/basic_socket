#include "MsgQueue.h"
#include <unistd.h>

CXSMsgQueue::CXSMsgQueue()
:m_queueMutex(PTHREAD_MUTEX_INITIALIZER),
    m_queueCond(PTHREAD_COND_INITIALIZER),
    m_isCancel(false),
    m_MaxQueueNum(MAX_QUEUE_NUM)
{

}

CXSMsgQueue::~CXSMsgQueue()
{
}

int CXSMsgQueue::pushback(const CSocketPacket *newMsg)
{
    int iRet = -1;

    pthread_mutex_lock(&m_queueMutex);
    if ( m_msgQueue.size() < MAX_QUEUE_NUM)
    {
        m_msgQueue.push((CSocketPacket *)newMsg);
        m_isCancel = false;
        pthread_mutex_unlock(&m_queueMutex);
        iRet = 0;
        
        /// 触发条件变量
        pthread_cond_signal(&m_queueCond);
    }
    else
    {
        pthread_mutex_unlock(&m_queueMutex);
        XSWarn("CHsMsgQue::pushback Queue Full\n");
    }
    
    return iRet;
}

CSocketPacket *CXSMsgQueue::popFront(void)
{
    CSocketPacket *pFront = NULL;
    pthread_mutex_lock(&m_queueMutex);

    do 
    {
        if (!m_msgQueue.empty()) 
        {
            pFront = m_msgQueue.front();
            m_msgQueue.pop();
            break;
        }
        else if (m_isCancel)
        {
            usleep(1);
            break;
        }
        
        pthread_cond_wait(&m_queueCond, &m_queueMutex);

    } while (!m_msgQueue.empty());

    pthread_mutex_unlock(&m_queueMutex);
    return pFront;
}

int CXSMsgQueue::clean(void)
{
    CSocketPacket *hNode;
    pthread_mutex_lock(&m_queueMutex);
    while ( !m_msgQueue.empty() )
    {
        hNode = m_msgQueue.front();
        if(NULL != hNode)
        {
            delete hNode;
            hNode = NULL;       
        }
        m_msgQueue.pop();
    }
    pthread_mutex_unlock(&m_queueMutex);
    return 0;
}

/// @取消队列
/// @para[in]无
/// @para[out]无
void CXSMsgQueue::cancel( void )
{
    /// 由于临界资源较小，且问题避免两次加锁的开销，所以复用锁
    pthread_mutex_lock(&m_queueMutex);
    m_isCancel = true;
    pthread_mutex_unlock(&m_queueMutex);

    pthread_cond_signal(&m_queueCond);
}


