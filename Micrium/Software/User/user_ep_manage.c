/*
*********************************************************************************************************
*
*                                          user ep manager 
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : user_ep_manage.c
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/
#include <includes.h>
#include <user_core.h>
#include <dlist.h>
#include <user_ep_manage.h>

EP_INFO_S   g_stMyEpInfo = {0};

/*--------------local global vars declare here-----------------------------*/
static DOUBLE_LINK_NODE *g_pEpDlistHead     = NULL;
static OS_EVENT         *g_QSemListGuard    = NULL;

/*--------------local global func declare here-----------------------------*/
static BOOL dlist_compare(void *pdata_left, void *pdata_right);
static void dlist_Print(void *pdata);

/*--------------local global function start here-----------------------------*/
BOOL dlist_compare(void *pdata_left, void *pdata_right)
{
	EP_INFO_S *pLeft = (EP_INFO_S *) pdata_left;
	EP_INFO_S *pRight = (EP_INFO_S *) pdata_right;

	if ((NULL == pLeft) || (NULL == pRight))
	{
		return FALSE;
	}

	if (pLeft->ucEpId == pRight->ucEpId)
	{
		return TRUE;
	}

	return FALSE;
}

void dlist_Print(void *pdata)
{
	EP_INFO_S *pInfo = (EP_INFO_S *) pdata;

	if (pInfo)
	{
		printf("%d\r\n", pInfo->ucEpId);
	}
}

/*--------------function start here-----------------------------*/
u8 UEM_Init(void)
{
	EP_INFO_S *pEp = NULL;

    /* init my ep */
    g_stMyEpInfo.ucEpType = EP_TYPE_INVALID;

    /* create sem */
    g_QSemListGuard = OSSemCreate(1);
    if (NULL == g_QSemListGuard)
    {
        printf("Create sem for dlist failed.\r\n");
        return UCORE_ERR_COMMON_FAILED;
    }

    /* alloc buff */
    pEp = (EP_INFO_S *) malloc(sizeof(EP_INFO_S));
    if (NULL == pEp)
    {
        printf("Alloc ep node buff failed.\r\n");
        return UCORE_ERR_MEM_ALLOC_FAILED;
    }

    /* clear buffer */
    memset(pEp, 0, sizeof(EP_INFO_S));

    /* asign values */
	pEp->ucEpId     = 0;/* 0 fixed for coordinator */
    pEp->ucEpType   = EP_TYPE_COOR;
    strncpy((char *) pEp->sEpName, "COOR", MAX_EP_NAME_LEN - 1);

	g_pEpDlistHead = create_double_link_node((void *) pEp);
    if (NULL == g_pEpDlistHead)
    {
        printf("Init ep dlist head failed.\r\n");

        /* free asigned buffer */
        free((void *) pEp);
        
        return UCORE_ERR_MEM_ALLOC_FAILED;
    }

    return UCORE_ERR_SUCCESS;
}

u8 UEM_AddEp(u8 ucEpId, u8 ucEpType, u16 usEpAddr, u8 *sEpName)
{
	EP_INFO_S *pEp = NULL;
    INT8U err;

    OSSemPend(g_QSemListGuard, 1000, &err);
    if (OS_NO_ERR != err)
    {
        printf("Aquire sem failed, ERR[%d].\r\n", err);
        return UCORE_ERR_COMMON_FAILED;
    }

    /* alloc buff */
    pEp = (EP_INFO_S *) malloc(sizeof(EP_INFO_S));
    if (NULL == pEp)
    {
        printf("Alloc ep node buff failed.\r\n");

        /* release sem first */
        OSSemPost(g_QSemListGuard);
        
        return UCORE_ERR_MEM_ALLOC_FAILED;
    }

    /* clear buffer */
    memset(pEp, 0, sizeof(EP_INFO_S));

    /* asign values */
	pEp->ucEpId     = ucEpId;/* 0 fixed for coordinator */
    pEp->ucEpType   = ucEpType;
    pEp->usEpAddr   = usEpAddr;

    if (NULL != sEpName)
        strncpy((char *) pEp->sEpName, (char *) sEpName, MAX_EP_NAME_LEN - 1);

    /* ok, add to list */
    if (TRUE != insert_data_into_double_link(&g_pEpDlistHead, (void *) pEp, dlist_compare))
    {
        /* free asigned buffer */
        free((void *) pEp);

        /* release sem */
        OSSemPost(g_QSemListGuard);
                
        return UCORE_ERR_MEM_ALLOC_FAILED;
    }

    /* release sem */
    OSSemPost(g_QSemListGuard);

    /* for testing */
    printf("Total Nodes:%d\r\n", count_number_in_double_link(g_pEpDlistHead));

    return UCORE_ERR_SUCCESS;
}

u8 UEM_DelEp(u8 ucEpId)
{
    EP_INFO_S stFindInfo = {0};
    INT8U err;

    OSSemPend(g_QSemListGuard, 1000, &err);
    if (OS_NO_ERR != err)
    {
        printf("Aquire sem failed, ERR[%d].\r\n", err);
        return UCORE_ERR_COMMON_FAILED;
    }
    
    stFindInfo.ucEpId = ucEpId;

    /* call delete */
    if (TRUE != delete_data_from_double_link(&g_pEpDlistHead, (void *) &stFindInfo, dlist_compare))
    {
        printf("Delete EP[%d] failed.\r\n", ucEpId);

        /* release sem */
        OSSemPost(g_QSemListGuard);
        
        return UCORE_ERR_COMMON_FAILED;
    }

    /* release sem */
    OSSemPost(g_QSemListGuard);

    return UCORE_ERR_SUCCESS;
}

u8 UEM_UpdateEp(u8 ucEpId, u8 ucEpType, u16 usEpAddr, u8 *sEpName)
{
    EP_INFO_S *pEpFind  = NULL;
    INT8U err;

    OSSemPend(g_QSemListGuard, 1000, &err);
    if (OS_NO_ERR != err)
    {
        printf("Aquire sem failed, ERR[%d].\r\n", err);
        return UCORE_ERR_COMMON_FAILED;
    }
    
    /* find ep? */
    pEpFind = UEM_FindEp(ucEpId);
    if (NULL == pEpFind)
    {
        /* release sem */
        OSSemPost(g_QSemListGuard);
    
        return UCORE_ERR_COMMON_FAILED;
    }

    /* ok, set new info */
    pEpFind->ucEpType = ucEpType;
    pEpFind->usEpAddr = usEpAddr;

    if (NULL != sEpName)
        strncpy((char *) pEpFind->sEpName, (char *) sEpName, MAX_EP_NAME_LEN - 1);    

    /* release sem */
    OSSemPost(g_QSemListGuard);

    return UCORE_ERR_SUCCESS;
}

EP_INFO_S *UEM_FindEp(u8 ucEpId)
{
    DOUBLE_LINK_NODE    *pNodeFind  = NULL;
    EP_INFO_S           stFindInfo  = {0};
    
    stFindInfo.ucEpId = ucEpId;

    /* call find */
	pNodeFind = find_data_in_double_link(g_pEpDlistHead, (void *) &stFindInfo, dlist_compare);
	if (NULL == pNodeFind)
	{
        return NULL;
	}

    /* check ep info data */
    if (NULL == pNodeFind->data)
    {
        printf("Check EP[%d] data failed.\r\n", ucEpId);        
        return NULL;    
    }
    
    /* ok, return ep info directly */
    return ((EP_INFO_S *) pNodeFind->data);
}

