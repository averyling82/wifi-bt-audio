/*
********************************************************************************************
*
*                Copyright (c):  Fuzhou Rockchip Electronic Co., Ltd
*                             All rights reserved.
*
* FileName: ..\Gui\GUISeekBar.h
* Owner: Benjo.lei
* Date: 2015.10.14
* Time: 15:07:35
* Desc:
* History:
*   <author>    <date>       <time>     <version>     <Desc>
* Benjo.lei      2015.10.14     15:07:35   1.0
********************************************************************************************
*/

#ifndef __GUI_GUISEEKBAR_H__
#define __GUI_GUISEEKBAR_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern rk_err_t GUI_SeekBarDrag(HGC pGc, uint32 drag);
extern rk_err_t GUI_SeekBarSetScale(HGC pGc, uint32 scale);
extern rk_err_t GUI_SeekBarGetScollSize(HGC pGc, int *x, int *y);
extern rk_err_t GUI_SeekBarSetStyle(HGC pGc, uint32 Scoll, uint32 Block);
extern rk_err_t GUI_SeekBarSetCoordinates(HDC pGc, int x, int y);
extern GUI_CONTROL_CLASS * GUI_SeekBarCreate(void *arg);
extern rk_err_t GUI_SeekBarDisplay(HGC pGc);
#endif

