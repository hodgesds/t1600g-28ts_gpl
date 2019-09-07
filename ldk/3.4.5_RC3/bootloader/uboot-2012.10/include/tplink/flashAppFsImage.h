/******************************************************************************
*
* Copyright (c) 2010 TP-LINK Technologies CO.,LTD.
* All rights reserved.
*
* file name  :   flashAppFsImage.h
* version    :   V1.0
* description:   �����������Ľӿ�.
*
* author     :   LiuZenglin <LiuZenglin@tp-link.net>
* data       :   06/25/2013
*
* history    :
* 01   06/25/2013  LiuZenglin     Create.
*
******************************************************************************/
#ifndef __FLASH_APP_FS_IMAGE_H__
#define __FLASH_APP_FS_IMAGE_H__

#include <tplink/flashAppInnerGev.h>
//#include <tplink/typedefs.h>
#include <tplink/flashConfig.h>

#define MD5_CHECKSUM_OFFSET		0		/*!< the offset of the md5 key (add by caipeifeng:2010-1-8 14:41:25)*/
#define MD5_DIGEST_LEN			16		/*!< length of MD5 digest result  (add by Cai Peifeng:2010-4-7 13:45:1) */
#define FLASH_PRODUCT_LENGTH 	16		/*!< the length of the product (add by caipeifeng:2010-1-8 14:41:9)*/
#define TP_IMAGE_NAME_SIZE			200
#define TP_IMAGE_HEADER_SIZE			512
/* ����ͷ���д洢������������������ */
#define FL_SIZE_LEN                16  /* "0x100000" */
#define FL_VER_LEN                      16   /* "0x10000" */

#define strtoul                     simple_strtoul   
#define TOLOWER(x) ((x) | 0x20)   

#define UBOOT_DEBUG

typedef struct
{
    char            md5[MD5_DIGEST_LEN];
    unsigned char   productId[FLASH_PRODUCT_LENGTH];
    char	        imagename[TP_IMAGE_NAME_SIZE];
    UINT32          usrImageSize;   /*�û������С*/
    UINT32          uImageSize;      /*�ں˾����С*/
    UINT32          bootromSize;        /* ��ѡ�bootromSize == 0��ʾ���񲻰���bootrom */
    UINT32          compressMethod;     /* ѹ���㷨 1 - LZMA */
    char            flBootSize[FL_SIZE_LEN];   /* BOOTROM���Ĵ�С */
    char            flKernelOffset[FL_SIZE_LEN];   /*�ں˵���ʼ��ַ*/
    char            flKernelSize[FL_SIZE_LEN];       /* �ں˵Ĵ�С */
    char            flUsrImg1Offset[FL_SIZE_LEN];   /* �û��������ʼ��ַ */
    char            flUsrImg1Size[FL_SIZE_LEN];       /* �û�����Ĵ�С */
    char            flUsrImg2Offset[FL_SIZE_LEN];   /* �û��������ʼ��ַ */
    char            flUsrImg2Size[FL_SIZE_LEN];       /* �û�����Ĵ�С */
    char            flUsrAppOffset[FL_SIZE_LEN];   /* �û�app�ļ�ϵͳ��ʼ��ַ */
    char            flUsrAppSize[FL_SIZE_LEN];       /* �û�app�ļ�ϵͳ��С */
    char            flVer[FL_VER_LEN];              /* Flash���ְ汾�� */
}_TP_IMAGE_HEADER;

typedef union
{
	UINT8			 padding[TP_IMAGE_HEADER_SIZE];	
	_TP_IMAGE_HEADER content;  	  
}TP_IMAGE_HEADER;

typedef struct _INNER_ITEM_INFO_NODE
{
    int type;
    int length;
    int offset;
    struct _INNER_ITEM_INFO_NODE* next;
}INNER_ITEM_INFO_NODE;

typedef enum _IMAGE_STATUS_T
{
    IMAGE_STATUS_NOT_EXIST = 0,
    IMAGE_STATUS_EXIST_OK,
    IMAGE_STATUS_EXIST_ERROR,
    IMAGE_STATUS_NUM
}IMAGE_STATUS_T;

typedef struct _IMAGE_INFO
{
    char imageName[BOOT_FILE_LEN];  /* image1.bin image2.bin */
    bool isValid;                   /* ������ǰ�ṹ�г�ImageName�������ֵ�Ƿ���Ч */
    IMAGE_STATUS_T ist;             /* image status */
    char imagename[TP_IMAGE_NAME_SIZE];
    /* ����ƫ�Ƶ�ַΪ�����ڽ��ܺ�ľ����е�ƫ�Ƶ�ַ */
    int  usrImage_offset;
    int  usrImage_len;
    int  uImage_offset;
    int  uImage_len;
    int  bootrom_offset;
    int  bootrom_len;       /* ��������в�����bootrom�������Ϊ0 */
    /* �ڲ������� */
    int  innerItemNumber;
    /* ���������������ڣ���ʼ��ʱ��������ڣ����ʼ��ΪV1.0.0�汾 */
    int  gevType;                        /* �����������ڲ����е�type */
    char flBootSize[FL_SIZE_LEN];   /* BOOTROM���Ĵ�С */
    char flKernelOffset[FL_SIZE_LEN];   /*�ں˵���ʼ��ַ*/
    char flKernelSize[FL_SIZE_LEN];       /* �ں˵Ĵ�С */
    char flUsrImg1Offset[FL_SIZE_LEN];   /* �û��������ʼ��ַ */
    char flUsrImg1Size[FL_SIZE_LEN];       /* �û�����Ĵ�С */
    char flUsrImg2Offset[FL_SIZE_LEN];   /* �û��������ʼ��ַ */
    char flUsrImg2Size[FL_SIZE_LEN];       /* �û�����Ĵ�С */
    char flUsrAppOffset[FL_SIZE_LEN];   /* �û�app�ļ�ϵͳ��ʼ��ַ */
    char flUsrAppSize[FL_SIZE_LEN];       /* �û�app�ļ�ϵͳ��С */
    char flVer[FL_VER_LEN];              /* Flash���ְ汾�� */
    /* �����ڲ����̬ */
    INNER_ITEM_INFO_NODE *innerItemHead;
} IMAGE_INFO;

/* ��������ʱ��ˢ��FLASH����ɰ汾�л��þ�����switch.tp��webImage.z�����ļ�ϵͳ */
int refreshFlash(const IMAGE_INFO *imageInfo, const char* pBuff,  int nRegion);

/* ��������ľ���buff��buff����Ϊlength, ��ȡ����ṹ */
/* ���������ϢpImageInfo */
int sysParseImage(const unsigned char* pImageBuff, int length, IMAGE_INFO *pImageInfo);

STATUS sysShowImage(const IMAGE_INFO *pImageInfo);

typedef struct _IMAGE_PRINT_INFO
{
    char imageName[BOOT_FILE_LEN];  /* image1.bin image2.bin */
    IMAGE_STATUS_T ist;             /* image status */
    char imagename[TP_IMAGE_NAME_SIZE];
    /* ����ƫ�Ƶ�ַΪ�����ڽ��ܺ�ľ����е�ƫ�Ƶ�ַ */
    /* ����ƫ�Ƶ�ַΪ�����ڽ��ܺ�ľ����е�ƫ�Ƶ�ַ */
    int  usrImage_offset;
    int  usrImage_len;
    int  uImage_offset;
    int  uImage_len;
    int  bootrom_offset;
    int  bootrom_len;       /* ��������в�����bootrom�������Ϊ0 */
    /* �ڲ������� */
    int  innerItemNumber;
    char flVer[FL_VER_LEN];              /* Flash���ְ汾�� */
}IMAGE_PRINT_INFO;

int sysGetImagePrintInfo(const unsigned char* pImageBuff, int length, IMAGE_PRINT_INFO *pImagePrintInfo);

STATUS sysShowImageByPrintInfo(const IMAGE_PRINT_INFO *pImagePrintInfo);

//STATUS sysUpgradeFileCheck(SWITCH_FILE_ATTRIBUTE sfa, unsigned char *pAppBuf, unsigned int nFileBytes);

//STATUS sysUpgradeFileApply(SWITCH_FILE_ATTRIBUTE sfa, unsigned char *pAppBuf, unsigned int nFileBytes);

#endif /* __FLASH_APP_FS_IMAGE_H__ */

