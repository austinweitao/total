/******************************************************************************************************
*******************************************************************************************************
*/

#ifndef __LOG_H__
#define __LOG_H__

#include <stdarg.h>

typedef enum 
{
    LOG_ERR = 0,/* ���ش��� */
    LOG_WAR,    /* ���� */
    LOG_NOT,    /* ֪ͨ */
    LOG_DBG,    /* ֻ�ǵ����õ���־ */
    LOG_LAST,
}LOG_LEV_EN;

#define     LOG_TTY             0x00       /*����*/
#define     LOG_FLASH           0x01       /*FLASH*/

/* ����Ĭ�ϵ���־�������ͼ��𣬵��������ļ�ʧ��ʱ��ȡ��ֵ */
//#define     LOG_DIRECT          LOG_TTY
//#define     LOG_LEVEL           LOG_DBG

/******************************************************************************************************
* ��������:     LOG��������־��ӡ����,��ӡʱ�����ǰ�ļ�д���ˣ���ѭ��д�µ��ļ�
* �������˵��: iLevel---��ӡ�ȼ�:
*                        LOG_ERR ���ش���
*                        LOG_WAN ����
*                        LOG_DBG ֻ�ǵ����õ���־
* �������˵��: ��
* ����ֵ:       ��
*******************************************************************************************************
*/
void SBS_LOG(int   iLevel,int  iLine,char *pcFile,const char *pcFunc,const char * fmt, ...);

/******************************************************************************************************
* ��������:     LogInit��������Ҫ��ʼ��дLog�ļ��е�һЩ����
* �������˵��: pcLogPath--Log�ļ�·������:/home/projects/rtsp/log/,��·���������׺��/��
*               pcLogPrefix--Log�ļ���ǰ׺
*               iMaxLogSize--Log�ļ��Ĵ�С
*               iMaxFileCount--Log�ļ���������Ҫ���ڿ��ƿռ��С�����ڹ涨���ļ�����ѭ��
*               iMaxFileSize--iMaxFileCount ��ȡ0ʱ����ʾȡĬ��ֵ 5�����Ϊ1M���ļ�
*               iLevel--��ӡ����,�������LOG_LEV_ENö������
*               iDirect--��ӡ�������,֧�ֵ����������LOG_TTY��LOG_FLASH
* �������˵��: ��
* ����ֵ:       �ɹ�����0��ʧ�ܷ���1
*******************************************************************************************************
*/
int LogInit(char *pcFilePath, char *pcFilePrefix, int iMaxFileSize, int iMaxFileCount,
              int iLevel,int iDirect);

/******************************************************************************************************
* ��������:     LogDestroy�������ͷ�LOG��Դ
* �������˵��: ��
* �������˵��: ��
* ����ֵ:       ��
*******************************************************************************************************
*/
void LogDestroy(void);

#define     LOGERR(fmt,args...)   LOG(LOG_ERR, __LINE__,__FILE__,__FUNCTION__,fmt,## args)
#define     LOGWAR(fmt,args...)   LOG(LOG_WAR,__LINE__,__FILE__,__FUNCTION__,fmt,## args)
#define     LOGNOT(fmt,args...)   LOG(LOG_NOT,__LINE__,__FILE__,__FUNCTION__,fmt,## args)
#define     LOGDBG(fmt,args...)   LOG(LOG_DBG,__LINE__,__FILE__,__FUNCTION__,fmt,## args)

/*��LOG*/
void SLOG(int   iLevel,char * plog);

//#define     LOGFTPDBG(fmt,args...)   LOG(LOG_DBG,"FTP",__LINE__,__FILE__,__FUNCTION__,fmt,## args)
#endif
