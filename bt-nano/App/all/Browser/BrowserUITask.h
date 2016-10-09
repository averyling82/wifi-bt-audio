/*
********************************************************************************************
*
*        Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\Browser\BrowserUITask.h
* Owner: cjh
* Date: 2015.12.28
* Time: 15:14:43
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    cjh     2015.12.28     15:14:43   1.0
********************************************************************************************
*/


#ifndef __APP_BROWSER_BROWSERUITASK_H__
#define __APP_BROWSER_BROWSERUITASK_H__

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

typedef enum
{
    BROWSER_UPDATA = 0x11,
    BROWSER_ENTER,
    BROWSER_DLETE_MSGBOX,

} eBROWSER_TYPE;

//data structure
//type is correspond with display icon and find file type,do not change order.
typedef enum _FileType
{                       //correspond icon   finding file type
    FileTypeFolder = 0, //diretion
    FileTypeAudio,      //music       AudioFileExtString
    FileTypeVideo,      //video        VideoFileExtString
    FileTypeText,       //text       PictureFileExtString
#ifdef _M3U_
    FileTypeM3u,        //M3U
#endif
    FileTypePicture,    //picture        TextFileExtString
    FileTypeALL,        //unkown       ALLFileExtString
    FileTypeDisk1,      //disk
    FileTypeDisk2,      //SD card
} FileType;

typedef enum _BROW_TYPE
{
    BROW_FILE_FROM_DATABASE = 0,
    BROW_FILE_FROM_FS,

}BROW_TYPE;

#define _APP_BROWSER_BROWSERUITASK_COMMON_  __attribute__((section("app_browser_browseruitask_common")))
#define _APP_BROWSER_BROWSERUITASK_INIT_  __attribute__((section("app_browser_browseruitask_common")))
#define _APP_BROWSER_BROWSERUITASK_SHELL_  __attribute__((section("app_browser_browseruitask_shell")))
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
extern rk_err_t BrowserUITask_Resume(uint32 ObjectID);
extern rk_err_t BrowserUITask_Suspend(uint32 ObjectID);
extern void BrowserUITask_Enter(void * arg);
extern rk_err_t BrowserUITask_DeInit(void *pvParameters);
extern rk_err_t BrowserUITask_Init(void *pvParameters, void *arg);



#endif
