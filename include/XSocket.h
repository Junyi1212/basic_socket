/******************************************************************************* 
* Description:  提供通过Socket 通信的功能
*       1.  硬件说明。 
*           支持socket 通信
*       2.  程序结构说明。 
*           模块化
*       3.  使用说明。 
*           应用先定义一个XSocketFops 的指针；
*	        调用createXSocket初始化库；
*	        再调用内部成员函数；
*       4.  局限性说明。 
*       5.  其他说明。 
*******************************************************************************/ 
#ifndef __LIB_X_SOCKET_H__
#define __LIB_X_SOCKET_H__

#include "Types.h"
#include "Defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/// \defgroup XSocketFopsAPI
/// XSocket操作集
///	\n 调用流程图:
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
	SOCKET_SERVER = 0x00,	///< socket作为服务端
	SOCKET_CLIENT = 0x01	///< socket作为客户端
} XSocketType;

/// 接收数据的回调接口原型，函数内禁止阻塞或者发送数据等操作
/// \param [out] cbPriv 上层设置的回调参数
/// \param [out] data 待发送缓冲区地址
/// \param [out] len  待发送数据长度
/// \param [out] errcode  错误码
///     errcode = 0 表示正常接收数据；
///     errcode > 0 服务端表示当前的连接数；客户端暂无意义
///     errcode < 0
///         -1 表示断开连接，上层需调用SONIA_stopConnect--->SONIA_startConnect重启连接
typedef void (*CallbackRecvFunc)(void* cbPriv, void* data, int datalen, int errcode);

/// 回调相关参数结构，128字节
typedef struct RecvCallBackParam
{
	CallbackRecvFunc	cbFunc;			///< 回调的指针
	void*				cbPriv;			///< 回调的参数
	int					reserved[30];	///< 保留
} RecvCallBackParam;

/// 操作接口集，512字节
typedef struct XSocketFops
{
	/// 底层私有数据
	void *priv;

	/// 设置接收数据回调函数
	///	
	/// \param [in] thiz 接口指针
	/// \param [in] param 指向数据接收回调参数结构RecvCallBackParam的指针
	/// \retval 0 设置成功
	/// \retval <0 设置失败
	int (*setCallback)(struct XSocketFops *thiz, RecvCallBackParam *param);

	/// 开启socket通信，同步接口
	///	
	/// \param [in] thiz 接口指针
	/// \param [in] type 类型，参见XSocketType
	/// \param [in] timeout 超时时间，单位毫秒
	/// \retval 0 开启成功
	/// \retval <0 开启失败或超时
	int (*startConnect)(struct XSocketFops *thiz, XSocketType type, uint timeout);

	/// 关闭socket通信，清空数据队列，同步接口
	///	
	/// \param [in] thiz 接口指针
	/// \retval 0 关闭成功
	/// \retval <0 关闭失败
	int (*stopConnect)(struct XSocketFops *thiz);

	/// 发送接口，同步接口
	///	
	/// \param [in] thiz 接口指针
	/// \param [in] data 待发送缓冲区地址
	/// \param [in] len  待发送数据长度
	/// \param [in] timeout 超时时间，单位毫秒
	/// 			timeout > 0 正常超时时间, timeout = 0 不超时,立即返回
	/// \retval =0  发送成功
	/// \retval <0  发送失败或超时
	int (*sendData)(struct XSocketFops *thiz, const void *data, uint len, uint timeout);

	///保留
	void* reserved[123];
} XSocketFops;


/// 接口描述，32位系统下128字节
typedef struct XSocketDesc
{
	int reserved[32];	///< 保留
} XSocketDesc;

/// 创建并初始化操作集
///
/// \param [in] desc 接口描述结构XSocketDesc指针    
/// \param [out] fops 返回创建好的XSocketFops对象的指针
/// \retval 0   成功
/// \retval <0   失败
int createXSocket(XSocketDesc *desc, XSocketFops **fops);

/// 销毁操作集
///
/// \param [in] thiz 接口指针
/// \retval 0  成功
/// \retval <0  失败
int destroyXSocket(struct XSocketFops *thiz);

/// @} end of group

#ifdef __cplusplus
}
#endif

#endif // __LIB_X_SOCKET_H__

