#include "libsocket.h"
#include <getopt.h>
#include <string.h>

#define CLIENT_VERSION "0.1.0 20140723"

//∞Ê±æœ‘ æ
static struct option sg_opts[] = {
    { "version",  0, NULL, 'v' },
    { "help",     0, NULL, 'h' },
    { "ftp",      0, NULL, 'f' },
    { "status",   0, NULL, 's' },
    { "reboot",   0, NULL, 'r' },
    { "test",     0, NULL, 't' },
    { 0, 0, NULL, 0}
};

void version(const char *ver)
{
    printf("client version %s\n", ver);
}

void usage( const char *ver)
{
    printf("  -v, --version    get capture version\n");
    printf("  -h, --help       get help\n");
    printf("  -f, --ftp        ftp test\n");
    printf("  -s, --status     get meter status\n");
    printf("  -r, --reboot     send reboot command to server\n");
    printf("  -t, --test       just for test\n");
}

int get_options(int argc, char **argv, const char *ver)
{
    int c,i,ret=0;
    /* parse cmdline - overrides */
    while (1) {
        c = getopt_long(argc, argv, "vhfsrt", sg_opts, &i);
        if (c == -1)
            break;
        switch (c) {
        case 'v':
            version(ver);
            ret = 1;
            break;
        case 'h':
            usage(ver);
            ret = 1;
            break;
        case 'f':
        {
            SBS_Init();

            int result = -1;
            char ftp_status[1024]={0};	
            ret = SBS_GetFtpTestResult((char **)&ftp_status,1024);
            printf("%s\n", ftp_status);
    
            SBS_Close();
        }
            break;
        case 's':
        {
            SBS_Init();

            char meter_status[1024]={0};	
    
            ret = SBS_GetMeterStatus((char**)&meter_status, 1024);
            //DEBUG("SBS_GetMeterStatus ret = %d\n",ret);
             
            printf("%s\n", meter_status);
            
            SBS_Close();
        }
            break;
        case 'r':
        {
            SBS_Init();

            ret = SBS_CaptureReboot();
            if (0 == ret)   //send success
                printf("1\n");
            else            //send failed
                printf("0\n");
    
            SBS_Close();
        }
            break;
        case 't':
        {
            SBS_Init();

            char meter_info[1024]={0};	    
            ret = SBS_GetMeterInfo((char**)&meter_info, 1024);
            //DEBUG("SBS_GetMeterInfo ret = %d\n",ret);
            printf("%s\n",meter_info);
    
            SBS_Close();
        }
            break;
        default:
            break;
        }
    }
    return ret;
}

int main(int argc,char **argv) 
{
    if (get_options(argc, argv, CLIENT_VERSION))
	{
		return 0;
	}

#if 0    
	DEBUG("*****************client********************\n");

    DEBUG("start init\n");
	int ret = SBS_Init();
	if (ret == -1)
		DEBUG("sbs_init failed\n");
	else
	    DEBUG("sbs_init success\n");



    ret = SBS_CaptureReboot();
    DEBUG("SBS_CaptureReboot ret = %d\n",ret);

    char meter_status[64]={0};	
    
    ret = SBS_GetMeterStatus((char**)&meter_status, 64);
    DEBUG("SBS_GetMeterStatus ret = %d\n",ret);
    int i;
    for(i=0; i<ret; i++)	
    {       
        DEBUG("meter_status[%d] = %c\n",i,(unsigned char)meter_status[i]);  
    }   
    DEBUG("\n");

    int result = -1;
    ret = SBS_GetFtpTestResult(&result);
    DEBUG("SBS_GetFtpTestResult ret = %d result = %d\n", ret, result);

    char meter_info[1024]={0};	    
    ret = SBS_GetMeterInfo((char**)&meter_info, 1024);
    DEBUG("SBS_GetMeterInfo ret = %d\n",ret);
    DEBUG("meter info: %s\n",meter_info);

    DEBUG("start close\n");
	ret = SBS_Close();
	if (ret != 0)
		DEBUG("sbs_UnInit failed\n");
	else
	    DEBUG("sbs close success\n");
#endif

	return 1;
}





