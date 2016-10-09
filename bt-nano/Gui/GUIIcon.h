/*
********************************************************************************************
*
*                Copyright (c):  Fuzhou Rockchip Electronic Co., Ltd
*                             All rights reserved.
*
* FileName: ..\Gui\GUIIcon.h
* Owner: Benjo.lei
* Date: 2015.10.14
* Time: 15:07:35
* Desc:
* History:
*   <author>    <date>       <time>     <version>     <Desc>
* Benjo.lei      2015.10.14     15:07:35   1.0
********************************************************************************************
*/

#ifndef __GUI_GUIICON_H__
#define __GUI_GUIICON_H__

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
typedef struct _GUI_ICON_CLASS
{
    GUI_CONTROL_CLASS pfControl;
    PICTURE_INFO_STRUCT psPictureInfo;
}GUI_ICON_CLASS;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern rk_err_t GUI_IconSetID(HDC pGc, uint32 ID);
extern  rk_err_t GUI_IconInit(HDC pGc, void * arg);
extern rk_err_t GUI_IconSetConceal(HGC pGc, int conceal);
extern rk_err_t GUI_IconSetContent(HGC pGc, uint32 ResourceId);
extern rk_err_t GUI_IconGetSize(HGC pGc, int *xSize, int *ySize);
extern rk_err_t GUI_DisplayResource(HGC pGc);
extern rk_err_t GUI_IconSetRotate(HGC Class, RK_LCD_ROTATE rotate);
extern rk_err_t GUI_IconSetCoordinates(HGC pGc, int x, int y);
extern rk_err_t GUI_IconDisplay(HDC pGc);
extern GUI_CONTROL_CLASS * GUI_IconCreate(void *arg);
extern rk_err_t GUI_IconDelete(HGC hIcon);
extern rk_err_t GUI_IconAreaUpdata(HDC hDisplay, int x, int y, int xSize, int ySize, int bckx, int bcky, PICTURE_INFO_STRUCT * psPictureInfo);
extern rk_err_t GUI_IconDisplayResource(HDC hDisplay, int x0, int y0, int x1, int y1, PICTURE_INFO_STRUCT * psPictureInfo);
#endif

