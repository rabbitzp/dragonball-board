/*
*********************************************************************************************************
*
*                                          user ep manager 
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : user_ep_manage.h
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/

#ifndef __USER_EP_MANAGE_H__
#define __USER_EP_MANAGE_H__

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*--------------macro declare here-----------------------------*/
#define MAX_EP_NAME_LEN 8

/*--------------enum declare here-----------------------------*/
typedef enum _tagEpType
{
    EP_TYPE_EP      = 0,
    EP_TYPE_ROUTER  = 1,
    EP_TYPE_COOR    = 2,

    EP_TYPE_MAX,
    EP_TYPE_END = 0xFF,
}EP_TYPE_E;

/*--------------struct declare here-----------------------------*/
typedef struct _tagEPInfo 
{
	u8  ucEpId;
	u8  ucEpType;
	u16 usEpAddr;
	u8  sEpName[MAX_EP_NAME_LEN];
}EP_INFO_S;

/*--------------functions declare here-----------------------------*/
u8          UEM_Init(void);
u8          UEM_AddEp(u8 ucEpId, u8 ucEpType, u16 usEpAddr, u8 *sEpName);
u8          UEM_DelEp(u8 ucEpId);
u8          UEM_UpdateEp(u8 ucEpId, u8 ucEpType, u16 usEpAddr, u8 *sEpName);
EP_INFO_S   *UEM_FindEp(u8 ucEpId);

#if defined(__cplusplus)
}
#endif

#endif

