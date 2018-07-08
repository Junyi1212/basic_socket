#include "XSocketProtocol.h"

CXSocketProtocol::CXSocketProtocol()
{
    m_xsProtocolBuffer.Reset();
}

CXSocketProtocol::~CXSocketProtocol()
{
}

int CXSocketProtocol::GetProtocolType(const char * pProtocolData)
{
    int iHttpType = -1;
    if(NULL == pProtocolData)
    {
        return iHttpType;   
    }
    char typeTmp[16] = {0};
    strncpy(typeTmp, pProtocolData, 1);
    iHttpType = strtol(typeTmp, NULL, 10);
    
    return iHttpType;
}

int CXSocketProtocol::GetProtocolContentLen(const char * pProtocolData)
{
    int iProtocolContentLength = -1;
    if(NULL == pProtocolData)
    {
        return iProtocolContentLength;  
    }
    char lenTmp[16] = {0};
    strncpy(lenTmp, pProtocolData, 4);
    iProtocolContentLength = strtol(lenTmp, NULL, 10);
    
    return iProtocolContentLength;
}

XReadStatus CXSocketProtocol::HandleChunkedRead(const char *pProtocolData, int iBufferLen, int& iProtocolMsgLen)
{
    //const char correct_head[] = "XSONIA";
    const char correct_tail[] = "xsonia";

    if(NULL == pProtocolData || iBufferLen <= 0)
    {
        XSWarn("Handle Chunked Read Data Is Null !!!\n");
        return MORE_DATA_EXPECTED;
    }

    m_xsProtocolBuffer.Append((unsigned char*)pProtocolData, iBufferLen);
    int iProtocolBufferSize = m_xsProtocolBuffer.Size();

    //查找head
    char *pBufIndex = (char *)m_xsProtocolBuffer.Buf();
    char *pOriginData = (char *)m_xsProtocolBuffer.Buf();

    if(iProtocolBufferSize <= PACKET_HEAD_LEN)// 头最短为 5
    {
        XSWarn("Head need more data, current data len : %d!!\n", iProtocolBufferSize);
        return MORE_DATA_EXPECTED;
    }

    bool bFindHead = false;
    bool bFindX = false;
    char *pFindX = NULL;
    int iLastIndex = 0;
    while(!bFindHead)
    {       
        bFindX = false;
        pFindX = NULL;

        //pBufIndex 此次查找的起始位置
        //pFindX 出现X 的位置
        //iLastIndex上次找到X 的偏移量
        for(int i = 0; i < iProtocolBufferSize - iLastIndex; i++)
        {
            if('X' == pBufIndex[i])
            {
                iLastIndex = i;
                pFindX = pBufIndex + i;
                bFindX = true;
                break;
            }
        }

        if(!bFindX && iProtocolBufferSize > SOCKET_MAX_SIZE)
        {
            m_xsProtocolBuffer.Reset(true);
            XSWarn("buffer is too long and clear Buffer!!\n");
            return DATA_TOO_LONG;
        }
        else if(!bFindX && iProtocolBufferSize < SOCKET_MAX_SIZE)
        {
            XSWarn("Parse head failed, need more data!!!\n");
            return MORE_DATA_EXPECTED;
        }

        std::string strLine;
        strLine.assign(pFindX, pFindX + PACKET_HEAD_LEN + 1);
    
        if(strLine.find("XSONIA") != std::string::npos)
        {
            //XSDbug("XSONIA Head!!!\n");
            break;
        }
        else
        {
            XSWarn("Parse head failed, need more data!!!\n");
            // 不是 包头
            pBufIndex = pFindX + 1;
            continue;
        }
    }


    //将head 之前的数据剔除
    int iPopdataLen = pFindX - pOriginData;
    //XSDbug("Pop Data Len : %d\n", iPopdataLen);
    m_xsProtocolBuffer.Pop(NULL, iPopdataLen);

    //重新定位size 和index
    iProtocolBufferSize = m_xsProtocolBuffer.Size();
    if(iProtocolBufferSize <= (PACKET_HEAD_LEN + 4 + 1 + 4))//SEQ = 4, TYPE =1, LEN = 4
    {
        XSWarn("Head ,Seq, Type, Len, need more data, current data len : %d!!\n", iProtocolBufferSize);
        return MORE_DATA_EXPECTED;  
    }
    //XSDbug("Len after pop: %d\n", iProtocolBufferSize);
    pBufIndex = (char *)m_xsProtocolBuffer.Buf();

    // 检查type
    pBufIndex += PACKET_HEAD_LEN + 4;
    int iType = GetProtocolType((const char *)pBufIndex);
    //XSDbug("Content Type : %d\n", iType);
    if(iType < 0)
    {
        ///将head 到type 之间的数据剔除
        iPopdataLen = PACKET_HEAD_LEN + 4 ;
        XSWarn("type is error!!\n");
        XSWarn("Pop Data Len : %d\n", iPopdataLen);
        m_xsProtocolBuffer.Pop(NULL, iPopdataLen);
        return DATA_CORRUPTED;  
    }
    
    // 确定Len
    pBufIndex += 1;
    int iProtocolContentLen = GetProtocolContentLen((const char *)pBufIndex);
    //XSDbug("Content Length : %d\n", iHttpContentLen);
    if( iProtocolContentLen >= DATA_MAX_SIZE)
    {
        ///将head 到len 之间的数据剔除
        iPopdataLen = PACKET_HEAD_LEN + 4 + 1;
        XSWarn("Data is too long!!\n");
        XSWarn("Pop Data Len : %d\n", iPopdataLen);
        m_xsProtocolBuffer.Pop(NULL, iPopdataLen);
        return DATA_TOO_LONG;
    }
    else if(PACKET_HEAD_LEN + 4 + 1 + 4 + iProtocolContentLen + PACKET_TAIL_LEN > iProtocolBufferSize)
    {
        XSWarn("need more data, current data len : %d!!\n", iProtocolBufferSize);
        return MORE_DATA_EXPECTED;
    }
    
    // 校验tail
    pBufIndex += 4 + iProtocolContentLen;
    if(strncmp(correct_tail, pBufIndex, PACKET_TAIL_LEN))
    {
        ///将head 到len 之间的数据剔除
        iPopdataLen = PACKET_HEAD_LEN + 4 + 1;
        XSWarn("Data is corrupted!!\n");
        XSWarn("Pop Data Len : %d\n", iPopdataLen);
        m_xsProtocolBuffer.Pop(NULL, iPopdataLen);
        return DATA_CORRUPTED;
    }
    else
    {
        iProtocolMsgLen = PACKET_HEAD_LEN + 4 + 1 + 4 + iProtocolContentLen + PACKET_TAIL_LEN;
        return ALL_DATA_READ;
    }
    
}

int CXSocketProtocol::OnRecvMsg(int fd, const void* buffer, int bufferLen)
{
    if ((NULL == buffer) || (bufferLen <= 0))
    {
        XSWarn("buffer[%p], bufferLen[%d]\n", buffer, bufferLen);
        return -1;
    }

    int iProtocolMsgLen = 0;

    char *pParseBuffer = (char *)buffer;
    int iParseBufferLen = bufferLen;
    bool bShouldParse = true;
    while(bShouldParse)
    {
        switch(HandleChunkedRead((const char*)pParseBuffer, iParseBufferLen, iProtocolMsgLen))
        {
            // 接收到一个完整的 HTTP 消息
            case ALL_DATA_READ:
            {           
                CSocketPacket * hRecv = new CSocketPacketRecv(fd);
                if(hRecv->Parse((uchar *)m_xsProtocolBuffer.Buf(), iProtocolMsgLen) < 0)
                {
                    delete hRecv;
                    hRecv = NULL;   
                }
                
                if(MsgQ.pushback(hRecv))
                {
                    XSWarn("pushback failed.\n");
                    delete hRecv;
                    hRecv = NULL;
                }

                m_xsProtocolBuffer.Pop(NULL, iProtocolMsgLen);
                
                if(iParseBufferLen > iProtocolMsgLen)
                {
                    XSWarn("more than a protocal.\n");
                    bShouldParse = true;
                    pParseBuffer += iProtocolMsgLen;
                    iParseBufferLen -= iProtocolMsgLen;
                    //如果有多余一条 MSG，那么要对接下来的数据进行解析
                    //m_xsProtocolBuffer.Reset();
                }
                else
                {
                    bShouldParse = false;
                }
                break;
            }
            // 接收到的数据有异常
            case DATA_CORRUPTED:
            case DATA_TOO_LONG:
            {
                XSWarn("Data Is Error!\n");
                bShouldParse = false;
                // 需要着重考虑下，是否需要重置接收缓冲、发送缓冲，关闭套接字
                return -1;
            }
            // 需要更多的数据
            case MORE_DATA_EXPECTED:
                XSWarn("Msg Need More Data!!!\n");
                bShouldParse = false;
                break;
            default :
                XSWarn("Unknown Msg Type!!!\n");
                bShouldParse = false;
                break;
        }
    }   
    
    return 0;
}

CSocketPacket *CXSocketProtocol::PopFront()
{
    return MsgQ.popFront();
}

void CXSocketProtocol::cancel( void )
{
    MsgQ.cancel();
}


