/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: System\syssever\PowerManager.h
* Owner: aaron.sun
* Date: 2015.11.8
* Time: 14:41:56
* Version: 1.0
* Desc: power manager
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.11.8     14:41:56   1.0
********************************************************************************************
*/


#ifndef __SYSTEM_SYSSEVER_POWERMANAGER_H__
#define __SYSTEM_SYSSEVER_POWERMANAGER_H__

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

#define _SYSTEM_SYSSEVER_POWERMANAGER_COMMON_  __attribute__((section("system_syssever_powermanager_common")))
#define _SYSTEM_SYSSEVER_POWERMANAGER_INIT_  __attribute__((section("system_syssever_powermanager_init")))
#define _SYSTEM_SYSSEVER_POWERMANAGER_SHELL_  __attribute__((section("system_syssever_powermanager_shell")))

typedef enum _FREQ_APP
{
    FREQ_IDLE = 0,
    FREQ_MIN,
    FREQ_INIT,
    FREQ_BLON,
    FREQ_AUDIO_INIT,
    FREQ_SBC,
    FREQ_MP3,
    FREQ_MP3H,
    FREQ_XXX,         //XXX is other decode by third company developed
    FREQ_XXXH,        //XXXH is other decode by third company developed
    FREQ_AAC,
    FREQ_WAV,
    FREQ_APE,
    FREQ_FLAC,
    FREQ_OGG,
    FREQ_NOGG,
    FREQ_HOGG,
    FREQ_EHOGG,
    FREQ_HAPE,
    FREQ_HFLAC,
    FREQ_HALAC,
    FREQ_HWAV,
    FREQ_EQ_L,
    FREQ_EQ,
    FREQ_EQ_H,
    FREQ_JPG,
    FREQ_FM,
    FREQ_RECORDWAV,
    FREQ_RECORDAMR,
    FREQ_RECORDXXX,
    FREQ_FMAUTOSEARCH,
    FREQ_MEDIAUPDATA,
    FREQ_USB,
    FREQ_BEEP,
    FREQ_DC_FILTER_LOW,
    FREQ_DC_FILTER_HIGH,
    FREQ_MEDIA_INIT,
    FREQ_AVI,
    FREQ_WIFI,
    FREQ_DLNA,
    FREQ_BT,
    FREQ_RKI6000,
    FREQ_5,
    FREQ_AMR,
    FREQ_MAX,
    FREQ_APP_MAX

}eFREQ_APP;

typedef struct tagCRU_CLK_INFO
{
    uint32 armFreq;        //ARM PLL FREQ
    uint32 armFreqLast;

} CRU_CLK_INFO,*pCRU_CLK_INFO;

typedef struct tagFREQ_APP_TABLE
{
    uint8  scuAppId;
    uint32 syshclk; //sysfclk
    uint32 calhclk; //calfclk

}FREQ_APP_TABLE,*pFREQ_APP_TABLE;

/*Battery*/
typedef struct
{
    UINT16 Batt_Level;
    UINT32 Batt_Value;
    UINT16 Batt_LowCnt;
    BOOL   Batt_LowPower;
    UINT16 Batt_Count;
    UINT16 UsbBatt_Level;
    BOOL   IsBatt_Charge;
    BOOL   Enable_Charge;
    BOOL   IsBattChange;//USB take it when pull out or power up.
}BATT_INFO;

#define PM_TIME 100

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern BATT_INFO    gBattery;
extern uint64       g_APPList;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern rk_err_t CheckSystemIdle(void);
extern void ClearSytemIdle(void);
extern void ChargeDisable(void);
extern void ChargeEnable(void);
extern void PowerManagerEnd(void);
extern void PowerManagerStart(void);
extern void PowerManagerInit(void);
extern void SetSysFreq(uint32 nMhz);
extern int32 FREQ_Enable(void);
extern int32 FREQ_Disable(void);
extern int32 FREQ_ExitModule(eFREQ_APP modulename);
extern int32 FREQ_EnterModule(eFREQ_APP modulename);
extern int32 FREQ_Resume(void);
#endif
