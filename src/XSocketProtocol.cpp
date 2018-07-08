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

    //����head
    char *pBufIndex = (char *)m_xsProtocolBuffer.Buf();
    char *pOriginData = (char *)m_xsProtocolBuffer.Buf();

    if(iProtocolBufferSize <= PACKET_HEAD_LEN)// ͷ���Ϊ 5
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

        //pBufIndex �˴β��ҵ���ʼλ��
        //pFindX ����X ��λ��
        //iLastIndex�ϴ��ҵ�X ��ƫ����
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
            // ���� ��ͷ
            pBufIndex = pFindX + 1;
            continue;
        }
    }


    //��head ֮ǰ�������޳�
    int iPopdataLen = pFindX - pOriginData;
    //XSDbug("Pop Data Len : %d\n", iPopdataLen);
    m_xsProtocolBuffer.Pop(NULL, iPopdataLen);

    //���¶�λsize ��index
    iProtocolBufferSize = m_xsProtocolBuffer.Size();
    if(iProtocolBufferSize <= (PACKET_HEAD_LEN + 4 + 1 + 4))//SEQ = 4, TYPE =1, LEN = 4
    {
        XSWarn("Head ,Seq, Type, Len, need more data, current data len : %d!!\n", iProtocolBufferSize);
        return MORE_DATA_EXPECTED;  
    }
    //XSDbug("Len after pop: %d\n", iProtocolBufferSize);
    pBufIndex = (char *)m_xsProtocolBuffer.Buf();

    // ���type
    pBufIndex += PACKET_HEAD_LEN + 4;
    int iType = GetProtocolType((const char *)pBufIndex);
    //XSDbug("Content Type : %d\n", iType);
    if(iType < 0)
    {
        ///��head ��type ֮��������޳�
        iPopdataLen = PACKET_HEAD_LEN + 4 ;
        XSWarn("type is error!!\n");
        XSWarn("Pop Data Len : %d\n", iPopdataLen);
        m_xsProtocolBuffer.Pop(NULL, iPopdataLen);
        return DATA_CORRUPTED;  
    }
    
    // ȷ��Len
    pBufIndex += 1;
    int iProtocolContentLen = GetProtocolContentLen((const char *)pBufIndex);
    //XSDbug("Content Length : %d\n", iHttpContentLen);
    if( iProtocolContentLen >= DATA_MAX_SIZE)
    {
        ///��head ��len ֮��������޳�
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
    
    // У��tail
    pBufIndex += 4 + iProtocolContentLen;
    if(strncmp(correct_tail, pBufIndex, PACKET_TAIL_LEN))
    {
        ///��head ��len ֮��������޳�
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
            // ���յ�һ�������� HTTP ��Ϣ
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
                    //����ж���һ�� MSG����ôҪ�Խ����������ݽ��н���
                    //m_xsProtocolBuffer.Reset();
                }
                else
                {
                    bShouldParse = false;
                }
                break;
            }
            // ���յ����������쳣
            case DATA_CORRUPTED:
            case DATA_TOO_LONG:
            {
                XSWarn("Data Is Error!\n");
                bShouldParse = false;
                // ��Ҫ���ؿ����£��Ƿ���Ҫ���ý��ջ��塢���ͻ��壬�ر��׽���
                return -1;
            }
            // ��Ҫ���������
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


