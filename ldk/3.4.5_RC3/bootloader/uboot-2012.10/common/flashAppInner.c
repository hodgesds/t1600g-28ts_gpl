/******************************************************************************
*
* Copyright (c) 2010 TP-LINK Technologies CO.,LTD.
* All rights reserved.
*
* file name  :   flashAppInner.c
* version    :   V1.0
* description:   �����ڲ����Ĳ��ֺͲ����ӿ�.
*
* author     :   LiuZenglin <LiuZenglin@tp-link.net>
* data       :   06/21/2013
*
* history    :
* 01   06/21/2013  LiuZenglin     Create.
* �ڲ������ݲ�ȡ����TLV�ĸ�ʽ�洢�������ڲ�����Ŀ��ÿ����Ŀ�ĳ��ȶ���1���򼸸�
* sector����profile��Ŀ�ܹ�ռ128K bytes��ʽ����:
* 4B    4B   4B     value_len   pad_len
* ----- ---- ------ ---------   -------
* valid type length value       FF
* 
* ���ļ������е�offset����ָ���FLASHͷ��ַ��ƫ��
* �ڲ�������Ч��Ŀ��������һ��ΪȫFF��sector�������ȡʱ��������
******************************************************************************/
#include<common.h>
#include<vsprintf.h>
#include <tplink/flashApp.h>
#include <tplink/flashAppInner.h>
#include <tplink/flashAppInnerGev.h>


int flash_app_inner_debug = 0;

#define DBG(arg...) do{if(0){printf("%s,%d:",__FUNCTION__, __LINE__);printf(arg);}}while(0)

#define IF_FAIL_RET(expression) \
	do{\
		int __rv;\
		__rv = (expression);\
		if ( ERR_NO_ERROR != __rv ){\
			DBG("E - %d %s line %d\n",__rv,#expression,__LINE__);\
			return __rv;\
		}\
	}while(0)

#define IF_FAIL_DONE(ret,expression) \
	do{\
		ret = (expression);\
		if ( ERR_NO_ERROR != ret ){\
			DBG("E - %d %s line %d\n",ret,#expression,__LINE__);\
			goto done;\
		}\
	}while(0)
        
#define IF_FAIL_PRINT(expression) \
            do{\
                int __rv;\
                __rv = (expression);\
                if ( ERR_NO_ERROR != __rv ){\
                    DBG("E - %d %s line %d\n",__rv,#expression,__LINE__);\
                }\
            }while(0)

/* д��ʱ�Ͷ���ʱ����ͬ���ֽ��� */
#define FLASH_INNER_ITEM_VALID_KEY  (0xAA55AA55)
#define FLASH_INNER_DFT_SIZE        (0x300000)
#define FLASH_INNER_ITEM_KEY_SIZE        (4)
#define FLASH_INNER_ITEM_TYPE_SIZE       (4)
#define FLASH_INNER_ITEM_LENGTH_SIZE     (4)
#define FLASH_INNER_ITEM_VALUE_OFFSET    (12)
#define FLASH_FIRST_INNER_ITEM      (FLASH_SIZE)    /* ��һ����Ŀ�����ʼ */
/* ����ֵ�ĳ�������Ŀ�ĳ��� */
#define FLASH_INNER_ITEM_LENGTH(length)      (ROUND_SECTOR_UP(length+FLASH_INNER_ITEM_VALUE_OFFSET))

#define GET_FLASH_INNER_ITEM_VALID_KEY(offset, key) \
    readFlash((char*)key, FLASH_INNER_ITEM_KEY_SIZE, ROUND_ADDR_UP(offset-FLASH_INNER_ITEM_KEY_SIZE))
#define SET_FLASH_INNER_ITEM_VALID_KEY(offset, key) \
    writeFlash((char*)key, FLASH_INNER_ITEM_KEY_SIZE, ROUND_ADDR_UP(offset-FLASH_INNER_ITEM_KEY_SIZE), 0)
#define GET_FLASH_INNER_ITEM_TYPE(offset, type) \
    readFlash((char*)type, FLASH_INNER_ITEM_TYPE_SIZE, \
    ROUND_ADDR_UP(offset-FLASH_INNER_ITEM_KEY_SIZE-FLASH_INNER_ITEM_TYPE_SIZE))
#define SET_FLASH_INNER_ITEM_TYPE(offset, type) \
    writeFlash((char*)type, FLASH_INNER_ITEM_TYPE_SIZE, \
    ROUND_ADDR_UP(offset-FLASH_INNER_ITEM_KEY_SIZE-FLASH_INNER_ITEM_TYPE_SIZE), 0)
#define GET_FLASH_INNER_ITEM_LENGTH(offset, length) \
    readFlash((char*)length, FLASH_INNER_ITEM_LENGTH_SIZE, \
    ROUND_ADDR_UP(offset-FLASH_INNER_ITEM_KEY_SIZE-FLASH_INNER_ITEM_TYPE_SIZE-FLASH_INNER_ITEM_LENGTH_SIZE))
#define SET_FLASH_INNER_ITEM_LENGTH(offset, length) \
    writeFlash((char*)length, FLASH_INNER_ITEM_LENGTH_SIZE, \
    ROUND_ADDR_UP(offset-FLASH_INNER_ITEM_KEY_SIZE-FLASH_INNER_ITEM_TYPE_SIZE-FLASH_INNER_ITEM_LENGTH_SIZE), 0)

/* �����������ʽ�����ڲ��������ݣ��ڶ�ȡ�ڲ�����ʱ��ʼ��fii_head */
typedef struct _FLASH_INNER_ITEM
{
    FLASH_INNER_ITEM_TYPE type;
    int                   value_offset;     /* ֵ��ƫ���� */
    int                   length;
    unsigned char*        pBuf;             /* ɾ���ڲ���ʱʹ�ã���ʼ��ʱΪNULL */
}FLASH_INNER_ITEM;

typedef struct _FII_NODE
{
    FLASH_INNER_ITEM    flInnerItem;
    struct _FII_NODE*   next;
}FII_NODE;

/* ��¼�ڲ����ı߽�ƫ�ƣ�����ˢ���ڲ��� */
LOCAL int flashInnerEdgeOffset = 0;
LOCAL FII_NODE *fii_head;
LOCAL int flashAppInnerInited = FALSE;
LOCAL void l_freeFlashInnerItems(void)      /* �ͷ����� */
{
    FII_NODE* pFree = fii_head;
    FII_NODE* pTemp = NULL;
    while (pFree != NULL)
    {
        pTemp = pFree->next;
        if (pFree->flInnerItem.pBuf != NULL)
        {
            free(pFree->flInnerItem.pBuf);
        }
        free(pFree);
        pFree = pTemp;
    }
    fii_head = NULL;
    return;
}

/* ��ȡ����ʼ��fii_list��ֻ�������� */
int flashAppInnerInit_patch(void)
{
    const int valid_key = FLASH_INNER_ITEM_VALID_KEY;
    int key = 0;
    int item_offset = FLASH_FIRST_INNER_ITEM;   /* �ڲ�����Ŀ����ʼλ�� */
    int value_offset = 0;                       /* ֵ�ֶε���ʼλ�� */
    int type = 0;
    int length = 0;
    FII_NODE* pCur = NULL;    /* ��ǰ��������ڵ� */
    FII_NODE* pParent = NULL;
    int count = 0;

    if (fii_head != NULL)
    {
        DBG("Reinit the inner item list.\r\n");
        l_freeFlashInnerItems();
    }

    flashInnerEdgeOffset = FLASH_FIRST_INNER_ITEM;

    GET_FLASH_INNER_ITEM_VALID_KEY(item_offset, &key);
    DBG("count=%d, key=%#x, item_offset=%#x\r\n", count, key, item_offset);
    while (key == valid_key)
    {
        GET_FLASH_INNER_ITEM_TYPE(item_offset, &type);
        //type = ntohl(type);
        GET_FLASH_INNER_ITEM_LENGTH(item_offset, &length);
        //length = ntohl(length);

        /* �����ڲ����ȱ��� */
        value_offset = ROUND_ADDR_DOWN(item_offset - FLASH_INNER_ITEM_VALUE_OFFSET - length);

        DBG("%d Type %#x, item_offset %#x, length %#x, value_offset %#x\r\n", 
            count, type, item_offset, length, value_offset);
        pCur = (FII_NODE*)calloc(1, sizeof(FII_NODE));
        if (pCur == NULL)
        {
            printf("Fatal error: Failed to allocate memory.\r\n");
            l_freeFlashInnerItems();
            flashAppInnerInited = TRUE;
            return ERROR;
        }

        pCur->flInnerItem.length = length;
        pCur->flInnerItem.type   = type;
        pCur->flInnerItem.value_offset = value_offset;
        pCur->flInnerItem.pBuf = NULL;      /* ���� */
        pCur->next = NULL;

        if (count == 0)
        {
            fii_head = pCur;
            pParent  = fii_head;
            pCur = NULL;
        }
        else
        {
            pParent->next = pCur;
            pParent = pCur;
            pCur = NULL;
        }
        count++;

        /* ֵ�ֶε���ʼλ��Ҳ����һ����Ŀ�Ŀ�ʼ�� */
        item_offset = ROUND_SECTOR_DOWN(value_offset); /* �ڲ�����Ŀ�ı߽綼Ӧ����sector���� */
        flashInnerEdgeOffset = item_offset;
        GET_FLASH_INNER_ITEM_VALID_KEY(item_offset, &key);
        DBG("count=%d, key=%#x, item_offset=%#x\r\n", count, key, item_offset);
    }

    flashAppInnerInited = TRUE;
    return OK;
}

void flashAppInnerEnum(int *sn,int *license)
{
	*sn = FLASH_INNER_SN;
	*license = FLASH_INNER_LICENSE;
}
/* �������ʱ�����ܳ���gev���򱻲�����ʹ��profile�������������ʧ��
 * ��֤GEV���Ϊ���һ���ڲ�����Ŀ�����޸�bootrom��ǰ���¶����ᶪʧ�������� */
int flashAppInnerInit(void)
{
    int offset = 0;
    int sectorSize = flashGetSectorSize();
    char* pbuf = NULL;
    int key,type = 0;
    unsigned int length,size = 0;
    const int valid_key = FLASH_INNER_ITEM_VALID_KEY;
    int temp = 0;
    int valid_edge = FLASH_FIRST_INNER_ITEM;
    int gev_edge = -1;

    /* ��֤GEV�������һ���ڲ�����Ŀ*/
    GET_FLASH_INNER_ITEM_VALID_KEY(FLASH_FIRST_INNER_ITEM, &key);
    GET_FLASH_INNER_ITEM_TYPE(FLASH_FIRST_INNER_ITEM, &type);
    GET_FLASH_INNER_ITEM_LENGTH(FLASH_FIRST_INNER_ITEM, &length);
    DBG("Inner items precheck: key=%#x, type=%#x, length=%#x, item_offset=%#x\r\n", 
        key, type, length, FLASH_FIRST_INNER_ITEM);
    while ((key == valid_key) && (offset < FLASH_INNER_DFT_SIZE) && (length > 0))
    {
        if (type == FLASH_INNER_ITEM_GEV)
        {
            gev_edge = FLASH_FIRST_INNER_ITEM - offset - ROUND_SECTOR_UP(length);
        }
        valid_edge = FLASH_FIRST_INNER_ITEM - offset - ROUND_SECTOR_UP(length);
        offset = offset + ROUND_SECTOR_UP(length);


        GET_FLASH_INNER_ITEM_VALID_KEY(FLASH_FIRST_INNER_ITEM-offset, &key);
        GET_FLASH_INNER_ITEM_TYPE(FLASH_FIRST_INNER_ITEM-offset, &type);
        GET_FLASH_INNER_ITEM_LENGTH(FLASH_FIRST_INNER_ITEM-offset, &length);
        DBG("Inner items precheck: key=%#x, type=%#x, length=%#x, item_offset=%#x\r\n", 
                key, type, length, FLASH_FIRST_INNER_ITEM-offset);
    }
    


    DBG("valid edge is %#x, gev edge is %#x\r\n", valid_edge, gev_edge);
    if (valid_edge == gev_edge)   /* GEV�����һ���ڲ��� */
    {
        DBG("Gev is stored in the last inner item\r\n");
    }
    else
    {
        /* gev�����ڣ��������һ���ڲ���Ŀ��д��Ĭ��gev */
        if (gev_edge == -1)
        {
            gev_edge = valid_edge - sectorSize;
            flashErase(gev_edge/sectorSize, 1);
            if (FLASH_FIRST_INNER_ITEM - gev_edge > FLASH_INNER_DFT_SIZE)
            {
                printf("Fatal error: inner area is full, valid_edge is %#x\r\n", valid_edge);
                return ERROR;
            }
            
            pbuf = getDftGevContent(&temp);
            if (pbuf == NULL)
            {
                printf("Fatal error: get default gev content failed.\r\n");
                return ERROR;
            }
            size = temp;

            /* ���ó��� */
            SET_FLASH_INNER_ITEM_LENGTH(valid_edge, &temp); 
            temp = FLASH_INNER_ITEM_GEV;
            SET_FLASH_INNER_ITEM_TYPE(valid_edge, &temp);  
            SET_FLASH_INNER_ITEM_VALID_KEY(valid_edge, &valid_key); 
        }
        else    /* ���ڣ����ֻ� */
        {

            size = gev_edge-valid_edge+sectorSize;
            //pbuf = (char*)calloc(size, 1);
            pbuf = BUF_ADDR;
            if (pbuf == NULL)
            {
                printf("Fatal error: alloc memory failed.\r\n");
                return ERROR;
            }
            readFlash(pbuf, sectorSize, gev_edge);
            readFlash(pbuf+sectorSize, size-sectorSize, valid_edge);

            gev_edge = valid_edge;
        }

        /* д���ڲ��� */
        DBG("pbuf=%#x, size=%#x, gev_edge=%#x\r\n", (int)pbuf, size, gev_edge);
        writeFlash(pbuf, size, gev_edge, FALSE);

        //free(pbuf);
        pbuf = NULL;
    }

    return flashAppInnerInit_patch();
}

int flashAppInnerSet(FLASH_INNER_ITEM_TYPE type, char* buff, int len, int prompt)
{
    FII_NODE *pCur = NULL;
    FII_NODE *pParent = NULL;
    unsigned char * pbuf1 = NULL;   /* ��������ֵ */
    unsigned char * ptemp = NULL;
    unsigned char * pbuf2 = NULL;   /* ���ɽ�Ҫд���ֵ */
    int oldSize = 0;                /* ��д֮ǰ�ڲ����ĳ��� */
    int newSize = 0;                /* ��дʱ�ڲ����ĳ��� */
    int itemStartOffset = 0;        /* ��д�ڲ�����Ŀ����ʼƫ�� */
    int itemNextStartOffset = 0;    /* ��д�ڲ�����Ŀ����һ������ʼƫ�� */
    int itemOldLength = 0;          /* ��д�ڲ�����Ŀ��ԭ���� */
    int itemNewLength = 0;          /* ��д�ڲ�����Ŀ���³��� */
    int tempSize = 0;
    int sectorSize = FLASH_SECTOR_SIZE;
    const int valid_key = FLASH_INNER_ITEM_VALID_KEY;

    if ((NULL == buff) || (0 == len))
    {
        printf("Fatal error: buff=%#x, len=%#x", (int)buff, len);
        return ERROR;
    }

    if (flashAppInnerInited == FALSE)
    {
        if (ERROR == flashAppInnerInit())
        {
            printf("Fatal error: Failed to init inner area structure.\r\n");
            return ERROR;
        }
    }

    pParent = fii_head;
    pCur = fii_head;

    while (pCur != NULL)
    {
        DBG("flashAppInnerSet: type=%#x, pCur->flInnerItem.type=%#x\r\n",
            type, pCur->flInnerItem.type);
        if (pCur->flInnerItem.type == type)
        {
            DBG("Modify the inner item, type %d, length %d, value_offset %d.\r\n",
                type, len, pCur->flInnerItem.value_offset);

            DBG("Rewrite the inner area, new length is %#x, old length is %#x\r\n",
                len, pCur->flInnerItem.length);
            /* flash�����в�����д��ͬ���ݣ����Զ����������ڲ��� */
            goto REFRESH_INNER_AREA;
            break;
        }
        else
        {
            pParent = pCur;
            pCur = pCur->next;
        }
    }

    if (pCur == NULL)
    {
        DBG("Write a new item to flash inner, type %d, length %d.\r\n", type, len);
    }

    itemNewLength = FLASH_INNER_ITEM_LENGTH(len);
    oldSize = FLASH_FIRST_INNER_ITEM - flashInnerEdgeOffset;
    newSize = oldSize + itemNewLength;
    itemStartOffset = FLASH_FIRST_INNER_ITEM - oldSize;

    DBG("itemnewlen=%#x,itemstartoffset=%#x,oldsize=%#x,newsize=%#x\r\n",
        itemNewLength,itemStartOffset,oldSize,newSize+ sectorSize);

    /* ��������д��һ����Ŀ��Ϊ�˱�����ʷ��Ϣ��Ӱ�죬�Ѻ���һ��sectorд��ȫFF */
    //pbuf1 = (unsigned char*)calloc(1, newSize+sectorSize);
    pbuf1 = BUF_ADDR;
    memset(pbuf1, 0, (newSize+sectorSize));
    if (NULL == pbuf1)
    {
        printf("Fatal error: Failed to allocate memory for write inner area.\r\n");
        return ERROR;
    }
    memset(pbuf1, 0xff, newSize+sectorSize);
    ptemp = pbuf1 + sectorSize;
    
    if (0 != oldSize)   /* �ڲ�����Ϊ�� */
    {
        /* ��ȡ���һ��gev */
        if (ERROR == readFlash(ptemp, sectorSize, itemStartOffset))
        {
            //free(pbuf1);
            printf("Fatal error: Failed to read inner area.\r\n");
            return ERROR;        
        }

        if (ERROR == readFlash(ptemp + sectorSize + itemNewLength, oldSize-sectorSize, itemStartOffset+sectorSize))
        {
            //free(pbuf1);
            printf("Fatal error: Failed to read inner area.\r\n");
            return ERROR;        
        }

        ptemp = ptemp+sectorSize+itemNewLength; /* ��item��ƫ�� */
    }
    else    /* ���ڲ��� */
    {
        /* ����Ҫ��ȡ�κα��� */
        ptemp = ptemp+itemNewLength;
    }

    /* д��KEY��buffer */
    memcpy(ptemp-FLASH_INNER_ITEM_KEY_SIZE, (unsigned char*)(&valid_key), FLASH_INNER_ITEM_KEY_SIZE);

    /* д��TYPE��buffer */
    //type = ntohl(type);
    memcpy(ptemp-FLASH_INNER_ITEM_KEY_SIZE-FLASH_INNER_ITEM_TYPE_SIZE, 
        (unsigned char*)(&type), FLASH_INNER_ITEM_TYPE_SIZE);

    /* д��LENGTH��buffer */
    //int wlen = ntohl(len);
    memcpy(ptemp-FLASH_INNER_ITEM_KEY_SIZE-FLASH_INNER_ITEM_TYPE_SIZE-FLASH_INNER_ITEM_LENGTH_SIZE, 
        (unsigned char*)(&len), FLASH_INNER_ITEM_LENGTH_SIZE);
    /* д��VALUE��buffer */
    memcpy(ptemp-FLASH_INNER_ITEM_KEY_SIZE-FLASH_INNER_ITEM_TYPE_SIZE-FLASH_INNER_ITEM_LENGTH_SIZE-len,
        buff, len);

    /* д������Ŀʱ�����뱣֤����һ��sectorΪȫFF */
    flashInnerEdgeOffset = FLASH_FIRST_INNER_ITEM-newSize;
    flashErase((flashInnerEdgeOffset-sectorSize)/sectorSize, (newSize+sectorSize)/sectorSize);
    if (ERROR == writeFlash(pbuf1, newSize+sectorSize, flashInnerEdgeOffset-sectorSize, prompt))
    {
        //free(pbuf1);
        printf("Fatal error: Failed to write inner area.\r\n");
        return ERROR;
    }    
    DBG("\r\nSet inner item ok, type=%d startOffset=%#x length=%d flashInnerEdgeOffset=%#x.\r\n", 
        type, itemStartOffset, len, flashInnerEdgeOffset);

    //free(pbuf1);
    flashAppInnerInited = FALSE;    /* ��Ҫ���³�ʼ�� */

    return OK;

REFRESH_INNER_AREA:
    itemOldLength = FLASH_INNER_ITEM_LENGTH(pCur->flInnerItem.length);
    itemNewLength = FLASH_INNER_ITEM_LENGTH(len);
    itemNextStartOffset = ROUND_SECTOR_DOWN(pCur->flInnerItem.value_offset);
    itemStartOffset = itemNextStartOffset + itemOldLength;
    oldSize = FLASH_FIRST_INNER_ITEM - flashInnerEdgeOffset;
    newSize = oldSize - itemOldLength + itemNewLength;
    DBG("itemoldlen=%#x,itemnewlen=%#x,itemstartoffset=%#x,itemnextstartoffset=%#x,oldsize=%#x,newsize=%#x\r\n",
        itemOldLength,itemNewLength,itemStartOffset,itemNextStartOffset,oldSize,newSize);
    //pbuf1 = (unsigned char*)calloc(1, oldSize);
    pbuf1 = BUF_ADDR;
    if (NULL == pbuf1)
    {
        printf("Fatal error: Failed to allocate memory for write inner area, size=%#x.\r\n", oldSize);
        return ERROR;
    }
    //pbuf2 = (unsigned char*)calloc(1, newSize);
    pbuf2 = BUF_ADDR + oldSize;
    if (NULL == pbuf2)
    {
        //free(pbuf1);
        printf("Fatal error: Failed to allocate memory for write inner area, size=%#x.\r\n", newSize);
        return ERROR;
    }

    if (ERROR == readFlash(pbuf1, oldSize, flashInnerEdgeOffset))
    {
        //free(pbuf1);
        //free(pbuf2);
        printf("Fatal error: Failed to read inner area.\r\n");
        return ERROR;        
    }
    memset(pbuf2, 0xff, newSize);

    /* ����Ҫ�޸ĵ��ڲ����ǰ�����ݿ������µ�ַ�� */
    tempSize = itemNextStartOffset-flashInnerEdgeOffset;
    memcpy(pbuf2 , pbuf1, tempSize);

    /* KEY��TYPE�ںϲ��󲿷�ʱд�룬���ֲ��� */
    /* ���³��� */
    //len = ntohl(len);
    memcpy(pbuf2+tempSize+itemNewLength\
    -FLASH_INNER_ITEM_KEY_SIZE-FLASH_INNER_ITEM_TYPE_SIZE-FLASH_INNER_ITEM_LENGTH_SIZE,\
    (unsigned char*)(&len), FLASH_INNER_ITEM_LENGTH_SIZE);
    /* ���������ݵ�ָ����ַ */
    memcpy(pbuf2+tempSize+itemNewLength\
    -FLASH_INNER_ITEM_KEY_SIZE-FLASH_INNER_ITEM_TYPE_SIZE-FLASH_INNER_ITEM_LENGTH_SIZE-len,\
    buff, len);

    /* ���ΪPROFILE����Ҫ����MAC��ַ������ǩ�� */
    if (pCur->flInnerItem.type == FLASH_INNER_ITEM_PROFILE)
    {
       
        /* MAC��RSAΪǰ���6+128���ַ� */
        memcpy(pbuf2+tempSize+itemNewLength\
    -FLASH_INNER_ITEM_KEY_SIZE-FLASH_INNER_ITEM_TYPE_SIZE-FLASH_INNER_ITEM_LENGTH_SIZE-len,\
        pbuf1+tempSize+itemOldLength\
    -FLASH_INNER_ITEM_KEY_SIZE-FLASH_INNER_ITEM_TYPE_SIZE-FLASH_INNER_ITEM_LENGTH_SIZE-len,\
        6+128);
    }


    /* ����Ҫ�޸ĵ��ڲ���ĺ������ݿ������µ�ַ�� */
    /* ����TYPE��key */
    memcpy(pbuf2+tempSize+itemNewLength-FLASH_INNER_ITEM_KEY_SIZE-FLASH_INNER_ITEM_TYPE_SIZE , 
    pbuf1+tempSize+itemOldLength-FLASH_INNER_ITEM_KEY_SIZE-FLASH_INNER_ITEM_TYPE_SIZE, 
        FLASH_FIRST_INNER_ITEM-itemStartOffset+FLASH_INNER_ITEM_KEY_SIZE+FLASH_INNER_ITEM_TYPE_SIZE);

    flashInnerEdgeOffset = FLASH_FIRST_INNER_ITEM-newSize;

    /* �����Ŀ��С�����ں������0xff, ʹ��buf1������д�� */
    if (newSize < oldSize)
    {
        unsigned char* temp = pbuf1;
        memset(pbuf1, 0xff, oldSize);
        memcpy(pbuf1, pbuf2, newSize);
        /* ����ָ���ֵ�����ں���ͳһ���� */
        pbuf1 = pbuf2;
        pbuf2 = temp;
        newSize = oldSize;
    }
    //free(pbuf1);

    flashErase((FLASH_FIRST_INNER_ITEM-newSize)/sectorSize, newSize/sectorSize);
    if (ERROR == writeFlash(pbuf2, newSize, FLASH_FIRST_INNER_ITEM-newSize, prompt))
    {
        //free(pbuf2);
        printf("Fatal error: Failed to write inner area.\r\n");
        return ERROR;
    }

    DBG("Set inner item ok, type=%d length=%d, flashInnerEdgeOffset=%#x.\r\n", 
        type, len, flashInnerEdgeOffset);

    //free(pbuf2);

    /* ����Ŀ���Ȳ�һ��ʱ��Ҫ���»�ȡ�ڲ����Ĳ��� */
    if (len != pCur->flInnerItem.length)
    {
        flashAppInnerInited = FALSE;
    }

    return OK;
}

/* ɾ���ڲ���Ŀ */
int flashAppInnerUnset(FLASH_INNER_ITEM_TYPE type, int prompt)
{
    FII_NODE *pCur = NULL;
    FII_NODE *pParent = NULL;
    FII_NODE *pTempParent = NULL;
    FII_NODE *pTemp = NULL;
    unsigned int valid_key = 0;
    int len = 0;
    int  innerItemSize = 0;
    int  innerItemWriteOffset = 0;          /* дflash��ƫ�Ƶ�ַ */
    unsigned char* innerItemBuf = 0;

    if (flashAppInnerInited == FALSE)
    {
        if (ERROR == flashAppInnerInit())
        {
            printf("Fatal error: Failed to init inner area structure.\r\n");
            return ERROR;
        }
    }

    /* �������нڵ� */
    pCur = fii_head;
    pParent = fii_head;
    while (pCur != NULL)
    {
        DBG("Unset inner item, type=%#x, pCur type=%#x\r\n", type, pCur->flInnerItem.type);
        if (pCur->flInnerItem.type == type)
        {
            break;
        }
        else
        {
            pParent = pCur;
            pCur = pCur->next;
        }
    }

    if (pCur == NULL)
    {
        DBG("Inner item is not exist, type=%#x.\r\n", type);
    }
    else
    {
        DBG("Remove inner item, type=%#x inner_item_len=%#x value_offset=%#x.\r\n",
            pCur->flInnerItem.type, pCur->flInnerItem.length, pCur->flInnerItem.value_offset);
        /* ��ʱ�Ѿ�ȷ���ڲ�������ˣ���Ҫɾ������ */
        pTempParent = pParent;
        innerItemWriteOffset = (pCur == fii_head)? \
            FLASH_FIRST_INNER_ITEM : ROUND_SECTOR_DOWN(pParent->flInnerItem.value_offset);
        pTemp = pCur;   /* ����Ҫɾ���Ľڵ� */

        /* �����������нڵ㣬����ȡ��Ӧֵ */
        pParent = pCur;
        pCur = pCur->next;
        while (pCur != NULL)
        {
            DBG("Get inner item in unsetting, type=%#x\r\n", pCur->flInnerItem.type);

            /* �ⲿ���ڴ���д��FLASH֮����ͷ� */
            pCur->flInnerItem.pBuf = (unsigned char*)calloc(1, pCur->flInnerItem.length);
            if (pCur->flInnerItem.pBuf == NULL)
            {
                printf("Fatal error: Failed to allocate memory, size=%#x.\r\n", pCur->flInnerItem.length);
                return ERROR;
            }
            readFlash(pCur->flInnerItem.pBuf, pCur->flInnerItem.length, pCur->flInnerItem.value_offset);

            pParent = pCur;
            pCur = pCur->next;
        }

        DBG("Remove inner item, refresh flash.\r\n");

        /* ��Ҫ���³�ʼ�� */
        flashAppInnerInited = FALSE;
        pParent = pTempParent;
        pCur = pTemp->next;

        /* ���û�к�����Ŀ�������õ�ǰ��Ϊȫ0XFF */
        if (pCur == NULL)
        {
            //innerItemBuf = (unsigned char*)calloc(1, FLASH_SECTOR_SIZE);
            innerItemBuf = BUF_ADDR;
            if (NULL == innerItemBuf)
            {
                printf("Fatal error: Failed to allocate memory for write inner area.\r\n");
                return ERROR;
            }
            memset(innerItemBuf, 0xff, FLASH_SECTOR_SIZE);

            DBG("Clear current item.\r\n");                
            if (ERROR == writeFlash(innerItemBuf, FLASH_SECTOR_SIZE, innerItemWriteOffset-FLASH_SECTOR_SIZE, TRUE))
            {
                printf("\r\nError: Failed to write flash, innerItemBuf=%#x, innerItemSize=%#x, innerItemStartOffset=%#x\r\n",
                    (int)innerItemBuf, innerItemSize, innerItemWriteOffset);
            }
            DBG("\r\n");
        }
        else
        {
            /* �Ѻ������Ŀд��FLASH */
            while (pCur != NULL)
            {
                if (NULL != pCur->flInnerItem.pBuf)
                {
                    innerItemSize = FLASH_INNER_ITEM_LENGTH(pCur->flInnerItem.length);
                    //innerItemBuf = (unsigned char*)calloc(1, innerItemSize+FLASH_SECTOR_SIZE);
                    innerItemBuf = BUF_ADDR;
                    if (NULL == innerItemBuf)
                    {
                        printf("Fatal error: Failed to allocate memory for write inner area.\r\n");
                        return ERROR;
                    }

                    /* ����д���ƫ�Ƶ�ַ */
                    innerItemWriteOffset -= innerItemSize;
                    
                    memset(innerItemBuf, 0xff, innerItemSize+FLASH_SECTOR_SIZE);

                    /* д��KEY��buffer */
                    valid_key = FLASH_INNER_ITEM_VALID_KEY;
                    memcpy(innerItemBuf+FLASH_SECTOR_SIZE+innerItemSize-FLASH_INNER_ITEM_KEY_SIZE, (unsigned char*)(&valid_key), FLASH_INNER_ITEM_KEY_SIZE);
                    /* д��TYPE��buffer */
                    //type = ntohl(pCur->flInnerItem.type);
                    memcpy(innerItemBuf+FLASH_SECTOR_SIZE+innerItemSize-FLASH_INNER_ITEM_KEY_SIZE-FLASH_INNER_ITEM_TYPE_SIZE, 
                        (unsigned char*)(&type), FLASH_INNER_ITEM_TYPE_SIZE);
                    /* д��LENGTH��buffer */
                    //len = ntohl(pCur->flInnerItem.length);
                    memcpy(innerItemBuf+FLASH_SECTOR_SIZE+innerItemSize-FLASH_INNER_ITEM_KEY_SIZE-FLASH_INNER_ITEM_TYPE_SIZE-FLASH_INNER_ITEM_LENGTH_SIZE, 
                        (unsigned char*)(&len), FLASH_INNER_ITEM_LENGTH_SIZE);
                    /* д��VALUE��buffer */
                    memcpy(innerItemBuf+FLASH_SECTOR_SIZE+innerItemSize-FLASH_INNER_ITEM_KEY_SIZE-FLASH_INNER_ITEM_TYPE_SIZE-FLASH_INNER_ITEM_LENGTH_SIZE-pCur->flInnerItem.length,
                        pCur->flInnerItem.pBuf, pCur->flInnerItem.length);
                    DBG("innerItemSize=%#x,value_length=%#x,innerItemStartOffset=%#x\r\n",
                        innerItemSize,pCur->flInnerItem.length,innerItemWriteOffset);                
                    if (ERROR == writeFlash(innerItemBuf, innerItemSize+FLASH_SECTOR_SIZE, innerItemWriteOffset-FLASH_SECTOR_SIZE, TRUE))
                    {
                        printf("\r\nError: Failed to write flash, innerItemBuf=%#x, innerItemSize=%#x, innerItemStartOffset=%#x\r\n",
                            (int)innerItemBuf, innerItemSize, innerItemWriteOffset);
                    }

                    free(pCur->flInnerItem.pBuf);   /* �ͷŶ�ȡ�������ڴ� */
                    pCur->flInnerItem.pBuf = NULL;
                }
                pParent = pCur;
                pCur = pCur->next;
            }
        }
        DBG("\r\nRemove inner item done.\r\n");
    }
    return OK;
}

/* ��ȡ�ڲ���Ŀ��������TLVָ�� */
/* ���ض�̬������ڴ棬�ϲ�ʹ����֮���ͷ�, lengthΪ����buf�ĳ��� */
unsigned char* flashAppInnerGet(FLASH_INNER_ITEM_TYPE type, int *length)
{
    FII_NODE *pCur = NULL;
    FII_NODE *pParent = NULL;
    unsigned char* buf = NULL;

    if (flashAppInnerInited == FALSE)
    {
        if (ERROR == flashAppInnerInit())
        {
            printf("Fatal error: Failed to init inner area structure.\r\n");
            return NULL;
        }
    }

    /* �������нڵ� */
    pCur = fii_head;
    pParent = fii_head;
    while (pCur != NULL)
    {
        DBG("Get inner item, type=%#x, pCur type=%#x\r\n", type, pCur->flInnerItem.type);
        if (pCur->flInnerItem.type == type)
        {
            *length = pCur->flInnerItem.length;
            buf = (unsigned char*)calloc(1, *length);
            if (buf == NULL)
            {
                printf("Fatal error: Failed to allocate memory, size=%#x.\r\n", *length);
                return NULL;
            }
            readFlash(buf, *length, pCur->flInnerItem.value_offset);
            break;
        }
        else
        {
            pParent = pCur;
            pCur = pCur->next;
        }
    }

    if (pCur == NULL)
    {
        DBG("Failed to get inner item, type=%#x.\r\n", type);
        return NULL;
    }
    else
    {
        DBG("Get inner item ok, type=%#x inner_item_len=%#x value_offset=%#x.\r\n",
            pCur->flInnerItem.type, pCur->flInnerItem.length, pCur->flInnerItem.value_offset);
        return buf;
    }
}

/* �����ڲ�����FLASH�е�ƫ�Ƶ�ַ */
int flashAppInnerGetOffset(FLASH_INNER_ITEM_TYPE type, int *value_offset)
{
    FII_NODE *pCur = NULL;
    FII_NODE *pParent = NULL;

    if (flashAppInnerInited == FALSE)
    {
        if (ERROR == flashAppInnerInit())
        {
            printf("Fatal error: Failed to init inner area structure.\r\n");
            return ERROR;
        }
    }
    *value_offset = 0;

    /* �������нڵ� */
    pCur = fii_head;
    pParent = fii_head;
    while (pCur != NULL)
    {
        DBG("Get inner item, type=%#x, pCur type=%#x\r\n", type, pCur->flInnerItem.type);
        if (pCur->flInnerItem.type == type)
        {
            *value_offset = pCur->flInnerItem.value_offset;
            break;
        }
        else
        {
            pParent = pCur;
            pCur = pCur->next;
        }
    }

    if (pCur == NULL)
    {
        DBG("Failed to get inner item value_offset, type=%#x.\r\n", type);
        return ERROR;
    }
    else
    {
        DBG("Get inner item value_offset ok, type=%#x inner_item_len=%#x value_offset=%#x.\r\n",
            pCur->flInnerItem.type, pCur->flInnerItem.length, pCur->flInnerItem.value_offset);
        return OK;
    }

}

/* ��λ�ڲ�����ɾ����profile��gev֮������������� */
int flashAppInnerReset(const char*flBootSize, const char*flFsOffset, const char*flFsSize, const char*flVer)
{
    FII_NODE *pCur = fii_head;

    if (flashAppInnerInited == FALSE)
    {
        if (ERROR == flashAppInnerInit())
        {
            printf("Fatal error: Failed to init inner area structure.\r\n");
            return ERROR;
        }
    }

    if ((flBootSize == NULL) ||
        (flFsOffset == NULL) ||
        (flFsSize == NULL) ||
        (flVer == NULL))
    {
        printf("Fatal error: Bad parameter, flBootSize=%#x, flFsOffset=%#x, flFsSize=%#x, flVer=%#x\r\n",
            (int)flBootSize, (int)flFsOffset, (int)flFsSize, (int)flVer);
        return ERROR;
    }
    DBG("Reset inner item with: flBootSize=%s, flFsOffset=%s, flFsSize=%s, flVer=%s\r\n",
            flBootSize, flFsOffset, flFsSize, flVer);

    while (pCur != NULL)
    {
        DBG("Try to reset inner item, type=%#x, value_offset=%#x, length=%#x\r\n", 
            pCur->flInnerItem.type, pCur->flInnerItem.value_offset, pCur->flInnerItem.length);
        if (pCur->flInnerItem.type == FLASH_INNER_ITEM_PROFILE) /* ��ɾ��profile */
        {
            pCur = pCur->next;
            continue;
        }
        else if (pCur->flInnerItem.type == FLASH_INNER_ITEM_GEV)
        {
            /* ��ɾ��GEV�зǱ����� */
            if (ERROR == updateGevItem(flBootSize, flFsOffset, flFsSize, flVer))
            {
                printf("Fatal error: Failed to clear gev.\r\n");
                return ERROR;
            }

            DBG("Update gev ok.\r\n");
            pCur = pCur->next;
            continue;      
        }
        else    /* ɾ�������ڲ��� */
        {
            if (ERROR == flashAppInnerUnset(pCur->flInnerItem.type, FALSE))
            {
                printf("Fatal error: Failed to remove inner item, type=%#x, value_offset=%#x, length=%#x\r\n", 
            pCur->flInnerItem.type, pCur->flInnerItem.value_offset, pCur->flInnerItem.length);
                return ERROR;                
            }
            DBG("Remove innner item ok, type=%#x, value_offset=%#x, length=%#x\r\n", 
            pCur->flInnerItem.type, pCur->flInnerItem.value_offset, pCur->flInnerItem.length);

            /* ����unset֮��ԭ������Ҫ����, �ݹ���� */
            if (ERROR == flashAppInnerReset(flBootSize, flFsOffset, flFsSize, flVer))
            {
                DBG("Failed to recurse.\r\n");
                return ERROR;
            }

            return OK;
        }
    }

    return OK;   
}


