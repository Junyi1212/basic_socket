#ifndef _SOCKET_COMMON_H
#define _SOCKET_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "Types.h"
#include "XSPrint.h"
#include <iostream>
using namespace std;

#define SOCKET_MAX_SIZE (10*1024)
#define PACKET_HEAD_LEN 6
#define PACKET_TAIL_LEN 6

#define DATA_MAX_SIZE (SOCKET_MAX_SIZE-PACKET_HEAD_LEN-PACKET_TAIL_LEN-4-4)

#define IS_CONNECT_INPROGRESS(code) (code==EINPROGRESS)
#define IS_ACCEPT_IGNORABLE(code) ((code==EAGAIN)||(code==ECONNABORTED)||(code==EINTR))
#define IS_DISCONECTED(code) ((code==ECONNREFUSED)||(code==ECONNRESET)||(code==EPIPE)||(code==ENOTCONN))
#define IS_IGNORABLE_ERROR(code) ((code == EAGAIN) || (code == EINTR))
#define IS_SEND_IGNORABLE(code) ((code==EAGAIN)||(code==EWOULDBLOCK)||(code == EINTR))
#define IS_RECV_IGNORABLE(code) ((code==EAGAIN)||(code==EWOULDBLOCK)||(code == EINTR))
#define IS_RECVFROM_IGNORABLE(code) ((code==EAGAIN)||(code==EINTR)||(code==ECONNREFUSED))
#define IS_SENDTO_IGNORABLE(code) ((code==EAGAIN)||(code==EWOULDBLOCK)||(code==EINTR)||(code==EPIPE)||(code==ECONNRESET))
#define IS_INVALID_SOCKET(code) (code==EBADF)
#define IS_INTERRUPTED(code) (code==EINTR)


/*
    内部数据格式
    --------------------------------------------------------------------
       Head  |  Seq   | Type |  Len  |     Data         | Tail
    --------------------------------------------------------------------
      0--5   |   6--9   |   10  | 11--14    |  15--Len+15  | LEN+15+1--LEN+15+1 + 5
    --------------------------------------------------------------------
 */ 

class CSocketPacket
{
public: 
    CSocketPacket();
    virtual ~CSocketPacket();
    
    virtual int Parse(uchar* packetData, int packetLen);
    virtual int Pack(int seq, uchar* data, uint len);   

    virtual int PackResponse(uchar* packetData);
    virtual int PackRequest(uchar* packetData);

    virtual bool IsRequest(void);
    virtual uchar* getValidData(void);
    virtual int getValidLen(void);
    virtual int getSeq(void);
    virtual int getFd(void);
};
    
class CSocketPacketRecv : public CSocketPacket
{
public:
    CSocketPacketRecv(int fd);
    ~CSocketPacketRecv();
    int Parse(uchar* packetData, int packetLen);
    int PackResponse(uchar* packetData);
    
    bool IsRequest(void);
    uchar* getValidData(void);
    int getValidLen(void);
    int getSeq(void);
    int getFd(void);
private:
    int m_fd;
    //SocketPacket m_recvPacket; 

    std::string sHead; 
    std::string sSeq;
    std::string sType;
    std::string sLen;
    std::string sData;
    std::string sTail;  
};


class CSocketPacketSend : public CSocketPacket
{
public:

    CSocketPacketSend();
    ~CSocketPacketSend();
    int Pack(int seq, uchar* data, uint len);
    int PackRequest(uchar* packetData);
    int getSeq(void);
    
private:
    //SocketPacket m_sendPacket;
    
    std::string sHead; 
    std::string sSeq;
    std::string sType;
    std::string sLen;
    std::string sData;
    std::string sTail;
};


#endif

