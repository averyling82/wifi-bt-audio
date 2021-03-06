/*
********************************************************************************************
*
*                Copyright (c):  Fuzhou Rockchip Electronic Co., Ltd
*                             All rights reserved.
*
* FileName: ..\Gui\GUISelect.h
* Owner: Benjo.lei
* Date: 2015.10.20
* Time: 15:07:35
* Desc:
* History:
*   <author>    <date>       <time>     <version>     <Desc>
* Benjo.lei      2015.10.14     15:07:35   1.0
********************************************************************************************
*/

#ifndef __GUI_GUISELECT_H__
#define __GUI_GUISELECT_H__

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
extern rk_err_t GUI_SelectDisplay(HGC pGc);
extern GUI_CONTROL_CLASS * GUI_SelectCreate(void *arg);
extern rk_err_t GUI_SelectDelete(HGC pGc);
#endif

