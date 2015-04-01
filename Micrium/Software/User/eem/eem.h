/*
*********************************************************************************************************
*
*                                      External Exchange Message
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : eem.h
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/

#ifndef  __USER_EEM_H__
#define  __USER_EEM_H__

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*--------------macros declare here-----------------------------*/

/*--------------enums declare here------------------------------*/
typedef enum _tagEEMCommand
{
    /* 0 - 99 reseverd */

    /* 100 - 199 system */
    EEM_COMMAND_COOR_STATCHAG           = 100,
    EEM_COMMAND_EP_ONLINE               = 101,
    EEM_COMMAND_EP_OFFLINE              = 102,
    EEM_COMMAND_QUERY_EPINFO            = 103,
    EEM_COMMAND_REPORT_GPS_DATA         = 105,
    EEM_COMMAND_COOR_ONLINE             = 106,
    EEM_COMMAND_SET_PANDID              = 107,
    EEM_COMMAND_GET_PANDID              = 108,
    
    /* 200 - 299 for zstack using */
    EEM_COMMAND_REGIST_EP               = 200,

    /* user command from 500 */
    EEM_COMMAND_TRANS_COM_READ          = 500,    
    EEM_COMMAND_TRANS_COM_WRITE         = 501,

    EEM_COMMAND_SET_EPID                = 600,
    EEM_COMMAND_GET_EPID                = 601,    
    

    EEM_COMMAND_MAX,
    /* END OF COMMAND, WILL NOT REACH!! */
    EEM_COMMAND_END = 0XFFFF,
}EEM_COMMAND_E;

typedef enum _tagEEMPalyloadType
{
    /* FROM 1, 0 NOT USED! */
    EEM_PAYLOAD_TYPE_RAW_DATA       = 1,    
    EEM_PAYLOAD_TYPE_EP_ID          = 2,
    EEM_PAYLOAD_TYPE_EP_TYPE        = 3,    
    EEM_PAYLOAD_TYPE_EP_ADDR        = 4,
    EEM_PAYLOAD_TYPE_EP_INFO        = 5,
    EEM_PAYLOAD_TYPE_EP_GPS_INFO    = 6,
    EEM_PAYLOAD_TYPE_COORD_NWK_ID   = 7,

    /* for zstack using 500 - 1000 */
    EEM_PAYLOAD_TYPE_REGIST_INFO    = 500,

    EEM_PAYLOAD_TYPE_MAX,
    
    EEM_PAYLOAD_TYPE_END = 0XFFFF,
}EEM_PAYLOAD_TYPE_E;

/*--------------struct declare here-----------------------------*/

/*
* Message Header Len: 12 Bytes
* Message format:| EEM_HEADER_S | padload | CRC |
*/
typedef struct _tagEEMHeader
{
    u8  ucMagic;        /* magic word, fixed to 0xFE */
    u8  ucEpId;         /* end point id, coordinator is 0 */
    u16 usCommand;      /* ref to >EEM_COMMAND_E< */
    u16 usSeq;          /* from 1, recircle */
    u16 usPayloadLen;   /* data len, not include header */
    u8  ucResult;       /* ref to >UCORE_ERROR_E< */
    u8  ucSPara;        /* Simple Para */
    u16 usTransId;      /* for transaction */
}EEM_HEADER_S;

typedef struct _tagEEM_PL_HEAD
{
	u16 type;
	u16 length;
}EEM_PL_HEAD;

/*--------------functions declare here-----------------------------*/

/* be carefully using, this function will asign buffer */
EEM_HEADER_S    *EEM_CreateHeader(u8 EpId, u16 Command, u8 result);

/* be carefully using, this function will asign buffer */
EEM_HEADER_S    *EEM_CreateRespHeader(EEM_HEADER_S *ReqHeader, u8 result);

/* be carefully using, this function will asign buffer */
u8              EEM_CreateBody(EEM_HEADER_S **ppHeader, u16 len, void *data);

u8              EEM_GetBody(EEM_HEADER_S *Header, u16 *len, void **data);

/* be carefully using, this function will asign buffer */
u8              EEM_AppendPayload(EEM_HEADER_S **ppHeader, u16 PayloadType, u16 PayloadLen, void *data);

void            *EEM_GetPayload(EEM_HEADER_S *Header, u16 PayloadType, u16 *PayloadLen);

u8              *EEM_GetBuff(EEM_HEADER_S *Header, u16 *len);

/* be carefully using, this function will asign buffer to ppHeader */
u8              EEM_GetMessage(u8 *Buff, u32 *BufSize, EEM_HEADER_S **ppHeader);

void            EEM_Delete(void **Header);

/* for debug using */
void			EEM_DumpMessage(EEM_HEADER_S *Header);

/*---------------extern vars & functinos here-----------------------*/

#if defined(__cplusplus)
  }
#endif

#endif

