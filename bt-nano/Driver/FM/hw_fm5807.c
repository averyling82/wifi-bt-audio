/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\FM\hw_fm5807.c
* Owner: cjh
* Date: 2016.3.18
* Time: 14:59:26
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    cjh     2016.3.18     14:59:26   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __DRIVER_FM_HW_FM5807_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "BSP.h"
#include "DeviceInclude.h"
#include "hw_fm5807.h"


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#if (RKNANOD_CHIP_TYPE == RKNANOD_N)
#define FM5807_I2C_IOPORT  I2C_DEV0_PB
#define FM5807_I2C_DEV  I2C_DEV0
//#define FM5807_I2C_PORT     0
#else
#define FM5807_I2C_IOPORT  I2C_DEV1_PA
#define FM5807_I2C_DEV  I2C_DEV1
//#define FM5807_I2C_PORT     0
#endif


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
_DRIVER_FM_HW_FM5807_COMMON_ UINT8   TEA5800ControlData[5];
_DRIVER_FM_HW_FM5807_COMMON_ UINT16  RDA5800REG_IMG[60];
_DRIVER_FM_HW_FM5807_COMMON_ UINT8   READCHAN;
//UINT16  cwFMDefault[] =
_DRIVER_FM_HW_FM5807_COMMON_
UINT16    cwFM5807Default[]={                                    //init data table
0x0000,
0x0000,

//0xC0D1, //0xC451, //02H:  //24M crystal : 0xC0, 0xD1, 32.768k crystal: 0xC0, 0x01;
0xc081,//0x02        //d881-->9881 for first init don't need voice output,0xb881 no stereo

0x1a10,            //97100KHZ
0x0600,            //no interruput
0x9cad, //05h    0x88, 0xAF
0x0000,
0x46c6,
0x5096,
0x0000,
0x0468, //0x0a
0x650f,
0x0000,
0x0000,
0x0000,
0x0000,
0xf027,//4c08//0x4808,//0x10
0x9000,
0xf483,
0x7001,
0x40f0,
0x6180, //0x15
0x01D6,
0x10bb,
0x55a4,
0xe848,
0x5080,//0x1a
0x0000,
0x37b6,
0x420c,
0x079b,
0x4c1d,
0x8810,//0x20
0x45c0,
0x0923,
0x98a4,
0x0500,
0x1928, //0x25
0x2d04,
0x002f,
0x17af,
0xd34e,
0x348c,//0x2a
0xa004,
0x7eac,
0x0014,
0x0CFF,
0x3C1E,
0x0200, //30H:
0x16F4,
0xBD00,
0x830C,
0xB9FC,
0x8287, //35H:
0x0000,
0x0000,
0x028B,
0x8895,
0x8D9F, //3AH:
0x92AA,
};

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
rk_err_t FM5807_Tuner_SetInitArea(HDC dev, UINT8 area);
rk_err_t FM5807_Tuner_SetFrequency(HDC dev, UINT32 n10KHz, UINT8 HILO, UINT8 ForceMono,UINT16 Area);
rk_err_t FM5807_Tuner_SetStereo(HDC dev, UINT8 bStereo);
rk_err_t FM5807_Tuner_Vol_Set(HDC dev, UINT8 gain);
UINT16 FM5807_Tuner_SearchByHand(HDC dev, UINT32 FmFreq);
rk_err_t FM5807_Tuner_PowerOffDeinit(HDC dev);
rk_err_t FM5807_Tuner_PowerOnInit(HDC dev);
rk_err_t FM5807_Tuner_SetMute(HDC dev, UINT8 mute);
BOOLEAN FM5807_GetStereoStatus(HDC dev);


rk_err_t FM5807_Tuner_Disable(HDC dev);
rk_err_t FM5807_Tuner_Enable(HDC dev, UINT8 area);
UINT16 FM5807_FmSeek(HDC dev, UINT32 FmFreq);
UINT32 FM5807_FmChanToFreq(UINT8 channel);
UINT8 FmFreqToChan(UINT32 frequency);
rk_err_t FmTune(HDC dev, UINT32 frequency);
rk_err_t FmInitData(HDC dev);
rk_err_t FM5807_FmInitPowerOn(HDC dev, UINT8 area);
rk_size_t FM5807_Tuner_ReadData(HDC dev, UINT8 *pdata, UINT8 size);
rk_size_t FM5807_Tuner_SendCommand(HDC dev,UINT8 *pdata, UINT8 size);

RK_FM_DRIVER fm5807Driver =
{
    .Tuner_SetInitArea = FM5807_Tuner_SetInitArea,
    .Tuner_SetFrequency = FM5807_Tuner_SetFrequency,
    .Tuner_SetStereo  = FM5807_Tuner_SetStereo,
    .Tuner_SetVolume    = FM5807_Tuner_Vol_Set,
    .Tuner_SearchByHand    = FM5807_Tuner_SearchByHand,
    .Tuner_PowerDown   = FM5807_Tuner_PowerOffDeinit,
    .Tuner_MuteControl     = FM5807_Tuner_SetMute,
    .GetStereoStatus = FM5807_GetStereoStatus,
    .next = NULL,
};


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*******************************************************************************
** Name: FM5807_Tuner_SetFrequency
** Input:HDC dev, UINT32 n10KHz, UINT8 HILO, BOOL ForceMono,UINT16 Area
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.23
** Time: 13:40:57
*******************************************************************************/
_DRIVER_FM_HW_FM5807_COMMON_
COMMON API rk_err_t FM5807_Tuner_SetFrequency(HDC dev, UINT32 n10KHz, UINT8 HILO, UINT8 ForceMono,UINT16 Area)
{
    return FmTune(dev, n10KHz);
}
/*******************************************************************************
** Name: FM5807_Tuner_Vol_Set
** Input:HDC dev, UINT8 gain
** Return: k_err_t
** Owner:cjh
** Date: 2016.3.22
** Time: 17:51:12
*******************************************************************************/
_DRIVER_FM_HW_FM5807_COMMON_
COMMON API rk_err_t FM5807_Tuner_Vol_Set(HDC dev, UINT8 gain)
{
    UINT8 writeData8[8];

    writeData8[0] = RDA5800REG_IMG[2] >> 8;
    writeData8[1] = RDA5800REG_IMG[2] & 0xff;

    writeData8[2] = RDA5800REG_IMG[3] >> 8;
    writeData8[3] = RDA5800REG_IMG[3] & 0xff;

    writeData8[4] = RDA5800REG_IMG[4] >> 8;
    writeData8[5] = RDA5800REG_IMG[4] & 0xff;

    writeData8[6] = RDA5800REG_IMG[5] >> 8;
    writeData8[7] = RDA5800REG_IMG[5] & 0xff;
    writeData8[7] &= 0xF0;
    //writeData8[7] |= (Fm5807_Vol_Table[gain] & 0x0F);
    writeData8[7] |= (gain & 0x0F);
    writeData8[3] &= ~0x10; // for audio break

    RDA5800REG_IMG[5] = (RDA5800REG_IMG[5] & 0xfff0) + (writeData8[7] & 0x0f);

    if(FM5807_Tuner_SendCommand(dev, writeData8, 8) < 0)
    {
        return RK_ERROR;
    }
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FM5807_Tuner_SearchByHand
** Input:HDC dev, UINT16 direct, UINT32 *FmFreq
** Return: UINT16
** Owner:cjh
** Date: 2016.3.22
** Time: 17:46:16
*******************************************************************************/
_DRIVER_FM_HW_FM5807_COMMON_
COMMON API UINT16 FM5807_Tuner_SearchByHand(HDC dev, UINT32 FmFreq)
{
    UINT16 channelflag = 0;

    channelflag = FM5807_FmSeek(dev, FmFreq);

    //PaintCHAndFreq();

    if ((channelflag&0x0001) == 1)
        return 2;   //found a valid station
    else
        return 0;   //FM_SearchFail; found no valid station.
}
/*******************************************************************************
** Name: FM5807_Tuner_PowerOffDeinit
** Input:HDC dev
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.22
** Time: 17:33:04
*******************************************************************************/
_DRIVER_FM_HW_FM5807_COMMON_
COMMON API rk_err_t FM5807_Tuner_PowerOffDeinit(HDC dev)
{
    return FM5807_Tuner_Disable(dev);
}
/*******************************************************************************
** Name: FM5807_GetStereoStatus
** Input:HDC dev
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.22
** Time: 17:32:18
*******************************************************************************/
_DRIVER_FM_HW_FM5807_COMMON_
COMMON API BOOLEAN FM5807_GetStereoStatus(HDC dev)
{
    UINT8 readData8[1];
    FM5807_Tuner_ReadData(dev, readData8, 1);
    return(readData8[0]&0x04);
}
/*******************************************************************************
** Name: FM5807_Tuner_SetMute
** Input:UINT8 mute
** Return: rk_er_t
** Owner:cjh
** Date: 2016.3.22
** Time: 17:30:30
*******************************************************************************/
_DRIVER_FM_HW_FM5807_COMMON_
COMMON API rk_err_t FM5807_Tuner_SetMute(HDC dev, UINT8 mute)
{
    UINT8 writeData8[2];

    writeData8[0] = RDA5800REG_IMG[2] >> 8;
    writeData8[1] = RDA5800REG_IMG[2] & 0xff;

    if (mute == 0)
    {
        writeData8[0] |= 0x40;//unmute
    }
    else
    {
       writeData8[0] &= ~0x40;//mute
    }
    //rk_printf("FM5807_SetMute 1 =%d\n", mute);
    if(FM5807_Tuner_SendCommand(dev, writeData8, 2) < 0)
    {
        return RK_ERROR;
    }
    //rk_printf("FM5807_Tuner_SetMute over\n");
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FM5807_Tuner_SetStereo
** Input:HDC dev,  UINT8 bStereo
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.22
** Time: 17:28:37
*******************************************************************************/
_DRIVER_FM_HW_FM5807_COMMON_
COMMON API rk_err_t FM5807_Tuner_SetStereo(HDC dev, UINT8 bStereo)
{
    UINT8 writeData8[1];
    UINT8 ReadData8[1];

    if (bStereo)
    {
        writeData8[0] = ((RDA5800REG_IMG[2] >> 8) & (~TUNER_MONO));
    }
    else
    {
        writeData8[0] = ((RDA5800REG_IMG[2] >> 8) | TUNER_MONO);
    }
    if(FM5807_Tuner_SendCommand(dev, writeData8, 1) < 0)
    {
        return RK_ERROR;
    }
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FM5807_Tuner_PowerOnInit
** Input:HDC dev
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.22
** Time: 17:26:27
*******************************************************************************/
_DRIVER_FM_HW_FM5807_COMMON_
COMMON API rk_err_t FM5807_Tuner_PowerOnInit(HDC dev)
{
     FM5807_Tuner_Disable(dev);
}
/*******************************************************************************
** Name: FM5807_Tuner_SetInitArea
** Input:HDC dev, UINT8 area
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.22
** Time: 14:19:09
*******************************************************************************/
_DRIVER_FM_HW_FM5807_COMMON_
COMMON API rk_err_t FM5807_Tuner_SetInitArea(HDC dev, UINT8 area)
{
    if(RADIO_JAPAN == area)
    {
        rk_printf("RADIO_JAPAN\n");
        return FM5807_Tuner_Enable(dev, 1);
    }
    else
    {
        rk_printf("RADIO other\n");
        return FM5807_Tuner_Enable(dev, 0);
    }
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: FM5807_Tuner_Disable
** Input:HDC dev
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.22
** Time: 17:25:33
*******************************************************************************/
_DRIVER_FM_HW_FM5807_COMMON_
COMMON FUN rk_err_t FM5807_Tuner_Disable(HDC dev)
{
    UINT8 writeData8[2];
    writeData8[0] = 0xd8;  //0x02   0xd881
    writeData8[1] = 0x80;  //0x81;
    if(FM5807_Tuner_SendCommand(dev, writeData8, 2) < 0)
    {
        return RK_ERROR;
    }
    //DelayMs_nops(100);
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FM5807_Tuner_Enable
** Input:HDC dev, UINT8 area
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.22
** Time: 17:24:17
*******************************************************************************/
_DRIVER_FM_HW_FM5807_COMMON_
COMMON FUN rk_err_t FM5807_Tuner_Enable(HDC dev, UINT8 area)
{
    if(FM5807_FmInitPowerOn(dev, area) < 0)
    {
        return RK_ERROR;
    }
    if(FmInitData(dev) < 0)
    {
        return RK_ERROR;
    }
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FM5807_FmSeek
** Input:HDC dev, INT8 seekDirection, UINT32 FmFreq
** Return: UINT16
** Owner:cjh
** Date: 2016.3.22
** Time: 14:36:18
*******************************************************************************/
_DRIVER_FM_HW_FM5807_COMMON_
COMMON FUN UINT16 FM5807_FmSeek(HDC dev, UINT32 FmFreq)
{
#define TUNE_FIND_TUNE 0X00
#define TUNE_CHECK_FD  0X01
#define TUNE_CHECK_FW  0X02

    UINT8 readData8[4];
    UINT8 writeData8[8];
    BOOL fStopSeek = 0;
    BOOL falseStation = 0;
    UINT16 channelflag = 0;
    UINT8  seekover = 0;
    UINT8 TuneState, LastRSSI;
    UINT8 Sensitivity;//PAGE
    TuneState = TUNE_FIND_TUNE;
    //PAGE
    if(gSysConfig.RadioConfig.ScanSensitivity==0){
        Sensitivity = 40;// Hign Scan Sensitivity
    }
    else
    {
        Sensitivity = 30;// Low Scan Sensitivity
    }

    //FREQ_EnterModule(FREQ_AUDIO_INIT);

TUNE_CHECK:
    FmTune(dev, FmFreq);
    rkos_delay(60);
    //DelayMs(160);
    do
    {
        seekover++;
        //DelayMs(5);
        rkos_delay(5);
        FM5807_Tuner_ReadData(dev, readData8, 4);
        //printf("seekover = %d\n", seekover);
    }
    while((readData8[0] & 0x40 == 0) && (seekover <= 8));

    //printf("RSSI = %d, fmreq = %d\n", readData8[2], FmFreq);

    if (readData8[2] < Sensitivity)//PAGE
    {
        switch(TuneState)
        {
          case TUNE_FIND_TUNE:
            falseStation = 1;
            break;

          case TUNE_CHECK_FD:
            FmFreq -= 20;
            TuneState = TUNE_CHECK_FW;
            goto TUNE_CHECK;

          case TUNE_CHECK_FW:
            if(LastRSSI & 0X01)
            {
                falseStation = 0;
            }
            else
            {
                falseStation = 1;
            }
            break;
        }

    }
    else
    {
        switch(TuneState)
        {
          case TUNE_FIND_TUNE:
            LastRSSI = readData8[2];
            FmFreq += 10;
            TuneState = TUNE_CHECK_FD;
            goto TUNE_CHECK;

          case TUNE_CHECK_FD:
            if(LastRSSI <= readData8[2])
            {
                falseStation = 1;
                break;
            }
            else
            {
                FmFreq -= 20;
                TuneState = TUNE_CHECK_FW;
                goto TUNE_CHECK;
            }


          case TUNE_CHECK_FW:
            if(LastRSSI <= readData8[2])
            {
                falseStation = 1;
            }
            else
            {
                if(LastRSSI & 0X01)
                {
                    falseStation = 0;
                }
                else
                {
                    falseStation = 1;
                }
            }

            break;

        }
    }

    if (falseStation == 0)
    {
        channelflag |= 0x0001;
        return channelflag;
    }
    else
    {
        return channelflag;
    }
    //FREQ_ExitModule(FREQ_AUDIO_INIT);

    #undef TUNE_FIND_TUNE
    #undef TUNE_CHECK_FD
    #undef TUNE_CHECK_FW
}
/*******************************************************************************
** Name: FM5807_FmChanToFreq
** Input:UINT8 channel
** Return: UINT32
** Owner:cjh
** Date: 2016.3.22
** Time: 14:34:26
*******************************************************************************/
_DRIVER_FM_HW_FM5807_COMMON_
COMMON FUN UINT32 FM5807_FmChanToFreq(UINT8 channel)
{
    UINT16 channelSpacing;
    UINT16 bottomOfBand;
    UINT16 frequency;
    if ((RDA5800REG_IMG[3] & 0x0002) == 0x0000)
        bottomOfBand = 870;
    else
        bottomOfBand = 760;
    if ((RDA5800REG_IMG[3] & 0x0001) == 0x0000)
        channelSpacing = 1;
    else if ((RDA5800REG_IMG[3] & 0x0001) == 0x0001)
        channelSpacing = 2;
    else
        channelSpacing = 1;
    frequency = (bottomOfBand + channelSpacing * channel);
    return (frequency*10);
}
/*******************************************************************************
** Name: FmFreqToChan
** Input:UINT32 frequency
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.22
** Time: 14:32:39
*******************************************************************************/
_DRIVER_FM_HW_FM5807_COMMON_
COMMON FUN UINT8 FmFreqToChan(UINT32 frequency)
{
    UINT16 channelSpacing;
    UINT16 bottomOfBand;
    UINT8 channel;
    frequency = frequency / 10;

    if ((RDA5800REG_IMG[3] & 0x000C) == 0x0000)
        bottomOfBand = 870;
    else if( ((RDA5800REG_IMG[3] & 0x000C) == 0x0004) ||((RDA5800REG_IMG[3] & 0x000C) == 0x0008))
        bottomOfBand = 760;
    else if ((RDA5800REG_IMG[3] & 0x000C) == 0x000C)
     bottomOfBand = 650;


    if ((RDA5800REG_IMG[3] & 0x0003) == 0x0000)
        channelSpacing = 1;
    else if ((RDA5800REG_IMG[3] & 0x0003) == 0x0001)
        channelSpacing = 2;
    else
        channelSpacing = 1;
    channel = (UINT8)((frequency - bottomOfBand) / channelSpacing);
    return (channel);
}
/*******************************************************************************
** Name: FmTune
** Input:HDC dev, UINT32 frequency
** Return: k_err_t
** Owner:cjh
** Date: 2016.3.22
** Time: 14:30:41
*******************************************************************************/
_DRIVER_FM_HW_FM5807_COMMON_
COMMON FUN rk_err_t FmTune(HDC dev, UINT32 frequency)
{
    UINT8  writeData8[4];
    int temp = 0;
    temp = FmFreqToChan(frequency);
    RDA5800REG_IMG[3] &= 0x003f;
    RDA5800REG_IMG[3] |= (((int)(temp)&0x3) <<6);
    RDA5800REG_IMG[3] |= ((((int)(temp)>>2)<<8) );

    writeData8[0] = (RDA5800REG_IMG[2] >> 8) & ~TUNER_MUTE; //0x02   0xd081
    writeData8[1] = (RDA5800REG_IMG[2]);
    writeData8[2] = (RDA5800REG_IMG[3] >> 8); //0x03
    writeData8[3] = (RDA5800REG_IMG[3]);

    if(FM5807_Tuner_SendCommand(dev, writeData8, 4) < 0)
    {
        return RK_ERROR;
    }

    /*
    DelayMs(160);
    do
    {
        DelayMs(5);
        FM5807_Tuner_ReadData(writeData8, 4);
    }
    while ((writeData8[0] & 0x40 == 0));
    printf("RSSI = %d, fmreq = %d\n", writeData8[2], frequency);
   */
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FmInitData
** Input:HDC dev
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.22
** Time: 14:24:39
*******************************************************************************/
_DRIVER_FM_HW_FM5807_COMMON_
COMMON FUN rk_err_t FmInitData(HDC dev)
{
    int i, j;
    rk_err_t ret;
    UINT8 writeData8[86], xbTemp;
    for (xbTemp = 0; xbTemp < 24; xbTemp++)
    {
        writeData8[xbTemp*2] = (RDA5800REG_IMG[xbTemp+2] >> 8);
        writeData8[xbTemp*2+1] = (RDA5800REG_IMG[xbTemp+2]);
    }

    ret = FM5807_Tuner_SendCommand(dev, writeData8, 48);
    return ret;
}
/*******************************************************************************
** Name: FM5807_FmInitPowerOn
** Input:HDC dev, UINT8 area
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.22
** Time: 14:22:35
*******************************************************************************/
_DRIVER_FM_HW_FM5807_COMMON_
COMMON FUN rk_err_t FM5807_FmInitPowerOn(HDC dev, UINT8 area)
{
    int i, j;
    rk_err_t ret;
    UINT8 writeData8[2]={0x00,0x02}, xbTemp;
    ret = FM5807_Tuner_SendCommand(dev, writeData8, 2);
    DelayMs(10);
    for (xbTemp = 0; xbTemp < 60; xbTemp++)
        RDA5800REG_IMG[xbTemp] = cwFM5807Default[xbTemp];
    if (area == 1)
        RDA5800REG_IMG[3] |= RUNER_AREA_JA;
    else
        RDA5800REG_IMG[3] &= (~RUNER_AREA_JA);

    return ret;
}
/*******************************************************************************
** Name: FM5807_Tuner_ReadData
** Input:HDC dev, UINT8 *pdata, UINT8 size
** Return: rk_size_t
** Owner:cjh
** Date: 2016.3.22
** Time: 14:04:17
*******************************************************************************/
_DRIVER_FM_HW_FM5807_COMMON_
COMMON FUN rk_size_t FM5807_Tuner_ReadData(HDC dev, UINT8 *pdata, UINT8 size)
{
    rk_size_t ret;
    I2C_CMD_ARG stArg;

    //I2CDevInit(dev);//210

    stArg.SlaveAddress = FM5800;
    stArg.RegAddr = 0;
    stArg.RWmode = DirectMode;
    stArg.speed = 400;
    stArg.addr_reg_fmt = I2C_7BIT_ADDRESS_8BIT_REG;
    //rk_printf("read hI2C=0x%x\n",dev);
    ret = I2cDev_ReadData(dev, pdata, size, &stArg);
    //I2CDevDeInit(dev);
    return ret;
}
/*******************************************************************************
** Name: FM5807_Tuner_SendCommand
** Input:UINT8 *pdata, UINT8 size
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.22
** Time: 13:47:21
*******************************************************************************/
_DRIVER_FM_HW_FM5807_COMMON_
COMMON FUN rk_size_t FM5807_Tuner_SendCommand(HDC dev,UINT8 *pdata, UINT8 size)
{
    rk_size_t ret;
    I2C_CMD_ARG stArg;

    //I2CDevInit(dev);

    stArg.SlaveAddress = FM5800;
    stArg.RegAddr = 0;
    stArg.RWmode = DirectMode;
    stArg.speed = 400;
    stArg.addr_reg_fmt = I2C_7BIT_ADDRESS_8BIT_REG;
    //FM5807_I2C_DEV

    //rk_printf("FM5807 I2cDev_SendData dev =0x%x\n", dev);
    ret = I2cDev_SendData(dev, pdata, size, &stArg);
    //rk_printf("FM5807 I2cDev_SendData OVER\n");

    //I2CDevDeInit(dev);

    return ret;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



#endif
