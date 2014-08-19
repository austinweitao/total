#ifdef __cplusplus
extern "C"{
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdarg.h>

#include "unsock.h"

#define UNSOCK_LOOP_MAX     (3)            /* ѭ���շ��������� */
#define UNSOCK_TIMEOUT      (0)             /* select��ʱ */
#define UNSOCK_AGAIN_TV     (200*1000)      /* �ط��շ�ʱ���� */

/***************************************************************************************
*�������ͻ�����������һ��ϵͳ���嵽socker����Ŀ�����
*���ϵͳ���ܣ�������ȫ�н���buffer���л��塣
****************************************************************************************
*/
#define UNSOCK_SENDBUF_SZ   (0)             
#define UNSOCK_RECVBUF_SZ   (1*1024)

void  DebugMsg(const char * fmt, ...)
{
	//printf("dbgmsg:%s\n",sg_logname);
    va_list vap;
    va_start(vap, fmt);
    //FILE * dbgfile = fopen(DEBUG_FILE_NAME, "a+");
    FILE * dbgfile = fopen("/sdcard/libsocket.log", "a+");
    if (dbgfile != NULL){
        vfprintf(dbgfile, fmt, vap);
        va_end(vap);
        fclose(dbgfile);
    }
}



/***************************************************************************************
*
* �������ܣ�    UnSocketBind����ʼ��unix���udp_socket
*               iModuleid    --Ϊģ��id
*               iMode   --Ϊ0ʱ�����նˣ�1Ϊ���Ͷ�
* ����ֵ��      �ɹ������׽��־����ʧ�ܷ���-1 
****************************************************************************************
*/
int UnSocketBind(int iModuleid, UN_MDL_TYPE_EN iMode)
{
	int	fd = -1;
	struct sockaddr_un saddr;
	int family = AF_LOCAL;
	int type = SOCK_DGRAM;
	int protocol = 0; 
	int flags = 0;

	if ( (fd = socket(family, type, protocol)) < 0) 
	{
		DEBUG("socket error:%s\n", strerror(errno));
		return -1;
	}

    if(UN_SERVER == iMode)
    {
       char pPath[128];
	if(iModuleid == MODULE_HOSTID)
	{
		strcpy(pPath,UN_SERVER_PATH);
	}
	else
	{
		sprintf(pPath,"%s%03d",UN_CLIENT_PATH,iModuleid);
	}
	//printf("create socket service %s\n",pPath);
    	unlink(pPath);
    	bzero(&saddr, sizeof(struct sockaddr_un));
    	saddr.sun_family = family;
    	strcpy(saddr.sun_path, pPath);
    	if (bind(fd, (struct sockaddr *) &saddr, sizeof(saddr)) < 0) 
	{
    		DEBUG("bind path:%s, error:%s\n", pPath, strerror(errno));
			goto CloseSock;
    	}
    }

	/*��ʽ����socketΪ����������ͬƽ̨Ĭ���Ƿ�����Ҳ��ͬ*/
	if(0 > (flags=fcntl(fd, F_GETFL, 0)))
	{
	    DEBUG("Failed to F_GETFL, error:%s\n", strerror(errno));
	    goto CloseSock;
	}
	if(0 > fcntl(fd, F_SETFL, flags|O_NONBLOCK))
	{
	    DEBUG("Failed to F_SETFL, error:%s\n", strerror(errno));
	    goto CloseSock;
	}

	return fd;
CloseSock:;
    UnSocketDestroy(fd, iModuleid, iMode);
    return -1;
}
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
void UnSocketDestroy(int iFd, int iModuleid, UN_MDL_TYPE_EN iMode)
{
	if(iFd <= 0)
	{
		return ;
	}
	close(iFd);
	if((UN_SERVER == iMode))
	{
		char pPath[128];
		if(iModuleid == MODULE_HOSTID)
		{
			strcpy(pPath,UN_SERVER_PATH);
		}
		else
		{
			sprintf(pPath,"%s%03d",UN_CLIENT_PATH,iModuleid);
		}
	    unlink(pPath);
	}
}

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
int UnRecvFrom(int iFd, void *pBuf, size_t nBytes)
{
    struct sockaddr_un from;
    socklen_t addrlen = sizeof(struct sockaddr_un);
    int iRet = -1;
    int iCnt = 0;       /*���ռ���*/
    fd_set iBits;
    struct timeval tout;

recvAgain:
    FD_ZERO(&iBits);
    FD_SET(iFd, &iBits);
    tout.tv_sec = UNSOCK_TIMEOUT;
    tout.tv_usec = 100*1000;
    iRet = select(iFd+1, &iBits, NULL, NULL, &tout);
    if(0 >= iRet)
    {
        return (errno==EINTR) ? 0 : iRet;       /*EINTR����ɽ��ܴ��󣬺ͳ�ʱһ������0*/
    }

    if(!FD_ISSET(iFd, &iBits))
    {
        return 0;
    }
    
    iRet = recvfrom(iFd, pBuf, nBytes, 0, (struct sockaddr*)&from, &addrlen);
    if(0>iRet)
    {
        if (((errno == EINTR)||(errno == EAGAIN)) && (iCnt<UNSOCK_LOOP_MAX))
        {
            usleep(UNSOCK_AGAIN_TV);
            iCnt ++;
            goto recvAgain;
        }
    }
     //DEBUG("ret:%d,cnt:%d\n",iRet,iCnt);
	return iRet;  
}

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
int UnSendTo(int iFd, const void *pBuf, size_t nbytes, int iDst)
{
    socklen_t addrlen = sizeof(struct sockaddr_un);
    struct sockaddr_un to;
    int iRet = -1;
    int iCnt = 0;       /*�ط�����*/

    fd_set oBits;
    struct timeval tout;
	
    if((0>iFd)  ||(NULL==pBuf) || (0>=nbytes))
    {
        DEBUG("Failed to do UnSendTo, input args invalid\n");
        return iRet;
    }

	to.sun_family = AF_LOCAL;
	if(iDst == MODULE_HOSTID)
	{
		strcpy(to.sun_path,UN_SERVER_PATH);
	}
	else
	{
		sprintf(to.sun_path,"%s%03d",UN_CLIENT_PATH,iDst);
	}

sendAgain:;
    FD_ZERO(&oBits);
    FD_SET(iFd, &oBits);
    tout.tv_sec = UNSOCK_TIMEOUT;
    tout.tv_usec = 0;

    iRet = select(iFd+1, NULL, &oBits, NULL, &tout);
    if(0 >= iRet)
    {
    	 DEBUG("send socket select ret:%d\n",iRet);
        return (errno==EINTR) ? 0 : iRet;       /*EINTR����ɽ��ܴ��󣬺ͳ�ʱһ������0*/
    }
    if(!FD_ISSET(iFd, &oBits))
    {
    	 DEBUG("socket %s can not write\n",to.sun_path);
        return 0;
    }
    iRet = sendto(iFd, pBuf, nbytes, 0, (struct sockaddr *) &to, addrlen);
    if(0>iRet)
    {
        if (((errno == EINTR)||(errno == EAGAIN)) && (iCnt<UNSOCK_LOOP_MAX))
        {
            usleep(UNSOCK_AGAIN_TV);
            iCnt ++;
            goto sendAgain;
        }
	//perror("UnSendTo failed once\n");
    }
    //printf("send result:%d\n",iRet);
    return iRet;
}

#ifdef __cplusplus
}
#endif

