/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\FM\FmDevice.h
* Owner: cjh
* Date: 2016.3.15
* Time: 11:20:49
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    cjh     2016.3.15     11:20:49   1.0
********************************************************************************************
*/


#ifndef __DRIVER_FM_FMDEVICE_H__
#define __DRIVER_FM_FMDEVICE_H__

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
#define     FREQMAXNUMBLE           30

#define     RADIO_CHINA         0                       //china
#define     RADIO_EUROPE        1                       //europe
#define     RADIO_JAPAN         2                       //japan
#define     RADIO_USA           3                       //america

typedef struct _RK_FM_DRIVER
{
    rk_err_t   (*Tuner_SetInitArea)(HDC, UINT8 );
    rk_err_t   (*Tuner_SetFrequency)(HDC, UINT32 , UINT8 , UINT8 ,UINT16 );
    rk_err_t   (*Tuner_SetStereo)(HDC, UINT8);
    rk_err_t   (*Tuner_SetVolume)(HDC, uint8);

    UINT16 (*Tuner_SearchByHand)(HDC, UINT32);
    rk_err_t   (*Tuner_PowerDown)(HDC);
    rk_err_t   (*Tuner_MuteControl)(HDC, UINT8);
    BOOLEAN    (*GetStereoStatus)(HDC);

    void *next;

}RK_FM_DRIVER;

typedef enum _FMDEV_STATE
{
    FM_STATE_INVALID_STATION,
    FM_STATE_SEARCH_FAIL,
    FM_STATE_CHANGE_STATION,

}FMDEV_STATE;

typedef enum
{
    FM_SearchFail = 0,
    FM_InvalidStation,
    FM_FoundStation
}FM_SEARCH_RESULT;

typedef  struct _FM_DEV_ARG
{
    HDC hControlBus;     //i2c
    RK_FM_DRIVER *pFMDriver;
    UINT16 FmArea;
    void * pfmStateCallBcak;

}FM_DEV_ARG;

#define _DRIVER_FM_FMDEVICE_COMMON_  __attribute__((section("driver_fm_fmdevice_common")))
#define _DRIVER_FM_FMDEVICE_INIT_  __attribute__((section("driver_fm_fmdevice_init")))
#define _DRIVER_FM_FMDEVICE_SHELL_  __attribute__((section("driver_fm_fmdevice_shell")))
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
extern UINT32 FmDevGetMinOrMaxFreq(HDC dev, uint32 minMax);
extern rk_err_t FMDevPowerOffDeinit(HDC dev);
extern rk_err_t FMDevStepChangeFreq(HDC dev, int16 updownflag, UINT32 *pFmFreq, UINT16 step);
extern BOOLEAN FmDevGetStereoStatus(HDC dev);
extern rk_err_t FmDevVolSet(HDC dev, UINT8 gain);
extern rk_err_t FmDevMuteControl(HDC dev, UINT8 active);
extern rk_err_t FmDevPlay(HDC dev);
extern rk_err_t FmDevPause(HDC dev);
extern rk_err_t FmDevStop(HDC dev);
extern rk_err_t FmDevStart(HDC dev, UINT32 FmFreq, BOOLEAN FmStereo, UINT32 FmArea);
extern UINT16 FmDevSearchByHand(HDC dev, UINT32 FmFreq);
extern rk_err_t FmDevSetStereo(HDC dev, UINT8 bStereo);
extern rk_err_t FmDevSetInitArea(HDC dev, UINT8 area);
extern rk_err_t FmDev_Shell(HDC dev, uint8 * pstr);
extern rk_err_t FmDev_Write(HDC dev);
extern rk_err_t FmDev_Read(HDC dev);
extern rk_err_t FmDev_Delete(uint32 DevID, void * arg);
extern HDC FmDev_Create(uint32 DevID, void * arg);



#endif
