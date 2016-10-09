/*
********************************************************************************************
*
*  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                        All rights reserved.
*
* FileName: ..\Gui\GUITask.h
* Owner: zhuzhe
* Date: 2014.4.28
* Time: 16:01:47
* Desc:
* History:
*     <author>     <date>       <time>     <version>       <Desc>
* zhuzhe     2014.4.28     16:01:47   1.0
********************************************************************************************
*/

#ifndef __GUI_GUITASK_H__
#define __GUI_GUITASK_H__

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
#define _GUI_GUITASK_SHELL_  __attribute__((section("gui_guitask_shell")))
#define _GUI_GUITASK_COMMON_ __attribute__((section("gui_guitask_common")))
#define _GUI_GUITASK_INIT_   __attribute__((section("gui_guitask_common")))

#define GUI_EVENT_KEY            0x01

#define APP_REQUEST_FOCUS        0x05
#define APP_REQUEST_RECIVE_MSG   0x06
#define GUI_EVENT_DELETE         0x07
#define GUI_EVENT_CREATE         0x08
#define GUI_EVENT_OPERATE        0x09
#define GUI_OPERATE_RESOURCE     0x0A

typedef void * HGC;
typedef enum
{
    APP_RECIVE_MSG_EVENT_KEY = 0,
    APP_RECIVE_MSG_EVENT_WARING,
    APP_RECIVE_MSG_EVENT_DIALOG,
} APP_RECIVE_MSG_EVENT;
typedef rk_err_t (* P_APP_RECIVE_MSG)(APP_RECIVE_MSG_EVENT evnet_type, int evnet, void * arg, HGC pGc);

#define __TEXT(quote) L##quote
#define TEXT(quote) __TEXT(quote)

#define RKGUI_MAX_RESOURCE    256

typedef struct _RKGUI_OPERATE_ARG
{
    HGC pGc;
    void * arg;
}RKGUI_OPERATE_ARG;

typedef  struct _GUI_RESP_QUEUE
{
    uint32 error_code;
    void * arg;
}GUI_RESP_QUEUE;

typedef enum
{
    GUI_CLASS_IMAGE = 0,
    GUI_CLASS_ICON,
    GUI_CLASS_SELECT,
    GUI_CLASS_TEXT,
    GUI_CLASS_CHAIN,
    GUI_CLASS_MSG_BOX,
    GUI_CLASS_TEXTBOX,
    GUI_CLASS_PROGRESSBAR,
    GUI_CLASS_SPECTRUM,
} eGc;

typedef enum
{
    OPERATE_DISPLAY = 0,
    OPERATE_SET_CONTENT,
    OPERATE_SET_DISPLAY,
    OPERATE_START_MOVE,
} eRKGUI_OPERATE_CMD;

typedef enum
{
    LUCENCY = 0,
    OPACITY,
}LUCENCY_TYPE;

typedef struct _GUI_CONTROL_CLASS
{
    struct _GUI_CONTROL_CLASS * next;
    struct _GUI_CONTROL_CLASS * prev;
    struct _GUI_CONTROL_CLASS * updata;
    eGc ClassID;

    HDC hDisplay;

    int x0;
    int y0;
    int xSize;
    int ySize;
    uint32 display;
    RK_LCD_ROTATE rotate;
    int level;

    int flag;
    LUCENCY_TYPE lucency;
    int translucence;
    void * focus;
}GUI_CONTROL_CLASS;

typedef enum
{
    LANGUAGE_CHINESE_S = 0,                //Simplified Chinese.
    LANGUAGE_CHINESE_T ,                //Traditional Chinese
    LANGUAGE_ENGLISH ,                    //Englis
    LANGUAGE_KOREAN ,                    //Korean
    LANGUAGE_JAPANESE ,                    //Japanese
    LANGUAGE_FRENCH ,                    //French
    LANGUAGE_GERMAN ,                    //German
    LANGUAGE_PORTUGUESE ,                //Portuguess
    LANGUAGE_RUSSIAN ,                    //Russian
    LANGUAGE_SPAISH ,                    //Spanish
    LANGUAGE_ITALIAN ,                     //Italian
    LANGUAGE_SWEDISH ,                    //Swedish
    LANGUAGE_THAI ,                        //Thai
    LANGUAGE_POLAND    ,                    //Polish
    LANGUAGE_DENISH ,                    //Danish
    LANGUAGE_DUTCH ,                    //Dutch
    LANGUAGE_HELLENIC ,                    //Greek
    LANGUAGE_CZECHIC ,                    //Czech
    LANGUAGE_TURKIC    ,                    //Turkish
    LANGUAGE_RABBINIC ,                    //Hebrew
    LANGUAGE_ARABIC    ,                    //Arabic
    LANGUAGE_MAX_COUNT,
} LANGUAGE_TYPE;

#ifdef _SPI_BOOT_
typedef struct _SPI_BOOT_RESOURCE
{
    HDC  Font12;
    HDC  Font16;
    HDC  Image;
    HDC  Menu;
    void * osReadSource;
} SPI_BOOT_RESOURCE;

extern SPI_BOOT_RESOURCE gstSpiResoure;
#endif

#define          MENU_ITEM_LENGTH            (unsigned short)254             // The length of menu item in bytes
#define          TOTAL_MENU_ITEM             (unsigned short)171
#define          TOTAL_LANAUAGE_NUM          (unsigned short)19
#define          MAX_SUBMENU_NUM             (unsigned short)19
#define          MENU_CONTENT_OFFSET         (unsigned short)54


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern HDC hDisplay0;
extern HDC hDisplay1;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/

extern void GUITask_Enter(void);
extern rk_err_t GUITask_DeInit(void *pvParameters);
extern rk_err_t GUITask_Init(void *pvParameters, void *arg);
extern rk_err_t GUITask_SendMsg(uint32 evnet_type, uint32 event, void * arg, uint32 mode);

extern HGC GUITask_CreateWidget(eGc ClassID, void * arg);
extern rk_err_t GuiTask_DeleteWidget(HGC pGc);
extern rk_err_t GuiTask_OperWidget(HGC pGc, eRKGUI_OPERATE_CMD cmd, void *arg, uint32 mode);

extern rk_err_t GuiTask_AppReciveMsg(P_APP_RECIVE_MSG pApp);
extern rk_err_t GuiTask_AppUnReciveMsg(P_APP_RECIVE_MSG pApp);

extern rk_err_t GuiTask_UnloadResource(void);
extern rk_err_t GuiTask_LoadResource(void);

extern rk_err_t GuiTask_TimerLock(void);
extern rk_err_t GuiTask_TimerUnLock(void);
#ifdef _GUI_SHELL_
extern rk_err_t GuiShell(HDC dev, uint8 * pstr);
#endif
#endif

