#ifndef _X_BUFFER_H
#define _X_BUFFER_H

/*!
	\class CEncode
	\brief buffer managerment

*/
class CHSBuffer
{
public:
	CHSBuffer();
	virtual ~CHSBuffer();
	 
    /*! \fn int Append(unsigned char * pszStr, const unsigned int stSize)
        \brief append data to buffer
    
        \param[in] pszStr pointer of data
        \param[in] stSize data size
        \return 
          < 0 : error
          >=0 data size
    */
	int Append(unsigned char * pszStr, const unsigned int stSize);
    
    /*! \fn int Get(unsigned char * pszStr, const unsigned int stSize)
        \brief Get data from buffer
    
        \param pszStr pointer of buf
        \param stSize data size
        \return 
          < 0 : error
          0 :success
    */
	int Get(unsigned char * pszStr, const unsigned int stSize);
    
    /*! \fn int Pop(unsigned char * pszStr, const unsigned int stSize)
        \brief Pop data from buffer
    
        \param pszStr pointer of buf
        \param stSize data size
        \return 
          < 0 : error
          0 :success
    */
	int Pop(unsigned char * pszStr, const unsigned int stSize);
    
    /*! \fn int Pour(unsigned char * pszStr, const unsigned int stSize)
        \brief Pour data from buffer
    
        \param pszStr pointer of buf
        \param stSize data size
        \return 
          < 0 : error
          0 :success
    */
	int Pour(unsigned char * pszStr, const unsigned int stSize);
    
    /*! \fn void Reset(const bool isClear = false)
        \brief Reset buffer
        \param isClear true:free buffer false:clear data only
        \return buf
    */
	void Reset(const bool isClear = false);
    
    /*! \fn unsigned int Size()
        \brief Get size of buf
        \return size of data
    */
	unsigned int Size();
    
    /*! \fn unsigned const char * Buf() const
        \brief get the pointer of data
        \return pointer of data
    */
	unsigned const char * Buf() const;
    
    /*! \fn void SetIncreaseSize(const unsigned int IncreaseSize)
        \brief set Increase step
        \param IncreaseSize alloc memory step when buf is full
        \return void
    */
	void SetIncreaseSize(const unsigned int IncreaseSize);


private:
    
    /*! \fn void Initialize()
        \brief intialize the buf
        \return void
    */
	void Initialize();
    
public:
    
private:
    
    /*! \var unsigned char * m_pBuf
        \brief buf pointer
    */
	unsigned char * m_pBuf;
    
    /*! \var unsigned int  m_iDataSize
        \brief data size
    */
	unsigned int  m_iDataSize;
    
    /*! \var unsigned int m_iBufSize
        \brief current buf size
    */
	unsigned int m_iBufSize;
    
    /*! \var unsigned int m_iIncreaseSize
        \brief the step to alloc mem when need ,the default value is 32 Byte
    */
	unsigned int m_iIncreaseSize;
};

#endif //_ABUFFER_H

