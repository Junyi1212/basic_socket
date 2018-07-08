#include "XSocketServer.h"

/*
CXSocketServer* CXSocketServer::instance()
{
    static CXSocketServer * _serverInstance = NULL;
    static pthread_mutex_t s_mutex_server;
    
    if(NULL == _serverInstance)
    {
        pthread_mutex_lock(&s_mutex_server);
        if(NULL == _serverInstance)
        {
            _serverInstance = new CXSocketServer();
        }
    }
    pthread_mutex_unlock(&s_mutex_server);
    return _serverInstance;
}
*/
CXSocketServer::CXSocketServer()
{
    m_socketFd = -1;
    
    m_threadListen = 0;
    m_threadDeal = 0;

    //m_mutex = PTHREAD_MUTEX_INITIALIZER;/*��ʼ��������*/ 
    //m_cond = PTHREAD_COND_INITIALIZER;/*��ʼ����������*/  
    pthread_mutex_init(&m_mutex, NULL);
    pthread_cond_init(&m_cond, NULL); 

    hSend = NULL;
    m_cbServerRecvFunc = NULL;
    
    memset(&m_servAddr, 0, sizeof(m_servAddr));  
    m_servAddr.sin_family = AF_INET;  
    m_servAddr.sin_addr.s_addr = htonl(INADDR_ANY);//IP��ַ���ó�INADDR_ANY,��ϵͳ�Զ���ȡ������IP��ַ�� 
    m_servAddr.sin_port = htons(DEFAULT_PORT);//���õĶ˿�ΪDEFAULT_PORT  

    for (int i = 0; i < FD_SETSIZE; i++) 
    {
       m_client[i].fd = -1; 
    }

    m_recvTmpBuf = new uchar[SOCKET_MAX_SIZE]();
    m_sendTmpBuf = new uchar[SOCKET_MAX_SIZE]();
}

CXSocketServer::~CXSocketServer()
{
    pthread_mutex_destroy(&m_mutex);
    pthread_cond_destroy(&m_cond);

    delete [] m_recvTmpBuf;
    delete [] m_sendTmpBuf;
    m_recvTmpBuf = NULL;
    m_sendTmpBuf = NULL;
}

int CXSocketServer::XSocketSend(int fd, uchar* buffer, int len, uint timeout)
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
        //ret = send(fd, (char *) buffer + iSended, 10, 0);//���Է���ָ�����ȵ�����

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

int CXSocketServer::XSocketRecv(int fd, uchar* buffer, int len)
{
    if((fd <= 0) || (NULL == buffer))
    {
        XSWarn("Server fd or buffer error\n");
        return -1;
    }

    int ret = -1;
    ret = recv(fd, buffer, len, 0);

    return ret;
}

int CXSocketServer::XSocketClose(int fd)
{
    close(fd);
    XSWarn("Client closed connection %d.\n", fd);
    return 0;
}

void *CXSocketServer::ThreadListen(void *arg)
{
    CXSocketServer* This = static_cast<CXSocketServer*>(arg);
    
    int maxi,maxfd,sockfd;
    fd_set rset, allset;
    int realReadLen;
    int i;
    int selectRet;

    /*client socket descriptors */
    int connectClientFd;   
    struct sockaddr_in connectClientAddr;
    socklen_t sinSize= sizeof(struct sockaddr_in);
    
    /*initialize for select */
    maxfd = This->m_socketFd;   
    maxi = -1;          

    FD_ZERO(&allset);
    FD_SET(This->m_socketFd, &allset);

    XSInfo("Server Enter ThreadListen\n"); 
    This->setlistenLooping(true);

    while(This->listenLooping())
    {
        rset = allset;  

        //struct timeval tm = {0, 40000};
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
                XSWarn("Server select error: %s(errno: %d)\n",strerror(errno),errno);  
                This->XSocketClose(sockfd); 
                continue;
            }
        }

        //�����µĿͻ�������
        if (FD_ISSET(This->m_socketFd, &rset)) 
        {   /* new client connection */
           /* Accept connection */
           if ((connectClientFd = accept(This->m_socketFd,(struct sockaddr *)&connectClientAddr,&sinSize))==-1) 
           {
              perror("Server accept() error\n"); 
              continue; 
           }
           XSInfo("Server A new client request!\n");
           
           /* Put new fd to client */
           for (i = 0; i < FD_SETSIZE; i++)
           {
              if (This->m_client[i].fd < 0) 
              {
                 This->m_client[i].fd = connectClientFd;    /* save descriptor */
                 This->m_client[i].addr = connectClientAddr;
                 This->m_curClientFd = connectClientFd;
                 XSInfo("Server You got a connection from (%d)%s.\n",connectClientFd, inet_ntoa(This->m_client[i].addr.sin_addr)); 
                 break;
               }
            }
            if(i == FD_SETSIZE) 
            {
                XSWarn("Server too many clients\n");
            }
            
            FD_SET(connectClientFd, &allset);   /* add new descriptor to set */
        
            int flag;
            flag = fcntl(connectClientFd, F_GETFL, 0);
            flag |=O_NONBLOCK;
            fcntl(connectClientFd, F_SETFL, flag);
            if(NULL != This->m_cbServerRecvFunc)
            {
                This->m_cbServerRecvFunc(NULL, 0, ERR_CODE_NEW_CONNECTION);
            }           
        
            if(connectClientFd > maxfd)  
            {
                maxfd = connectClientFd;
            }
            if(i > maxi)    
            {
                maxi = i;   
            }
            if(--selectRet <= 0) 
            {
                continue;   /* no more readable descriptors */
            }
        }

        //���������ӿͻ�������
        for(i = 0; i <= maxi; i++) 
        {   /* check all clients for data */
            if( (sockfd = This->m_client[i].fd) < 0)    
            {
                continue;
            }
            if(FD_ISSET(sockfd, &rset)) 
            {   
                realReadLen = recv(sockfd, This->m_recvTmpBuf, SOCKET_MAX_SIZE,0);
                if(realReadLen == 0) 
                {
                    //�Զ˶Ͽ�
                    XSWarn("Server recv error: %s(errno: %d)\n",strerror(errno),errno);
                    This->XSocketClose(sockfd);
                    FD_CLR(sockfd, &allset);
                    This->m_client[i].fd = -1;                  
                }                   
                else if(realReadLen < 0) 
                {                     
                    if((0 != errno) && (!IS_RECV_IGNORABLE(errno)))
                    {
                        //���ɺ��ԵĴ����룬�ϱ�
                        XSWarn("Server recv error: %s(errno: %d)\n",strerror(errno),errno);
                        This->XSocketClose(sockfd);
                        FD_CLR(sockfd, &allset);
                        This->m_client[i].fd = -1;  
                    }
                } 
                else
                {
                    This->hRecvProtocol.OnRecvMsg(sockfd, This->m_recvTmpBuf, realReadLen);
                }

                if(--selectRet <= 0)    
                    break;  /* no more readable descriptors */
             }
       }
    }
    XSInfo("Server Exit ThreadListen\n"); 
    return NULL;
}

void *CXSocketServer::ThreadDeal(void *arg)
{
    //����Ϣ������ȡ����Ϣ������
    CXSocketServer* This = static_cast<CXSocketServer*>(arg);
    
    int ackLen;
    int preSeq = 0;
    int curSeq = 0;
    XSInfo("Server Enter ThreadDeal\n"); 
    This->setdealLooping(true);
    
    while(This->dealLooping())
    {
        CSocketPacket *hRecv = This->hRecvProtocol.PopFront();
        if(NULL == hRecv)
        {
            XSWarn("Server popFront failed\n");
            continue;
        }

        ///�����������������ǻظ���
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
                This->XSocketSend(hRecv->getFd(), This->m_sendTmpBuf, ackLen, 1000);
                pthread_mutex_unlock(&(This->m_mutex));
                XSDbug("Server send ack : %d\n", hRecv->getSeq());

                ///ͨ���ص�������������
                if(NULL != This->m_cbServerRecvFunc)
                {
                    This->m_cbServerRecvFunc(hRecv->getValidData(), hRecv->getValidLen(), ERR_CODE_SUCCESS);
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
                    XSDbug("Server recv ack : %d\n", hRecv->getSeq());
                    pthread_mutex_lock(&(This->m_mutex));
                    pthread_cond_signal(&(This->m_cond));
                    pthread_mutex_unlock(&(This->m_mutex));
                }
                else
                {
                    XSWarn("Server not my ack(%d-%d)\n", hRecv->getSeq(), This->hSend->getSeq());
                }   
            }
        }
        
        delete hRecv;
        hRecv = NULL;
    }
    XSInfo("Server Exit ThreadDeal\n"); 
    return NULL;
}

int CXSocketServer::XStart(uint timeout)
{
    //��ʼ��Socket
    if( (m_socketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
    {  
        XSWarn("Server create socket error: %s(errno: %d)\n",strerror(errno),errno);  
        return -1;  
    } 
    
    ///�����׽���Ϊ������
    int flag;
    flag = fcntl(m_socketFd, F_GETFL, 0);
    flag |= O_NONBLOCK;
    fcntl(m_socketFd, F_SETFL, flag);

    int opt = SO_REUSEADDR;
    setsockopt(m_socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    //�����ص�ַ�󶨵����������׽�����  
    if( bind(m_socketFd, (struct sockaddr*)&m_servAddr, sizeof(m_servAddr)) == -1)
    {  
        XSWarn("Server bind socket error: %s(errno: %d)\n",strerror(errno),errno);  
        return -1;  
    }

    //��ʼ�����Ƿ��пͻ�������  
    if( listen(m_socketFd, BACKLOG) == -1)
    {  
        XSWarn("Server listen socket error: %s(errno: %d)\n",strerror(errno),errno);  
        return -1;  
    }  
 
    ///�����̼߳����ͻ���
    pthread_create(&m_threadListen, NULL, &CXSocketServer::ThreadListen, this);

    ///�����̴߳������ݶ���
    pthread_create(&m_threadDeal, NULL, &CXSocketServer::ThreadDeal, this);

    return 0;
}

int CXSocketServer::XSend(int seq, const void *data, uint len, uint timeout)
{
    struct timespec ts;
    int iPacketLen;

    XSInfo("Server Send:\n");
    pthread_mutex_lock(&m_mutex);

    hSend = new CSocketPacketSend();
    memset(m_sendTmpBuf, 0, SOCKET_MAX_SIZE);
    hSend->Pack(seq, (uchar *)data, len);
    iPacketLen = hSend->PackRequest(m_sendTmpBuf);

    if(XSocketSend(m_curClientFd, m_sendTmpBuf, iPacketLen, timeout)!= iPacketLen)
    {
        XSWarn("Server send loss\n");

        delete hSend;
        hSend = NULL;       
        pthread_mutex_unlock(&m_mutex);
        return -1;  
    }
    
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += timeout / 1000;//sec
    ts.tv_nsec += (timeout % 1000) * 1000000;//nsec
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

int CXSocketServer::XAttachRecvCBFunc(SocketManageCbFunc cb)
{
    m_cbServerRecvFunc = cb;
    return 0;
}

int CXSocketServer::XStop()
{
    for(int i = 0; i < FD_SETSIZE; i++)
    {
        if (m_client[i].fd > 0) 
        {
            close(m_client[i].fd);
        }
    }

    close(m_socketFd);
    setlistenLooping(false);
    setdealLooping(false);
    hRecvProtocol.cancel();
    pthread_join(m_threadListen, NULL);
    pthread_join(m_threadDeal, NULL);
    XSInfo("Server stop done\n");   
    return 0;
}

