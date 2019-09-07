/******************************************************************************
*
* Copyright (c) 2010 TP-LINK Technologies CO.,LTD.
* All rights reserved.
*
* file name  :   flashAppGev.c
* version    :   V1.0
* description:   ����������ؽӿ�.
*
* author     :   LiuZenglin <LiuZenglin@tp-link.net>
* data       :   06/18/2013
*
* history    :
* 01   06/18/2013  LiuZenglin     Create.
*   ����������NAME VALUE����ʽ�洢��������������128K-12B(valid-key, type, length)��
*   ��ʼֵΪȫFF���ʼ512BΪbootrom������������254B��byteͼ
*   ��������246BYTE��Ԥ���ֶΣ�ʣ��127Kȫ�����ڱ��滷��������
*   ÿ����������ռ512B������NAME��VALUE��ռ256B�������������ܹ����Ա���254������������
*
*   Ĭ�ϻ���������:
* 
��� NAME               DEFAULT VALUE                   DESCRIPTION
1	START_NO_CONFIG	    0	                            ��ʾ����ʱ�Ƿ��ȡ���ã�
                                                        ����ʵ������ʱ�������õĹ���
2	START_CONFIG	    flash:startup_config.cfg	    �������ļ�
3	BACKUP_CONFIG	    flash:backup_config.cfg	        ���������ļ�
4	START_IMAGE	        flash:image1.bin	            ������
5	BACKUP_IMAGE	    flash:image2.bin	            ���ݾ���
6	FL_VER	            0x10000                         ������ǰFlash�汾��ΪV1.0.0
7	FL_SIZE_BOOT	    0x100000	                    bootrom���Ĵ�С
8	FL_OFFSET_FS	    0x100000	                    �ļ�ϵͳ����ƫ�Ƶ�ַ
9	FL_SIZE_FS	        0x1c00000(32M) 0xc00000(16M)    �ļ�ϵͳ���Ĵ�С    (FL_SIZE-4M)
*���»�����������:
*  Ϊ��ʹ��������������Ч�ʣ����ٲ�������
*  ����byteλ��ͼ��ָʾ����254������������ʹ�����:
* 0  --- invalid
* 1  --- valid
* ff --- blank
* ����ӻ����޸Ļ��������󣬼���Ƿ���BLANK�ռ�(λͼΪ0xff)���û�����������
* sector������bootrom������512B������������������������ȫ�����³�ʼ��������λ��ͼ����
* gevSetBack()ʵ��
******************************************************************************/
#include<common.h>
#include<vsprintf.h>
#include <tplink/flashApp.h>
#include <tplink/flashAppInner.h>
#include <tplink/flashAppFsImage.h>
#include <tplink/flashAppInnerGev.h>


/* ��������������� by liuzenglin, 15Aug12 */
/* GEV�ܹ�ռ128K-12B */
/* 512B */
#define ENV_BOOTPARAM_OFFSET    0                   /* �ʼΪbootrom���� */
#define ENV_BOOTPARAM_LENGTH    (BOOT_PARAM_SIZE)
/* 500B */
#define ENV_BMP_OFFSET          (ENV_BOOTPARAM_OFFSET + ENV_BOOTPARAM_LENGTH)
#define ENV_BMP_LENGTH          254     /* �洢����254������������״̬ */
#define ENV_RESERVE_OFFSET      (ENV_BMP_OFFSET + ENV_BMP_LENGTH)
#define ENV_RESERVE_LENGTH      246     /* �����ֶΣ�������ʣ127K */
/* 127K */
#define ENV_CONTENT_OFFSET        (ENV_RESERVE_OFFSET + ENV_RESERVE_LENGTH)
//#define ENV_CONTENT_LENGTH        (127*1024)     /* �����ֶΣ�������ʣ127K */
#define ENV_CONTENT_LENGTH      (64*1024-ENV_CONTENT_OFFSET-12) /* �����ֶ�ʣ��127Kʵ���õĺ��� */

typedef enum _GEV_ITEM_TYPE
{
    GEV_ITEM_BOOTPARAM = 0,
    GEV_ITEM_BMP,
    GEV_ITEM_RESERVE,
    GEV_ITEM_VALUE,
    GEV_ITEM_BMP_AND_VALUE, /* ͬʱ������bootrom����֮�����������,����_gevSetBack */
    GEV_ITEM_TYPE_NUM
}GEV_ITEM_TYPE;

#define IS_VALID_GEV_ITEM(git)  ((git >= 0) && (git < GEV_ITEM_TYPE_NUM))
typedef struct _GEV_MAP
{
    GEV_ITEM_TYPE git;
    int offset;
    int length;
}GEV_MAP;

LOCAL GEV_MAP gevMap[GEV_ITEM_TYPE_NUM] = {
{GEV_ITEM_BOOTPARAM,ENV_BOOTPARAM_OFFSET,ENV_BOOTPARAM_LENGTH},
{GEV_ITEM_BMP,      ENV_BMP_OFFSET,      ENV_BMP_LENGTH},
{GEV_ITEM_RESERVE,  ENV_RESERVE_OFFSET,  ENV_RESERVE_LENGTH},
{GEV_ITEM_VALUE,    ENV_CONTENT_OFFSET,  ENV_CONTENT_LENGTH},
{GEV_ITEM_BMP_AND_VALUE,    ENV_BMP_OFFSET,  ENV_BMP_LENGTH+ENV_RESERVE_LENGTH+ENV_CONTENT_LENGTH},
};

/* ����ȫ���ǻ��������� */
#define ENV_ITEM_LENGTH         (ENV_ITEM_NAME_LEN+ENV_ITEM_VALUE_LEN)    /* ÿһ����ſռ�Ϊ512B */
#define ENV_ITEM_MAX_NUM        (254)
#define ENV_ITEM_NAME_OFFSET(x) (ENV_ITEM_LENGTH * (x))
#define ENV_ITEM_VALUE_OFFSET(x) (ENV_ITEM_LENGTH * (x) + ENV_ITEM_NAME_LEN)
/* ����������������� */

int gev_debug = 0;

#define DBG(arg...) do{if(0){printf("%s,%d:",__FUNCTION__, __LINE__);printf(arg);}}while(0)

typedef struct _gev_s
{
    unsigned char   name[ENV_ITEM_NAME_LEN];
    unsigned char   value[ENV_ITEM_VALUE_LEN];
    int   index;                      /* ���ڼ�¼�洢��FLASH�еڼ���������������0��ʼ */
}gev_s;

/* ����ʱ������ȡ�������������浽�ڴ��� by liuzenglin, 18Jun13 */
typedef struct ENV_NODE_S
{
    gev_s  gev;
    struct ENV_NODE_S *next;
} ENV_NODE_T;
LOCAL ENV_NODE_T* pEnvHead = NULL; /* ���ڴ洢�������� */
LOCAL int  flashGevInited = FALSE;

/* ����Ĭ�ϵĻ������� */
/*{
    {TP_SWITCH_START_NO_CONFIG, "0",                            1},
    {TP_SWITCH_START_CONFIG,    TP_SWITCH_CONFIG_NAME_1,     1},
    {TP_SWITCH_BACKUP_CONFIG,   TP_SWITCH_CONFIG_NAME_2,   1},
    {TP_SWITCH_START_IMAGE,     TP_SWITCH_IMAGE_NAME_1,         1},
    {TP_SWITCH_BACKUP_IMAGE,    TP_SWITCH_IMAGE_NAME_2,         1},
    {TP_SWITCH_FL_VER,          "0x10000",                      1},
    {TP_SWITCH_FL_SIZE_BOOT,    "0x100000",                     1},
    {TP_SWITCH_FL_OFFSET_FS,    "0x100000",                     1},
    {TP_SWITCH_FL_SIZE_FS,      "0x1c00000"| "0xc00000",                    1},    T3700����16Mflash
};*/
/* ���ض�̬�����ڴ棬��Ҫ�ֶ��ͷ� */
char* getDftGevContent(int *otGevSize)
{
    unsigned char* gev = NULL;
    unsigned char* pbm = NULL;
    unsigned char* value = NULL;
    int  gevSize = flashGetSectorSize() -12;
    char strFsDftSize[ENV_ITEM_VALUE_LEN] = {0};

    *otGevSize = gevSize;

    //gev = (unsigned char*)calloc(1, gevSize);
    gev = BUF_ADDR_NOFREE;
    if (gev == NULL)
    {
        DBG("Fatal error: Failed to allocate memory.\r\n");
        return NULL;
    }

    memset(gev, 0xff, gevSize);
    pbm = gev+gevMap[GEV_ITEM_BMP].offset;
    value = gev+gevMap[GEV_ITEM_VALUE].offset;
    
    if (ERROR == flashGetDftGevFsSize(strFsDftSize))
    {
        printf("Fatal error: Failed to get default fs size.\r\n");
        //free(gev);
        return NULL;
    }
    DBG("Default fs size is %s\r\n", strFsDftSize);

    pbm[0]=0x1;
    strncpy(value+ENV_ITEM_NAME_OFFSET(0), TP_SWITCH_START_CONFIG, ENV_ITEM_NAME_LEN);
    strncpy(value+ENV_ITEM_VALUE_OFFSET(0), TP_SWITCH_CONFIG_NAME_1, ENV_ITEM_VALUE_LEN);
    pbm[1]=0x1;
    strncpy(value+ENV_ITEM_NAME_OFFSET(1), TP_SWITCH_BACKUP_CONFIG, ENV_ITEM_NAME_LEN);
    strncpy(value+ENV_ITEM_VALUE_OFFSET(1), TP_SWITCH_CONFIG_NAME_2, ENV_ITEM_VALUE_LEN);
    pbm[2]=0x1;
    strncpy(value+ENV_ITEM_NAME_OFFSET(2), TP_SWITCH_START_IMAGE, ENV_ITEM_NAME_LEN);
    strncpy(value+ENV_ITEM_VALUE_OFFSET(2), TP_SWITCH_IMAGE_NAME_1, ENV_ITEM_VALUE_LEN);
    pbm[3]=0x1;
    strncpy(value+ENV_ITEM_NAME_OFFSET(3), TP_SWITCH_BACKUP_IMAGE, ENV_ITEM_NAME_LEN);
    strncpy(value+ENV_ITEM_VALUE_OFFSET(3), TP_SWITCH_IMAGE_NAME_2, ENV_ITEM_VALUE_LEN);
    pbm[4]=0x1;
    strncpy(value+ENV_ITEM_NAME_OFFSET(4), TP_SWITCH_FL_VER, ENV_ITEM_NAME_LEN);
    strncpy(value+ENV_ITEM_VALUE_OFFSET(4), "0x10000", ENV_ITEM_VALUE_LEN);
    pbm[5]=0x1;
    strncpy(value+ENV_ITEM_NAME_OFFSET(5), TP_SWITCH_FL_SIZE_BOOT, ENV_ITEM_NAME_LEN);
    strncpy(value+ENV_ITEM_VALUE_OFFSET(5), "0x100000", ENV_ITEM_VALUE_LEN);
    pbm[6]=0x1;
    strncpy(value+ENV_ITEM_NAME_OFFSET(6), TP_SWITCH_FL_OFFSET_KERNEL, ENV_ITEM_NAME_LEN);
    strncpy(value+ENV_ITEM_VALUE_OFFSET(6), "0x100000", ENV_ITEM_VALUE_LEN);
    pbm[7]=0x1;
    strncpy(value+ENV_ITEM_NAME_OFFSET(7), TP_SWITCH_FL_SIZE_KERNEL, ENV_ITEM_NAME_LEN);
    strncpy(value+ENV_ITEM_VALUE_OFFSET(7), "0x600000", ENV_ITEM_VALUE_LEN);
    pbm[8]=0x1;
    strncpy(value+ENV_ITEM_NAME_OFFSET(8), TP_SWITCH_FL_OFFSET_USRIMG1, ENV_ITEM_NAME_LEN);
    strncpy(value+ENV_ITEM_VALUE_OFFSET(8), "0x700000", ENV_ITEM_VALUE_LEN);
    pbm[9]=0x1;
    strncpy(value+ENV_ITEM_NAME_OFFSET(9), TP_SWITCH_FL_SIZE_USRIMG1, ENV_ITEM_NAME_LEN);
    strncpy(value+ENV_ITEM_VALUE_OFFSET(9), "0x900000", ENV_ITEM_VALUE_LEN);
    pbm[10]=0x1;
    strncpy(value+ENV_ITEM_NAME_OFFSET(10), TP_SWITCH_FL_OFFSET_USRIMG2, ENV_ITEM_NAME_LEN);
    strncpy(value+ENV_ITEM_VALUE_OFFSET(10), "0x1000000", ENV_ITEM_VALUE_LEN);
    pbm[11]=0x1;
    strncpy(value+ENV_ITEM_NAME_OFFSET(11), TP_SWITCH_FL_SIZE_USRIMG2, ENV_ITEM_NAME_LEN);
    strncpy(value+ENV_ITEM_VALUE_OFFSET(11), "0x900000", ENV_ITEM_VALUE_LEN);
    pbm[12]=0x1;
    strncpy(value+ENV_ITEM_NAME_OFFSET(12), TP_SWITCH_FL_OFFSET_USRAPP, ENV_ITEM_NAME_LEN);
    strncpy(value+ENV_ITEM_VALUE_OFFSET(12), "0x1900000", ENV_ITEM_VALUE_LEN);
    pbm[13]=0x1;
    strncpy(value+ENV_ITEM_NAME_OFFSET(13), TP_SWITCH_FL_SIZE_USRAPP, ENV_ITEM_NAME_LEN);
    strncpy(value+ENV_ITEM_VALUE_OFFSET(13), "0x600000", ENV_ITEM_VALUE_LEN);
    pbm[14]=0x1;
    strncpy(value+ENV_ITEM_NAME_OFFSET(14), TP_SWITCH_START_NO_CONFIG, ENV_ITEM_NAME_LEN);
    strncpy(value+ENV_ITEM_VALUE_OFFSET(14), "0", ENV_ITEM_VALUE_LEN);

    return gev;
}

STATUS resetEnv(void)
{
    char *gev = NULL;
    int gevSize = 0;

    gev = getDftGevContent(&gevSize);
    if (gev == NULL)
    {
        printf("Get default content failed.\r\n");
        return ERROR;
    }
    
    if (ERROR == flashAppInnerSet(FLASH_INNER_ITEM_GEV, gev, gevSize, TRUE))
    {
        printf("Fata error: Failed to set gev to default.\r\n");
        //free(gev);
        return ERROR;
    }
    
    /* ���ù���FLASH����֮��Ļ�����������Ҫ���¸���FLASH���� */
    if (ERROR == flashAppInit())
    {
        printf("Fatal error: Failed to init flash app.\r\n");
        //free(gev);
        return ERROR;
    }

    //free(gev);
    DBG("Reset gev success.\r\n");
    return OK;
}


/* Ĭ��buff������gev����ĳ���һ�� by liuzenglin, 22Jun13 */
LOCAL int _getGevItem(GEV_ITEM_TYPE git, unsigned char* buff, int len)
{
    int length = 0;
    unsigned char* gevBuff = flashAppInnerGet(FLASH_INNER_ITEM_GEV, &length);

    if (NULL == gevBuff)
    {
        DBG("Failed to get gev at first read, and we will reset gev.\r\n");
        if (ERROR == resetEnv())
        {
            printf("Failed to reset gev.\r\n");
            return ERROR;
        }

        /* �ٶ�ȡһ�� */
        gevBuff = flashAppInnerGet(FLASH_INNER_ITEM_GEV, &length);
        if (NULL == gevBuff)
        {
            printf("Fatal error: Failed to get gev at second read.\r\n");
            return ERROR;
        }
    }

    if (! IS_VALID_GEV_ITEM(git))
    {
        DBG("Unkown git, git=%d\r\n", git);
        free(gevBuff);
        return ERROR;
    } 

    if(NULL == buff)
    {
        DBG("Null buff, buff=%#x\r\n", (int)buff);
        free(gevBuff);
        return ERROR;
    }
    memcpy(buff, gevBuff+gevMap[git].offset, len);

    DBG("Get gev item git=%#x,len=%d, gevMap len=%d.\r\n", 
        git, len, gevMap[git].length);

    free(gevBuff);

    return OK;    
}

/* Ĭ��buff������gev����ĳ���һ�� by liuzenglin, 22Jun13 */
LOCAL int _setGevItem(GEV_ITEM_TYPE git, const char* buff, int len)
{
    int length = 0;
    unsigned char* gevBuff = flashAppInnerGet(FLASH_INNER_ITEM_GEV, &length);

    if (NULL == gevBuff)
    {
        printf("Failed to get gev.\r\n");
        return ERROR;
    }

    if (! IS_VALID_GEV_ITEM(git))
    {
        DBG("Unkown git, git=%d\r\n", git);
        //free(gevBuff);
        return ERROR;
    }

    if(NULL == buff)
    {
        DBG("Null buff, buff=%#x\r\n", (int)buff);
        //free(gevBuff);
        return ERROR;
    }

    DBG("Set gev item git=%#x,len=%d, gevMap len=%d.\r\n", 
        git, len, gevMap[git].length);

    memcpy(gevBuff+gevMap[git].offset, buff, gevMap[git].length);

    if (ERROR == flashAppInnerSet(FLASH_INNER_ITEM_GEV, gevBuff, length, FALSE))
    {
        printf("Fatal error: Failed to set gev.\r\n");
        //free(gevBuff);
        return ERROR;
    }

    //free(gevBuff);

    return OK;

}


/* ������������û�п��õĿհ׿�ʱ�������û�ȥ */
LOCAL int _gevSetBack(void)
{
    unsigned char* pbm = NULL;
    unsigned char* content = NULL;
    unsigned char* buff = NULL;
    ENV_NODE_T* pRead = pEnvHead;
    int i = 0;
    int index = 0;

    //buff = (unsigned char*)calloc(1, gevMap[GEV_ITEM_BMP_AND_VALUE].length);
    buff = BUF_ADDR;
    if (buff == NULL)
    {
        printf("Fatal error: failed to allocate memory, size=%#x.\r\n", gevMap[GEV_ITEM_BMP_AND_VALUE].length);
        return ERROR;
    }

    memset(buff, 0xff, gevMap[GEV_ITEM_BMP_AND_VALUE].length);

    pbm = (unsigned char*)buff; /* ƫ�Ƶ�ַ��ͬ */

    if (ERROR == _getGevItem(GEV_ITEM_BMP, pbm, gevMap[GEV_ITEM_BMP].length))
    {
        //free(buff);
        printf("Fatal error: Failed to get gev bmp.\r\n");
        return ERROR;
    }

    for (i = 0; i < ENV_ITEM_MAX_NUM; i++)
    {
        if (pbm[i] == 0xff)
        {
            DBG("There are some blank gev, i=%d, and do not set back gev.\r\n", i);
            //free(buff);
            return OK;
        }
    }

    DBG("There is not any blank gev, and set back gev.\r\n");

    memset(pbm, 0xff, ENV_BMP_LENGTH);

    content = (unsigned char*)(buff+gevMap[GEV_ITEM_VALUE].offset-gevMap[GEV_ITEM_BMP_AND_VALUE].offset);

    index = 0;
    while (pRead != NULL)
    {
        pbm[index] = 0x1;
        strncpy(content+ENV_ITEM_NAME_OFFSET(index), pRead->gev.name, ENV_ITEM_NAME_LEN);
        strncpy(content+ENV_ITEM_VALUE_OFFSET(index), pRead->gev.value, ENV_ITEM_VALUE_LEN);
        DBG("\t%-2d, %s=%s\r\n", pRead->gev.index, pRead->gev.name, pRead->gev.value);
        pRead = pRead->next;
        index++;
    }
    
    if (ERROR == _setGevItem(GEV_ITEM_BMP_AND_VALUE, buff, gevMap[GEV_ITEM_BMP_AND_VALUE].length))
    {
        //free(buff);
        printf("Fatal error: Failed to set gev pbm and value.\r\n");
        return ERROR;
    }
    flashGevInited = FALSE; /* ��ʼ��������Ҫ���¸��� */

    //free(buff);
    DBG("Gev setback ok, index=%d.\r\n", index);
    return OK; 
}

LOCAL void _freeEnvItems(void)      /* �ͷ����� */
{
    ENV_NODE_T* pFree = pEnvHead;
    ENV_NODE_T* pTemp = NULL;
    while (pFree != NULL)
    {
        pTemp = pFree->next;
        free(pFree);
        pFree = pTemp;
    }
    pEnvHead = NULL;
    return;
}

/* ��ʼ��ʱ�жϱ���Ҫ�Ļ��������Ƿ���ڣ���������д�� */
STATUS _checkEnvIsValid(void)
{
    char envValue[ENV_ITEM_VALUE_LEN] = {0};

    /* �����ͱ��ݾ�������ֻ��ΪTP_SWITCH_IMAGE_NAME_1��TP_SWITCH_IMAGE_NAME_2 */
    if (ERROR == getEnvItem(TP_SWITCH_START_IMAGE, envValue) ||
        ((strncmp(envValue, TP_SWITCH_IMAGE_NAME_1, FN_MAX_LENGTH) != 0) && 
        (strncmp(envValue, TP_SWITCH_IMAGE_NAME_2, FN_MAX_LENGTH) != 0)) ||
        ERROR == getEnvItem(TP_SWITCH_BACKUP_IMAGE, envValue) ||
        ((strncmp(envValue, TP_SWITCH_IMAGE_NAME_1, FN_MAX_LENGTH) != 0) && 
        (strncmp(envValue, TP_SWITCH_IMAGE_NAME_2, FN_MAX_LENGTH) != 0)) ||
        (ERROR == getEnvItem(TP_SWITCH_START_CONFIG, envValue)) ||
        (ERROR == getEnvItem(TP_SWITCH_BACKUP_CONFIG, envValue)) ||
        (ERROR == getEnvItem(TP_SWITCH_FL_VER, envValue)) ||
        (ERROR == getEnvItem(TP_SWITCH_FL_SIZE_BOOT, envValue)) ||
        (ERROR == getEnvItem(TP_SWITCH_FL_OFFSET_KERNEL, envValue)) ||
        (ERROR == getEnvItem(TP_SWITCH_FL_SIZE_KERNEL, envValue)) ||
        (ERROR == getEnvItem(TP_SWITCH_FL_OFFSET_USRIMG1, envValue)) ||
        (ERROR == getEnvItem(TP_SWITCH_FL_SIZE_USRIMG1, envValue)) ||
        (ERROR == getEnvItem(TP_SWITCH_FL_OFFSET_USRIMG2, envValue)) ||
        (ERROR == getEnvItem(TP_SWITCH_FL_SIZE_USRIMG2, envValue)) ||
        (ERROR == getEnvItem(TP_SWITCH_FL_OFFSET_USRAPP, envValue)) ||
        (ERROR == getEnvItem(TP_SWITCH_FL_SIZE_USRAPP, envValue)) )
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*****************************************************************************
* fn            : _initReadGev()
* author        : LiuZenglin <LiuZenglin@tp-link.net>
* decription    : ��ȡFlash�л����������ڴ��У����쵥������
* param[in]     : N/A
* param[out]    : N/A
*
* return        : N/A
* retval        : N/A
* note          : �����ȡ��Ŀʧ�ܣ���ʧ����Ŀ֮��ı���
*****************************************************************************/
STATUS _initReadGev(void)    /* copy items for flash to ram */
{
    UINT32 nEnvNum = 0;
    int i = 0;
    ENV_NODE_T* pCur = NULL;    /* ��ǰ��������ڵ� */
    ENV_NODE_T* pParent = NULL;
    unsigned char* pbm = NULL;
    unsigned char* content = NULL;    

    if (NULL != pEnvHead)   /* ����Ѿ���ʼ���������ͷź��ٴλ�ȡ */
    {
        DBG("Reinit the environment list.\r\n");
        _freeEnvItems();
    }

    //pbm = (unsigned char*)calloc(1, ENV_BMP_LENGTH);
    pbm = BUF_ADDR;
    if (pbm == NULL)
    {
        flashGevInited = TRUE;
        printf("Fatal error: Failed to allocate memory, size=%#x.\r\n", ENV_BMP_LENGTH);
        return ERROR;
    }

    if (ERROR == _getGevItem(GEV_ITEM_BMP, pbm, ENV_BMP_LENGTH))
    {
        //free(pbm);
        flashGevInited = TRUE;
        printf("Fatal error: Failed to get gev bmp.\r\n");
        return ERROR;
    }

    //content = (unsigned char*)calloc(1, ENV_CONTENT_LENGTH);
    content = BUF_ADDR + ENV_BMP_LENGTH;
    if (content == NULL)
    {
        //free(pbm);
        flashGevInited = TRUE;
        printf("Fatal error: Failed to allocate memory, size=%#x.\r\n", ENV_CONTENT_LENGTH);
        return ERROR;
    }

    if (ERROR == _getGevItem(GEV_ITEM_VALUE, content, ENV_CONTENT_LENGTH))
    {
        //free(pbm);
        //free(content);
        flashGevInited = TRUE;
        printf("Fatal error: Failed to get gev value.\r\n");
        return ERROR;
    }

    /* ��ȡ�������� */
    for (i = 0; i < ENV_ITEM_MAX_NUM; i++)
    {
        DBG("ENV %d is %s.\r\n", i, 
            (pbm[i]==0)? "not active":((pbm[i]==1)?"active":((pbm[i]==0xff)?"blank":"unkown")));
        if (pbm[i] != 0x1)
        {
            continue;
        }

        if ((strlen(content+ENV_ITEM_NAME_OFFSET(i)) == 0) ||
            (strlen(content+ENV_ITEM_VALUE_OFFSET(i)) == 0))
        {
            DBG("Gev index=%d length is 0, and will be ignored.\r\n", i);
            continue;
        }

        pCur = (ENV_NODE_T*)calloc(1, sizeof(ENV_NODE_T));
        if (NULL == pCur)
        {
            DBG("Failed to allocate ENV_NODE_T.\r\n");
            _freeEnvItems();
            flashGevInited = TRUE;
            //free(pbm);
            //free(content);
            return ERROR;
        }
        memset(pCur->gev.name, 0, ENV_ITEM_NAME_LEN);
        memset(pCur->gev.value, 0, ENV_ITEM_VALUE_LEN);
        pCur->gev.index = i;
        pCur->next = NULL;

        strncpy((char*)(pCur->gev.name), content+ENV_ITEM_NAME_OFFSET(i), ENV_ITEM_NAME_LEN);
        strncpy((char*)(pCur->gev.value), content+ENV_ITEM_VALUE_OFFSET(i), ENV_ITEM_VALUE_LEN);

        nEnvNum++;
        DBG("%d index=%d %s=%s.\r\n", nEnvNum, i, pCur->gev.name, pCur->gev.value);
        if (nEnvNum == 1)
        {
            pEnvHead = pCur;
            pParent  = pEnvHead;
            pCur = NULL;
        }
        else
        {
            pParent->next = pCur;
            pParent = pCur;
            pCur = NULL;
        }
    }

    flashGevInited = TRUE;
    //free(pbm);
    //free(content);

    return OK;
}

int flashAppInnerGevInit(void)
{
    if (ERROR == _initReadGev())
    {
        printf("Failed to read gev to memory.\r\n");
        return ERROR;
    }
    
    /* ��黷�������ĺϷ��ԣ�Ĭ�������ļ����� */
    if (FALSE == _checkEnvIsValid())
    {
        printf("Gev is not valid, and The gev will be reset.\r\n");
        if (ERROR == resetEnv())
        {
            printf("Failed to reset gev.\r\n");
            return ERROR;
        }

        if (ERROR == _initReadGev())
        {
            printf("Failed to read gev to memory.\r\n");
            return ERROR;
        }
    }

    return OK;
}

/*****************************************************************************
* fn            : setEnvItem()
* author        : LiuZenglin <LiuZenglin@tp-link.net>
* decription    : ��ӻ��޸���Ŀ��ͬʱ����Flash���ڴ�������
* param[in]     : N/A
* param[out]    : N/A
*
* return        : N/A
* retval        : N/A
* note          : Flash��intȫ���Դ�˴洢
*****************************************************************************/
STATUS setEnvItem(const char* name, const char* value)
{
    ENV_NODE_T* pModify = pEnvHead;
    ENV_NODE_T* pEnd = NULL;   /* ���ڼ�¼���һ���ڵ� */
    UINT8 buffer[ENV_ITEM_LENGTH] = {0};   /* Flash����Ŀ��С������дFlash��Ŀ */
    UINT32 nNameLen = 0;
    UINT32 nValueLen = 0;
    int    index  =-1;         /* ������ǰҪ�޸ĵĻ����������ڵ����, ����½��򱣳�Ϊ-1 */
    UINT32 counter = 0;       /* ���ڱ�ʾ��ǰFlash�еĻ�����������Ŀ */
    UINT8 newNode = 0;     /* ���ڱ���Ƿ�����ӽڵ� */
    int i =0;
    unsigned char* pbm = NULL;
    int valueOffset = 0;

    if (flashGevInited == FALSE)
    {
        if (ERROR == flashAppInnerGevInit())
        {
            DBG("Fatal error: Failed to init the gev.\r\n");
            return ERROR;
        }
    }

    memset(buffer, 0x0, ENV_ITEM_LENGTH);
    nNameLen = strlen(name);
    nValueLen = strlen(value);

    if ((NULL == name) || (NULL == value)
        || (nNameLen >= ENV_ITEM_NAME_LEN)
        || (nValueLen >= ENV_ITEM_VALUE_LEN)
        || (nNameLen <= 0)
        || (nValueLen <= 0))
    {
        DBG("%s, invalid param.\r\n", __FUNCTION__);
        return ERROR;
    }

    strncpy(buffer, name, ENV_ITEM_NAME_LEN);
    strncpy(buffer + ENV_ITEM_NAME_LEN, value, ENV_ITEM_VALUE_LEN);

    pModify = pEnvHead;
    /* ���ҽڵ� */
    while (NULL != pModify)
    {
        if (0 == strncmp(pModify->gev.name, name, ENV_ITEM_NAME_LEN))   /* �޸� */
        {
            DBG("Modify gev item, name=%s, old_value=%s, new_value=%s, index=%d\r\n", 
                name, pModify->gev.value, value, pModify->gev.index);
            index = pModify->gev.index;
            if (0 == strncmp(pModify->gev.value, value, ENV_ITEM_VALUE_LEN))    /* �ýڵ㲻���޸� */
            {
                return OK;
            }
            break;
        }
        else
        {
            pEnd = pModify;
            pModify = pModify->next;
            counter++;
        }
    }

    /* ����Ҳ�����������Ŀ��δ��������ӽڵ� */
    if (NULL == pModify)   /* �½����� */
    {
        DBG("Write a new gev item, name=%s, new_value=%s\r\n", 
            name, value);
        /* ����Ѿ����ˣ�����ʾ���� */
        if  (counter >= ENV_ITEM_MAX_NUM)
        {
            printf("Warning: Failed to set %s=%s, %d items at most.\r\n",
                   name, value, ENV_ITEM_MAX_NUM);
            return OK;
        }

        pModify = (ENV_NODE_T*)calloc(1, sizeof(ENV_NODE_T));
        if (NULL == pModify)
        {
            DBG("Failed to allocate ENV_NODE_T.\r\n");
            return ERROR;
        }
        pModify->next = NULL;
        memset(pModify->gev.name, 0, ENV_ITEM_NAME_LEN);
        strncpy(pModify->gev.name, name, ENV_ITEM_NAME_LEN);

        if (0 == counter) /* Ϊ������ */
        {
            /* ��ʼ������ */
            pEnvHead = pModify;
        }
        else
        {
            /* ��ӽڵ㵽������ */
            pEnd->next = pModify;
        }
        newNode = 1;
    }
    memset(pModify->gev.value, 0, ENV_ITEM_VALUE_LEN);
    strncpy(pModify->gev.value, value, ENV_ITEM_VALUE_LEN);

    /* write flash */
    DBG("gev counter = %d, index = %d\r\n", counter, index);
    //pbm = (unsigned char*)calloc(1, ENV_BMP_LENGTH);
    pbm = BUF_ADDR;
    if (pbm == NULL)
    {
        printf("Fatal error: Failed to allocate memory, size=%#x.\r\n", ENV_BMP_LENGTH);
        return ERROR;
    }

    if (ERROR == _getGevItem(GEV_ITEM_BMP, pbm, ENV_BMP_LENGTH))
    {
        //free(pbm);
        printf("Fatal error: Failed to get gev bmp.\r\n");
        return ERROR;
    }

    for (i = 0; i < ENV_ITEM_MAX_NUM; i++)
    {
        /* �ҵ�û��ʹ�õĻ������� */
        if (pbm[i] == 0xff)
        {
            DBG("Find a blank gev space, index=%d\r\n", i);
            pbm[i] = 0x1;
            /* ������޸ģ�����Ҫ��ԭ����������Ϊinvalid״̬ */
            if (newNode != 1)
            {
                pbm[index] = 0;
            }
            pModify->gev.index = i;

            if (ERROR == flashAppInnerGetOffset(FLASH_INNER_ITEM_GEV, &valueOffset))
            {
                //free(pbm);
                printf("Fatal error: Failed to set gev content offset.\r\n");
                return ERROR; 
            }

            /* ��д���ݣ���дBMP��������Ϊ�ϵ�����ַ��ַ����Ļ������� */
            if (ERROR == writeFlash(buffer, ENV_ITEM_LENGTH, 
                valueOffset+gevMap[GEV_ITEM_VALUE].offset+ENV_ITEM_NAME_OFFSET(i), FALSE))
            {
                //free(pbm);
                printf("Fatal error: Failed to set gev content.\r\n");
                return ERROR;                
            }
            if (ERROR == writeFlash(pbm, ENV_BMP_LENGTH, valueOffset+gevMap[GEV_ITEM_BMP].offset, FALSE))
            {
                //free(pbm);
                printf("Fatal error: Failed to set gev pbm.\r\n");
                return ERROR;
            }

            /* ����Ƿ��ж���Ŀհ׿� */
            if (ERROR == _gevSetBack())
            {
                //free(pbm);
                printf("Fatal error: Failed to setback gev.\r\n");
                return ERROR;
            }

            /* ���ù���FLASH����֮��Ļ�����������Ҫ���¸���FLASH���� */
            if (ERROR == flashAppInit())
            {
                printf("Fatal error: Failed to init flash app.\r\n");
                //free(pbm);
                return ERROR;
            }

            //free(pbm);
            DBG("Set env ok,index=%d %s=%s\r\n", i, name, value);
            return OK;
        }
    }

    //free(pbm);
    /* ����ÿ������֮�󶼻����Ƿ���BLANK��gev���ʲ������Ҳ���BLANK gev */
    printf("Fatal error: Can't find a blank gev.\r\n");
    return ERROR;
}


/*****************************************************************************
* fn            : copyEnvItem()
* author        : LiuZenglin <LiuZenglin@tp-link.net>
* decription    : set gev named dstName with the same value of gev srcName 
* param[in]     : N/A
* param[out]    : N/A
*
* return        : N/A
* retval        : N/A
* note          : N/A
*****************************************************************************/
STATUS copyEnvItem(const char* dstName, const char* srcName)
{
    char gevValue[256] = {0};
    
    if ((NULL == dstName) ||
        (NULL == srcName))
    {
        DBG("Fatal error: dstName is %#x, srcName is %#x\r\n", (int)dstName, (int)srcName);
        return ERROR;
    }

    if (flashGevInited == FALSE)
    {
        if (ERROR == flashAppInnerGevInit())
        {
            DBG("Fatal error: Failed to init the gev.\r\n");
            return ERROR;
        }
    }

    if (ERROR == getEnvItem(srcName, gevValue))
    {
        DBG("Failed to get gev %s when copy gev.\r\n", srcName);
        return ERROR;
    }

    if (ERROR == setEnvItem(dstName, gevValue))
    {
        DBG("Failed to set gev %s when copy gev.\r\n", dstName);
        return ERROR;
    }

    DBG("Copy env ok,dst gev=%s src gev=%s\r\n", dstName, srcName);
    return OK;
}

/*****************************************************************************
* fn            : getEnvItem()
* author        : LiuZenglin <LiuZenglin@tp-link.net>
* decription    : ��ȡ����������ֻ�����ڴ�������
* param[in]     : name����������ֵ
* param[out]    : value, ��ȡʧ��valueȫ��Ϊ0
*
* return        : N/A
* retval        : N/A
* note          : �ú�������ʱ�������ڴ滷��������Flash�л�������һ��
*****************************************************************************/
STATUS getEnvItem(const char* name, char* value)
{

    ENV_NODE_T* pRead = pEnvHead;

    if (flashGevInited == FALSE)
    {
        if (ERROR == flashAppInnerGevInit())
        {
            DBG("Fatal error: Failed to init the gev.\r\n");
            return ERROR;
        }
    }

    /* ��շ���buffer */
    memset(value, 0, ENV_ITEM_VALUE_LEN);

    pRead = pEnvHead;

    /* �������� */
    while (pRead != NULL)
    {
        if (0 == strncmp(pRead->gev.name, name, ENV_ITEM_NAME_LEN))   /* �޸� */
        {
            DBG("Find a gev, index=%d.\r\n", pRead->gev.index);
            strncpy(value, pRead->gev.value, ENV_ITEM_VALUE_LEN);
            break;
        }
        else
        {
            pRead = pRead->next;
        }
    }


    if (pRead == NULL)
    {
        DBG("Warning: Environment variable %s doesn't exist.\r\n", name);
        return ERROR;
    }
    else
    {
        DBG("%s=%s\r\n", name, value);
        return OK;
    }
}

/*****************************************************************************
* fn            : delEnvItem()
* author        : LiuZenglin <LiuZenglin@tp-link.net>
* decription    : ɾ����������������Flash������
* param[in]     : name,������������
* param[out]    : N/A
*
* return        : N/A
* retval        : N/A
* note          : N/A
*****************************************************************************/
STATUS delEnvItem(const char* name)
{
    ENV_NODE_T* pTemp = pEnvHead;
    ENV_NODE_T* pDel = NULL;
    UINT8 buffer[ENV_ITEM_LENGTH] = {0};
    UINT32 nNameLen = 0;
    UINT32 index = 0;
    unsigned char* pbm = NULL;
    int valueOffset = 0;

    if (flashGevInited == FALSE)
    {
        if (ERROR == flashAppInnerGevInit())
        {
            DBG("Fatal error: Failed to init the gev.\r\n");
            return ERROR;
        }
    }
    pTemp = pEnvHead;

    memset(buffer, 0xff, ENV_ITEM_LENGTH);
    nNameLen = strlen(name);

    if (NULL == name)
    {
        DBG("Null ptr param.\r\n");
        return ERROR;
    }

    /* �ձ� */
    if (pEnvHead == NULL)
    {
        printf("Warning: Environment variable %s doesn't exist.\r\n", name);
        return OK;
    }

    /* ɾ��ͷ�ڵ� */
    if (0 == strncmp(pEnvHead->gev.name, name, ENV_ITEM_NAME_LEN))
    {
        pTemp = pEnvHead->next;
        free(pEnvHead);
        pEnvHead = pTemp;
        index = 0;
    }
    else
    {
        pTemp = pEnvHead;
        pDel = pTemp->next;

        while (NULL != pDel)
        {
            if (0 == strncmp(pDel->gev.name, name, ENV_ITEM_NAME_LEN))   /* �ҵ� */
            {
                break;
            }
            else
            {
                pTemp = pDel;
                pDel = pDel->next;
                index++;
            }
        }

        if (NULL == pDel)       /* �����ڸñ��� */
        {
            printf("Warning: Environment variable %s doesn't exist.\r\n", name);
            return OK;
        }

        /* ɾ���ڵ� */
        pTemp->next = pDel->next;
        index = pDel->gev.index;
        free(pDel);
    }

    /* write flash */
    //pbm = (unsigned char*)calloc(1, ENV_BMP_LENGTH);
    pbm = BUF_ADDR;
    if (pbm == NULL)
    {
        printf("Fatal error: Failed to allocate memory, size=%#x.\r\n", ENV_BMP_LENGTH);
        return ERROR;
    }

    if (ERROR == _getGevItem(GEV_ITEM_BMP, pbm, ENV_BMP_LENGTH))
    {
        //free(pbm);
        printf("Fatal error: Failed to get gev bmp.\r\n");
        return ERROR;
    }
    /* ����Ӧ��Ŀ����ΪINVALID״̬ */
    pbm[index] = 0;
    DBG("Set gev invalid, index=%d\r\n", index);
    if (ERROR == flashAppInnerGetOffset(FLASH_INNER_ITEM_GEV, &valueOffset))
    {
        //free(pbm);
        printf("Fatal error: Failed to set gev content offset.\r\n");
        return ERROR; 
    }
            
    //if (ERROR == _setGevItem(GEV_ITEM_BMP, pbm, ENV_BMP_LENGTH))
    if (ERROR == writeFlash(pbm, ENV_BMP_LENGTH, valueOffset+gevMap[GEV_ITEM_BMP].offset, FALSE))
    {
        //free(pbm);
        printf("Fatal error: Failed to set gev pbm.\r\n");
        return ERROR;
    }

    //free(pbm);

    return OK;
}

/* �������軷����������ɾ���Ǳ���Ļ���������������flashAppInnerReset���� */
STATUS updateGevItem(const char*flBootSize, const char*flFsOffset, const char*flFsSize, const char*flVer)
{
    ENV_NODE_T* pDel = pEnvHead;

    /* ɾ���Ǳ���Ľڵ� */
    pDel = pEnvHead;
    while (pDel != NULL)
    {
        DBG("Try to clear gev, %s=%s\r\n", pDel->gev.name, pDel->gev.value);
        /*  ������ */
        if ((0 != strncmp(pDel->gev.name, TP_SWITCH_START_IMAGE, ENV_ITEM_NAME_LEN)) &&
            (0 != strncmp(pDel->gev.name, TP_SWITCH_BACKUP_IMAGE, ENV_ITEM_NAME_LEN)) &&
            (0 != strncmp(pDel->gev.name, TP_SWITCH_START_NO_CONFIG, ENV_ITEM_NAME_LEN)) &&
            (0 != strncmp(pDel->gev.name, TP_SWITCH_START_CONFIG, ENV_ITEM_NAME_LEN)) &&
            (0 != strncmp(pDel->gev.name, TP_SWITCH_BACKUP_CONFIG, ENV_ITEM_NAME_LEN)) &&
            (0 != strncmp(pDel->gev.name, TP_SWITCH_FL_OFFSET_KERNEL, ENV_ITEM_NAME_LEN)) &&
            (0 != strncmp(pDel->gev.name, TP_SWITCH_FL_SIZE_BOOT, ENV_ITEM_NAME_LEN)) &&
            (0 != strncmp(pDel->gev.name, TP_SWITCH_FL_SIZE_KERNEL, ENV_ITEM_NAME_LEN)) &&
            (0 != strncmp(pDel->gev.name, TP_SWITCH_FL_OFFSET_USRIMG1, ENV_ITEM_NAME_LEN)) &&
            (0 != strncmp(pDel->gev.name, TP_SWITCH_FL_SIZE_USRIMG1, ENV_ITEM_NAME_LEN)) &&
            (0 != strncmp(pDel->gev.name, TP_SWITCH_FL_OFFSET_USRIMG2, ENV_ITEM_NAME_LEN)) &&
            (0 != strncmp(pDel->gev.name, TP_SWITCH_FL_SIZE_USRIMG2, ENV_ITEM_NAME_LEN)) &&
            (0 != strncmp(pDel->gev.name, TP_SWITCH_FL_OFFSET_USRAPP, ENV_ITEM_NAME_LEN)) &&
            (0 != strncmp(pDel->gev.name, TP_SWITCH_FL_SIZE_USRAPP, ENV_ITEM_NAME_LEN)) &&
            (0 != strncmp(pDel->gev.name, TP_SWITCH_FL_VER, ENV_ITEM_NAME_LEN)))
        {
            DBG("Remove gev, %s=%s\r\n", pDel->gev.name, pDel->gev.value);
            pDel = pDel->next;      /* ��ɾ��֮ǰ��¼��һ���ڵ��λ�� */
            if (ERROR == delEnvItem(pDel->gev.name))
            {
                printf("Fatal error: Failed to remove gev, %s=%s, index=%d\r\n",
                    pDel->gev.name, pDel->gev.value, pDel->gev.index);
                return ERROR;
            }
        }
        else
        {
            pDel = pDel->next;
        }
    }
    
    /* �������Ļ������� 
    if ((ERROR == setEnvItem(TP_SWITCH_FL_VER, flVer)) ||
        (ERROR == setEnvItem(TP_SWITCH_FL_SIZE_BOOT, flBootSize)) ||
        (ERROR == setEnvItem(TP_SWITCH_FL_SIZE_KERNEL, flFsSize)) ||
        (ERROR == setEnvItem(TP_SWITCH_FL_OFFSET_KERNEL, flFsOffset)))
    {
        printf("Fatal error: Failed to update gev.\r\n");
        return ERROR;
    }*/

    return OK;
}

/*****************************************************************************
* fn            : printEnvItems()
* author        : LiuZenglin <LiuZenglin@tp-link.net>
* decription    : ������л�������
* param[in]     : N/A
* param[out]    : N/A
*
* return        : N/A
* retval        : N/A
* note          : N/A
*****************************************************************************/
void printEnvItems(void)
{
    ENV_NODE_T* pRead = pEnvHead;
    UINT32 i = 0;

    if (flashGevInited == FALSE)
    {
        if (ERROR == flashAppInnerGevInit())
        {
            DBG("Fatal error: Failed to init the gev.\r\n");
            return;
        }
    }
    pRead = pEnvHead;

    printf("Global environment variables:\r\n");
    while (pRead != NULL)
    {
        i++;
        printf("\t%-2d, %s=%s\r\n", i, pRead->gev.name, pRead->gev.value);
        pRead = pRead->next;
    }
    printf("\r\n");
    return;
}

/*!
 *\fn                   STATUS sysGetBootFromFlash(char *s_pro)
 *\brief                get the mac from flash
 *
 *\param[in]    N/A
 *\param[out]   s_pro   store the mac address
 *
 *\return               error code
 *\retval               OK              success
 *\retval               ERROR   fail
 *
 *\note
 */
STATUS sysGetBootFromFlash(BOOT_PARAM *bootParam)
{

    int valueOffset = 0;

    if(NULL == bootParam)
    {
        return ERROR;
    }

    if (ERROR == flashAppInnerGetOffset(FLASH_INNER_ITEM_GEV, &valueOffset))
    {
        printf("Fatal error: Failed to set gev content offset.\r\n");
        return ERROR; 
    }

    /* if (ERROR == _getGevItem(GEV_ITEM_BOOTPARAM, (unsigned char*)(bootParam->padding), gevMap[GEV_ITEM_BOOTPARAM].length)) */
    if (ERROR == readFlash((char*)(bootParam->padding), 
        gevMap[GEV_ITEM_BOOTPARAM].length, valueOffset+gevMap[GEV_ITEM_BOOTPARAM].offset))
    {
        printf("Failed to get boot param.\r\n");
        return ERROR;
    }

    return OK;
}

/*!
 *\fn           STATUS sysSetBootToFlash(char *s_pro,int len)
 *\brief        set the bootutil config to flash
 *
 *\param[in]
 *\param[out]
 *
 *\return
 *\retval
 *
 *\note
 */
STATUS sysSetBootToFlash(BOOT_PARAM *bootParam)
{
    int valueOffset = 0;

    if(NULL == bootParam)
    {
        return ERROR;
    }

    if (ERROR == flashAppInnerGetOffset(FLASH_INNER_ITEM_GEV, &valueOffset))
    {
        printf("Fatal error: Failed to set gev content offset.\r\n");
        return ERROR; 
    }

    /* if (ERROR == _setGevItem(GEV_ITEM_BOOTPARAM, (char*)(bootParam->padding), gevMap[GEV_ITEM_BOOTPARAM].length)) */
    if (ERROR == writeFlash((char*)(bootParam->padding), 
        gevMap[GEV_ITEM_BOOTPARAM].length, valueOffset+gevMap[GEV_ITEM_BOOTPARAM].offset, FALSE))
    {
        printf("Failed to write boot config to flash\r\n");
        return ERROR;
    } 

    return OK;
}

#if 0
UINT32 inet_addr(register char *inetString);
STATUS sysGetBootLineFromFlash(char *bootLine, int len)
{
    int rc = 0;
    BOOT_PARAM *bootParam = NULL;
    UINT32 mask = 0;

    /* BOOTLINEĬ�ϳ���Ϊ240�ֽ� */
    if ((NULL == bootLine) && (len < 240))
    {
        DBG("Error: Bad parameter, len = %d.\r\n", len);
        return ERROR;
    }

    bootParam = (BOOT_PARAM*)calloc(1, sizeof(BOOT_PARAM));
    if( NULL == bootParam)
    {
        printf("Error: Failed to allocate memory.\r\n");
        return ERROR;
    }

    rc = sysGetBootFromFlash(bootParam);
    if (rc == ERROR)
    {
        free(bootParam);
        printf("Error: Failed to read boot parameters.\r\n");
        return ERROR;
    }

    memset(bootLine, 0, len);
    mask = inet_addr(bootParam->content.ipMask);
    sprintf(bootLine,
            "sc(0,0)host:%s h=%s e=%s:%x g=%s u=%s pw=%s",
            bootParam->content.fileName,
            bootParam->content.hostAddr,
            bootParam->content.ipAddr,
            mask,
            bootParam->content.routeAddr,
            bootParam->content.userName,
            bootParam->content.passWd
           );

    free(bootParam);
    return rc;
}


void testSetEnv(int num)
{
    int i =0;
    char name[20] = {0};
    char value[20] = {0};
    if (num <= 0)
    {
        return;
    }

    for (i = 0; i < num; i++)
    {
        sprintf(name, "n%d", i);
        sprintf(value, "v%d", i);
        if (ERROR == setEnvItem(name, value))
        {
            printf("Failed to set %s=%s.\r\n", name, value);
        }
        else
        {
            printf("Set %s=%s ok.\r\n", name, value);
        }
        memset(name, 0, 20);
        memset(value, 0, 20);
    }
}

void testDelEnv(int num)
{
    int i =0;
    char name[20] = {0};
    char value[20] = {0};
    if (num <= 0)
    {
        return;
    }

    for (i = 0; i < num; i++)
    {
        sprintf(name, "n%d", i);
        sprintf(value, "v%d", i);
        if (ERROR == delEnvItem(name))
        {
            printf("Failed to delete %s=%s.\r\n", name, value);
        }
        memset(name, 0, 20);
        memset(value, 0, 20);
    }
}

#endif

