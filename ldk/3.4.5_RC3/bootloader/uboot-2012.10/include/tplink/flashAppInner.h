/******************************************************************************
*
* Copyright (c) 2010 TP-LINK Technologies CO.,LTD.
* All rights reserved.
*
* file name  :   flashAppInner.h
* version    :   V1.0
* description:   �����ڲ����Ĳ��ֺͲ����ӿ�.
*
* author     :   LiuZenglin <LiuZenglin@tp-link.net>
* data       :   06/21/2013
*
* history    :
* 01   06/21/2013  LiuZenglin     Create.
*
******************************************************************************/
#ifndef __FLASH_APP_INNER_H__
#define __FLASH_APP_INNER_H__

/* �����ڲ�������Ŀ�����ͣ�V1.0.0��ֻ��������Ŀ������ʱ��ʵ�ʶ�ȡ��������Ϊ׼ */
/* ��FLASH��ռ��1 BYTE */
typedef enum _FLASH_INNER_ITEM_TYPE
{
    FLASH_INNER_ITEM_PROFILE = 0x0,
    FLASH_INNER_ITEM_GEV,
    FLASH_INNER_SN,
    FLASH_INNER_LICENSE = 0x5,
    FLASH_INNER_ITEM_REVERSE
}FLASH_INNER_ITEM_TYPE;

void flashAppInnerEnum(int *sn,int *license);
int flashAppInnerInit(void);
unsigned char* flashAppInnerGet(FLASH_INNER_ITEM_TYPE type, int *length);
/* �����ڲ�����FLASH�е�ƫ�Ƶ�ַ */
int flashAppInnerGetOffset(FLASH_INNER_ITEM_TYPE type, int *value_offset);
int flashAppInnerSet(FLASH_INNER_ITEM_TYPE type, char* buff, int len, int prompt);
int flashAppInnerUnset(FLASH_INNER_ITEM_TYPE type, int prompt);
int flashAppInnerReset(const char*flBootSize, const char*flFsOffset, const char*flFsSize, const char*flVer);

#endif /* __FLASH_APP_INNER_H__ */

