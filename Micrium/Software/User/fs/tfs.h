/*
*********************************************************************************************************
*
*                                      Tiny File System
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : tfs.h
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/

#ifndef __TFS_H__
#define __TFS_H__
#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif


typedef enum _tagTFSConfigItem
{
    TFS_CONFIG_ITEM_DEV_CONF = 1,
    TFS_CONFIG_ITEM_NET_CONF = 2,

    TFS_CONFIG_ITEM_MAX,
    TFS_CONFIG_ITEM_BUTT,
}TFS_CONFIG_ITEM_E;


typedef enum _tagTFSResult
{
    TFS_RESULT_SUCESS           = 0,
        
    TFS_RESULT_FAILED           = 1,
    TFS_RESULT_READ_FAILED      = 2,
    TFS_RESULT_WRITE_FAILED     = 3,
    TFS_RESULT_ITEM_NOT_FOUND   = 4,
    TFS_RESULT_FS_ERROR         = 5,

    TFS_RESULT_MAX,
    TFS_RESULT_BUTT,
}TFS_RESULT_E;


typedef enum _tagTFSItemType
{
    TFS_ITEM_TYPE_DEV_INFO = 1,
    TFS_ITEM_TYPE_NET_CONF = 2,

    TFS_ITEM_TYPE_MAX,
    TFS_ITEM_TYPE_BUTT,
}TFS_ITEM_TYPE_E;

typedef struct _tagTFSAttr
{
    char    name[4];
    u8      ver;
    u8      sub_v;
    u8      status;
    u16     items;
    u16     limit;
    u16     cur_data_addr;
}TFS_ATTR_S;

typedef struct _tagTFSItem
{
    u16 type;
    u16 size;
    u16 offset;
}TFS_ITEM_S;

typedef u8 (*pfnTFS_read)(u8 *, u16 , u16 );
typedef u8 (*pfnTFS_write)(u8 *, u16 , u16 );

u8 TFS_InstalFS(u8 stor_type, u32 capacity, pfnTFS_read pfRead, pfnTFS_read pfWrite);

u8 TFS_InitFS(void);
u8 TFS_Format(void);

u8 TFS_Read(u16 type, u16 size, void *data);
u8 TFS_Write(u16 type, u16 size, void *data);

u8 TFS_Dump(u8 *_pBuf, u16 _usAddress, u16 _usSize);

#if defined(__cplusplus)
  }
#endif

#endif /* __TFS_H__ */
