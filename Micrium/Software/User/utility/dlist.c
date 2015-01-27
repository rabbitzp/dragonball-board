/*
*********************************************************************************************************
*
*                                          dual list 
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : dlist.c
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dlist.h"

DOUBLE_LINK_NODE* create_double_link_node(void *pdata)
{
	DOUBLE_LINK_NODE* pDLinkNode = NULL;
	pDLinkNode = (DOUBLE_LINK_NODE*)malloc(sizeof(DOUBLE_LINK_NODE));
	if (NULL == pDLinkNode)
		return NULL;

	memset(pDLinkNode, 0, sizeof(DOUBLE_LINK_NODE));
	pDLinkNode->data = pdata;
	return pDLinkNode;
}

void delete_all_double_link_node(DOUBLE_LINK_NODE** pDLinkNode)
{
	DOUBLE_LINK_NODE* pNode;
	if(NULL == *pDLinkNode)
		return ;

	pNode = *pDLinkNode;
	*pDLinkNode = pNode->next;

	/* release data buff */
	if (pNode->data) free (pNode->data);

	/* release node buff */
	free(pNode);

	delete_all_double_link_node(pDLinkNode);
}

DOUBLE_LINK_NODE* find_data_in_double_link(const DOUBLE_LINK_NODE* pDLinkNode, void *pdata, fCompare fComp)
{
	DOUBLE_LINK_NODE* pNode = NULL;
	if(NULL == pDLinkNode)
		return NULL;

	pNode = (DOUBLE_LINK_NODE*)pDLinkNode;
	while(NULL != pNode){
		if(fComp(pdata, pNode->data))
			return pNode;
		pNode = pNode ->next;
	}
	
	return NULL;
}

BOOL insert_data_into_double_link(DOUBLE_LINK_NODE** ppDLinkNode, void *pdata, fCompare fComp)
{
	DOUBLE_LINK_NODE* pNode;
	DOUBLE_LINK_NODE* pIndex;

	if(NULL == ppDLinkNode)
		return FALSE;

	if(NULL == *ppDLinkNode){
		pNode = create_double_link_node(pdata);
		if (NULL == pNode)
			return FALSE;

		*ppDLinkNode = pNode;
		(*ppDLinkNode)->prev = (*ppDLinkNode)->next = NULL;
		return TRUE;
	}

	if(NULL != find_data_in_double_link(*ppDLinkNode, pdata, fComp))
		return FALSE;

	pNode = create_double_link_node(pdata);
	if (NULL == pNode)
		return FALSE;

	pIndex = *ppDLinkNode;
	while(NULL != pIndex->next)
		pIndex = pIndex->next;

	pNode->prev = pIndex;
	pNode->next = pIndex->next;
	pIndex->next = pNode;
	return TRUE;
}

BOOL delete_data_from_double_link(DOUBLE_LINK_NODE** ppDLinkNode, void *pdata, fCompare fComp)
{
	DOUBLE_LINK_NODE* pNode;
	if(NULL == ppDLinkNode || NULL == *ppDLinkNode)
		return FALSE;

	pNode = find_data_in_double_link(*ppDLinkNode, pdata, fComp);
	if(NULL == pNode)
		return FALSE;

	if(pNode == *ppDLinkNode){
		if(NULL == (*ppDLinkNode)->next){
			*ppDLinkNode = NULL;
		}else{
			*ppDLinkNode = pNode->next;
			(*ppDLinkNode)->prev = NULL;
		}

	}else{
		if(pNode->next)
		    pNode->next->prev = pNode->prev;
	    pNode->prev->next = pNode->next;
	}
	
	/* release data buff */
	if (pNode->data) free (pNode->data);

	/* release node buff */
	free(pNode);
	return TRUE;
}

int count_number_in_double_link(const DOUBLE_LINK_NODE* pDLinkNode)
{
	int count = 0;
	DOUBLE_LINK_NODE* pNode = (DOUBLE_LINK_NODE*)pDLinkNode;

	while(NULL != pNode){
		count ++;
		pNode = pNode->next;
	}
	return count;
}

void print_double_link_node(const DOUBLE_LINK_NODE* pDLinkNode, fPrint fPrin)
{
	DOUBLE_LINK_NODE* pNode = (DOUBLE_LINK_NODE*)pDLinkNode;

	while ((NULL != pNode) && (NULL != fPrin))
	{
		fPrin(pNode->data);

		pNode = pNode ->next;
	}
}

#if defined(__cplusplus)
  }
#endif
