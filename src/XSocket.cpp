#include "XSocket.h"
#include "XSocketManage.h"
#include "XSocketServer.h"
#include "XSocketClient.h"

#define REPEAT_SEND_TIMES   3

static RecvCallBackParam m_cbRecvFuncParam;
static int iSeq = 0;
static bool bConnectFlag = false;
static CXSocketManage *hXSocket = NULL; 

void xRecvCb(void* data, int datalen, int errcode)
{
    if(NULL != m_cbRecvFuncParam.cbFunc)
    {
        m_cbRecvFuncParam.cbFunc(m_cbRecvFuncParam.cbPriv, data, datalen, errcode);
    }
}

int xStartConnect(struct XSocketFops *thiz, XSocketType type, uint timeout)
{
    int ret = -1;
    if(NULL == thiz)
    {
        return ret;
    }
    if(true == bConnectFlag)
    {
        XSWarn("Already connect, you need stop first\n");
        return ret;
    }

    if(SOCKET_SERVER == type)//服务端
    {
        XSInfo("Server\n");
        hXSocket = new CXSocketServer();    
    }
    else if(SOCKET_CLIENT == type)//客户端
    {
        XSInfo("Client\n");
        hXSocket = new CXSocketClient();    
    }

    hXSocket->XAttachRecvCBFunc(xRecvCb);
    ret = hXSocket->XStart(timeout);
    if(0 == ret)
    {
        bConnectFlag = true;    
    }
    return ret;
}

int xSendData(struct XSocketFops *thiz, const void *data, uint len, uint timeout)
{
    if((NULL == thiz) || (NULL == hXSocket))
    {
        return -1;
    }

    int count = 0;
    iSeq++;
    if(iSeq >= 10000)
    {
        iSeq = 1;
    }

    while (hXSocket->XSend(iSeq, data, len, timeout/REPEAT_SEND_TIMES) < 0)
    {
        XSDbug("Cannot get response! and try to send again\n");
        count++;
        if(count == REPEAT_SEND_TIMES)
        {
            XSWarn("send failed \n");
            return -1;  
        }
    }

    XSDbug("send success! \n");
    return 0;
}



int xSetCallback(struct XSocketFops *thiz, RecvCallBackParam *param)
{
    if((NULL == thiz) || (NULL == param))
    {
        return -1;
    }

    m_cbRecvFuncParam.cbFunc = param->cbFunc;
    m_cbRecvFuncParam.cbPriv = param->cbPriv;
    
    return 0;
}

int xStopConnect(struct XSocketFops *thiz)
{
    if((NULL == thiz) || (NULL == hXSocket))
    {
        return -1;
    }

    bConnectFlag = false;
    m_cbRecvFuncParam.cbFunc = NULL;

    hXSocket->XStop();
    delete hXSocket;
    hXSocket = NULL;

    return 0;
}

int createXSocket(XSocketDesc *desc, XSocketFops **fops)
{
    XSocketFops *ops;
    
    if(*fops == NULL) 
    {
        ops = (XSocketFops *)malloc(sizeof(XSocketFops));
        if(NULL == ops)
        {
            XSWarn("malloc XSocketFops fail!\n");
            return 1;
        }
        ops->startConnect = xStartConnect;
        ops->sendData = xSendData;
        ops->setCallback = xSetCallback;
        ops->stopConnect = xStopConnect;
    
        *fops = ops;    
        return 0;
    }
    else
    {
        XSWarn("reenter createXSocket\n");
        return -1;
    }
}

int destroyXSocket(struct XSocketFops *thiz)
{
    if(thiz != NULL)
    {
        free(thiz);
        thiz = NULL;
    }
    return 0;
}

