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

#include <stdio.h>

#if   defined(OS_TYPE_UCOS)
    #include <includes.h>
	#include <user_core.h>
    #include <eem.h>
#elif defined(OS_TYPE_ZSTACK)
    #include <osal.h>
    #include <ucore\ucore_defs.h>
    #include <eem\eem.h>

    /* aditonal define */
    #define malloc osal_mem_alloc
    #define free osal_mem_free
    #define memcpy osal_memcpy
#else /* WIN32 simulation */
    #include <stdlib.h>
    #include <string.h>
    #include <eem_defs.h>
    #include <eem.h>
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

/* be carefully using, this function will asign buffer */
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
    pHeader->usTransId      = 0;
    pHeader->ucSPara        = 0;

    /* return header */
    return pHeader;
}

/* be carefully using, this function will asign buffer */
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
    pHeader->ucEpId         = ReqHeader->ucEpId;        /* end point id, coordinator is 0 */
    pHeader->usCommand      = ReqHeader->usCommand;     /* ref to >EEM_COMMAND_E< */
    pHeader->usSeq          = ReqHeader->usSeq;         /* from 1, recircle */
    pHeader->usPayloadLen   = 0;                        /* data len, not include header */
    pHeader->ucResult       = result;                   /* ref to >UCORE_ERROR_E< */  
    pHeader->usTransId      = 0;
    pHeader->ucSPara        = 0;

    /* return header */
    return pHeader;    
}

/* be carefully using, this function will asign buffer */
u8 EEM_CreateBody(EEM_HEADER_S **ppHeader, u16 len, void *data)
{
    u16             usTotalLen = 0;
    EEM_HEADER_S   *pNewHeader = NULL;
    
    /* check header first */
    if ((NULL == (*ppHeader)) || (0 == len))
    {
        return UCORE_ERR_PAPA_ERROR;
    }

    /* cal new buff size */
    usTotalLen = sizeof(EEM_HEADER_S) + len + 2;

    /* realloc buffer */
    pNewHeader = (EEM_HEADER_S *)malloc(usTotalLen);
    if (NULL == pNewHeader)
    {
        return UCORE_ERR_MEM_ALLOC_FAILED;
    }

    /* ok, copy old buffer first */
    memcpy((void *)pNewHeader, (void *)(*ppHeader), sizeof(EEM_HEADER_S));
    
    /* clean old buffer */
    free(*ppHeader);

    /* asign new memory addr */
    *ppHeader = pNewHeader;

    /* copy buffer */
    memcpy((u8 *)(*ppHeader) + sizeof(EEM_HEADER_S), data, len);

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

/* be carefully using, this function will asign buffer */
u8 EEM_AppendPayload(EEM_HEADER_S **ppHeader, u16 PayloadType, u16 PayloadLen, void *data)
{
    u16             usTotalLen      = 0;
    u16             usOldTotalLen   = 0;
    EEM_HEADER_S    *pNewHeader     = NULL;    
    EEM_PL_HEAD     stPLHeader;
    
    
    if ((NULL == (*ppHeader)) || (NULL == data) || (0 == PayloadLen))
    {
        return UCORE_ERR_PAPA_ERROR;
    }

    /* save old len */
    usOldTotalLen = sizeof(EEM_HEADER_S) + (*ppHeader)->usPayloadLen;
    
    /* cal new total len */
    usTotalLen = usOldTotalLen + sizeof(EEM_PL_HEAD) + PayloadLen + 2;

    /* realloc buffer */
    pNewHeader = (EEM_HEADER_S *)malloc(usTotalLen);
    if (NULL == pNewHeader)
    {
        return UCORE_ERR_MEM_ALLOC_FAILED;
    }

    /* ok, copy old buffer first */
    memcpy((void *)pNewHeader, (void *)(*ppHeader), usOldTotalLen);
    
    /* clean old buffer */
    free(*ppHeader);

    /* asign new memory addr */
    *ppHeader = pNewHeader;   

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

/* be carefully using, this function will asign buffer to ppHeader */
u8 EEM_GetMessage(u8 *Buff, u32 *BufSize, EEM_HEADER_S **ppHeader)
{
    u8              i           = 0;
    u8              ucStartPos  = 0;
    u8              ucTotalLen  = 0;
    u16             usCRC       = 0;
    u16             *pCRC       = 0;
    u8              bSearched   = 0;
    EEM_HEADER_S    *pTemp      = NULL;
    
    if ((NULL == Buff) || (NULL == BufSize))
    {
        return UCORE_ERR_NO_MESSAGE;
    }

    /* minimal message size check */
    if (sizeof(EEM_HEADER_S) + 2 > *BufSize)
    {
        return UCORE_ERR_NO_MESSAGE;
    }

    /* get a header */
    while (0 == bSearched)
    {
        /* search for magic word 0xFE */
        while ((0xFE != Buff[i++]) && (i < *BufSize));

        /* searched? */
        if (i >= *BufSize)
        {
            /* clear buff */
            *BufSize = 0;            
            return UCORE_ERR_NO_MESSAGE;
        }

        ucStartPos = i - 1;

        /* detail check, asign a header */
        pTemp = (EEM_HEADER_S *) &Buff[ucStartPos];

        /* check payload len, single message payload len must less than 255 */
        if (pTemp->usPayloadLen >= 0xFF)
        {
            /* invalid message, search next */
            i = ucStartPos + 1;

            /* check len */
            if (i >= *BufSize)
            {
                /* clear buff */
                *BufSize = 0;                
                return UCORE_ERR_NO_MESSAGE;
            }
        }
        else
        {
            /* break while */
            bSearched = 1;
        }
    }

    /* check payload size */
    if (pTemp->usPayloadLen > *BufSize - ucStartPos - sizeof(EEM_HEADER_S) - 2)
    {
        if (0 != ucStartPos)
        {
            *BufSize -= ucStartPos;
            /* if there is any buffer left? */
            if (*BufSize > 0)
            {
                memcpy((void *) Buff, (void *) (Buff + ucStartPos), *BufSize);
            }
        }
        
        return UCORE_ERR_NO_MESSAGE;
    }

    /* ok, now we have a full packet, alloc memory first */
    ucTotalLen = sizeof(EEM_HEADER_S) + pTemp->usPayloadLen + 2;    

    /* cal CRC and compare */
    usCRC = CalCRC(pTemp);
    pCRC  = (u16 *) ((u8 *)pTemp + sizeof(EEM_HEADER_S) + pTemp->usPayloadLen);
    if (usCRC != *pCRC)
    {
        /* ok, when crc check failed, erase this message  */
        *BufSize -= (ucStartPos + ucTotalLen);
        /* if there is any buffer left? */
        if (*BufSize > 0)
        {
            memcpy((void *) Buff, (void *) (Buff + ucStartPos + ucTotalLen), *BufSize);
        }
        
        return UCORE_ERR_INVALID_MESSAGE;
    }    

    /* alloc memory */
    *ppHeader = (EEM_HEADER_S *) malloc(ucTotalLen);

    /* copy message */
    memcpy((void *) *ppHeader, (void *) pTemp, ucTotalLen);

    /* finally, ajust user message buf and size */
    *BufSize -= (ucStartPos + ucTotalLen);
    /* if there is any buffer left? */
    if (*BufSize > 0)
    {
        memcpy((void *) Buff, (void *) (Buff + ucStartPos + ucTotalLen), *BufSize);
    }

    return UCORE_ERR_SUCCESS;
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

/* for debug using */
void EEM_DumpMessage(EEM_HEADER_S *Header)
{
    u16 i           = 0;
    u16 usTotalLen  = 0;
    u8  *pchar      = NULL;
    
    if (NULL == Header)
    {
        return;
    }

    pchar = (u8 *) Header;
    usTotalLen = sizeof(EEM_HEADER_S) + Header->usPayloadLen + 2;
    printf("Dump Message:\r\n");
    for (i = 0; i < usTotalLen; i++)
    {
        printf("%02X ", pchar[i]);
    }
    printf("\r\n");
}

#if defined(__cplusplus)
  }
#endif

