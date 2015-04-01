/*
*********************************************************************************************************
*
*                                      eem structs define
*
*                                            J.B.S
*                                            with the
*                                       DragonBall Project
*
* Filename      : eem_struct.h
* Version       : V1.00
* Programmer(s) : JohnniBlack
*********************************************************************************************************
*/

#ifndef __EEM_STRUCT_H__
#define __EEM_STRUCT_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* Exported structs ------------------------------------------------------- */
typedef struct _tagEemEpInfo
{
    u8  ucEpId;
    u8  ucEpType;
    u16 usEpAddr;
}EEM_EP_INFO_S;

typedef struct _tagGpsINFO
{
	u8			epid;		    /**< device id */    
	u8			status;			/**< Status (A = active or V = void) */
	u8			ns;				/**< [N]orth or [S]outh */
	u8			ew;				/**< [E]ast or [W]est */
	double  	lat;	        /**< Latitude in NDEG - [degree][min].[sec/60] */
	double		lon;	        /**< Longitude in NDEG - [degree][min].[sec/60] */
	double		speed;	        /**< Speed over the ground in knots */    
    double      course;         /**< direction, 000.0~359.9, base N */    
} EEM_EP_GPS_INFO_S;

#ifdef __cplusplus
}
#endif

#endif

