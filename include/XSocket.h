/******************************************************************************* 
* Description:  �ṩͨ��Socket ͨ�ŵĹ���
*       1.  Ӳ��˵���� 
*           ֧��socket ͨ��
*       2.  ����ṹ˵���� 
*           ģ�黯
*       3.  ʹ��˵���� 
*           Ӧ���ȶ���һ��XSocketFops ��ָ�룻
*	        ����createXSocket��ʼ���⣻
*	        �ٵ����ڲ���Ա������
*       4.  ������˵���� 
*       5.  ����˵���� 
*******************************************************************************/ 
#ifndef __LIB_X_SOCKET_H__
#define __LIB_X_SOCKET_H__

#include "Types.h"
#include "Defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \defgroup XSocketFopsAPI
/// XSocket������
///	\n ��������ͼ:
/// \code
///    ===========================
///                |
///          *createXSocket
///                |--------------+
///            setCallback        |
///                |              |
///           startConnect        |
///          +-----|              |
///          |  sendData          |
///          +-----|              |
///            stopConnect        |
///                |--------------+
///           destroyXSocket 
///                |
///    ===========================
/// \endcode
/// @{

typedef	enum XSocketType
{
	SOCKET_SERVER = 0x00,	///< socket��Ϊ�����
	SOCKET_CLIENT = 0x01	///< socket��Ϊ�ͻ���
} XSocketType;

/// �������ݵĻص��ӿ�ԭ�ͣ������ڽ�ֹ�������߷������ݵȲ���
/// \param [out] cbPriv �ϲ����õĻص�����
/// \param [out] data �����ͻ�������ַ
/// \param [out] len  ���������ݳ���
/// \param [out] errcode  ������
///     errcode = 0 ��ʾ�����������ݣ�
///     errcode > 0 ����˱�ʾ��ǰ�����������ͻ�����������
///     errcode < 0
///         -1 ��ʾ�Ͽ����ӣ��ϲ������SONIA_stopConnect--->SONIA_startConnect��������
typedef void (*CallbackRecvFunc)(void* cbPriv, void* data, int datalen, int errcode);

/// �ص���ز����ṹ��128�ֽ�
typedef struct RecvCallBackParam
{
	CallbackRecvFunc	cbFunc;			///< �ص���ָ��
	void*				cbPriv;			///< �ص��Ĳ���
	int					reserved[30];	///< ����
} RecvCallBackParam;

/// �����ӿڼ���512�ֽ�
typedef struct XSocketFops
{
	/// �ײ�˽������
	void *priv;

	/// ���ý������ݻص�����
	///	
	/// \param [in] thiz �ӿ�ָ��
	/// \param [in] param ָ�����ݽ��ջص������ṹRecvCallBackParam��ָ��
	/// \retval 0 ���óɹ�
	/// \retval <0 ����ʧ��
	int (*setCallback)(struct XSocketFops *thiz, RecvCallBackParam *param);

	/// ����socketͨ�ţ�ͬ���ӿ�
	///	
	/// \param [in] thiz �ӿ�ָ��
	/// \param [in] type ���ͣ��μ�XSocketType
	/// \param [in] timeout ��ʱʱ�䣬��λ����
	/// \retval 0 �����ɹ�
	/// \retval <0 ����ʧ�ܻ�ʱ
	int (*startConnect)(struct XSocketFops *thiz, XSocketType type, uint timeout);

	/// �ر�socketͨ�ţ�������ݶ��У�ͬ���ӿ�
	///	
	/// \param [in] thiz �ӿ�ָ��
	/// \retval 0 �رճɹ�
	/// \retval <0 �ر�ʧ��
	int (*stopConnect)(struct XSocketFops *thiz);

	/// ���ͽӿڣ�ͬ���ӿ�
	///	
	/// \param [in] thiz �ӿ�ָ��
	/// \param [in] data �����ͻ�������ַ
	/// \param [in] len  ���������ݳ���
	/// \param [in] timeout ��ʱʱ�䣬��λ����
	/// 			timeout > 0 ������ʱʱ��, timeout = 0 ����ʱ,��������
	/// \retval =0  ���ͳɹ�
	/// \retval <0  ����ʧ�ܻ�ʱ
	int (*sendData)(struct XSocketFops *thiz, const void *data, uint len, uint timeout);

	///����
	void* reserved[123];
} XSocketFops;


/// �ӿ�������32λϵͳ��128�ֽ�
typedef struct XSocketDesc
{
	int reserved[32];	///< ����
} XSocketDesc;

/// ��������ʼ��������
///
/// \param [in] desc �ӿ������ṹXSocketDescָ��    
/// \param [out] fops ���ش����õ�XSocketFops�����ָ��
/// \retval 0   �ɹ�
/// \retval <0   ʧ��
int createXSocket(XSocketDesc *desc, XSocketFops **fops);

/// ���ٲ�����
///
/// \param [in] thiz �ӿ�ָ��
/// \retval 0  �ɹ�
/// \retval <0  ʧ��
int destroyXSocket(struct XSocketFops *thiz);

/// @} end of group

#ifdef __cplusplus
}
#endif

#endif // __LIB_X_SOCKET_H__

