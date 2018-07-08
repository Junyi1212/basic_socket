#ifndef __X_SOCKET_SERVER_H__
#define __X_SOCKET_SERVER_H__

#include "XSocketManage.h"

struct ClientInfo
{
   int   fd;
   struct sockaddr_in addr; /* client's address information */
};  

class CXSocketServer: public CXSocketManage
{   
public:
    CXSocketServer();
    ~CXSocketServer();  
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
    int XSocketClose(int fd);
private:
    //static CXSocketServer* instance();
    static void *ThreadListen(void *arg);
    static void *ThreadDeal(void *arg); 
private:
    int m_socketFd;
    int m_curClientFd;
    struct sockaddr_in m_servAddr;
    
    pthread_t m_threadListen;
    pthread_t m_threadDeal;
    bool m_threadListenLoop;
    bool m_threadDealLoop;


    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
    
    ClientInfo m_client[FD_SETSIZE];

    uchar* m_sendTmpBuf;
    uchar* m_recvTmpBuf;

    CSocketPacketSend * hSend;
    SocketManageCbFunc m_cbServerRecvFunc;

    CXSocketProtocol hRecvProtocol;     
};

#endif
