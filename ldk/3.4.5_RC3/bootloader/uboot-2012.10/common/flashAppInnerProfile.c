/******************************************************************************
*
* Copyright (c) 2010 TP-LINK Technologies CO.,LTD.
* All rights reserved.
*
* file name  :   flashAppProfile.c
* version    :   V1.0
* description:   ����������ؽӿ�.
*
* author     :   LiuZenglin <LiuZenglin@tp-link.net>
* data       :   06/18/2013
*
* history    :
* 01   06/18/2013  LiuZenglin     Create.
******************************************************************************/

#include<common.h>
#include<vsprintf.h>
#include <tplink/flashApp.h>
#include <tplink/flashAppInner.h>
#include <tplink/flashAppInnerProfile.h>


typedef enum _PROFILE_ITEM_TYPE
{
    PROFILE_ITEM_MAC = 0,
    PROFILE_ITEM_SIGN,
    PROFILE_ITEM_PUBLICKEY,
    PROFILE_ITEM_PRODUCTID,
    PROFILE_ITEM_TYPE_NUM
}PROFILE_ITEM_TYPE;

#define IS_VALID_PROFILE_ITEM(pit)  ((pit >= 0) && (pit < PROFILE_ITEM_TYPE_NUM))
typedef struct _PROFILE_MAP
{
    PROFILE_ITEM_TYPE pit;
    int offset;
    int length;
}PROFILE_MAP;

/* profile�����ݵĲ��� by liuzenglin, 15Aug12 */
/* profile�и��ֶεĶ��� */
#define PROFILE_MAC_OFFSET          0
#define PROFILE_MAC_LENGTH                  6
#define PROFILE_SIGN_OFFSET         (PROFILE_MAC_OFFSET + PROFILE_MAC_LENGTH)
#define PROFILE_SIGN_LENGTH                 128
#define PROFILE_PUBLICKEY_OFFSET    (PROFILE_SIGN_OFFSET + PROFILE_SIGN_LENGTH)
#define PROFILE_PUBLICKEY_LENGTH        250
#define PROFILE_PRODUCTID_OFFSET    (PROFILE_PUBLICKEY_OFFSET + PROFILE_PUBLICKEY_LENGTH)
#define PROFILE_PRODUCTID_LENGTH        16

LOCAL PROFILE_MAP profileMap[PROFILE_ITEM_TYPE_NUM] = {
{PROFILE_ITEM_MAC,       PROFILE_MAC_OFFSET,         PROFILE_MAC_LENGTH},
{PROFILE_ITEM_SIGN,      PROFILE_SIGN_OFFSET,        PROFILE_SIGN_LENGTH},
{PROFILE_ITEM_PUBLICKEY, PROFILE_PUBLICKEY_OFFSET,   PROFILE_PUBLICKEY_LENGTH},
{PROFILE_ITEM_PRODUCTID, PROFILE_PRODUCTID_OFFSET,   PROFILE_PRODUCTID_LENGTH},
};

/* Ϊ�˼���V2.1�汾��profile */
#define PROFILE_BOOTPARAM_LENGTH    324
/* OEM defines����profile�� */
#define OEM_ITEM_OFFSET_1     (PROFILE_PRODUCTID_OFFSET + PROFILE_PRODUCTID_LENGTH)
#define OEM_ITEM_OFFSET_2     (PROFILE_PRODUCTID_OFFSET + PROFILE_PRODUCTID_LENGTH + PROFILE_BOOTPARAM_LENGTH)
static int oemItemOffset = OEM_ITEM_OFFSET_1;   /* Ϊ�˼�������profile, ������� */
#define OEM_ITEM_OFFSET       oemItemOffset
#define OEM_VALID_SIZE      4               /* OEM��Ŀ�ʼ֮ǰ�����ֽ����ڱ�ʶ��Ч�� */
#define OEM_VALID_WORD      0xaa55aa55     /* ��OEM profile һ�� */
#define OEM_ITEM_SIZE       4               /* ǰ���ֽڱ��泤�� */
#define OEM_ITEM_LEN        124             /* ÿ����ĿռFlash��124�ֽڣ��볤�ȹ�ͬ����128�ֽ� */
#define OEM_ITEM_NUM        8               /* OEM��ѡ���� */
#define OEM_OPTION_ITEM_OFFSET(x) (OEM_ITEM_OFFSET + OEM_VALID_SIZE + \
                        ((OEM_ITEM_NUM+x) * (OEM_ITEM_SIZE + OEM_ITEM_LEN)))
/* profile������� */

int profile_debug = 0;

#define DBG(arg...) do{if(0){printf("%s,%d:",__FUNCTION__, __LINE__);printf(arg);}}while(0)
#if 0
typedef struct oemItems
{
        UINT8 content[OEM_ITEM_LEN + 1];
        UINT32 size;
        UINT32 maxSize;         /* ����ucGlobalConf.c�е�ȫ�ֱ�����С����,������������ */
} OEM_ITEMS;

typedef enum profile_oem_items
{
    PROFILE_CORPORATION = 0,
    PROFILE_PROMPT,
    PROFILE_DEVICE,
    PROFILE_LOCATION,
    PROFILE_WEBEN,
    PROFILE_WEBCN,
    PROFILE_COPYRIGHT,
    PROFILE_DESCRIPTION,
    PROFILE_END
} PROFILE_ITEMS;


LOCAL OEM_ITEMS gOemItems[OEM_ITEM_NUM] =
{
        {"TP-LINK", 7, 29},/* gCorporation */
        {"TP-LINK", 7, 29},/* bootConfig.c :tPrompt[30] */
        {"TL-SL5428", 9, 29},/* gDevModelName */
        {"SHENZHEN", 8, 29},/* sysDeault_Location */
        {"http://www.tp-link.com", 22, 29},/* sysDeault_Contact */
        {"http://www.tp-link.com.cn", 25, 29},      /* sysDeault_ContactEn */
        {"TP-LINK TECHNOLOGIES CO., LTD.", 30, 39}, /* gCompanyName */
        {"24-Port 10/100Mbps + 4-Port Gigabit L2 Managed Switch", 53, 99}/* gSysDescription */
};

LOCAL int oemItemsLoaded = FALSE;


STATUS loadItems(void)
{
    PROFILE_ITEMS profileItem = PROFILE_CORPORATION;
    int length = 0;
    unsigned char* profileBuff = flashAppInnerGet(FLASH_INNER_ITEM_PROFILE, &length);
    UINT32 oemTag1 = 0;
    UINT32 oemTag2 = 0;
    int offset = 0;
    UINT32 itemSize = 0;

    if (NULL == profileBuff)
    {
        printf("Failed to get profile.\r\n");
        oemItemsLoaded = TRUE;
        return ERROR;
    }

    /* ���profile�в�����OEM����ȡ�����ĳ���С��OEM���ƫ�� */
    /* ����gOemItems��ֵ���䣬����Ĭ��ֵ */
    if (length <= OEM_ITEM_OFFSET_1)
    {
        DBG("There is not any oem item in profile, length=%#x, oem_offset=%#x.\r\n",
            length, OEM_ITEM_OFFSET_1);
        oemItemsLoaded = TRUE;
        free(profileBuff);
        return OK;
    }

    oemTag1 = *(UINT32*)((unsigned char*)profileBuff + OEM_ITEM_OFFSET_1);
    oemTag2 = *(UINT32*)((unsigned char*)profileBuff + OEM_ITEM_OFFSET_2);
    
    DBG("OEM_VALID_WORD1 in flash is 0x%X\r\n", oemTag1);
    DBG("OEM_VALID_WORD2 in flash is 0x%X\r\n", oemTag2);

    if ((OEM_VALID_WORD != oemTag1) && (OEM_VALID_WORD != oemTag2))
    {
        printf("\nCurrent profile is not an OEM profile!\n");
        free(profileBuff);
        oemItemsLoaded = TRUE;
        return ERROR;
    }
    else
    {
        OEM_ITEM_OFFSET = ((OEM_VALID_WORD == oemTag1) ? OEM_ITEM_OFFSET_1 : OEM_ITEM_OFFSET_2);
    }

    for (profileItem = PROFILE_CORPORATION; profileItem < PROFILE_END; profileItem++)
    {
        offset = OEM_ITEM_OFFSET \
                 + OEM_VALID_SIZE + profileItem * (OEM_ITEM_SIZE + OEM_ITEM_LEN);
        
        memcpy((UINT8*)(&itemSize), profileBuff+offset, OEM_ITEM_SIZE);
        itemSize = ntohl(itemSize);
        if (itemSize > gOemItems[profileItem].maxSize)
        {
            DBG("\nInvalid OEM item length. size = %d\r\n", itemSize);
            DBG("\nFailed to load OEM items %d\r\n", profileItem);
            free(profileBuff);
            oemItemsLoaded = TRUE;
            return ERROR;
        }
        else
        {
            memset(gOemItems[profileItem].content, 0, gOemItems[profileItem].maxSize);
            gOemItems[profileItem].size = itemSize;
            if (itemSize != 0)
            {
                offset += OEM_ITEM_SIZE;
                memcpy(gOemItems[profileItem].content, profileBuff + offset, itemSize);
            }
        }
    }
    
    free(profileBuff);
    oemItemsLoaded = TRUE;
    return OK;
}

STATUS _getItem(PROFILE_ITEMS pi, char* buff)
{
    if (NULL == buff)
    {
        printf("Null buff.\r\n");
        return ERROR;
    }

    if ((pi < 0) || (pi > PROFILE_END))
    {
        printf("Unkown oem item.\r\n");
        return ERROR;
    }

    if (oemItemsLoaded == FALSE)
    {
        if (ERROR == loadItems())
        {
            printf("Failed to load oem items.\r\n");
        }
        DBG("Load oem items.\r\n");
    }

    memcpy(buff, gOemItems[pi].content, gOemItems[pi].size + 1);
    return OK;   
}


STATUS getCorporation(char *corporation)
{
    if (ERROR == _getItem(PROFILE_CORPORATION, corporation))
    {
        printf("Failed to get oem corporation.\r\n");
        return FALSE;
    }
    return TRUE;
}
STATUS getDevice(char *device)
{
    if (ERROR == _getItem(PROFILE_DEVICE, device))
    {
        printf("Failed to get oem device.\r\n");
        return FALSE;
    }
    return TRUE;
}
STATUS getDescription(char *description)
{
    if (ERROR == _getItem(PROFILE_DESCRIPTION, description))
    {
        printf("Failed to get oem device.\r\n");
        return FALSE;
    }
    return TRUE;
}
STATUS getWebEn(char *webEn)
{
    if (ERROR == _getItem(PROFILE_WEBEN, webEn))
    {
        printf("Failed to get oem webEn.\r\n");
        return FALSE;
    }
    return TRUE;
}
STATUS getWebCn(char *webCn)
{
    if (ERROR == _getItem(PROFILE_WEBCN, webCn))
    {
        printf("Failed to get oem webCn.\r\n");
        return FALSE;
    }
    return TRUE;
}
STATUS getPrompt(char *prompt)
{
    if (ERROR == _getItem(PROFILE_PROMPT, prompt))
    {
        printf("Failed to get oem prompt.\r\n");
        return FALSE;
    }
    return TRUE;
}
STATUS getCpyright(char *cpyright)
{
    if (ERROR == _getItem(PROFILE_COPYRIGHT, cpyright))
    {
        printf("Failed to get oem cpyright.\r\n");
        return FALSE;
    }
    return TRUE;
}

STATUS getLocation(char *location)
{
    if (ERROR == _getItem(PROFILE_LOCATION, location))
    {
        printf("Failed to get oem location.\r\n");
        return FALSE;
    }
    return TRUE;
}


/*****************************************************************************
* fn            : getOptionOemItem()
* author        : LiuZenglin <LiuZenglin@tp-link.net>
* decription    : ��ȡ��ѡ��OEM��
* param[in]     : index : ��ѡoem��ı�� 1 ~ 50
* param[out]    : strOemItem:�����ѡOEM��124B��������ֹͣ��
* param[in]     : len   : strOemItem������ȣ�Ӧ�ò�����125
*
* return        : TRUE:��ȡ�ɹ�  FALSE:��ȡʧ�ܣ�ֱ�ӷ��ز�������
* retval        : N/A
* note          : N/A
*****************************************************************************/
STATUS getOptionOemItem(UINT32 index, char* strOemItem, UINT32 len)
{
    UINT32 offset = 0;
    UINT32 size = 0;
    UINT32 oemTag = 0;
    int    prfLen = 0;
    unsigned char* profileBuff = flashAppInnerGet(FLASH_INNER_ITEM_PROFILE, &prfLen);

    if (NULL == profileBuff)
    {
        printf("Failed to get profile.\r\n");
        return FALSE;
    }

    if ((index == 0) || (index > 50) ||
        (NULL == strOemItem) || (len == 0))
    {
        free(profileBuff);
        return FALSE;
    }

    /* ȷ��profile���Ƿ�Ԥ����boot param�ֶ� */
    if (oemItemsLoaded == FALSE)
    {
        if (ERROR == loadItems())
        {
            printf("Failed to load oem items.\r\n");
        }
        DBG("Load oem items.\r\n");
    }

    oemTag = *(UINT32*)((unsigned char*)profileBuff + OEM_ITEM_OFFSET);
    
    DBG("OEM_VALID_WORD in flash is 0x%X\r\n", oemTag);

    if (OEM_VALID_WORD != oemTag)
    {
        printf("\nCurrent profile is not an OEM profile!\n");
        free(profileBuff);
        return ERROR;
    }

    index--;    /* ת���ɴ�0��ʼ */
    memset(strOemItem, 0, len);

    offset = OEM_OPTION_ITEM_OFFSET(index);

    memcpy((UINT8*)(&size), profileBuff+offset, OEM_ITEM_SIZE);
    size = ntohl(size);

    if ((0 == size) || (size > 124) )
    {
        DBG("\nInvalid option OEM item length. size = %d\r\n", size);
        free(profileBuff);
        return FALSE;
    }
    else
    {
        offset += OEM_ITEM_SIZE;
        memcpy((UINT8*)(strOemItem), profileBuff+offset, size);
        strOemItem[size] = '\0';
        free(profileBuff);
        return TRUE;
    }
}
#endif

/* Ĭ��buff������profile����ĳ���һ�� by liuzenglin, 22Jun13 */
LOCAL int _getProfileItem(PROFILE_ITEM_TYPE pit, char* buff, int len)
{
    int length = 0;
    unsigned char* profileBuff = flashAppInnerGet(FLASH_INNER_ITEM_PROFILE, &length);

    DBG("Get profile item, pit=%#x, len=%#x, profileMap[pit].offset=%#x, profileMap[pit].length=%#x\r\n", 
        pit, len, profileMap[pit].offset, profileMap[pit].length);
    if (NULL == profileBuff)
    {
        printf("Failed to get profile.\r\n");
        return ERROR;
    }

    if (! IS_VALID_PROFILE_ITEM(pit))
    {
        DBG("Unkown pit, pit=%d\r\n", pit);
        free(profileBuff);
        return ERROR;
    } 

    if(NULL == buff)
    {
        DBG("Null buff, buff=%#x\r\n", (int)buff);
        free(profileBuff);
        return ERROR;
    }

    if (len != profileMap[pit].length)
    {
        //printf("len is %d,pit is %d,profile item length is not same.\r\n",len,pit);
    }

    memcpy(buff, profileBuff+profileMap[pit].offset, len);

    free(profileBuff);

    return OK;    
}

#if 0

/* Ĭ��buff������profile����ĳ���һ�� by liuzenglin, 22Jun13 */
LOCAL int _setProfileItem(PROFILE_ITEM_TYPE pit, const char* buff, int len)
{
    int length = 0;
    unsigned char* profileBuff = flashAppInnerGet(FLASH_INNER_ITEM_PROFILE, &length);

    DBG("Set profile item, pit=%#x, len=%#x, profileMap[pit].offset=%#x, profileMap[pit].length=%#x\r\n", 
        pit, len, profileMap[pit].offset, profileMap[pit].length);

    if (NULL == profileBuff)
    {
        printf("Failed to get profile.\r\n");
        return ERROR;
    }

    if (! IS_VALID_PROFILE_ITEM(pit))
    {
        DBG("Unkown pit, pit=%d\r\n", pit);
        free(profileBuff);
        return ERROR;
    }

    if(NULL == buff)
    {
        DBG("Null buff, buff=%#x\r\n", (int)buff);
        free(profileBuff);
        return ERROR;
    }

    memcpy(profileBuff+profileMap[pit].offset, buff, profileMap[pit].length);

    if (ERROR == flashAppInnerSet(FLASH_INNER_ITEM_PROFILE, profileBuff, length, 0))
    {
        printf("Failed to set profile.\r\n");
        free(profileBuff);
        return ERROR;
    }

    free(profileBuff);

    return OK;

}
#endif


/*!
*\fn                    STATUS sysGetMacFromFlash(unsigned char *s_pro)
*\brief                 get the mac from flash
*
*\param[in]     N/A
*\param[out]    s_pro   store the mac address
*
*\return                error code
*\retval                OK              success
*\retval                ERROR   fail
*
*\note
*/

int sysGetMacFromFlash(unsigned char *s_pro)
{
    unsigned char tmpMac[PROFILE_MAC_LENGTH];

    if(NULL == s_pro)
    {
        return ERROR;
    }

    if (ERROR == _getProfileItem(PROFILE_ITEM_MAC, s_pro, profileMap[PROFILE_ITEM_MAC].length))
    {
        DBG("Failed to get mac from flash.\r\n");
        s_pro[0] = 0x00;
        s_pro[1] = 0x0A;
        s_pro[2] = 0xEB;
        s_pro[3] = 0x00;
        s_pro[4] = 0x13;
        s_pro[5] = 0x01;
        return OK;
    }
    
    memset(tmpMac, 0xff, sizeof(tmpMac));
    if( 0 == memcmp(s_pro, tmpMac, sizeof(tmpMac)) )
    {
        s_pro[0] = 0x00;
        s_pro[1] = 0x0A;
        s_pro[2] = 0xEB;
        s_pro[3] = 0x00;
        s_pro[4] = 0x13;
        s_pro[5] = 0x01;
    }
 #if 0
    else if( s_pro[0] & 0x01)
    {
        s_pro[0] = 0x00;
        s_pro[1] = 0x0A;
        s_pro[2] = 0xEB;
        s_pro[3] = 0x00;
        s_pro[4] = 0x13;
        s_pro[5] = 0x01;
    }
#endif
    return OK;
}



/*!
 *\fn                   STATUS sysGetProductId(char *productId)
 *\brief                get the product id from the flash
 *
 *\param[in]    N/A
 *\param[out]   productId       store the product id
 *
 *\return               error code
 *\retval               TRUE    success
 *\retval               FALSE   fail
 *
 *\note
 */
STATUS sysGetProductId(unsigned char *productId)
{
    if(NULL == productId)
    {
        return ERROR;
    }

    if (ERROR == _getProfileItem(PROFILE_ITEM_PRODUCTID, productId, profileMap[PROFILE_ITEM_PRODUCTID].length))
    {
        printf("Failed to get productID.\r\n");
        return ERROR;
    }    

    return OK;
}
#if 0
/*!
 *\fn                   STATUS sysGetFlashSign(char *s_pro)
 *\brief                get the digit sign from flash
 *
 *\param[in]    N/A
 *\param[out]   s_pro   store the digit sign
 *\return               error code
 *\retval               OK              success
 *\retval               ERROR   fail
 *
 *\note
 */
STATUS sysGetFlashSign(char *s_pro, int len)
{
    if(NULL == s_pro)
    {
        return ERROR;
    }

    if (ERROR == _getProfileItem(PROFILE_ITEM_SIGN, s_pro, len))
    {
        printf("Failed to get sign.\r\n");
        return ERROR;
    }    

    return OK;

}

/*!
 *\fn                   STATUS sysGetFlashPubLicKey(char *s_pro)
 *\brief                get the public key from flash
 *
 *\param[in]    N/A
 *\param[out]   s_pro   store the digit sign
 *\return               error code
 *\retval               OK              success
 *\retval               ERROR   fail
 *
 *\note
 */
STATUS sysGetFlashPubLicKey(char *s_pro, int len)
{
    if(NULL == s_pro)
    {
        return ERROR;
    }

    if (ERROR == _getProfileItem(PROFILE_ITEM_PUBLICKEY, s_pro, len))
    {
        printf("Failed to get publickey.\r\n");
        return ERROR;
    }    

    return OK;
}

/*!
 *\fn                   STATUS sysSetMacToFlash(unsigned char *s_pro)
 *\brief                set the mac address to flash
 *
 *\param[in]    s_pro   the pointer of the mac address
 *\param[out]   N/A
 *
 *\return               error code
 *\retval               ERROR   fail
 *\retval               OK              success
 *
 *\note mac��ַ��Ҫֱ��д�룬����ͨ���ڲ����Ľӿ�д���ڲ����ӿڻ��Զ�����mac��rsa��д
 */
STATUS sysSetMacToFlash(unsigned char *s_pro)
{
    int valueOffset = 0;
    if(NULL == s_pro)
    {
        return ERROR;
    }
    
    if (ERROR == flashAppInnerGetOffset(FLASH_INNER_ITEM_PROFILE, &valueOffset))
    {
        printf("Fatal error: Failed to set gev content offset.\r\n");
        return ERROR; 
    }

    if (ERROR == writeFlash(s_pro, 
        profileMap[PROFILE_ITEM_MAC].length, valueOffset+profileMap[PROFILE_ITEM_MAC].offset, FALSE))
    {
        printf("Failed to set mac.\r\n");
        return ERROR;
    }    

    return OK;
}


/*!
 *\fn                   STATUS sysSetProductId(char *productId)
 *\brief                get the product id from the flash
 *
 *\param[in]    N/A
 *\param[out]   productId       store the product id
 *
 *\return               error code
 *\retval               TRUE    success
 *\retval               FALSE   fail
 *
 *\note
 */
STATUS sysSetProductId(unsigned char *productId)
{
    if(NULL == productId)
    {
        return ERROR;
    }

    if (ERROR == _setProfileItem(PROFILE_ITEM_PRODUCTID, productId, profileMap[PROFILE_ITEM_PRODUCTID].length))
    {
        printf("Failed to set productid.\r\n");
        return ERROR;
    }    

    return OK;
}

/*!
 *\fn                   STATUS sysSetFlashPublicKey(char *s_pro,int len)
 *\brief                set the public key to flash
 *
 *\param[in]
 *\param[out]
 *
 *\return
 *\retval
 *
 *\note
 */
STATUS sysSetFlashPublicKey(char *s_pro, int len)
{
    if(NULL == s_pro)
    {
        return ERROR;
    }

    if (ERROR == _setProfileItem(PROFILE_ITEM_PUBLICKEY, s_pro, len))
    {
        printf("Failed to set publickey.\r\n");
        return ERROR;
    }    

    return OK;
}

/*!
 *\fn                   STATUS sysSetFlashSign(char *s_pro,int len)
 *\brief                set the flash digit sign to flash
 *
 *\param[in]
 *\param[out]
 *
 *\return
 *\retval
 *
 *\note sign��Ҫֱ��д�룬����ͨ���ڲ����Ľӿ�д���ڲ����ӿڻ��Զ�����mac��rsa��д
 */
STATUS sysSetFlashSign(char *s_pro, int len)
{
    int valueOffset = 0;

    if(NULL == s_pro)
    {
        return ERROR;
    }
    
    if (ERROR == flashAppInnerGetOffset(FLASH_INNER_ITEM_PROFILE, &valueOffset))
    {
        printf("Fatal error: Failed to get gev content offset.\r\n");
        return ERROR; 
    }

    if (ERROR == writeFlash(s_pro, 
        profileMap[PROFILE_ITEM_SIGN].length, valueOffset+profileMap[PROFILE_ITEM_SIGN].offset, FALSE))
    {
        printf("Failed to set sign .\r\n");
        return ERROR;
    }
    return OK;
}

#endif

/*!
 *\fn           LOCAL UINT32 sysFirmwareCheckProduct(const unsigned char *pAppBuf)
 *\brief        check the product version of the firmware file.
 *
 *\param[in]    pAppBuf  pointer to the buffer
 *\param[out]   N/A
 *
 *\return       the product version identical with the switch or not
 *\retval       OK    the product version identical with the switch
 *\retval       ERROR the product version different from the switch
 *
 *\note
 */
UINT32 sysFirmwareCheckProduct(const unsigned char *pAppBuf)
{
    unsigned char curProdId[PROFILE_PRODUCTID_LENGTH];

    if (ERROR == sysGetProductId(curProdId))
    {
        return ERROR;
    }
    if( memcmp(pAppBuf, curProdId, PROFILE_PRODUCTID_LENGTH) )
    {
        return ERROR;
    }

    return OK;
}


