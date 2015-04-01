/*
*********************************************************************************************************
*
*                                      Tiny File System
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : tfs.c
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/

#include <includes.h>
#include "tfs.h"

#define TFS_DATA_BLOCK_START_ADDR 0x400

#define TFS_VERSION         1
#define TFS_SUB_VERSION     1

typedef struct _tagTFSCBlock
{
    u8 install;
    u8 status;
    u16 items;
    u16 limit;
    u16 cur_addr;
    u32 capacity;

    pfnTFS_read pfnRead;
    pfnTFS_read pfnWrite;    
}TFS_CBLOCK_S;

static TFS_CBLOCK_S gTFSCblock = 
{
    0,
    0,
    0,
    0,
    0,
    0,
    NULL,
    NULL,
};

u8 TFS_InstalFS(u8 stor_type, u32 capacity, pfnTFS_read pfRead, pfnTFS_read pfWrite)
{
    gTFSCblock.pfnRead = pfRead;
    gTFSCblock.pfnWrite = pfWrite;
    gTFSCblock.capacity = capacity;
    
    gTFSCblock.install = 1;
    
    return TFS_RESULT_SUCESS;
}

u8 TFS_InitFS(void)
{
    TFS_ATTR_S stTfsAttr = {0};
    
    /* read tfs attrs */
    if (!gTFSCblock.pfnRead((u8 *) &stTfsAttr, 0, sizeof(TFS_ATTR_S)))
    {
        return TFS_RESULT_READ_FAILED;
    }

    /* check */
    if (strncmp(stTfsAttr.name, "TFS", 3) != 0)
    {
        return TFS_RESULT_FS_ERROR;
    }

    if (stTfsAttr.status != 'A')
    {
        return TFS_RESULT_FS_ERROR;
    }

    if ((stTfsAttr.ver!= TFS_VERSION) || (stTfsAttr.sub_v!= TFS_SUB_VERSION))
    {
        return TFS_RESULT_FS_ERROR;
    }    

    /* set control block */
    gTFSCblock.cur_addr = 0x00;
    gTFSCblock.items = stTfsAttr.items;
    gTFSCblock.limit = stTfsAttr.limit;
    gTFSCblock.status = 1;
    gTFSCblock.cur_addr = stTfsAttr.cur_data_addr;

    return TFS_RESULT_SUCESS;
}

u8 TFS_Format(void)
{
    u32         i           = 0;
    u16         iProg       = 1;
    u16         addr        = 0;
    u8          erase       = 0xFF;
    TFS_ATTR_S  stTfsAttr   = {0};
    
    if (!gTFSCblock.install)
    {
        return TFS_RESULT_FAILED;
    }

    UCORE_DEBUG_PRINT("Start formating....\r\n");
    
    /* erase all */
    for (i = 0; i < gTFSCblock.capacity; i++)
    {
        if (!gTFSCblock.pfnWrite(&erase, addr++, sizeof(erase)))
        {
            UCORE_DEBUG_PRINT("Format fs error!\r\n");
            return TFS_RESULT_FAILED;
        }

        /* print progress */
        if (i == 1024 * iProg)
        {
            UCORE_DEBUG_PRINT("Formating:%d%%\r\n", (i*100)/gTFSCblock.capacity);
            iProg++;
        }
    }

    UCORE_DEBUG_PRINT("Formating:100%\r\n");

    UCORE_DEBUG_PRINT("formating success!\r\n");

    /* prepare attr */
    stTfsAttr.items = 0;
    stTfsAttr.limit = 0;
    strncpy(stTfsAttr.name, "TFS", 3);
    stTfsAttr.status = 'A';
    stTfsAttr.sub_v = TFS_SUB_VERSION;
    stTfsAttr.ver = TFS_VERSION;
    stTfsAttr.cur_data_addr = TFS_DATA_BLOCK_START_ADDR;

    /* write attr */
    if (!gTFSCblock.pfnWrite((u8 *) &stTfsAttr, 0x00, sizeof(TFS_ATTR_S)))
    {
        return TFS_RESULT_FAILED;
    }    

    /* delay a while */
    OSTimeDlyHMSM(0, 0, 0, 5);    
    
    return TFS_InitFS();
}

/* return addr */
u8 TFS_FindItem(u16 type, u16 size, u16 *item_addr)
{
    u16         index = 0;
    u16         addr = 0x00;
    TFS_ITEM_S  stItem = {0};
    
    if ((!gTFSCblock.status) || (0 == gTFSCblock.items))
    {
        return TFS_RESULT_ITEM_NOT_FOUND;
    }

    for (index = 0; index < gTFSCblock.items; index++)
    {
        addr = sizeof(TFS_ATTR_S) + index * sizeof(TFS_ITEM_S);
        
        /* read tfs attrs */
        if (!gTFSCblock.pfnRead((u8 *) &stItem, addr, sizeof(TFS_ITEM_S)))
        {
            return TFS_RESULT_READ_FAILED;
        }

        /* compare */
        if ((stItem.type == type) && (stItem.size == size))
        {
            *item_addr = stItem.offset;

            return TFS_RESULT_SUCESS;
        }
    }
    
    return TFS_RESULT_ITEM_NOT_FOUND;
}

u8 TFS_Read(u16 type, u16 size, void *data)
{
    u16         index = 0;
    u16         addr = 0x00;
    TFS_ITEM_S  stItem = {0};
    
    if (!gTFSCblock.status)
    {
        UCORE_DEBUG_PRINT("TSF not valid or item count is:%d.\r\n", gTFSCblock.items);
        
        return TFS_RESULT_ITEM_NOT_FOUND;
    }

    for (index = 0; index < gTFSCblock.items; index++)
    {
        addr = sizeof(TFS_ATTR_S) + index * sizeof(TFS_ITEM_S);
        
        /* read tfs attrs */
        if (!gTFSCblock.pfnRead((u8 *) &stItem, addr, sizeof(TFS_ITEM_S)))
        {
            UCORE_DEBUG_PRINT("TFS read item index failed, addr:%x, size:%d\r\n", addr, sizeof(TFS_ITEM_S));
            
            return TFS_RESULT_READ_FAILED;
        }

        /* compare */
        if ((stItem.type == type) && (stItem.size == size))
        {
            if (!gTFSCblock.pfnRead((u8 *) data, stItem.offset, size))
            {
                UCORE_DEBUG_PRINT("TFS read item data failed, offset:%x, size:%d\r\n", stItem.offset, size);
                
                return TFS_RESULT_READ_FAILED;
            }

            return TFS_RESULT_SUCESS;
        }
    }
    
    return TFS_RESULT_ITEM_NOT_FOUND;
}

u8 TFS_Write(u16 type, u16 size, void *data)
{
    u8          isNew      = 0;
    u16         index_addr = 0x00;
    u16         data_addr = 0x00;

    if (!gTFSCblock.status)
    {
        return 0;
    }

    /* try find */
    if (TFS_RESULT_SUCESS != TFS_FindItem(type, size, &data_addr))
    {
        /* cal index write addr */
        index_addr = sizeof(TFS_ATTR_S) + gTFSCblock.items * sizeof(TFS_ITEM_S);

        /* cal data write addr */
        data_addr = gTFSCblock.cur_addr;

        isNew = 1;
    }

    /* write data first */
    if (!gTFSCblock.pfnWrite((u8 *) data, data_addr, size))
    {
        UCORE_DEBUG_PRINT("write item data failed, data_addr:%x size:%d.\r\n", data_addr, size);
        
        return TFS_RESULT_FAILED;
    }

    /* ok, update index and tfs attr */
    if (isNew)
    {
        TFS_ATTR_S  stTfsAttr   = {0};
        TFS_ITEM_S  stIndex     = {0};

        /* update index */
        stIndex.type = type;
        stIndex.size = size;
        stIndex.offset = data_addr;
        
        /* write index */
        if (!gTFSCblock.pfnWrite((u8 *) &stIndex, index_addr, sizeof(TFS_ITEM_S)))
        {
            UCORE_DEBUG_PRINT("write item index failed, index_addr:%x size:%d.\r\n", index_addr, sizeof(TFS_ITEM_S));
            
            return TFS_RESULT_FAILED;
        }
        
        /* update attr */
        gTFSCblock.items++;
        gTFSCblock.cur_addr += size;

        /* prepare attr */
        stTfsAttr.items = gTFSCblock.items;
        stTfsAttr.limit = gTFSCblock.limit;
        strncpy(stTfsAttr.name, "TFS", 3);
        stTfsAttr.status = 'A';
        stTfsAttr.sub_v = TFS_SUB_VERSION;
        stTfsAttr.ver = TFS_VERSION;
        stTfsAttr.cur_data_addr = gTFSCblock.cur_addr;

        /* write attr */
        if (!gTFSCblock.pfnWrite((u8 *) &stTfsAttr, 0x00, sizeof(TFS_ATTR_S)))
        {
            UCORE_DEBUG_PRINT("write tfs attr failed.\r\n");
            
            return TFS_RESULT_FAILED;
        }
    } 

    /* delay a while */
    OSTimeDlyHMSM(0, 0, 0, 5);
        
    return TFS_RESULT_SUCESS;
}

u8 TFS_Dump(u8 *_pBuf, u16 _usAddress, u16 _usSize)
{
   return gTFSCblock.pfnRead(_pBuf, _usAddress, _usSize);
}


