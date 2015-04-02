/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                          (c) Copyright 2003-2006; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                      APPLICATION CONFIGURATION
*
*                                     ST Microelectronics STM32
*                                              with the
*                                   STM3210E-EVAL Evaluation Board
*
* Filename      : app_cfg.h
* Version       : V1.00
* Programmer(s) : BAN
*********************************************************************************************************
*/

#ifndef  __APP_CFG_H__
#define  __APP_CFG_H__
/*
*********************************************************************************************************
*                                       HARDWARE ENABLE / DISABLE
*********************************************************************************************************
*/
#define OS_USER_LCD_SUPPORT                     DEF_ENABLED /* set DEF_DISABLED, if not have one */
#define OS_USER_HMC_SUPPORT                     DEF_DISABLED /* set DEF_DISABLED, if not have one */
#define OS_USER_GPS_SUPPORT                     DEF_ENABLED  /* set DEF_DISABLED, if not have one */

/*
*********************************************************************************************************
*                                       MODULE ENABLE / DISABLE
*********************************************************************************************************
*/
#if 0
// jiaozi
#define  APP_OS_PROBE_EN                         DEF_DISABLED
#define  APP_PROBE_COM_EN                        DEF_DISABLED

#else
#define  APP_OS_PROBE_EN                         DEF_ENABLED
#define  APP_PROBE_COM_EN                        DEF_ENABLED
#endif
/*
*********************************************************************************************************
*                                              TASKS NAMES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            TASK PRIORITIES
*********************************************************************************************************
*/

#define  APP_TASK_START_PRIO                               3
#define  APP_TASK_KBD_PRIO                                 4

#define  OS_PROBE_TASK_PRIO              (OS_LOWEST_PRIO - 3)
#define  OS_TASK_TMR_PRIO                (OS_LOWEST_PRIO - 2)

/*
*********************************************************************************************************
*                                            TASK STACK SIZES
*                             Size of the task stacks (# of OS_STK entries)
*********************************************************************************************************
*/
#if (DEF_ENABLED == OS_USER_LCD_SUPPORT)
#define  APP_TASK_USER_IF_PRIO                            12
#define  APP_TASK_USER_IF_STK_SIZE                       512
#endif

#define  OS_PROBE_TASK_STK_SIZE                          128

/*
*********************************************************************************************************
*                                                  LIB
*********************************************************************************************************
*/

#define  uC_CFG_OPTIMIZE_ASM_EN                 DEF_ENABLED
#define  LIB_STR_CFG_FP_EN                      DEF_DISABLED

/*
*********************************************************************************************************
*                                                 PROBE
*********************************************************************************************************
*/

#define  OS_PROBE_TASK                                     1    /* Task will be created for uC/Probe OS Plug-In.        */
#define  OS_PROBE_TMR_32_BITS                              0    /* uC/Probe OS Plugin timer is a 16-bit timer.          */
#define  OS_PROBE_TIMER_SEL                                2    /* Select timer 2.                                      */
#define  OS_PROBE_HOOKS_EN                                 1    /* Hooks to update OS_TCB profiling members included.   */
#define  OS_PROBE_USE_FP                                   1


/*
*********************************************************************************************************
*                                                 ON/OFF define
*********************************************************************************************************
*/
#define ENABLE_UART_DEBUG_CONSOLE                          1    /* Set 0, if you not using debug console */

#endif