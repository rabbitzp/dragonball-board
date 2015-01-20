/*
*********************************************************************************************************
*                                             uC/GUI V3.98
*                        Universal graphic software for embedded applications
*
*                       (c) Copyright 2002, Micrium Inc., Weston, FL
*                       (c) Copyright 2002, SEGGER Microcontroller Systeme GmbH
*
*              µC/GUI is protected by international copyright laws. Knowledge of the
*              source code may not be used to write a similar product. This file may
*              only be used in accordance with a license and should not be redistributed
*              in any way. We appreciate your understanding and fairness.
*
----------------------------------------------------------------------
File        : LCDConf_1375_C8_C320x240.h
Purpose     : Sample configuration file
----------------------------------------------------------------------
*/

#ifndef LCDCONF_H
#define LCDCONF_H

/*********************************************************************
*
*                   General configuration of LCD
*
**********************************************************************
*/

#define LCD_XSIZE      (220)   /* X-resolution of LCD, Logical coor. */
#define LCD_YSIZE      (220)   /* Y-resolution of LCD, Logical coor. */

#define LCD_BITSPERPIXEL    (16)
#define LCD_FIXEDPALETTE    (565)

#define LCD_CONTROLLER   -1

#if 0
#define LCD_INIT_CONTROLLER()                                                                    \
        LCD_WRITE_REGLH(0x00>>1,LCD_REG0, LCD_REG1);                                             \
        LCD_WRITE_REGLH(0x02>>1,LCD_REG2, LCD_REG3);                                             \
        LCD_WRITE_REGLH(0x04>>1,LCD_REG4, LCD_REG5);                                             \
        LCD_WRITE_REGLH(0x06>>1,LCD_REG6, LCD_REG7);                                             \
        LCD_WRITE_REGLH(0x08>>1,LCD_REG8, LCD_REG9);                                             \
        LCD_WRITE_REGLH(0x0a>>1,LCD_REGA, LCD_REGB);                                             \
        LCD_WRITE_REGLH(0x0c>>1,LCD_REGC, LCD_REGD);                                             \
        LCD_WRITE_REG  (0x0e>>1,0x00);                 /* 0, screen 2 start l*/                  \
        LCD_WRITE_REG  (0x10>>1,0x00);                 /* screen 2 start h */                    \
        LCD_WRITE_REGLH(0x12>>1,LCD_REG12, LCD_REG13); /* mem adr. offset, screen 1 vsize(lsb)*/ \
        LCD_WRITE_REGLH(0x14>>1,LCD_REG14, 0);                                                   \
        LCD_WRITE_REGLH(0x1a>>1,0, LCD_REG1B);                                                   \
        LCD_WRITE_REGLH(0x1c>>1,LCD_REG1C, 0)
#endif

#define LCD_INIT_CONTROLLER()     LCD_R61505W_Init();
 
#endif /* LCDCONF_H */

	 	 			 		    	 				 	  			   	 	 	 	 	 	  	  	      	   		 	 	 		  		  	 		 	  	  			     			       	   	 			  		    	 	     	 				  	 					 	 			   	  	  			 				 		 	 	 			     			 
