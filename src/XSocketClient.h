#ifndef __X_SOCKET_CLIENT_H__
#define __X_SOCKET_CLIENT_H__

#include "XSocketManage.h"

class CXSocketClient: public CXSocketManage
{
public:
    CXSocketClient();
    ~CXSocketClient();  
    int XStart(uint timeout);
    int XSend(int seq, const void *data, uint len, uint timeout);
    int XAttachRecvCBFunc(SocketManageCbFunc cb);   
    int XStop();
private:
    bool listenLooping(){return m_threadListenLoop;}
    void setlistenLooping(bool bLoop){m_threadListenLoop = bLoop;}
    bool dealLooping(){return m_threadDealLoop;}
    void setdealLooping(bool bLoop){m_threadDealLoop = bLoop;}
    int XSocketSend(int fd, uchar* buffer, int len, uint timeout);
    int XSocketRecv(int fd, uchar* buffer, int len);    
    int XSocketSelect(int fd, bool bRead, bool bWrite, struct timeval *tv); 
    int XSocketClose(int fd);
private:
    //static CXSocketClient* instance();
    static void *ThreadListen(void *arg);
    static void *ThreadDeal(void *arg);     
private:
    int m_socketFd;
    struct sockaddr_in m_servAddr;
    
    pthread_t m_threadListen;
    pthread_t m_threadDeal;
    bool m_threadListenLoop;
    bool m_threadDealLoop;

    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
    
    uchar* m_sendTmpBuf;
    uchar* m_recvTmpBuf;

    CSocketPacketSend * hSend;
    SocketManageCbFunc m_cbClientRecvFunc;

    CXSocketProtocol hRecvProtocol;     
};

#endif

