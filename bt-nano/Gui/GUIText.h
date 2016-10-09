/*
********************************************************************************************
*
*                Copyright (c):  Fuzhou Rockchip Electronic Co., Ltd
*                             All rights reserved.
*
* FileName: ..\Gui\GUIText.h
* Owner: Benjo.lei
* Date: 2015.10.14
* Time: 15:07:35
* Desc:
* History:
*   <author>    <date>       <time>     <version>     <Desc>
* Benjo.lei      2015.10.14     15:07:35   1.0
********************************************************************************************
*/

#ifndef __GUI_GUITEXT_H__
#define __GUI_GUITEXT_H__

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
typedef rk_err_t (* P_TEXT_RECIVE)(HGC pGc);


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef struct _GUI_TEXT_CLASS
{
    GUI_CONTROL_CLASS pfControl;
    eTEXT_DISPLAY_CMD cmd;
    int align;

    int StartX;
    int StartY;
    int xSize;
    int ySize;

    int xAuto;
    int yAuto;

    int Backdrop;
    int BackdropX;
    int BackdropY;

    int ForegroundR;
    int ForegroundG;
    int ForegroundB;

    int BackgroundR;
    int BackgroundG;
    int BackgroundB;

    FONT_TYPE font;

    int speed;

    Ucs2 * text;
    Ucs2 * text_bck;
}GUI_TEXT_CLASS;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern rk_err_t GUI_TextSizeCmp(FONT_TYPE font, Ucs2 * text, int size);
extern rk_err_t GUI_TextSetMove(HGC pGc, int speed);
extern uint32 GUI_TextGetStrWidth(HGC pGc);
extern rk_err_t GUI_TextSetContent(HGC pGc, void * arg);
extern rk_err_t GUI_TextDisplay(HGC pGc);
extern GUI_CONTROL_CLASS * GUI_TextCreate(void *arg);
extern rk_err_t GUI_TextDelete(HGC hIcon);
extern Ucs2 * GUI_TextExtractString(int resource);
extern int GUI_TextExtractSize(FONT_TYPE font, Ucs2 * text);
extern rk_err_t GUI_TextDisplayBuff(HDC hDisplay, int x0, int y0, int x1, int y1, Ucs2 * text, FONT_TYPE font, int R, int G, int B, int transparency);
extern rk_err_t GUI_TextDisplayID(HDC hDisplay, int x0, int y0, int x1, int y1, int id, FONT_TYPE font, int R, int G, int B, int transparency);
extern rk_err_t GUI_TextAutoAlign(FONT_TYPE font, int align, int x0, int y0, int xSize, int ySize,int *StartX, int *StartY, int *DestXSize, int *DestYSize, Ucs2 *text);
#endif

