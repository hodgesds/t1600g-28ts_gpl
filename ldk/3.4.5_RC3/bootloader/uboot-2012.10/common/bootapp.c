/*! Copyright(c) 2008-2010 Shenzhen TP-LINK Technologies Co.Ltd.
 *
 *\file		bootapp.c
 *\brief		
 *\details	
 *
 *\author	Cai Peifeng
 *\version	
 *\date		02Sep10
 *
 *\warning	
 *
 *\history \arg	02Sep10, Cai Peifeng, Create the file
 */
/**********************************************************************/
/*                          CONFIGURATIONS                            */
/**********************************************************************/
/**********************************************************************/
/*                          INCLUDE_FILES                             */
/**********************************************************************/

/*from uboot*/
#include<common.h>
#include<linux/ctype.h>
#include<vsprintf.h>
#include<linux/time.h>
#include<rtc.h>
#include<jffs2/jffs2.h>

/*our files*/
#include<tplink/flashUtil.h>
#include<tplink/flashDrvLib.h>
#include<tplink/flashAppFsImage.h>
#include <tplink/flashAppInner.h>



/**********************************************************************/
/*                           DEFINES                                  */
/**********************************************************************/


#define TFTP_LOAD_ADDR		0x61000000	
//#define UBOOT_DEBUG
#define BUF_LINE			255		/*!< the max characters of a line*/
#define CMD_LEN				32

#define BOOT_ADDR_LEN		30
#define BOOT_FILE_LEN		160	/* max chars in file name */
#define BOOT_USR_LEN		20	/* max chars in user name */
#define BOOT_PASSWORD_LEN	20	/* max chars in password */		
#define MAC_ADDRESS_LENGTH	6
#define BOOT_VERSION		"1.0.0"


#define MAC_ADDRESS_LENGTH	6
#define IP_ADDRESS_LENGTH	17


/* 判断是否输入数字 */
#define IS_NOT_DIGITAL_LINE(line)    (_isDigitalString(line) == 0)    

#define ERR_NOT_DIGITAL_PRINT()      printf("Input string contains illegal characters.\r\n")

#define IS_NOT_NEW_LINE(line)		(strlen(line) != 0) 

/* 直接RAM启动配置选项 */
 #define TP_KERNEL_IMAGE_NAME   "uImage.img"
 #define TP_KERNEL_IMAGE_DL_ADDR 0x60000000
 #define TP_KERNEL_IMAGE_MAX_SIZE 8*1024*1024 /*8M*/


 #define TP_USER_IMAGE_NAME   "usrimage.jffs2"
 
 /*   
 	@def   : TP_USER_IMAGE_DL_ADDR
 	@brief : 将usrimage.jffs2下载到RAM的地址
	linux配置文件linux-3.6.5-flash.config(buildroot\board\broadcom\hurricane2目录下)的
	配置项CONFIG_TPLINK_ROOFS_LOAD_ADDR需要与该值对应，且为RAM的偏移地址:
	CONFIG_TPLINK_ROOFS_LOAD_ADDR = TP_USER_IMAGE_DL_ADDR - RAM的基地址(0x60000000)
 */
 #define TP_USER_IMAGE_DL_ADDR  0x63000000 

 /*
	@def   : TP_USER_IMAGE_MAX_SIZE
	@brief : usrimage.jffs2的最大大小
 */
 #define TP_USER_IMAGE_MAX_SIZE 10*1024*1024  /*10M*/


/**********************************************************************/
/*                            TYPES                                   */
/**********************************************************************/

typedef struct
{
	char ipAddr[BOOT_ADDR_LEN];			/*!< the ip of the switch*/
	char ipMask[BOOT_ADDR_LEN];			/*!< the mask of the switch*/
	char routeAddr[BOOT_ADDR_LEN];		/*!< the route of the switch*/
	char hostAddr[BOOT_ADDR_LEN];		/*!< the tftp host address*/
	char fileName[BOOT_FILE_LEN];		/*!< the file that to be download*/
	int flags;							/*!< the current mode*/
}BOOT_CONFIG;

typedef struct _cmdStruct
{
	bool	(*cmdFunc)();				/*命令回调函数*/
	char*	description;				/*功能描述*/
}cmdStruct;
/**********************************************************************/
/*                            FUNCTIONS                               */
/**********************************************************************/

void printBootromLogo();
bool cmdHelp(void);
bool cmdReboot(void);
bool cmdReset(void);
bool cmdStart(void);
bool cmdActivateBackupImage(void);
bool cmdShowImage(void);


/**********************************************************************/
/*                            VARIABLES                               */
/**********************************************************************/

/*!
  var				bootCommand
   \brief			the comand list
   \description	
   \range		
   \usage		
*/
cmdStruct bootCommand[] = 
{
    {cmdHelp,					"- Print this boot menu"},
    {cmdReboot, 				"- Reboot"},
    {cmdReset,					"- Reset"},
    {cmdStart,					"- Start"},
    {cmdActivateBackupImage,	"- Activate Backup Image"},
    {cmdShowImage,              "- Display image(s) info"},	
	{NULL,NULL}
};


static char* imageName[2] = 
{
	"image1.bin",
	"image2.bin"	
};	


/**********************************************************************/
/*                            EXTERN_PROTOTYPES                       */
/**********************************************************************/

extern int readline_into_buffer(const char *const prompt, 
								char *buffer, int timeout);


extern IPaddr_t string_to_ip(const char *s);

extern void ip_to_string(IPaddr_t x, char *s);

extern void to_tm(int tim, struct rtc_time * tm);
/**********************************************************************/
/*                            LOCAL_PROTOTYPES                        */
/**********************************************************************/
/**********************************************************************/
/*                            LOCAL_FUNCTIONS                         */
/**********************************************************************/

#define is_digit(c)	((c) >= '0' && (c) <= '9')
static int skip_atoi(const char **s)
{
	int i=0;

	while (is_digit(**s))
		i = i*10 + *((*s)++) - '0';
	return i;
}



static int _isDigitalString(const char* str)
{
    int len = 0;
    char *temp = NULL;

    if ((str == NULL) ||
       ((len = strlen(str)) == 0))
    {
        return 0;
    }

    temp = str;
    while (*temp != '\0')
    {
        if (((*temp) < '0') ||
           ((*temp) > '9'))
        {
            return 0;
        }
        temp++;
    }
    return 1;
}

/******************************************************************************
*
*
* skipSpace - advance pointer past white space
*
* Increments the string pointer passed as a parameter to the next
* non-white-space character in the string.
*/
void skipSpace(char **strptr)
{
	char *pLine;
	int len;
	
	if( NULL == strptr || NULL == *strptr)
	{
		return;
	}
	
	/*skip the last space*/
	len = strlen( *strptr);

	pLine = *strptr + len - 1;

	while( pLine >= *strptr && (*pLine == ' ') )
	{
		*pLine = EOS;
		pLine --;
	}

	/*skip the first space*/
	while( isspace( (UINT32) ** strptr) )
		++ *strptr;	
}

/**************************************************************
* Function	: swChkDotIpAddr()
* Abstract	: To check if the input dot IP is legal
* Input		: strIp - IP address in dot format
* Output	: N/A
* Return	: TRUE - legal, FALSE - illegal
*/
static bool swChkDotIpAddr(unsigned char *strIp)
{
	UINT8*  pt = strIp;
	UINT16	 ipFirst=0;
	UINT16	 nTemp = 0;
	UINT16	 nPosition = 0;

	if(pt == NULL)
	{
		return FALSE;
	}

	if (strlen(strIp) < 7 || strlen(strIp) > 15)
	{
		return FALSE;
	}

	while(*pt != '\0')
	{
		switch(*pt)
		{
			case '0':		 case '1':
			case '2':		 case '3':
			case '4':		 case '5':
			case '6':		 case '7':
			case '8':		 case '9':

			nTemp = nTemp * 10 + *pt - '0';
			if(nTemp > 255)
			{
				return FALSE;
			}
			break;

			case '.':
			if(*(pt + 1) == '.')
			{
				return FALSE;
			}

			if(nPosition==0)
			{
				ipFirst = nTemp;
			}
			
			nPosition ++;
			nTemp = 0;
			
			break;

			default:
				return FALSE;
		}
		pt++;
	}

	if(nPosition != 3)
	{
		return FALSE;
	}

	return TRUE;
}

 /**************************************************************
 * Function	: swChkLegalIpAddr()
 * Abstract : To check if the input value of IP address is normal IP address
 * Input  	: ulIp - (This IP must be in big-endian format)
 * Output	: N/A
 * Return	: TRUE - Legal, FALSE - Illegal
 */
static bool swChkLegalIpAddr(UINT32 ulIp) 
{
	UINT32 ipAddr = ulIp;	
	/* The hostid of IP address can not be all 0 or all 1*/
	/*modify for A/B/C type ip address check together
	*/
	if ((ipAddr & 0xF0000000) >= 0xe0000000 
     	|| (ipAddr & 0xFF000000) == 0 /*D类(1110) E类(1111)*/
		|| ipAddr == 0x7F000001) /*环回,127.0.0.1*/
	{
		printf("Ip check error!\n\r");
		return FALSE;
	}

	if( (ipAddr & 0x000000FF) == 0xFF 
		|| (ipAddr & 0x000000FF) == 0x00)
	{
		return FALSE;
	}
		
	return TRUE;
}


 /**************************************************************
 * Function	: maskCheck()
 * Abstract : To check if the input value of MASK is legal
 * Input  	: ulMask
 * Output	: N/A
 * Return	: TRUE - legal, FALSE - illegal
 */
static bool maskCheck(char* param, UINT32 *mask)
{
	int i;
	UINT32 ulTmp,ulCmp ;
	UINT32 ulMask;

	ulCmp = (UINT32)0xFFFFFFFF;
	ulTmp = (UINT32)0x00000001;


	if( NULL == param)
	{
		return FALSE;
	}

	if( FALSE == swChkDotIpAddr(param) )
	{
		return FALSE;
	}

	ulMask = (UINT32)string_to_ip(param);

	ulMask = ntohl(ulMask);
	
	if (ulMask == 0x00)
	{
		return FALSE;
	}

	for (i = 0; i < 32; i++)
	{
		if ((ulTmp & ulMask) != 0x00)
		{
			if(ulMask == (ulCmp << i))
			{
				*mask = ulMask;
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
		else
		{
			ulTmp = ulTmp << 1;
		}
	}
	return FALSE;	
}

/*!
 *\fn			bool ipCheck(char *param)
 *\brief   		check the ip address valid or not
 *
 *\param[in]   	param	the dot format address
 *\param[out]  	N/A
 *
 *\return  		TRUE	valid
 				FALSE 	invalid
 *\retval  
 *
 *\note		  
 */
static bool ipCheck(char *param)
{
    UINT32 ulMask;
	
	if( NULL == param)
	{
		return FALSE;
	}

	if(FALSE == swChkDotIpAddr(param) )
	{
		return FALSE;
	}

    ulMask = (UINT32)string_to_ip(param);

	ulMask = ntohl(ulMask);
	
	if(FALSE == swChkLegalIpAddr(ulMask))
	{
		return FALSE;
	}

	return TRUE;
}



/*!
 *\fn			bool cmdReadLine(char *param)
 *\brief   		read a line
 *
 *\param[in]   
 *\param[out]  
 *
 *\return  
 *\retval  
 *
 *\note		  
 */
static bool cmdReadLine(char *param, const char *const prompt)
{
	int len = 0;

	if( NULL == param)
	{
		return FALSE;
	}
	
	/*read string*/
	len = readline_into_buffer(prompt, param, 0);

	if( 0 == len)
	{
		return FALSE;
	}
	else
	{
		param[len] = '\0';
	}

	/*turn the line feed to end*/
	if(param[strlen(param)-1] == 0xa)
	{
		param[strlen(param)-1] = EOS;
	}

	return TRUE;
}


/*!
   \function	cmdGetName(char *param,char *pName)	
   \brief		
   \descrition	get the commander name of the string.
   \param[in]	param:the string that to be parsse.
   \param[out]	pName:store the commander name.
   \return		TRUE:success;FALSE:some error has occur.
   \calls		
   \others		
*/
static bool cmdGetName(char *param, char *pName)
{
	int len;
	int i;
	
	if(NULL == param || NULL == pName)
	{
		return FALSE;
	}

	len = strlen(param);
	
	if( 0 == len)
	{
		return FALSE;
	}
	
	for(i=0; i < len; i++)
	{
		if( i > CMD_LEN)
		{
			return FALSE;
		}
		if(isspace((int)param[i]) )
		{
			break;
		}
	}
	
	strncpy(pName,param,i);
	pName[i] = '\0';
	return TRUE;	
}

/*!
   \function	cmdGetNextArg(char **param,char *pName,int *pEnd)	
   \brief		get the next argument
   \descrition	
   \param[in]	param:the string that to be parse
   \param[out]	pName:store the next argument
   \param[out]  pEnd:the param has the next argument or not.
   				FALSE:has the next argument;TRUE:not has the next argument.
   \return		FALSE:some error has occur.TRUE:success
   \calls		
   \others		
*/
static int cmdGetNextArg(char **param, char *pName, int *pEnd)
{
	int len;
	int i;
	char *pFirst;

	if( NULL == param || NULL == *param || NULL == pEnd)
	{
		return FALSE;
	}
	
	skipSpace(param);

	if (**param == EOS)
	{
		*pEnd = TRUE;
		return TRUE;
	}
	
	len = strlen(*param);
	
	pFirst = *param;
	
	for(i=0; i < len; i++)
	{
		if( i > BUF_LINE)
		{
			printf("The param length must less than 255.\r\n");
			*pEnd = FALSE;
			return FALSE;
		}
		if(isspace((int)**param))
		{
			break;
		}
		(*param)++;					
	}

	if( NULL != pName)
	{
		strncpy(pName,pFirst,i);
		pName[i] = '\0';
	}

	*pEnd = FALSE;
	
	return TRUE;	
}

/*!
 *\fn			void cmdParseCmd(char *param)
 *\brief   		parse the command and run the comand
 *
 *\param[in]   
 *\param[out]  
 *
 *\return  
 *\retval  
 *
 *\note		  
 */
void cmdParseCmd(char *param)
{
	
    int index = 0;
    int cmdNumber = 0;
    bool rc = TRUE;

    if (NULL == param)
    {
        return FALSE;
    }

    if (strlen(param) == 0)
    {
        return FALSE;
    }

    if (strncmp(param, "?", BUF_LINE) == 0)
    {
        cmdHelp();
        return TRUE;
    }

    cmdNumber = sizeof(bootCommand) / sizeof(cmdStruct) - 1;

    if (IS_NOT_DIGITAL_LINE(param))
    {
        ERR_NOT_DIGITAL_PRINT();
        return FALSE;
	}

    index = simple_strtoul(param, NULL, 10);

	if (index < 0 || index >= cmdNumber)
	{
		printf("Invalid boot option.\r\n");
	}	
    if ((index >= 0) && (index < cmdNumber) && bootCommand[index].cmdFunc != NULL)
    {
        rc = bootCommand[index].cmdFunc();
    }
    return rc;	

}




/*!
 *\fn			bool ftpGetImage(int fSocket,char **pData,int *fileLength, int maxSize)
 *\brief   
 *
 *\param[in]   
 *\param[out]  
 *
 *\return  
 *\retval  
 *
 *\note		  
 */
static bool ftpGetImage(char **pData, char* pFileName,int *fileLength, int maxSize)
{
	char *pBuf;				
    char command[CONFIG_SYS_CBSIZE] = {0};
    int flag = 0;

    if (NULL == pFileName || NULL == fileLength)
    {
        return FALSE;
    }    
   

    pBuf = TFTP_LOAD_ADDR;
	
    sprintf(command, "tftpboot 0x%0x %s 0x%0x 0x%0x", pBuf, pFileName, fileLength, maxSize);
    run_command(command, flag);

	*pData = pBuf;
	return TRUE;

}

static void sysReboot(void)
{
    char command[CONFIG_SYS_CBSIZE] = {0};
    int flag = 0; 
    sprintf(command, "reset");
    run_command(command, flag); 
}



/*!
   \function	helpIfconfig()
   \brief		show the ifconfig help.
   \descrition
   \param[in]	N/A
   \param[out]	N/A
   \return
   \calls
   \others
*/
static void helpIfconfig()
{
	printf("Usage (ifConfig): Parameters: [[<option> <value> ...]]\r\n");
	printf("With no parameters, all current interfaces are listed.\r\n");
	printf("Value options are:\r\n");
	printf("\tip <address>		- IP address associated with interface\r\n");
	printf("\tmask <value>		- Subnet mask associated with interface\r\n");
	printf("\tgateway <adress>	- Default gateway\r\n");
}

static image_header_t *image_get_kernel(ulong img_addr, int verify)
{
	image_header_t *hdr = (image_header_t *)img_addr;

	if (!image_check_magic(hdr)) {
		/*puts("Bad Magic Number\n");*/
		bootstage_error(BOOTSTAGE_ID_CHECK_MAGIC);
		return NULL;
	}
	bootstage_mark(BOOTSTAGE_ID_CHECK_HEADER);

	if (!image_check_hcrc(hdr)) {
		/*puts("Bad Header Checksum\n");*/
		bootstage_error(BOOTSTAGE_ID_CHECK_HEADER);
		return NULL;
	}

	bootstage_mark(BOOTSTAGE_ID_CHECK_CHECKSUM);
	/*image_print_contents(hdr);*/

	/*if (verify) {
		puts("   Verifying Checksum ... ");
		if (!image_check_dcrc(hdr)) {
			printf("Bad Data CRC\n");
			bootstage_error(BOOTSTAGE_ID_CHECK_CHECKSUM);
			return NULL;
		}
		puts("OK\n");
	}*/
	bootstage_mark(BOOTSTAGE_ID_CHECK_ARCH);

	if (!image_check_target_arch(hdr)) {
		/*printf("Unsupported Architecture 0x%x\n", image_get_arch(hdr));*/
		bootstage_error(BOOTSTAGE_ID_CHECK_ARCH);
		return NULL;
	}
	return hdr;
}


static image_header_t* getImageHeadInfo(ulong img_addr, ulong* os_data, ulong* os_len)
{
	
	image_header_t	*hdr = NULL;
	
	/* copy from dataflash if needed */
	img_addr = genimg_get_image(img_addr);

	
	bootstage_mark(BOOTSTAGE_ID_CHECK_MAGIC);

	/* check image type, for FIT images get FIT kernel node */
	*os_data = *os_len = 0;
	switch (genimg_get_format((void *)img_addr)) {
		case IMAGE_FORMAT_LEGACY:
			/*printf("## Booting kernel from Legacy Image at %08lx ...\n",
					img_addr);*/
			hdr = image_get_kernel(img_addr, 0);
			if (!hdr)
				return NULL;
			bootstage_mark(BOOTSTAGE_ID_CHECK_IMAGETYPE);

			/* get os_data and os_len */
			switch (image_get_type(hdr)) {
			case IH_TYPE_KERNEL:
			case IH_TYPE_KERNEL_NOLOAD:
				*os_data = image_get_data(hdr);
				*os_len = image_get_data_size(hdr);
				break;
			case IH_TYPE_MULTI:
				image_multi_getimg(hdr, 0, os_data, os_len);
				break;
			case IH_TYPE_STANDALONE:
				*os_data = image_get_data(hdr);
				*os_len = image_get_data_size(hdr);
				break;
			default:
				/*printf("Wrong Image Type for\n");*/
				bootstage_error(BOOTSTAGE_ID_CHECK_IMAGETYPE);
				return NULL;
			}
		bootstage_mark(BOOTSTAGE_ID_DECOMP_IMAGE);
		break;
	default:
		/*printf("Wrong Image Format\n");*/
		bootstage_error(BOOTSTAGE_ID_FIT_KERNEL_INFO);
		return NULL;
	}

	return hdr;

}


static bool imageFileExist(ulong nRegion)
{
    image_header_t* phdr = NULL;
    ulong os_data = 0;
    ulong os_len = 0;
    int img_addr = 0;
    int kernel_addr = 0;
    int i = 0;
    struct jffs2_unknown_node head;
	struct jffs2_unknown_node crcnode;
	uint32_t crc;
    int nRet = ERROR;


    phdr = getImageHeadInfo(IMAGE_ADDR, &os_data, &os_len);

    if (NULL == phdr)
    {
        nRet = AutoFixKernel();
        if(ERROR == nRet)
        {
            return FALSE;
        }        
    }

    img_addr = (1 == nRegion)? (sysGetFlAddr("usrimg1")):(sysGetFlAddr("usrimg2"));
    memset(&head, 0, sizeof(head));

    l_readFlash(&head, sizeof(struct jffs2_unknown_node), img_addr);

	/*check magic number*/
	if (head.magic != JFFS2_MAGIC_BITMASK)
	{
		//printf("wrong magic number %d\n", head.magic);
		return FALSE;
	}	
	
	/* OK, now check for node validity and CRC */
	crcnode.magic = JFFS2_MAGIC_BITMASK;
	crcnode.nodetype = head.nodetype;
	crcnode.totlen = head.totlen;
	crc = crc32_no_comp(0, (uchar *)&crcnode, sizeof(crcnode)-4);

	if (head.hdr_crc != crc) {
		//printf("head.hdr %d, crc %d\n", head.hdr_crc, crc);
		return FALSE;
	}


    return TRUE;
    
}


int getStartUpIndex()
{
    char value[ENV_ITEM_VALUE_LEN] = {0};

    getEnvItem(TP_SWITCH_START_IMAGE, value);
	
    /*
    char * pArea = NULL;
    
    pArea = getenv("bootarea");

    if((pArea[0] != '1') && (pArea[0] != '2'))
    {
        return -1;
    }
	
    return pArea[0] - '1' + 1;
    */
    if(strcmp(value, TP_SWITCH_IMAGE_NAME_1) == 0)
    {
        return 1;
    }
    else if(strcmp(value, TP_SWITCH_IMAGE_NAME_2) == 0)
    {
        return 2;
    }
    else
    {
        return 1;//默认1区启动
    }
}

void setStartUpIndex(int index)
{
    char tmp[BUF_LINE] = {0};

    if (1 != index && 2 != index)
    {
    return;
    }

    if(1 == index)
    {
        setEnvItem(TP_SWITCH_START_IMAGE, TP_SWITCH_IMAGE_NAME_1);
        setEnvItem(TP_SWITCH_BACKUP_IMAGE, TP_SWITCH_IMAGE_NAME_2);
    }
    else
    {
        setEnvItem(TP_SWITCH_START_IMAGE, TP_SWITCH_IMAGE_NAME_2);
        setEnvItem(TP_SWITCH_BACKUP_IMAGE, TP_SWITCH_IMAGE_NAME_1);
    }

    sysSetActiveInfo(index);
    sprintf(tmp, "%d", index);
    setenv("bootarea", tmp);	
    saveenv();
 
}	


BOOL resaveStartupIndex()
{
    char line[BUF_LINE] = {0};
    ulong img_addr = 0;

    if(1 == getStartUpIndex())
    {
        setStartUpIndex(1);
    }
    else
    {
        setStartUpIndex(2);
    }
    

    return TRUE;
}

	
/**********************************************************************/
/*                            PUBLIC_FUNCTIONS                        */
/**********************************************************************/



//extern int rtc_get(struct rtc_time *tmp);

/*!
 *\fn			void printBootromLogo()
 *\brief   		printf bootrom logo
 *
 *\param[in]   	N/A
 *\param[out]  	N/A
 *
 *\return  		N/A
 *\retval  		N/A
 *
 *\note		  
 */
void printBootromLogo()
{
   
    char buf[100];

	char* time = __DATE__;

	int year = 0;

	int i = 0;


	for (i = strlen(time) - 1; i > 0 ; i--)
	{
		if (time[i] == 32)
		{
			break;
		}	
	}	

	year = simple_strtoul(&time[i + 1], NULL, 10);

    sprintf(buf, "Create Date: %s - %s\n", __DATE__, __TIME__);

    printf("\t\t%s","*********************************************\n");
    printf("\t\t%s%s%s","*         TP-LINK  BOOTUTIL(v",BOOT_VERSION,")         *\n");
    printf("\t\t%s","*********************************************\n");
    printf("\t\t%s%d%s","Copyright (c) ", year, " TP-LINK Tech. Co., Ltd    \n");
    printf("\t\t%s\r\n",buf);


}  


/*!
 *\fn			bool cmdHelp(char *param)
 *\brief   
 *
 *\param[in]   
 *\param[out]  
 *
 *\return  
 *\retval  
 *
 *\note		  
 */
 
bool cmdHelp(void)
{
	int i = 0;

	int cmdNumber = sizeof(bootCommand) / sizeof(cmdStruct) - 1;
	
	
	printf("   Boot Menu\r\n");
	
	for (i = 0; i < cmdNumber; i++)
	{
		printf("%-2d %s\n", i, bootCommand[i].description);
	}
    
	printf("\r\nEnter your choice(0-%d)\r\n", cmdNumber-1);
	
	printf ("\n");

	return TRUE;
}


bool cmdReboot(void)
{
    char line[BUF_LINE] = {0};

    printf("Are you sure to reboot the device?[Y/N]:");
    cmdReadLine(line, NULL);
	
    if (tolower(line[0]) != 'y')
    {
        return TRUE;
    }

    //printf("Rebooting...\r\n");
    mdelay(1000);    /* pause a second */
    sysReboot();
    return TRUE;
}


bool cmdReset(void)
{
    char line[BUF_LINE] = {0};

    printf("Are you sure to reset the device?[Y/N]:");
  	cmdReadLine(line, NULL);

    if (tolower(line[0]) != 'y')
    {
        return TRUE;
    }
    setEnvItem(TP_SWITCH_START_CONFIG, "NULL");
    //printf("Rebooting...\r\n");
    mdelay(1000);    /* pause a second */
    sysReboot();
    
    return TRUE;
}



bool cmdStart(void)
{
    ulong img_addr = 0;
    char command[CONFIG_SYS_CBSIZE];
    char value[ENV_ITEM_VALUE_LEN] = {0};
    char tmp[BUF_LINE] = {0};
    int nRet = 0;
    
    flashAppInit();
    getEnvItem(TP_SWITCH_START_CONFIG, value);
    if(strcmp(value, "NULL") == 0)
    {
        char * configarea = NULL;
        configarea = getenv("config_area");
        if(strcmp(configarea, "null") != 0)
        {
            setenv("config_area", "null");
            saveenv();           
        }
         setEnvItem(TP_SWITCH_START_CONFIG, TP_SWITCH_CONFIG_NAME_1);
    }
    else
    {
        char * configarea = NULL;
        configarea = getenv("config_area");
        if(strcmp(configarea, TP_SWITCH_CONFIG_NAME_1) != 0)
        {
            setenv("config_area", TP_SWITCH_CONFIG_NAME_1);
            saveenv();
        }
    }  

    if (FALSE == imageFileExist(getStartUpIndex()))
    {
        printf("The Startup Image does not exist\r\n");
        //return FALSE;
    }

    /*启动前需要检查更新启动区域*/
    int index = getStartUpIndex();
    int curArea = 1;
    char * pArea = NULL;
    pArea = getenv("bootarea");

    curArea = pArea[0] - '1' + 1;

    if(curArea != index)
    {
        //sysSetActiveInfo(index);
        sysSetActiveInfoWithOutFlash(index);/*去掉不必要的save操作*/
        sprintf(tmp, "%d", index);
        setenv("bootarea", tmp);	
        //saveenv();/*去掉不必要的save操作*/
    }

    printf("\nBegin to startup system, please wait a moment...\n");

    memset(command, 0, sizeof(command));
    sprintf(command, "bootm 0x%x", IMAGE_ADDR);
    nRet = run_command(command, 0);
    if(1 == nRet )
    {
        nRet = AutoFixKernel();
        if(ERROR == nRet)
        {
            return FALSE;
        }
    }

    return TRUE;
}



bool cmdActivateBackupImage(void)
{
    char line[BUF_LINE] = {0};
    ulong nRegion = 1;

    	
    if (1 == getStartUpIndex())
    {
    	    nRegion = 2;
    }
    else
    {
    	    nRegion = 1;
    	
    }	

    if (FALSE == imageFileExist(nRegion))
    {
        	printf("The Backup Image does not exist in system\r\n");
        	return FALSE;
    }
    
    /* 设置全局变量 */
    printf("Are you sure to set the Backup Image as Startup Image?[Y/N]:");
	line[0] = 0;
    cmdReadLine(line, NULL); 
    if (tolower(line[0]) == 'y')
    {
		if(1 == getStartUpIndex())
		{
			setStartUpIndex(2);
		}
		else
		{
			setStartUpIndex(1);
		}
				 
        /* 显示文件列表 */
        cmdShowImage();
		
    }
    printf("\r\n");
    return TRUE;
}


bool cmdShowImage(void)
{ 
	image_header_t* phdr = NULL;
	int index = 0;

	ulong img_len = 0;
	uint32_t time = 0;
       char cImgName[16] = {0};
       char * pImgName = "NULL";

	char* startUpLabel = "(*)"; 
	char* backUpLabel = "(b)";

	printf("Images in system:\r\n");
	printf("index  Attribute  Size        Filename\r\n");
	printf("-----  ---------  ----------  ---------------------\r\n");

	for (index = 0; index < sizeof(imageName)/ sizeof(char*); index ++)
	{
            char* label = NULL;
            if (FALSE == imageFileExist(index + 1))
            {
                img_len = 0;
                pImgName = "NULL";
            }
            else
            {
                sprintf(cImgName, "usrimg%d", (index+1));  
                img_len = sysGetMtdpartsInfo(cImgName);
                pImgName = imageName[index];
            }
            if (index + 1 == getStartUpIndex())
            {
                label = startUpLabel;
            }
            else
            {
                label = backUpLabel;
            }	

            printf("%-5d  %-9s  %-10d  %-21s\r\n",
								index + 1, label, img_len,
								pImgName);
	}
	
    printf("-----  ---------  ----------  ---------------------\r\n");
    printf("\r\n");
    printf("(*) - with the Startup attribute\r\n");
    printf("(b) - with the Backup attribute\r\n");	
	return TRUE;    
}

/**********************************************************************/
/*                            GLOBAL_FUNCTIONS                        */
/**********************************************************************/

void menu_shell()
{
    char readBuf[BUF_LINE];
    int status;
    char *pLine;

    cmdHelp();

#define CONFIG_SYS_PROMPT	"tplink> "  

    while(1)
    {	
        /*!< read the input command (add by Cai Peifeng:2010-9-3 10:35:16)*/
        readBuf[0] = 0;
        status = cmdReadLine(readBuf, CONFIG_SYS_PROMPT);

        if( FALSE == status)
        {
            continue;
        }
        /*skip the space*/
        pLine = readBuf;
        skipSpace(&pLine);

        /*handle the command*/
        cmdParseCmd(pLine);
    }

}



int abortboot(int bootdelay)
{
	int abort = 0;


	if (bootdelay >= 0)
		printf("\nHit any key to stop autoboot: %2d ", bootdelay);


	while ((bootdelay > 0) && (!abort)) {
		int i;

		--bootdelay;
		/* delay 100 * 10ms */
		for (i=0; !abort && i<100; ++i) {
			if (tstc()) {	/* we got a key press	*/
				abort  = 1;	/* don't auto boot	*/
				bootdelay = 0;	/* no more delay	*/

				(void) getc();  /* consume input	*/

				break;
			}
			udelay(10000);
		}

		printf("\b\b\b%2d ", bootdelay);
	}

	putc('\n');

	return abort;
}

void AutoStart()
{
    if(!abortboot(1))
    {
        cmdStart();
    }
}


int AutoFixKernel()
{
    int nRet = OK;
    char *buffer = NULL;
    int maxSize = FLASH_SIZE;
    int recSize = 0;
    char downFile[BUF_LINE] = {0};
    char command[CONFIG_SYS_CBSIZE];

    printf("The kernel has been damaged!\nbegin to fix kernel...\n");
    printf("Please put the uimage.img into the tftpserver directory\n");
    printf("You can press CTRL-C to stop loading\r\n");

    sprintf(downFile, "%s:uImage.img", getenv("serverip"));

    /* 获取文件 */
    if( FALSE == ftpGetImage(&buffer, downFile, &recSize, maxSize))
    {
        printf("Download failed\r\n");
        return ERROR;
    }

    if (0 == recSize)
    {
        printf("Download failed\r\n");
        return ERROR;		
    }	

    nRet = sysFixKernel(buffer, recSize);
    if(ERROR == nRet)
    {
        printf("the kernel fixing is fail\n");
        return ERROR;
    }

    printf("The kernel has been repaired, begin to startup.\n");

    memset(command, 0, sizeof(command));
    sprintf(command, "bootm 0x%x", IMAGE_ADDR);
    nRet = run_command(command, 0);
    return OK;
}


