#include "XSocketClient.h"
/*
CXSocketClient* CXSocketClient::instance()
{
    static CXSocketClient * _clientInstance = NULL;
    static pthread_mutex_t s_mutex_client;
    
    if(NULL == _clientInstance)
    {
        pthread_mutex_lock(&s_mutex_client);
        if(NULL == _clientInstance)
        {
            _clientInstance = new CXSocketClient();
        }
    }
    pthread_mutex_unlock(&s_mutex_client);
    return _clientInstance;
}
*/
CXSocketClient::CXSocketClient()
{
    m_socketFd = -1;
    //m_sinSize = sizeof(struct sockaddr_in); 
    
    m_threadListen = 0;
    m_threadDeal = 0;

    //m_mutex = PTHREAD_MUTEX_INITIALIZER;/*��ʼ��������*/ 
    //m_cond = PTHREAD_COND_INITIALIZER;/*��ʼ����������*/ 
    pthread_mutex_init(&m_mutex, NULL);
    pthread_cond_init(&m_cond, NULL);  

    hSend = NULL;
    m_cbClientRecvFunc = NULL;
    
    memset(&m_servAddr, 0, sizeof(m_servAddr)); 
    m_servAddr.sin_family = AF_INET; 
    m_servAddr.sin_port = htons(DEFAULT_PORT);//���õĶ˿�ΪDEFAULT_PORT  
    if( inet_pton(AF_INET, "127.0.0.1", &m_servAddr.sin_addr) <= 0)
    {  
        XSWarn("Client inet_pton error\n");   
    } 

    m_recvTmpBuf = new uchar[SOCKET_MAX_SIZE]();
    m_sendTmpBuf = new uchar[SOCKET_MAX_SIZE]();
}

CXSocketClient::~CXSocketClient()
{
    pthread_mutex_destroy(&m_mutex);
    pthread_cond_destroy(&m_cond);

    delete [] m_recvTmpBuf;
    delete [] m_sendTmpBuf;
    m_recvTmpBuf = NULL;
    m_sendTmpBuf = NULL;
}

int CXSocketClient::XSocketSend(int fd, uchar* buffer, int len, uint timeout)
{
    int ret = -1;  // ����ֵ
    int iSended = 0;           // �ѷ�������
    int totalLen = len;        // �����ܳ���

    uint timeleft = 0;      // ʣ��ʱ��
    struct timeval tmptv;       // ��ʼ����ʱ��
    struct timeval current;     // ��ǰʱ��

    // ��¼��ǰʱ��
    if (0 != timeout)
    {
        gettimeofday(&tmptv, NULL);
    }

    // �ж��Ƿ�ʱ
    do
    {
        ret = send(fd, (char *) buffer + iSended, len - iSended, 0);

        // �жϷ����Ƿ����
        if (-1 == ret)
        {
            int err = errno;

            // �ж��Ƿ�Ϊ�ɺ��Դ���
            if (err != 0 && !IS_SEND_IGNORABLE(err))
            {
                iSended = ret;
                break;
            }
            else
            {
                ret = 0;
            }
        }
        else
        {
            iSended += ret;

            // �ж��Ƿ��ѷ������
            if (iSended == totalLen)
            {
                break;
            }
        }

        if (0 != timeout)
        {
            // ÿ�η��ͽ���������1us
            usleep(1);
            gettimeofday(&current, NULL);
            timeleft = (current.tv_sec - tmptv.tv_sec) * 1000 + (current.tv_usec - tmptv.tv_usec) / 1000;
        }
    } while (0 != timeout && timeleft <= timeout);

    return iSended;
}

int CXSocketClient::XSocketRecv(int fd, uchar* buffer, int len)
{
    if((fd <= 0) || (NULL == buffer))
    {
        XSWarn("Client fd or buffer error\n");
        return -1;
    }

    int ret = -1;
    ret = recv(fd, buffer, len, 0);

    return ret;
}

int CXSocketClient::XSocketSelect(int fd, bool bRead, bool bWrite, struct timeval *tv)
{
    int iRet = -1;
    int err;
    fd_set wds;
    fd_set rds;
    fd_set *prds = bRead ? &rds : NULL;
    fd_set *pwds = bWrite ? &wds : NULL;
    socklen_t len;

    // ����select�Ķ�д��¼��
    FD_ZERO(&wds);
    FD_SET(fd, &wds);
    rds = wds;

    iRet = select(fd + 1, prds, pwds, NULL, tv);
    if(iRet <= 0)
    {
        XSWarn("Client select fail:%d/n", iRet);
        close(fd);
        return iRet;        
    }
    else
    {
        len=sizeof(err);
        getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len);
        XSInfo("Client connection %d - %s\n", err, strerror(err));
        if(err)//fail
        {       
            return -1;
        }
        else//success
        {
            return 1;
        }
    }
}


int CXSocketClient::XSocketClose(int fd)
{
    close(fd);
    XSWarn("Server closed connection %d.\n", fd);
    ///ͨ���ص�����֪ͨ�ϲ�
    if(NULL != m_cbClientRecvFunc)
    {
        m_cbClientRecvFunc(NULL, 0, ERR_CODE_CLOSED);
    }
    setlistenLooping(false);
    return 0;
}

void *CXSocketClient::ThreadListen(void *arg)
{
    CXSocketClient* This = static_cast<CXSocketClient*>(arg);

    int maxfd;
    fd_set rset;
    int realReadLen;
    int selectRet;

    maxfd = This->m_socketFd;           

    XSInfo("Client Enter ThreadListen\n"); 
    This->setlistenLooping(true);

    while(This->listenLooping())
    {
        FD_ZERO(&rset);
        FD_SET(This->m_socketFd, &rset);    

        //struct timeval tm = {0, 1};
        selectRet = select(maxfd+1, &rset, NULL, NULL, NULL);

        if(selectRet == 0)
        {
            //��ʱ
            continue;
        }
        else if(selectRet < 0)
        {
            if (!IS_IGNORABLE_ERROR(errno))
            {
                //���ɺ��ԵĴ����룬�ϱ�
                XSWarn("Client select error: %s(errno: %d)\n",strerror(errno),errno);  
                This->XSocketClose(This->m_socketFd);   
                continue;
            }
        }
        
        if(FD_ISSET(This->m_socketFd, &rset)) 
        {           
            realReadLen = recv(This->m_socketFd, This->m_recvTmpBuf, SOCKET_MAX_SIZE,0);
            if(realReadLen == 0) 
            {
                //�Զ˶Ͽ�
                XSWarn("Client recv error: %s(errno: %d)\n",strerror(errno),errno);
                This->XSocketClose(This->m_socketFd);   
            } 
            else if(realReadLen < 0)
            {
                if((0 != errno) && (!IS_RECV_IGNORABLE(errno)))
                {
                    //���ɺ��ԵĴ����룬�ϱ�
                    XSWarn("Client recv error: %s(errno: %d)\n",strerror(errno),errno);
                    This->XSocketClose(This->m_socketFd);   
                }           
            }
            else
            {
                This->hRecvProtocol.OnRecvMsg(This->m_socketFd, This->m_recvTmpBuf, realReadLen);
            }   
        }
    }
    XSInfo("Client Exit ThreadListen\n"); 
    return NULL;
}

void *CXSocketClient::ThreadDeal(void *arg)
{
    //����Ϣ������ȡ����Ϣ������
    CXSocketClient* This = static_cast<CXSocketClient*>(arg);
    
    int ackLen;
    int preSeq = 0;
    int curSeq = 0; 
    XSInfo("Client Enter ThreadDeal\n"); 
    This->setdealLooping(true);
    
    while(This->dealLooping())
    {
        CSocketPacket *hRecv = This->hRecvProtocol.PopFront();
        if(NULL == hRecv)
        {
            XSWarn("Client popFront failed\n");
            continue;
        }

        ///��������������ǻظ���
        if(hRecv->IsRequest())
        {
            curSeq = hRecv->getSeq();
            if(curSeq != preSeq)//ͬһ�����ݰ����ٶ�λص�
            {       
                preSeq = curSeq;
                ///��������ظ�ACK
                pthread_mutex_lock(&(This->m_mutex));           
                memset(This->m_sendTmpBuf, 0, SOCKET_MAX_SIZE);
                ackLen = hRecv->PackResponse(This->m_sendTmpBuf);
                This->XSocketSend(This->m_socketFd, This->m_sendTmpBuf, ackLen, 1000);
                pthread_mutex_unlock(&(This->m_mutex));
                XSDbug("Client send ack : %d\n", hRecv->getSeq());
    
                ///ͨ���ص�������������
                if(NULL != This->m_cbClientRecvFunc)
                {
                    This->m_cbClientRecvFunc(hRecv->getValidData(), hRecv->getValidLen(), ERR_CODE_SUCCESS);
                }   
            }
        }
        else
        {       
            if(NULL != This->hSend)
            {           
                if(hRecv->getSeq() == This->hSend->getSeq())///���к����
                {
                    ///Ӧ���������send����
                    XSDbug("Client recv ack : %d\n", hRecv->getSeq());
                    pthread_mutex_lock(&(This->m_mutex));
                    pthread_cond_signal(&(This->m_cond));
                    pthread_mutex_unlock(&(This->m_mutex));
                }
                else
                {
                    XSWarn("Client not my ack(%d-%d)\n", hRecv->getSeq(), This->hSend->getSeq());
                }
            }
        }
        
        delete hRecv;
        hRecv = NULL;
    }
    XSInfo("Client Exit ThreadDeal\n"); 
    return NULL;
}

int CXSocketClient::XStart(uint timeout)
{
    int ret = -1;
    //��ʼ��Socket
    if( (m_socketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
    {  
        XSWarn("Client create socket error: %s(errno: %d)\n",strerror(errno),errno);  
        return ret;  
    } 

    ///�����׽���Ϊ������
    int flag;
    flag = fcntl(m_socketFd, F_GETFL, 0);
    flag |= O_NONBLOCK;
    fcntl(m_socketFd, F_SETFL, flag);

    
    ///��ʼ���ӷ����
    ret = connect(m_socketFd, (struct sockaddr*)&m_servAddr, sizeof(m_servAddr));

    //XSInfo("connect ret:%d, error: %s(errno: %d)\n",ret, strerror(errno),errno);  
    /// �������������ӵĴ���, ��ֱ�ӷ��ش���
    if((0 != ret) && (!IS_CONNECT_INPROGRESS(errno)))
    {  
        XSWarn("Client connect error: %s(errno: %d)\n",strerror(errno),errno);  
        close(m_socketFd);
        return ret;  
    }  

    // �ж��Ƿ���Ҫ�����ȴ�
    if (timeout > 0)
    {
        // �ж���timeoutʱ�����Ƿ��д
        struct timeval tv = { 0 };
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;

        ret = XSocketSelect(m_socketFd, false, true, &tv);

        if (ret > 0)
        {
            ret = 0;
        }
        else if (0 == ret) 
        {
            XSWarn("Client select write timeout\n");
            ret = -1;
        }
        else
        {
            XSWarn("Client select write fail\n");
            ret = -1;
        }
    }
    else
    {
        ret = 0;
    }

    if(0 == ret)
    {
        ///�����̼߳����ͻ���
        pthread_create(&m_threadListen, NULL, &CXSocketClient::ThreadListen, this);

        ///�����̴߳������ݶ���
        pthread_create(&m_threadDeal, NULL, &CXSocketClient::ThreadDeal, this);
    }
    else
    {
        close(m_socketFd);  
    }

    return ret;
}

int CXSocketClient::XSend(int seq, const void *data, uint len, uint timeout)
{
    struct timespec ts;
    int iPacketLen;

    XSInfo("Client Send:\n");
    pthread_mutex_lock(&m_mutex);
    
    hSend = new CSocketPacketSend();
    memset(m_sendTmpBuf, 0, SOCKET_MAX_SIZE);
    hSend->Pack(seq, (uchar *)data, len);
    iPacketLen = hSend->PackRequest(m_sendTmpBuf);

    //send(m_socketFd, m_sendTmpBuf, iPacketLen, 0); 
    if(XSocketSend(m_socketFd, m_sendTmpBuf, iPacketLen, timeout) != iPacketLen)
    {
        XSWarn("Client send loss\n");
        delete hSend;
        hSend = NULL;       
        pthread_mutex_unlock(&m_mutex);
        return -1;  
    }

    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeout / 1000;//sec
    ts.tv_nsec += ( timeout % 1000) * 1000000;//nsec
    if(ts.tv_nsec > NSECS_PER_SEC)
    {//adjust the nsec
        ts.tv_sec += 1;
        ts.tv_nsec %= NSECS_PER_SEC;
    }
    
    if(pthread_cond_timedwait(&m_cond, &m_mutex, &ts))
    {
        /* ��ʱ */
        delete hSend;
        hSend = NULL;       
        pthread_mutex_unlock(&m_mutex);
        return -1;
    }
    
    delete hSend;
    hSend = NULL;

    pthread_mutex_unlock(&m_mutex);
    return 0;
}

int CXSocketClient::XAttachRecvCBFunc(SocketManageCbFunc cb)
{
    m_cbClientRecvFunc = cb;
    return 0;
}

int CXSocketClient::XStop()
{
    close(m_socketFd);
    setlistenLooping(false);
    setdealLooping(false);
    hRecvProtocol.cancel();
    pthread_join(m_threadListen, NULL);
    pthread_join(m_threadDeal, NULL);
    XSDbug("Client stop done\n");
    return 0;
}

