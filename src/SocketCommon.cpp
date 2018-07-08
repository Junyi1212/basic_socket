#include "SocketCommon.h"


CSocketPacket::CSocketPacket()
{

}

CSocketPacket::~CSocketPacket()
{

}

int CSocketPacket::Parse(uchar * packetData, int packetLen)
{
    return 0;
}

int CSocketPacket::Pack(int seq,uchar * data,uint len)
{
    return 0;
}

int CSocketPacket::PackResponse(uchar * packetData)
{
    return 0;
}

int CSocketPacket::PackRequest(uchar * packetData)
{
    return 0;
}

bool CSocketPacket::IsRequest(void)
{
    return true;
}

uchar* CSocketPacket::getValidData(void)
{
    return NULL;
}

int CSocketPacket::getValidLen(void)
{
    return 0;
}

int CSocketPacket::getSeq(void)
{
    return 0;
}

int CSocketPacket::getFd(void)
{
    return 0;
}

//==================================================
CSocketPacketRecv::CSocketPacketRecv(int fd)
{
    m_fd = fd;
}

CSocketPacketRecv::~CSocketPacketRecv()
{
}

///解包
int CSocketPacketRecv::Parse(uchar* packetData, int packetLen)
{
    if(NULL == packetData)
    {
        XSWarn("packetData is null\n");
        return -1;
    }

    if(packetLen <= SOCKET_MAX_SIZE)
    {
        char* buffer = (char *)packetData;

        sHead.assign(buffer, PACKET_HEAD_LEN);
        buffer += PACKET_HEAD_LEN;
        sSeq.assign(buffer, 4);
        buffer += 4;
        sType.assign(buffer, 1);
        buffer += 1;
        sLen.assign(buffer, 4);
        buffer += 4;

        int iLen = strtol(sLen.c_str(), NULL, 10);
        sData.assign(buffer, iLen);
        buffer += iLen;

        sTail.assign(buffer, PACKET_TAIL_LEN);  
        return 0;
    }
    else
    {
        XSWarn("Packet Data length is too long:%d.\n", packetLen);
        return -1;
    }

    return 0;
}

///组成ACK 包，通过packetData 返回
///返回ACK 包的长度
int CSocketPacketRecv::PackResponse(uchar* packetData)
{
    if(NULL == packetData)
    {
        XSWarn("packetData is null\n");
        return -1;  
    }
    if(sType == "1")
    {
        XSWarn("need request， not response.\n");
        return -1;      
    }
    std::string sPacket;
    sPacket += sHead;
    sPacket += sSeq;
    sPacket += "1";//ACK;
    sPacket += sLen;
    sPacket += sData;
    sPacket += sTail;

    uchar *cPacket = (uchar *)sPacket.c_str();
    int dataLen = strtol(sLen.c_str(), NULL, 10);
    int packetLen = PACKET_HEAD_LEN+4+1+4+dataLen+PACKET_TAIL_LEN;//(int)strlen((const char*)cPacket);
    
    memcpy(packetData, cPacket, packetLen);
    XSDbug("send response(%d)\n",packetLen);
/*
    for(int i=0; i<packetLen; i++)
    {
        printf("%c", packetData[i]);
    }
    printf("\n");
*/
    return packetLen;   
}

bool CSocketPacketRecv::IsRequest(void)
{
    bool bRequest;
    bRequest = sType == "0"? true : false;
    return bRequest;
}

uchar* CSocketPacketRecv::getValidData(void)
{
    uchar* cValidData = (uchar*)(sData.c_str());
    return cValidData;
}

int CSocketPacketRecv::getValidLen(void)
{
    int iLen = strtol(sLen.c_str(), NULL, 10);
    return iLen;
}

int CSocketPacketRecv::getSeq(void)
{
    int iSeq = strtol(sSeq.c_str(), NULL, 10);
    return iSeq;
}

int CSocketPacketRecv::getFd(void)
{
    return m_fd;
}

//==================================================
CSocketPacketSend::CSocketPacketSend()
{
}

CSocketPacketSend::~CSocketPacketSend()
{
}

///打包
int CSocketPacketSend::Pack(int seq, uchar* data, uint len)
{
    if((NULL == data) || (0 == len))
    {
        XSWarn("data or len is null\n");
        return -1;
    }   
    if(len <= DATA_MAX_SIZE)
    {
        sHead = "XSONIA";
        
        char tmpSeq[16] = {0};
        sprintf(tmpSeq, "%04d", seq); //例如："0097"
        sSeq = tmpSeq;
        
        sType = "0";

        char tmpLen[16] = {0};
        sprintf(tmpLen, "%04d", len);
        sLen = tmpLen;
        
        sData.assign(data, data+len);
        sTail = "xsonia";

        return 0;
    }
    else
    {
        XSWarn("Data length is too long.\n");
        return -1;
    }

    return 0;
}

///组成请求 包，通过packetData 返回
///返回请求包的长度
int CSocketPacketSend::PackRequest(uchar* packetData)
{
    if(NULL == packetData)
    {
        XSWarn("packetData is null\n");
        return -1;  
    }
    if(sType == "1")
    {
        XSWarn("need request， not response.\n");
        return -1;      
    }
    std::string sPacket;
    sPacket += sHead;
    sPacket += sSeq;
    sPacket += sType;
    sPacket += sLen;
    sPacket += sData;
    sPacket += sTail;

    uchar *cPacket = (uchar *)sPacket.c_str();
    int dataLen = strtol(sLen.c_str(), NULL, 10);
    int packetLen = PACKET_HEAD_LEN+4+1+4+dataLen+PACKET_TAIL_LEN;//(int)strlen((const char*)cPacket);
    
    memcpy(packetData, cPacket, packetLen);
    XSDbug("send request(%d)\n",packetLen);
/*  
    for(int i=0; i<packetLen; i++)
    {
        printf("%c", packetData[i]);
    }
    printf("\n");
*/  
    return packetLen;
}

int CSocketPacketSend::getSeq(void)
{
    int iSeq = strtol(sSeq.c_str(), NULL, 10);
    return iSeq;
}


