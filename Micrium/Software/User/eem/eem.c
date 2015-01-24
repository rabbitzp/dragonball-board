/*
*********************************************************************************************************
*
*                                      External Exchange Message
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : eem.c
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/
#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

#include <includes.h>
#include <stdio.h>
#include <eem.h>
#ifndef WIN32
	#include <user_core.h>
#else
    #include <stdlib.h>
    #include <string.h>
#endif

/*-------------------------------local global vars declare here----------------------------*/
static u16 g_usEEMSequence = 1;

/*-------------------------------local functions declare here----------------------------*/
u16 CalCRC(EEM_HEADER_S *header)
{
    u16 usLen = 0;
    u16 i     = 0;
    u16 usCRC = 0;    
    u8  *pc   = NULL;
    
    if (NULL == header)
    {
        return 0;
    }

    /* cal total len */
    usLen   = sizeof (EEM_HEADER_S) + header->usPayloadLen;
    pc      = (u8 *) header;

    for (i=0; i<usLen; i++)
    {
        usCRC += pc[i];
    }

    return usCRC;
}

EEM_HEADER_S *EEM_CreateHeader(u8 EpId, u16 Command, u8 result)
{
    EEM_HEADER_S *pHeader = NULL;

    /* alloc buff */
    pHeader = (EEM_HEADER_S *)malloc(sizeof (EEM_HEADER_S) + 2);    /* 2 for crc bytes */
    if (NULL == pHeader)
    {
        return NULL;
    }

    /* check seq first */
    if (g_usEEMSequence >= 0xFFFF)
    {
        g_usEEMSequence = 1;
    }

    /* init header */
    pHeader->ucMagic        = 0xFE;                     /* magic word, fixed to 0xFE */
    pHeader->ucEpId         = EpId;                     /* end point id, coordinator is 0 */
    pHeader->usCommand      = Command;                  /* ref to >EEM_COMMAND_E< */
    pHeader->usSeq          = g_usEEMSequence++;        /* from 1, recircle */
    pHeader->usPayloadLen   = 0;                        /* data len, not include header */
    pHeader->ucResult       = result;                   /* ref to >UCORE_ERROR_E< */  
    pHeader->ucRev[0]       = 0;
    pHeader->ucRev[1]       = 0;
    pHeader->ucRev[2]       = 0;    

    /* return header */
    return pHeader;
}

EEM_HEADER_S *EEM_CreateRespHeader(EEM_HEADER_S *ReqHeader, u8 result)
{
    EEM_HEADER_S *pHeader = NULL;
    
    /* check header first */
    if (NULL == ReqHeader)
    {
        return NULL;
    }

    /* alloc buff */
    pHeader = (EEM_HEADER_S *)malloc(sizeof (EEM_HEADER_S) + 2);    /* 2 for crc bytes */
    if (NULL == pHeader)
    {
        return NULL;
    }

    /* init header */
    pHeader->ucMagic        = 0xFE;                     /* magic word, fixed to 0xFE */
    pHeader->ucEpId         = ReqHeader->ucEpId;                     /* end point id, coordinator is 0 */
    pHeader->usCommand      = ReqHeader->usCommand + 1;                  /* ref to >EEM_COMMAND_E< */
    pHeader->usSeq          = ReqHeader->usSeq;        /* from 1, recircle */
    pHeader->usPayloadLen   = 0;                        /* data len, not include header */
    pHeader->ucResult       = result;                   /* ref to >UCORE_ERROR_E< */  
    pHeader->ucRev[0]       = 0;
    pHeader->ucRev[1]       = 0;
    pHeader->ucRev[2]       = 0;    

    /* return header */
    return pHeader;    
}

u8 EEM_CreateBody(EEM_HEADER_S **ppHeader, u16 len, void *data)
{
    u16 usTotalLen = 0;
    
    /* check header first */
    if ((NULL == (*ppHeader)) || (0 == len))
    {
        return UCORE_ERR_PAPA_ERROR;
    }

    /* cal new buff size */
    usTotalLen = sizeof(EEM_HEADER_S) + len + 2;

    /* realloc buffer */
    (*ppHeader) = (EEM_HEADER_S *) realloc((void *) (*ppHeader), usTotalLen);
    if (NULL == (*ppHeader))
    {
        return UCORE_ERR_MEM_ALLOC_FAILED;
    }

    /* copy buffer */
    memcpy((u8 *)(*ppHeader) + sizeof (EEM_HEADER_S), data, len);

    /* set new payload len */
    (*ppHeader)->usPayloadLen = len;

    return UCORE_ERR_SUCCESS;
}

u8 EEM_GetBody(EEM_HEADER_S *Header, u16 *len, void **data)
{
    if ((NULL == Header) || (NULL == len) || (NULL == data))
    {
        return UCORE_ERR_PAPA_ERROR;
    }

    if (0 == Header->usPayloadLen)
    {
        *len = 0;
        *data = NULL;
        return UCORE_ERR_PAPA_ERROR;
    }

    *len = Header->usPayloadLen;
    *data = (u8 *)Header + sizeof (EEM_HEADER_S);
    
    return UCORE_ERR_SUCCESS;
}

u8 EEM_AppendPayload(EEM_HEADER_S **ppHeader, u16 PayloadType, u16 PayloadLen, void *data)
{
    u16         usTotalLen = 0;
    EEM_PL_HEAD stPLHeader;
    
    if ((NULL == (*ppHeader)) || (NULL == data) || (0 == PayloadLen))
    {
        return UCORE_ERR_PAPA_ERROR;
    }

    /* cal new total len */
    usTotalLen = sizeof (EEM_HEADER_S) + (*ppHeader)->usPayloadLen + sizeof(EEM_PL_HEAD) + PayloadLen + 2;

    /* realloc buffer */
    (*ppHeader) = (EEM_HEADER_S *) realloc((void *) (*ppHeader), usTotalLen);
    if (NULL == (*ppHeader))
    {
        return UCORE_ERR_MEM_ALLOC_FAILED;
    }

    /* prepare pl header */
    stPLHeader.type = PayloadType;
    stPLHeader.length = PayloadLen + sizeof(EEM_PL_HEAD);

    /* append payload header */
    memcpy((u8 *)(*ppHeader) + sizeof(EEM_HEADER_S) + (*ppHeader)->usPayloadLen, (void *) &stPLHeader, sizeof(EEM_PL_HEAD));

    /* append payload data */
    memcpy((u8 *)(*ppHeader) + sizeof(EEM_HEADER_S) + (*ppHeader)->usPayloadLen + sizeof(EEM_PL_HEAD), data, PayloadLen);

    /* cal new payload size */
    (*ppHeader)->usPayloadLen += sizeof(EEM_PL_HEAD) + PayloadLen;
    
    return UCORE_ERR_SUCCESS;
}

void *EEM_GetPayload(EEM_HEADER_S *Header, u16 PayloadType, u16 *PayloadLen)
{
	u16         readlen = 0;
    EEM_PL_HEAD *p_head = NULL;
	u8*         p_pos   = NULL;

    /* para check */
    if ((NULL == Header) || (NULL == PayloadLen))
    {
        return NULL;
    }

    /* prepare */
    p_pos   = (u8 *)Header + sizeof(EEM_HEADER_S);
    readlen = Header->usPayloadLen;

    /* search payload node */
	while (readlen > 0)
	{
	    /* ptr to a pl header */
		p_head = (EEM_PL_HEAD *) p_pos;

        /* compare type */
		if (p_head->type == PayloadType)
		{
		    /* set data len */
		    *PayloadLen = p_head->length - sizeof(EEM_PL_HEAD);
            
			return ((u8 *) p_head + sizeof(EEM_PL_HEAD));
		}

        /* next */
		p_pos += p_head->length;
		readlen -= p_head->length;
	}

	return NULL;
}

u8 *EEM_GetBuff(EEM_HEADER_S *Header, u16 *len)
{
    u16 i        = 0;
    u16 *pCRC    = NULL;
    u8  *pcArray = NULL;
    
    if ((NULL == Header) || (NULL == len))
    {
        return NULL;
    }

    /* cal total len */
    *len = sizeof(EEM_HEADER_S) + Header->usPayloadLen + 2; /* 2 for crc */

    /* get crc bytes */
    pcArray = (u8 *)Header;
    pCRC    = (u16 *) ((u8 *)Header + sizeof(EEM_HEADER_S) + Header->usPayloadLen);

    /* init crc */
    *pCRC = 0;
    
    /* then, cal crc, simplely plus all value */
    for (i = 0; i < *len - 2; i++)
    {
        *pCRC += pcArray[i];
    }
    
    return (u8 *)Header;
}

void EEM_Delete(void **Header)
{
    /* check header first */
    if (NULL != Header)
    {
        /* free memory */
        free(*Header);

        /* ptr to NULL */
        *Header = NULL;
    }
}

#if defined(__cplusplus)
  }
#endif

