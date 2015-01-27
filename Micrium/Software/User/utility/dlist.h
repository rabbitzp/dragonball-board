/*
*********************************************************************************************************
*
*                                          dual list 
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : dlist.h
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/

#ifndef __DLIST_H__
#define __DLIST_H__

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

#ifndef BOOL
#define BOOL int
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

typedef BOOL (*fCompare)(void *pdata_left, void *data_right);
typedef void (*fPrint)(void *pdata);

typedef struct _DOUBLE_LINK_NODE
{
	void *data;
	struct _DOUBLE_LINK_NODE* prev;
	struct _DOUBLE_LINK_NODE* next;
}DOUBLE_LINK_NODE;

DOUBLE_LINK_NODE*	create_double_link_node(void *pdata);
DOUBLE_LINK_NODE*	find_data_in_double_link(const DOUBLE_LINK_NODE* pDLinkNode, void *pdata, fCompare fComp);
BOOL				insert_data_into_double_link(DOUBLE_LINK_NODE** ppDLinkNode, void *pdata, fCompare fComp);
BOOL				delete_data_from_double_link(DOUBLE_LINK_NODE** ppDLinkNode, void *pdata, fCompare fComp);
void				delete_all_double_link_node(DOUBLE_LINK_NODE** pDLinkNode);
int					count_number_in_double_link(const DOUBLE_LINK_NODE* pDLinkNode);
void				print_double_link_node(const DOUBLE_LINK_NODE* pDLinkNode, fPrint fPrin);

#if defined(__cplusplus)
}
#endif

#endif

