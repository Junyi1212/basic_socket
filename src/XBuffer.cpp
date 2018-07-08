#include <string.h>
#include <stdlib.h>

#include "XBuffer.h"

#define W_FREE(p) if(NULL != (p)) {free((p)); (p)=NULL; }

#ifdef _DEBUG_THIS
    #define DEB(x) x
    #define DBG(x) x
#else
    #define DEB(x)
    #define DBG(x)
#endif

/*****************************************************************************


*****************************************************************************/
CHSBuffer::CHSBuffer():m_iIncreaseSize(32)
{
    m_pBuf = NULL;
    Initialize();
}

/*****************************************************************************

*****************************************************************************/

CHSBuffer::~CHSBuffer()
{
    W_FREE(m_pBuf);
}

/*****************************************************************************

*****************************************************************************/
void CHSBuffer::Initialize()
{
    W_FREE(m_pBuf);

    m_iDataSize = 0;
    m_iBufSize = 0;

    if (m_iIncreaseSize < 1)
    {
        m_iIncreaseSize = 32;
    }
}


/*****************************************************************************


*****************************************************************************/
int CHSBuffer::Append(unsigned char * pszStr, const unsigned int stSize)
{
    if (0 == stSize)
    {
        //
        // Do not return error here
        //
        return 0;
    }

    //
    // I add many conditiion to
    //      avoid delete the buffered data
    // If want pre alloc mem,
    //      you should use Reset(true) yourself
    //
    if(NULL == pszStr)
    {
        if(NULL == m_pBuf && 0 == m_iDataSize)
        {
            m_pBuf = (unsigned char *) malloc(stSize);
            
            if (NULL == m_pBuf)
            {
                m_iBufSize = 0;
                return -1;
            }
            else
            {
                memset(m_pBuf, 0, stSize);
                m_iBufSize = stSize;
                return 0;
            }
        }
        else
        {

            return -1;
        }
    }

    size_t tsize;
    tsize = stSize+m_iDataSize;

    // should alloc mem
    if ( tsize > m_iBufSize )
    {
        tsize = ( (unsigned int)( (tsize/m_iIncreaseSize) + 1 ) )
                * m_iIncreaseSize;

        //alloc a new mem
        unsigned char *tmpBuf = (unsigned char *) malloc(tsize);

        if (NULL == tmpBuf)
        {
            return -1;
        }
        memset(tmpBuf, 0, tsize);

        // ins the old one
        memcpy(tmpBuf, m_pBuf, m_iDataSize);
        // ins the new one
        memcpy(tmpBuf+m_iDataSize, pszStr, stSize);

        m_iBufSize = tsize;

        // delete the old buf
        W_FREE(m_pBuf);

        m_pBuf = tmpBuf;
    }
    else
    {
        memcpy(m_pBuf+m_iDataSize, pszStr, stSize);
    }

    m_iDataSize += stSize;

    return stSize;
}

/*****************************************************************************

*****************************************************************************/
int CHSBuffer::Get(unsigned char * pszStr, const unsigned int stSize)
{
    // check input
    if (NULL == pszStr
        || 0 == stSize
        || stSize > m_iDataSize)
    {
        return -1;
    }

    memcpy(pszStr, m_pBuf, stSize);

    return 0;
}

/*****************************************************************************


*****************************************************************************/
int CHSBuffer::Pop(unsigned char * pszStr, const unsigned int stSize)
{
    // check input
    if (0 == stSize)
    {
        return 0;
    }
    else if (stSize > m_iDataSize)
    {
        return -1;
    }

    //
    if (NULL != pszStr)
    {
        memcpy(pszStr, m_pBuf, stSize);
    }

    m_iDataSize -= stSize;
    memmove(m_pBuf, m_pBuf+stSize, m_iDataSize);
    memset(m_pBuf + m_iDataSize, 0, stSize);

    return 0;
}

/*****************************************************************************


*****************************************************************************/
int CHSBuffer::Pour(unsigned char * pszStr, const unsigned int stSize)
{
    // check input
    if (0 == stSize)
    {
        return 0;
    }
    else if (stSize > m_iDataSize)
    {
        return -1;
    }

    m_iDataSize -= stSize;

    if (NULL != pszStr)
    {
        memcpy(pszStr, m_pBuf+m_iDataSize, stSize);
    }

    return 0;
}

/*****************************************************************************


*****************************************************************************/
void CHSBuffer::Reset(const bool isClear)
{
    if (true == isClear)
    {
        Initialize();
    }
    else
    {
        if(NULL != m_pBuf)
        {
            memset(m_pBuf, 0, m_iBufSize);
        }
        m_iDataSize = 0;
    }
}

/*****************************************************************************


*****************************************************************************/
unsigned const char *  CHSBuffer::Buf() const
{
    return m_pBuf;
}

/*****************************************************************************


*****************************************************************************/
unsigned int CHSBuffer::Size()
{
    return m_iDataSize;
}

/*****************************************************************************


*****************************************************************************/
void CHSBuffer::SetIncreaseSize(const unsigned int IncreaseSize)
{
    if (IncreaseSize > 0)
    {
        m_iIncreaseSize = IncreaseSize;
    }
}

