/******************************************************************************
*
* Copyright (c) 2010 TP-LINK Technologies CO.,LTD.
* All rights reserved.
*
* file name  :   flashApp.c
* version    :   V1.0
* description:   FLASH���ֺ�Ӧ�ýӿ�.
*
* author     :   LiuZenglin <LiuZenglin@tp-link.net>
* data       :   06/20/2013
*
* history    :
* 01   06/20/2013  LiuZenglin     Create.
*
* FLASH���ַ�Ϊ
*      BOOTROM��
*      �ļ�ϵͳ��
*      �ڲ���
******************************************************************************/
#include<common.h>
#include<vsprintf.h>
#include <tplink/flashDrvLib.h>
#include <tplink/flashApp.h>
#include <tplink/flashAppInnerGev.h>
#include <tplink/flashUtil.h>


int flash_app_debug = 0;

#define DBG(arg...) do{if(0){printf("%s,%d:",__FUNCTION__, __LINE__);printf(arg);}}while(0)

#define IF_FAIL_RET(expression) \
	do{\
		int __rv;\
		__rv = (expression);\
		if ( OK != __rv ){\
			DBG("E - %d %s line %d\n",__rv,#expression,__LINE__);\
			return __rv;\
		}\
	}while(0)

#define IF_FAIL_DONE(ret,expression) \
	do{\
		ret = (expression);\
		if ( OK != ret ){\
			DBG("E - %d %s line %d\n",ret,#expression,__LINE__);\
			goto done;\
		}\
	}while(0)
        
#define IF_FAIL_PRINT(expression) \
            do{\
                int __rv;\
                __rv = (expression);\
                if ( OK != __rv ){\
                    DBG("E - %d %s line %d\n",__rv,#expression,__LINE__);\
                }\
            }while(0)

/* Flash���֣���ȡ����������ȡ by liuzenglin, 21Jun13 */
typedef struct _FLASH_DISTRIBUTE_INFO
{
    int  valid;
    int  flashVer;
    int  flashBootromSize;
    int  flashKernelOffset;
    int  flashKernelSize;
    int  flashUsrImg1Offset;
    int  flashUsrImg1Size;
    int  flashUsrImg2Offset;
    int  flashUsrImg2Size;
    int  flashUsrAppOffset;
    int  flashUsrAppSize;
    
}FLASH_DISTRIBUTE_INFO;

/* Ĭ��ֵΪV1.0.0�汾 */
LOCAL FLASH_DISTRIBUTE_INFO dftFlashDistributeInfo = {TRUE, 0x10000, 0x100000, 0x100000, 0x600000, 0x700000, 0x900000, 0x1000000, 0x900000,0x1900000, 0x60000};
/* ���浱ǰϵͳ��FLASH�����ź� */
LOCAL FLASH_DISTRIBUTE_INFO flashDistributeInfo = {FALSE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

//extern SEM_ID gFlashSemId;

/* BOOTROM����С */
#define FLASH_AREA_BOOTROM_SIZE     (flashDistributeInfo.valid ? flashDistributeInfo.flashBootromSize : -1)
/* �ļ�ϵͳƫ�Ƶ�ַ */
#define FLASH_AREA_FS_OFFSET        (flashDistributeInfo.valid ? flashDistributeInfo.flashFsOffset : -1)
/* �ļ�ϵͳ��С */
#define FLASH_AREA_FS_SIZE          (flashDistributeInfo.valid ? flashDistributeInfo.flashFsSize : -1)

/**************************************************************************************************/
/*                                      VARIABLES                                                 */
/**************************************************************************************************/
/**************************************************************************************************/
/*                                      LOCAL_FUNCTIONS                                           */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                      PUBLIC_FUNCTIONS                                          */
/**************************************************************************************************/
LOCAL int flashAppInited = FALSE;
/* ��TFFS��ʼ��֮ǰ���ã����ڳ�ʼ������ */
int   flashAppInit(void)
{
    int flashSize = 0;
    int flashVer = 0;
    int bootSize = 0;
    int fsOffset = 0;
    int fsSize   = 0;
    int kerneloffset = 0;
    int kernelsize = 0;
    int usrimg1offset  = 0;
    int usrimg1size = 0;
    int usrimg2offset  = 0;
    int usrimg2size = 0;
    int usrappoffset = 0;
    int usrappsize = 0;
    int bootSN = 0;
    int fsSN = 0;
    int innerSN = 0;
    int innerEN = 0;
    char value[ENV_ITEM_VALUE_LEN] = {0};

	if (TRUE == flashAppInited)
	{
    	return OK;
	}	

    /* �ȳ�ʼ������ */
    if (ERROR == flashDrvLibInit())
    {
        DBG("fatal error: Failed to init flash driver and app.\r\n");
        return ERROR;
    }

		

    /* ��ȡFLASH����, ��ʮ�����Ƶ��ַ����洢 */
    IF_FAIL_PRINT(getEnvItem(TP_SWITCH_FL_VER, value));
    flashVer = simple_strtoul(value, NULL, 16);
    IF_FAIL_PRINT(getEnvItem(TP_SWITCH_FL_SIZE_BOOT, value));
    bootSize = simple_strtoul(value, NULL, 16);
    IF_FAIL_PRINT(getEnvItem(TP_SWITCH_FL_OFFSET_KERNEL, value));
    kerneloffset = simple_strtoul(value, NULL, 16);
    IF_FAIL_PRINT(getEnvItem(TP_SWITCH_FL_SIZE_KERNEL, value));
    kernelsize = simple_strtoul(value, NULL, 16);
    IF_FAIL_PRINT(getEnvItem(TP_SWITCH_FL_OFFSET_USRIMG1, value));
    usrimg1offset= simple_strtoul(value, NULL, 16);
    IF_FAIL_PRINT(getEnvItem(TP_SWITCH_FL_SIZE_USRIMG1, value));
    usrimg1size= simple_strtoul(value, NULL, 16);
    IF_FAIL_PRINT(getEnvItem(TP_SWITCH_FL_OFFSET_USRIMG2, value));
    usrimg2offset= simple_strtoul(value, NULL, 16);
    IF_FAIL_PRINT(getEnvItem(TP_SWITCH_FL_SIZE_USRIMG2, value));
    usrimg2size= simple_strtoul(value, NULL, 16);
    IF_FAIL_PRINT(getEnvItem(TP_SWITCH_FL_OFFSET_USRAPP, value));
    usrappoffset= simple_strtoul(value, NULL, 16);
    IF_FAIL_PRINT(getEnvItem(TP_SWITCH_FL_SIZE_USRAPP, value));
    usrappsize= simple_strtoul(value, NULL, 16);

    

    flashSize = FLASH_SIZE;

    #if 0
    DBG("Flash size %#x\r\n", flashSize);
    if (fsOffset + fsSize > flashSize)
    {
        DBG("Bad gev, fsOffset=%#x, fsSize=%#x, flashSize=%#x\r\n",
            fsOffset, fsSize, flashSize);
        while (fsOffset + fsSize > flashSize)
        {
            /* ����FLASH��СΪ��16MΪһ���ݶ� */
            fsSize -= 0x1000000;    /* �ļ�ϵͳ��С16M���ٽ��бȽ� */
        }
        DBG("Adjust fsSize to %#x\r\n", fsSize);
        memset(value, 0, ENV_ITEM_VALUE_LEN);
        sprintf(value, "%#x", fsSize);
        IF_FAIL_PRINT(setEnvItem(TP_SWITCH_FL_SIZE_FS, value));
    }
    #endif

    DBG("flashVer=%#x, bootSize=%#x, fsOffset=%#x, fsSize=%#x\r\n",
        flashVer, bootSize, fsOffset, fsSize);

    /* ��ʼ������ */
    flashDistributeInfo.flashVer = flashVer;
    flashDistributeInfo.flashBootromSize = bootSize;
    flashDistributeInfo.flashKernelOffset = kerneloffset;
    flashDistributeInfo.flashKernelSize = kernelsize;
    flashDistributeInfo.flashUsrImg1Offset = usrimg1offset;
    flashDistributeInfo.flashUsrImg1Size = usrimg1size;
    flashDistributeInfo.flashUsrImg2Offset = usrimg2offset;
    flashDistributeInfo.flashUsrImg2Size = usrimg2size;
    flashDistributeInfo.flashUsrAppOffset = usrappoffset;
    flashDistributeInfo.flashUsrAppSize = usrappsize;
    flashDistributeInfo.valid = 1;

    //bootSN = ((ROUND_SECTOR_UP(bootSize)) / FLASH_SECTOR_SIZE);
    //fsSN = ((ROUND_SECTOR_UP(fsOffset)) / FLASH_SECTOR_SIZE);
    //innerSN = ((ROUND_SECTOR_DOWN(fsOffset+fsSize)) / FLASH_SECTOR_SIZE);
    //innerEN = FLASH_SIZE_SECTORS;
    //flashSetDistribute(bootSN, fsSN, innerSN, innerEN);

    flashAppInited = TRUE;
    return OK;
}

int flashGetBootSize(void)
{
    if (flashAppInited == FALSE)
    {
        if (ERROR == flashAppInit())
        {
            DBG("Fatal error: Failed to init flash app.\r\n");
            return -1;
        }
    }

    if (flashDistributeInfo.valid == FALSE)
    {
        DBG("Bad flash distribute info getting fs offset.\r\n");
    }
    return flashDistributeInfo.flashBootromSize;
}

/* ��ȡ�ں˵�������ַ*/
int flashGetKernelOffset(void)
{
    if (flashAppInited == FALSE)
    {
        if (ERROR == flashAppInit())
        {
            DBG("Fatal error: Failed to init flash app.\r\n");
            return -1;
        }
    }

    if (flashDistributeInfo.valid == FALSE)
    {
        DBG("Bad flash distribute info getting fs offset.\r\n");
    }
    return flashDistributeInfo.flashKernelOffset;
}

/* ��ȡ�ں˵Ĵ�С */
int flashGetKernelSize(void)
{
    int flashSize = FLASH_SIZE;

    DBG("Flash size %#x\r\n", flashSize);
    if (flashAppInited == FALSE)
    {
        if (ERROR == flashAppInit())
        {
            DBG("Fatal error: Failed to init flash app.\r\n");
            return -1;
        }
    }

    if (flashDistributeInfo.valid == FALSE)
    {
        DBG("Bad flash distribute info getting fs size.\r\n");
    }

    return flashDistributeInfo.flashKernelSize;
}

/* ��ȡ�ں˵�������ַ*/
int flashGetUsrImgOffset(int nRegion)
{
    CHECKREGION(nRegion);
    if (flashAppInited == FALSE)
    {
        if (ERROR == flashAppInit())
        {
            DBG("Fatal error: Failed to init flash app.\r\n");
            return -1;
        }
    }

    if (flashDistributeInfo.valid == FALSE)
    {
        DBG("Bad flash distribute info getting fs offset.\r\n");
    }

    return (1 == nRegion)?(flashDistributeInfo.flashUsrImg1Offset):(flashDistributeInfo.flashUsrImg2Offset);
}

/* ��ȡ�ں˵Ĵ�С */
int flashGetUsrImgSize(int nRegion)
{
    int flashSize = FLASH_SIZE;

    CHECKREGION(nRegion);

    DBG("Flash size %#x\r\n", flashSize);
    if (flashAppInited == FALSE)
    {
        if (ERROR == flashAppInit())
        {
            DBG("Fatal error: Failed to init flash app.\r\n");
            return -1;
        }
    }

    if (flashDistributeInfo.valid == FALSE)
    {
        DBG("Bad flash distribute info getting fs size.\r\n");
    }

    return (1 == nRegion)?(flashDistributeInfo.flashUsrImg1Size):(flashDistributeInfo.flashUsrImg1Size);
}

/* ��ȡ�ں˵�������ַ*/
int flashGetUsrAppOffset(void)
{
    if (flashAppInited == FALSE)
    {
        if (ERROR == flashAppInit())
        {
            DBG("Fatal error: Failed to init flash app.\r\n");
            return -1;
        }
    }

    if (flashDistributeInfo.valid == FALSE)
    {
        DBG("Bad flash distribute info getting fs offset.\r\n");
    }
    return flashDistributeInfo.flashUsrAppOffset;
}

/* ��ȡ�ں˵Ĵ�С */
int flashGetUsrAppSize(void)
{
    int flashSize = FLASH_SIZE;

    DBG("Flash size %#x\r\n", flashSize);
    if (flashAppInited == FALSE)
    {
        if (ERROR == flashAppInit())
        {
            DBG("Fatal error: Failed to init flash app.\r\n");
            return -1;
        }
    }

    if (flashDistributeInfo.valid == FALSE)
    {
        DBG("Bad flash distribute info getting fs size.\r\n");
    }

    return flashDistributeInfo.flashUsrAppSize;
}

/* ��ȡFLASH���ְ汾�� */
int flashGetFlVer(void)
{
    if (flashAppInited == FALSE)
    {
        if (ERROR == flashAppInit())
        {
            DBG("Fatal error: Failed to init flash app.\r\n");
            return -1;
        }
    }

    if (flashDistributeInfo.valid == FALSE)
    {
        DBG("Bad flash distribute info getting fs size.\r\n");
        return dftFlashDistributeInfo.flashVer;
    }
    return flashDistributeInfo.flashVer;
}

/* ��ȡĬ�ϵ��ļ�ϵͳ��С�Ļ�������ֵ */
/* ��ΪT3700��16M��32M֮�֣�16MΪ0xc00000��32MΪ0x1c00000 */
int  flashGetDftGevFsSize(char* pBuf)
{
    char* dft1 = "0xc00000";
    char* dft2 = "0x1c00000";
    char* dft = NULL;
    int flashSize = FLASH_SIZE;

    if ((pBuf == NULL) || (flashSize == ERROR))
    {
        DBG("Fatal error: Bad parameter, pBuf=%#x, flashSize=%#x.\r\n",
            (int)pBuf, flashSize);
        return ERROR;
    }

    dft = (flashSize == 0x2000000) ? dft2:dft1;
    strcpy(pBuf, dft);
    return OK;
}

#if 0
extern STATUS rm(const char * fileName     /* name of file to remove */);
STATUS tpRm(const char *    fileName    /* name of file to remove */)
{
    int rc = OK;
    if (NULL == fileName)
    {
        return ERROR;
    }
    rc = rm(fileName);

    if (rc != OK)
    {
        DBG("[ERROR] Failed to remove file %s.\r\n", fileName);

    }

    return rc;
}
#endif

UINT32 writeFlash(char *source, unsigned int size, unsigned int flashOffset, int prompt)
{
    return l_writeFlash_noErase(source, size, flashOffset, prompt);
}


UINT32 readFlash(char *source, unsigned int size, unsigned int flashOffset)
{
    return l_readFlash(source, size, flashOffset);
}


STATUS writeBootrom(char * source, UINT32 size)
{
    if( NULL == source)
    {
        return ERROR;
    }

    if( ERROR == writeFlash(source, size, 0, TRUE))
    {
        DBG("write bootrom error\r\n");
        return ERROR;
    }

    return OK;
}




