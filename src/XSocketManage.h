#ifndef __X_SOCKET_MANAGE_H__
#define __X_SOCKET_MANAGE_H__

#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <errno.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include "SocketCommon.h"
#include "XSPrint.h"
#include "XSocketProtocol.h"

#define DEFAULT_PORT 8000
#define BACKLOG 5   /* Number of allowed connections */ 
#define NSECS_PER_SEC       1000000000

#define ERR_CODE_NEW_CONNECTION 1
#define ERR_CODE_SUCCESS 0
#define ERR_CODE_CLOSED -1

typedef void (*SocketManageCbFunc)(void* data, int datalen, int errcode);

class CXSocketManage
{
public:
	CXSocketManage();
	virtual ~CXSocketManage();	
	virtual int XStart(uint timeout);
	virtual int XSend(int seq, const void *data, uint len, uint timeout);
	virtual int XAttachRecvCBFunc(SocketManageCbFunc cb);	
	virtual int XStop(); 	
};


#endif
