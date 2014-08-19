
#ifndef __UNSOCK_H__
#define __UNSOCK_H__

#include <sys/un.h>
#include <fcntl.h>
#include <stdbool.h>
#include "sbslog.h"

#define	MODULE_HOSTID	0

#define	UN_SERVER_PATH		"/tmp/.SBS_SERVER"
#define	UN_CLIENT_PATH		"/tmp/.SBS_CLIENT"


//��־���
#define     LOG_PREFIX 	"socket"  
#define     LOG_PATH 		"/tmp/socket/"  
#define     LOG_DIRECT  	LOG_FLASH  
#define     LOG_LEVERL  	LOG_DBG 

#define     LOG_SIZE 		(1*1024*1024) 	//1MB
#define     LOG_FILE_CNT 	10 

//#define GLOBAL_DEBUG                //������־����
#define DEBUG_FILE_NAME 	"/mnt/datas/logs/libsocket.log"

#ifdef GLOBAL_DEBUG
//#define   DEBUG(fmt,args...)   SBS_LOG(LOG_DBG, __LINE__,__FILE__,__FUNCTION__,fmt,## args)
//#define DEBUG(format, args...)  DebugMsg("[%s][%s()] " format, __FILE__,__func__, ## args)
//#define DEBUG(format, args...)  printf("Function:%s()=>" format, __func__, ## args)
#define DEBUG(format, args...)  printf("" format, ## args)
#else
#define DEBUG(format, args...) do{}while(0)
#endif


#ifndef ERR
#define ERR(X...)	do { \
						printf("\nTRACE--[File: %s]--[Line: %d]--[Function: %s]--ERR:", \
						__FILE__, __LINE__,__FUNCTION__); printf(X);\
					}while(0) 
#endif

/*ʱ��ȽϺ꣬��ֹ�������*/
/*
 * Check at compile time that something is of a particular type.
 * Always evaluates to 1 so you may use it easily in comparisons.
 */
#define	ONE_SECOND		100
#define typecheck(type,x) \
({	type __dummy; \
	typeof(x) __dummy2; \
	(void)(&__dummy == &__dummy2); \
	1; \
})
#define TimeAfter(a,b)  \
    (typecheck(unsigned long, a) && typecheck(unsigned long, b) && ((long)(b) - (long)(a) < 0))
#define TimeBefore(a,b) TimeAfter(b,a)

#define TimeAfterEq(a,b)    \
    (typecheck(unsigned long, a) && typecheck(unsigned long, b) && ((long)(a) - (long)(b) >= 0))
#define TimeBeforeEq(a,b) TimeAfterEq(b,a)

void  DebugMsg(const char * fmt, ...);

#define	MSG_MAGIC			0x2579
#define	MSG_STRING_LEN		1024
/*���͵���Ϣ�ṹ��*/
typedef struct  _MSG_HEAD
{
	unsigned short magic;
	unsigned short msgid;
	unsigned short appid;
	unsigned short srcmodule;
	unsigned short len;
}MSG_HEAD;
#define	HEAD_LEN	sizeof(MSG_HEAD)
typedef struct  _MSG_BODY
{
	int		iParam[5];
	char	sParam[MSG_STRING_LEN];
}MSG_BODY;
#define	BODY_INT_LEN	sizeof(int)*5

typedef enum UN_MDL_TYPE_en
{
	UN_SERVER = 0,
	UN_CLIENT,
	UN_MDL_CNT
}UN_MDL_TYPE_EN;

/***************************************************************************************
*
* �������ܣ�    UnSocketBind����ʼ��unix���udp_socket
*               iModuleid    --Ϊģ��id
*               iMode   --Ϊ0ʱ�����նˣ�1Ϊ���Ͷ�
* ����ֵ��      �ɹ������׽��־����ʧ�ܷ���-1 
****************************************************************************************
*/
int UnSocketBind(int iModuleid, UN_MDL_TYPE_EN iMode);

/***************************************************************************************
*
* �������ܣ�    UnSocketDestroy������unix���udp_socket
* �������˵����
*   		   iFd     --socket��fd
*               iModuleid    --Ϊģ��id
*               iMode   --Ϊ0ʱ�����նˣ�1Ϊ���Ͷ�
* ����ֵ��      �ɹ�����0��ʧ�ܷ���-1 
****************************************************************************************
*/
void UnSocketDestroy(int iFd, int iModuleid, UN_MDL_TYPE_EN iMode);

/***************************************************************************************
*
* �������ܣ�    UnRecvFrom������socket����
* �������˵����
*		   iFd:	Ϊģ��id
*               pBuf:	��������ָ��
*               nBytes :	���ݳ���
* ����ֵ��      �ɹ����ؽ��յ��ֽ�������ʱ����EINTR����0, ʧ�ܷ���-1 
****************************************************************************************
*/
int UnRecvFrom(int iFd, void *pBuf, size_t nBytes);

/***************************************************************************************
*
* �������ܣ�    UnSendTo������socket����
* �������˵����
*		   iFd :		Ϊģ��id
*               pBuf:		��������ָ��
*               nBytes:	���ݳ���
*               iModuleid:	Ŀ���ַ����
* ����ֵ��      �ɹ����ط��͵��ֽ�����ʧ�ܷ���-1����ʱ����EINTR����0 
****************************************************************************************
*/
int UnSendTo(int iFd, const void *pBuf, size_t nbytes, int iModuleid);


#endif/*__UNSOCK_H__*/





