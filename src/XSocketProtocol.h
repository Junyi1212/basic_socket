#ifndef __X_SOCKET_PROTOCOL_H__
#define __X_SOCKET_PROTOCOL_H__

#include <string.h> 
#include "XSPrint.h"
#include "XBuffer.h"
#include "MsgQueue.h"
#include "SocketCommon.h"


enum XReadStatus {
    ALL_DATA_READ = 1,
    MORE_DATA_EXPECTED = 0,
    DATA_CORRUPTED = -1,
    REQUEST_CANCELED = -2,
    DATA_TOO_LONG = -3
};


class CXSocketProtocol
{
public:
    CXSocketProtocol();
    ~CXSocketProtocol();
    int GetProtocolType(const char * pProtocolData);
    int GetProtocolContentLen(const char * pProtocolData);
    XReadStatus HandleChunkedRead(const char *pProtocolData, int iBufferLen, int& iProtocolMsgLen);
    int OnRecvMsg(int fd, const void* buffer, int bufferLen);
    CSocketPacket *PopFront();  
    void cancel(void);
private:
    //定义消息队列
    CXSMsgQueue MsgQ;
    
    CHSBuffer m_xsProtocolBuffer;
};
#endif
