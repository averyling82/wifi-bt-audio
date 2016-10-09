/*
********************************************************************************
*                   Copyright (c) 2008,Yangwenjie
*                         All rights reserved.
*
* File Name£º   Lcd.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             yangwenjie      2009-1-15          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _LCD_H_
#define _LCD_H_

#undef  EXT
#ifdef _IN_LCD_
#define EXT
#else
#define EXT extern
#endif

/*
--------------------------------------------------------------------------------

                        Macro define

--------------------------------------------------------------------------------
*/
//#define LCD_WIDTH 	128
//#define LCD_HEIGHT 	160

#define LCD_OFFSET_X    4
#define LCD_OFFSET_Y    4
//#define EMCTL_BASE  		(UINT32)0x61000000
//#define EMCTL_LCDWAIT_REG  	(UINT32)0x00000010
//#define EMCTL_LCD_DATA     	(UINT32)0x00000a04
/*
--------------------------------------------------------------------------------

                        Struct Define

--------------------------------------------------------------------------------
*/

typedef enum
{
    LCD_ROTARY_0 = (UINT32)0,
    LCD_ROTARY_90,
    LCD_ROTARY_180,
    LCD_ROTARY_270,

    LCD_ROTARY_MAX

} LCD_ROTARY_INFO;

typedef struct _LCD_ROTARY_PAPA_T
{
  //Lcd driver parameter.
  UINT16 Rotarypapa;           //rotation
  UINT16 XMirrorPapa;          //x mirror
  UINT16 YMirrorPapa;          //y mirror

} LCD_ROTARY_PAPA_T;

/*
--------------------------------------------------------------------------------

                        Variable Define

--------------------------------------------------------------------------------
*/


/*
--------------------------------------------------------------------------------

                        Funtion Declaration

--------------------------------------------------------------------------------
*/
/*
********************************************************************************
*
*                         End of Lcd.h
*
********************************************************************************
*/
#endif
