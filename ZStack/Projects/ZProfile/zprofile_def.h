/*
*********************************************************************************************************
*
*                                zigbee profile and endpoint define 
*
*                                            J.B.S
*                                            with the
*                                       General Purpose Project
*
* Filename      : zprofile_def.h
* Version       : V1.00
* Programmer(s) : JohnniBlack
* Describe      : 此头文件描述所有相关ZIGBEE设备的端点应用类型，属性，未经允许，不得随意修改!
*********************************************************************************************************
*/

#ifndef __ZPROFILE_DEF_H__
#define __ZPROFILE_DEF_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* 设备应用版本号定义 */
#define ZP_DEVICE_APP_VERSION     0
#define ZP_DEVICE_APP_FLAGS       0


/**********************************************************************
* 1. 关于PROFILE ID，按ZIGBEE联盟规定，0X0000-0X7FFF为公共规范ID，制造商私有的范围:0XBF00-0XFFFF
*/
/*********************************************************************
* 共用类设备定义
*/
#define ZP_GENERAL_DEVICE_ALL_ENDPOINT  0XFF

/*********************************************************************
 * 智能终端类设备定义
 */
/* PROFILE ID */
#define ZP_SA_PROFILE_ID                        0xBF02

/* Device IDs */
#define ZP_SA_DEVICEID_DRAGONBALL               0x0000  /* 龙珠 */

/* Device End Points */
#define ZP_SA_DRAGONBALL_ENDPOINT               10

/* Device clusters */
#define ZP_SA_MAX_CLUSTERS                      6

#define ZP_SA_CLUSTER_ID_GEN_BASIC              1
#define ZP_SA_CLUSTER_CONNECT_REQ               2  
#define ZP_SA_CLUSTER_CONNECT_RSP               3    
#define ZP_SA_CLUSTER_COMMAND_REQ               4    
#define ZP_SA_CLUSTER_COMMAND_RSP               5  
#define ZP_SA_CLUSTER_KEEP_ALIVE                6

#ifdef __cplusplus
}
#endif

#endif

