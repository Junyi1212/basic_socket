#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include "XSocket.h"
#include <iostream>
using namespace std;
XSocketFops *fops;

//uchar sendData[8]={0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};
#define MAX_DATA_LINE   100
char recvline[MAX_DATA_LINE];
char sendline[MAX_DATA_LINE];  

void RecieveFuncCB(void* cbPriv, void* data, int datalen, int errcode)
{
    if(-1 == errcode)
    {
        printf("Peer Close Socket, you need stopConnect--->startConnect\n");
    }
    else if(1 == errcode)
    {
        printf("A new Connect!!!\n");
    }
    else if(0 == errcode)
    {
        char *buffer = (char *)data;
        printf("*********************************************\n");
        printf("datalen:%d, errcode:%d\n",datalen, errcode);
/*
        memset(recvline, 0, MAX_DATA_LINE);
        memcpy(recvline, data, datalen);
        printf("%s\n",recvline);
*/
        
        int i;
        for(i=0; i<datalen; i++)
        {
            printf("%x ", buffer[i]);
        }
        printf("\n");
        
    }
}

pthread_t threadSend_t;
bool threadLoop = false;
void *ThreadSend(void *arg)
{
    int ret = -1;
    threadLoop = true;
    static int iLen = 0;
    while(threadLoop)
    {
        iLen++;
        if(iLen == MAX_DATA_LINE)
        {
            iLen = 1;
        }
        memset(sendline, 0, MAX_DATA_LINE);
        for(int i=0; i<iLen; i++)
        {
            if(i%2 == 0)
            {
                sendline[i] = 0;
            }
            else
            {
                sendline[i] = i;
            }
        }
        ret= fops->sendData(fops, sendline, iLen, 3000);
        if(ret == -1)
        {
            printf("Exit\n");
            return NULL;
        }
        usleep(100000);
    }
}


void printHelp(void)
{
    printf("Test Begin!\n");
    printf("Please Enter:\n"
            "a---(server connect)\n"
            "b---(client connect)\n"
            "c---(send data)\n"
            "d---(stop)\n"
            "e---(thread create)\n"
            "f---(thread exit)\n"
            );  

}

int main(int argc, char **argv)
{
    int ret = 0;
    uchar ch;
    XSocketType type; 
    RecvCallBackParam param;
    param.cbFunc = RecieveFuncCB;
    param.cbPriv = NULL;
    
    printHelp();

    createXSocket(NULL, &fops);
    
    while((ch = getchar())!='Q')
    {
        switch(ch)
        {
            case 'a'://server connect
                type = SOCKET_SERVER;
                
                fops->setCallback(fops, &param);
                ret = fops->startConnect(fops, type, 5000);
                printf("ret:%d\n",ret);
                break;
            case 'b'://client connect
                type = SOCKET_CLIENT;
                
                fops->setCallback(fops, &param);
                ret = fops->startConnect(fops, type, 5000);     
                printf("ret:%d\n",ret);
                break;
            case 'c'://send data
                printf("send msg : \n");
                memset(sendline, 0, MAX_DATA_LINE);
                scanf("%s", &sendline);

                ret= fops->sendData(fops, sendline, strlen(sendline), 3000);
                printf("ret:%d\n",ret);
                break;
            case 'd'://stop
                ret = fops->stopConnect(fops);
                printf("ret:%d\n",ret);
                break;
            case 'e'://create pthread       
                pthread_create(&threadSend_t, NULL, ThreadSend, NULL);
                break;
            case 'f'://exit pthread
                threadLoop = false;
                break;
            default:
                //printHelp();
                break;
        }
        
        usleep(10000);
    }   
    
    return 0;
}
