
#ifdef __cplusplus
extern "C"{
#endif

/* Standard Linux headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#include "sbslog.h"

#define     MAX_LOG_SIZE					(1048576*10) /*��СΪ�ֽڣ�Ĭ��Ϊ10M=1024 * 1024Byte*/
#define     MAX_FILE_CNT					10 /*ui��־���Ϊ5M*/

const char *sg_acLOG_LEV[] =
{
    "ERR",
    "WAN",
    "NOT",
    "DBG",
    "",
};

/* ����LOG_FILE�ṹ�� */
typedef struct LOG_FILE_s
{
	char    pcFilePath[256];    /* Log��ǰ�ļ�·�� */
	char   pcFilePrefix[10];    /* Logǰ׺ */
	int     iMaxFileSize;       /* Log��С */
	int     iMaxFileCount;      /* Log�ļ���������Ҫ���ƿռ��С */

	FILE *pfFileHandle;         /* �ļ���� */ 
	int iSeq;                   /* Log�ļ���� */
	int iSize;                  /* ��ǰ�ļ���С */
    int iLevel;                 /* ��ӡ���� */
    int iDirect;                /* �Ƿ�д��־�ļ�������ֱ���������̨ */
    pthread_mutex_t sLockMutex; /* �߳��� */
} LOG_FILE_T;

static LOG_FILE_T   sg_LogArgs;

/******************************************************************************************************
* ��������:     LogGetSeqID��������ȡLOGĿ¼�µ�ui_log_id.dat�ļ���ȡ��ǰLOG��־��
* �������˵��: ��  
* �������˵��: ��
* ����ֵ:       �ɹ�����0,ʧ�ܷ���-1
*******************************************************************************************************
*/
static int LogGetSeqID(void)
{
     char        acPath[256],acTemp[64];
     FILE       *pIDFile = NULL;
     
     sprintf(acPath,"%s%s.index",sg_LogArgs.pcFilePath,sg_LogArgs.pcFilePrefix);
     pIDFile = fopen(acPath, "r");
     if(pIDFile != NULL)
     {
         if(fgets(acTemp, 64, pIDFile))
         {
               sg_LogArgs.iSeq = atoi(acTemp);
         }
         fclose(pIDFile);
         //printf("logGetSeqID:%d,%s\n",sg_LogArgs.iSeq,acTemp);
         return 0;
     }
     return -1;
}

/******************************************************************************************************
* ��������:     LogGetFSize�������õ���ǰ������־�Ĵ�С
* �������˵��: ��
* �������˵��: ��
* ����ֵ:       ��
*******************************************************************************************************
*/
static void LogGetFSize(void)
{
    char acPath[256];
    struct stat stBuf;
     
    memset(&stBuf, 0, sizeof(stBuf));
    sprintf(acPath,"%s%s%02d.log",sg_LogArgs.pcFilePath,sg_LogArgs.pcFilePrefix,sg_LogArgs.iSeq);
    stat(acPath, &stBuf);//�õ�ָ�����ļ�����Ϣ
    sg_LogArgs.iSize = stBuf.st_size; 
    //printf("logGetFSize:%d\n",sg_LogArgs.iSize);
}

/******************************************************************************************************
* ��������:     LogSetSeqID����������LOGĿ¼�µ�ui_log_id.dat�ļ���ȡ��ǰLOG��־��
* �������˵��: ��
* �������˵��: ��
* ����ֵ:       �ɹ�����SEQID��ʧ�ܷ���0
*******************************************************************************************************
*/
static void LogSetSeqID(void)
{
     char        acPath[256],acTemp[64];
     FILE       *pIDFile;
     sprintf(acPath,"%s%s.index",sg_LogArgs.pcFilePath,sg_LogArgs.pcFilePrefix);
     pIDFile = fopen(acPath, "w");
     if(pIDFile != NULL)
     {
        sprintf(acTemp, "%d", sg_LogArgs.iSeq);
        fputs(acTemp, pIDFile);
        fclose(pIDFile);
     }
}

/******************************************************************************************************
* ��������:     LogChangeFile�������л���־�ļ�
* �������˵��: ��
* �������˵��: ��
* ����ֵ:       ��
*******************************************************************************************************
*/
static void LogChangeFile(void)
{
     char acPath[256] = {0};
     
     /*��д���ļ�*/
     if(sg_LogArgs.pfFileHandle != NULL)
     {
        fflush(sg_LogArgs.pfFileHandle);
        fclose(sg_LogArgs.pfFileHandle);
     }
     /*�������ļ�*/
     sg_LogArgs.iSeq ++;
     if(sg_LogArgs.iSeq > sg_LogArgs.iMaxFileCount)
     {
        sg_LogArgs.iSeq = 1;
     }
     sprintf(acPath,"%s%s%02d.log",sg_LogArgs.pcFilePath,sg_LogArgs.pcFilePrefix,sg_LogArgs.iSeq);
     unlink(acPath);
     sg_LogArgs.pfFileHandle = fopen(acPath, "a+");
     sg_LogArgs.iSize = 0;
     //printf("create new file:%s\n",acPath);
     LogSetSeqID();
}

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
             int iLevel,int iDirect)
{
    char        acPath[128];
    if(pcFilePath != NULL)
    {
        strncpy(sg_LogArgs.pcFilePath,pcFilePath,256);
    }
    else
    {
        return 1;
    }
    /*Ŀ¼���棬�򴴽�*/
    if (access(pcFilePath,F_OK) < 0)
    {
    	mkdir(pcFilePath,0777);
    }
    /*��Ŀ¼������/*/
     if(sg_LogArgs.pcFilePath[strlen(sg_LogArgs.pcFilePath) -1] != '/')
    {
        strcat(sg_LogArgs.pcFilePath,"/");
    }
    
    if(pcFilePrefix != NULL)
    {
        strncpy(sg_LogArgs.pcFilePrefix,pcFilePrefix,10);
    }
    else
    {
        strcpy(sg_LogArgs.pcFilePrefix,"xx_log_");
    }
    /*�����ļ���ӡ����*/
    sg_LogArgs.iLevel = iLevel%LOG_LAST;
    /*�����ļ���־�������*/
    sg_LogArgs.iDirect = !!iDirect;
     
    if(iMaxFileSize > MAX_LOG_SIZE || iMaxFileSize == 0)
    {
        iMaxFileSize = MAX_LOG_SIZE;
    }
    if(iMaxFileCount > MAX_FILE_CNT || iMaxFileCount == 0)
    {
        iMaxFileCount = MAX_FILE_CNT;
    }
    
    sg_LogArgs.iMaxFileSize = iMaxFileSize;
    sg_LogArgs.iMaxFileCount = iMaxFileCount;
    sg_LogArgs.iSeq = 1;
    pthread_mutex_init(&sg_LogArgs.sLockMutex, NULL);
    /* �õ�SEQ */
    if(LogGetSeqID())
    {
        LogSetSeqID();
    }
     /* �õ��ļ���С */
    LogGetFSize();
     
    sprintf(acPath,"%s%s%02d.log",sg_LogArgs.pcFilePath,sg_LogArgs.pcFilePrefix,sg_LogArgs.iSeq);
    //printf("get isize:%d,path:%s\n",sg_LogArgs.iSize,acPath);
    ///printf("get file:%s\n",acPath);
    if((sg_LogArgs.pfFileHandle = fopen(acPath, "a+")) == NULL)
    {
        return 1;
    }
    else
    {
        return 0;
    }
    
}

/******************************************************************************************************
* ��������:     LogDestroy�������ͷ�LOG��Դ
* �������˵��: ��
* �������˵��: ��
* ����ֵ:       ��
*******************************************************************************************************
*/
void LogDestroy(void)
{
	if(sg_LogArgs.pfFileHandle != NULL)
	{
		pthread_mutex_lock(&sg_LogArgs.sLockMutex);
		/* ����ļ��еĻ���ͼ�¼д�ļ��е�λ�� */
		fflush(sg_LogArgs.pfFileHandle);
		fclose(sg_LogArgs.pfFileHandle);
		sg_LogArgs.pfFileHandle = NULL;
		pthread_mutex_unlock(&sg_LogArgs.sLockMutex);
		pthread_mutex_destroy(&sg_LogArgs.sLockMutex);
	}
}

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
void SBS_LOG(int   iLevel,int  iLine,char *pcFile,const char *pcFunc,const char * fmt, ...)
{
    int     iCnt = 0;
    char    acTime[32];
    struct tm stTime;
    struct timeval stDetailTtime;
    FILE    *pOut = NULL;
   
    /* ��ӡ�������� */
    if(iLevel > sg_LogArgs.iLevel)
    {
        return;
    }
    
    acTime[0] = 0;
    pthread_mutex_lock(&sg_LogArgs.sLockMutex);
    
    /* �ж��ļ��Ƿ��� */
    if(sg_LogArgs.iSize >= sg_LogArgs.iMaxFileSize)
    {
        LogChangeFile();
    }
    /* �õ�ϵͳʱ�䣬��Ҫ�õ�΢��ʱ�� */
    gettimeofday(&stDetailTtime, NULL);
    localtime_r(&stDetailTtime.tv_sec, &stTime);
    /* ÿ��Log����ǰ�����ʱ�� */
    sprintf(acTime, "%02d/%02d %02d:%02d:%02d",
    	stTime.tm_mon + 1,stTime.tm_mday,stTime.tm_hour,stTime.tm_min,stTime.tm_sec);
    
    va_list vap;
    va_start(vap, fmt);
    
    if(sg_LogArgs.iDirect == LOG_TTY)
    {
        pOut = stdout;
    }
    else
    {
        pOut = sg_LogArgs.pfFileHandle;
    }
    
    if (pOut != NULL)
    {
        if (pcFile && pcFunc)
        {
            iCnt = fprintf(pOut, "[%s][%s]-[%s][%s][%d]-", \
                           sg_LogArgs.pcFilePrefix,sg_acLOG_LEV[iLevel%LOG_LAST], acTime, pcFunc, iLine);
        }
        else
        {
            /* ����ļ����ͺ�������һ��Ϊ��,�򲻽��к�,�ļ���,����������������д����־ */
            iCnt = fprintf(pOut, "[%s][%s]-[%s]-", \
                           sg_LogArgs.pcFilePrefix,sg_acLOG_LEV[iLevel%LOG_LAST], acTime);
        }
        iCnt += vfprintf(pOut, fmt, vap);
        va_end(vap);
        sg_LogArgs.iSize += iCnt;
		fflush(pOut);
    }
    pthread_mutex_unlock(&sg_LogArgs.sLockMutex);
    
}

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
void SLOG(int   iLevel,char * plog)
{
    int     iCnt = 0;
    char    acTime[32];
    struct tm stTime;
    struct timeval stDetailTtime;
    FILE    *pOut = NULL;
   
    /* ��ӡ�������� */
    if(iLevel > sg_LogArgs.iLevel || plog == NULL)
    {
        return;
    }
    
    acTime[0] = 0;
    pthread_mutex_lock(&sg_LogArgs.sLockMutex);
    
    /* �ж��ļ��Ƿ��� */
    if(sg_LogArgs.iSize >= sg_LogArgs.iMaxFileSize)
    {
        LogChangeFile();
    }
    /* �õ�ϵͳʱ�䣬��Ҫ�õ�΢��ʱ�� */
    gettimeofday(&stDetailTtime, NULL);
    localtime_r(&stDetailTtime.tv_sec, &stTime);
    /* ÿ��Log����ǰ�����ʱ�� */
    sprintf(acTime, "%02d/%02d %02d:%02d:%02d",
    	stTime.tm_mon + 1,stTime.tm_mday,stTime.tm_hour,stTime.tm_min,stTime.tm_sec);
    
    if(sg_LogArgs.iDirect == LOG_TTY)
    {
        pOut = stdout;
    }
    else
    {
        pOut = sg_LogArgs.pfFileHandle;
    }
    
    if (pOut != NULL)
    {
        /* ����ļ����ͺ�������һ��Ϊ��,�򲻽��к�,�ļ���,����������������д����־ */
        iCnt = fprintf(pOut, "[%s]-[%s]-%s", \
                       sg_acLOG_LEV[iLevel%LOG_LAST], acTime,plog);
        sg_LogArgs.iSize += iCnt;
		fflush(pOut);
    }
    pthread_mutex_unlock(&sg_LogArgs.sLockMutex);
    
}


#ifdef __cplusplus
}
#endif

