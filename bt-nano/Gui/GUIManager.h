/*
********************************************************************************************
*
*  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* FileName: ..\Gui\GUIManager.h
* Owner: Benjo.lei
* Date: 2015.10.14
* Time: 15:07:35
* Desc:
* History:
*   <author>    <date>       <time>     <version>     <Desc>
* Benjo.lei      2015.10.14     15:07:35   1.0
********************************************************************************************
*/

#ifndef __GUI_GUIMANAGER_H__
#define __GUI_GUIMANAGER_H__

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
#define ICON_PIXEL_WIDE  2
#define MAX_DISP_ITEM_NUM       8 //the max item number that one screen can display.

typedef struct
{
    int x;
    int y;
}RKGUI_XY_ARG;

typedef enum
{
    TYPE12X12,
    TYPE16X16,
}FONT_TYPE;

typedef struct
{
    int display;
    uint32 resource;
    int x;
    int y;
    int level;
}RKGUI_ICON_ARG;

typedef struct
{
    int display;
    uint32 resource;
    int num;
    int x;
    int y;
    int delay;
    int level;
    int blurry;
}RKGUI_CHAIN_ARG;

typedef enum
{
    PROGRESSBAR_SET_CONTENT,
} ePROGRESSBAR_OPERATE_CMD;

typedef struct
{
    int display;
    int x;
    int y;
    int level;

    int Backdrop;
    int BackdropX;
    int BackdropY;

    int Bar;
    int percentage;

    ePROGRESSBAR_OPERATE_CMD cmd;
}RKGUI_PROGRESSBAR_ARG;

typedef enum
{
    IMAGE_CMD_IMAGE = 0,
    IMAGE_CMD_RECT,
    IMAGE_CMD_PIXEL,
} eIMAGE_DISPLAY_CMD;

typedef  struct
{
    int display;
    int level;
    eIMAGE_DISPLAY_CMD cmd;
    LUCENCY_TYPE lucency;
    int x;
    int y;
    int xSize;
    int ySize;
    void * buf;
    uint8 R;
    uint8 G;
    uint8 B;
    uint8 transparency;
}RKGUI_IMAGE_ARG;

typedef enum
{
    TEXT_CMD_BUF = 0,
    TEXT_CMD_ID,
} eTEXT_DISPLAY_CMD;


#define TEXT_ALIGN_X_LEFT     0x00
#define TEXT_ALIGN_X_RIGHT    0x01
#define TEXT_ALIGN_X_CENTER   0x02
#define TEXT_ALIGN_X_MASK     0x0F

#define TEXT_ALIGN_Y_UP       0x00
#define TEXT_ALIGN_Y_DOWN     0x10
#define TEXT_ALIGN_Y_CENTER   0x20
#define TEXT_ALIGN_Y_MASK     0xF0

typedef enum
{
    TEXT_SET_CONTENT,
    TEXT_SET_COLOUR,
    TEXT_SET_ALIGN,
    TEXT_SET_BACKGROUND
} eTEXT_OPERATE_CMD;

typedef  struct _RKGUI_TEXT_ARG
{
    eTEXT_DISPLAY_CMD cmd;
    int align;

    int display;
    int level;
    int resource;
    void * text;
    int x;
    int y;
    int xSize;
    int ySize;

    LUCENCY_TYPE lucency;
    int Backdrop;
    int BackdropX;
    int BackdropY;

    int ForegroundR;
    int ForegroundG;
    int ForegroundB;

    int BackgroundR;
    int BackgroundG;
    int BackgroundB;

    eTEXT_OPERATE_CMD opercmd;
}RKGUI_TEXT_ARG;

typedef struct _RKGUI_SELECT_ITEM
{
    eTEXT_DISPLAY_CMD cmd;
    Ucs2 * text;
    int text_id;
    int sel_icon;
    int unsel_icon;
}RKGUI_SELECT_ITEM;

typedef enum
{
    SELECT_ENVEN_ENTER,
    SELECT_ENVEN_UPDATA,
    SELECT_ENVEN_MOVE,
} eSELECT_EVENT_TYPE;

typedef rk_err_t (* P_SELECT_RECIVE_MSG)(HGC pGc, eSELECT_EVENT_TYPE evnet_type, void * item, int offset);
typedef struct
{
    int x;
    int y;
    int xSize;
    int ySize;
    int level;
    int display;

    int itemNum;
    int Cursor;
    int ItemStartOffset;
    int Background;
    int MaxDisplayItem;

    int IconBoxSize;
    int SeekBar;
    int CursorStyle;
    P_SELECT_RECIVE_MSG pReviceMsg;
}RKGUI_SELECT_ARG;

typedef struct
{
    int itemNum;
    int icon_sel;
}RKGUI_SELECT_CONTENT;

typedef enum
{
    SPECTRUM_SET_CONTENT,
} eSPECTRUM_OPERATE_CMD;

typedef struct
{
    int display;
    int x;
    int y;
    int level;

    int StripStyle;
    int Backdrop;
    int BackdropX;
    int BackdropY;
    int StripNum;
    uint8 * StripDat;

    eSPECTRUM_OPERATE_CMD cmd;
}RKGUI_SPECTRUM_ARG;

typedef enum
{
    MSGBOX_CMD_DIALOG = 0,
    MSGBOX_CMD_WARNING,
} eMSGBOX_DISPLAY_CMD;

typedef struct
{
    int display;
    int level;
    int x;
    int y;
    eMSGBOX_DISPLAY_CMD cmd;

    int title;
    int Button_On;
    int Button_Off;

    eTEXT_DISPLAY_CMD text_cmd;
    int align;
    void * text;
}RKGUI_MSGBOX_ARG;
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

#endif

