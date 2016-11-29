/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: System\syssever\PowerManager.c
* Owner: aaron.sun
* Date: 2015.11.8
* Time: 14:41:34
* Version: 1.0
* Desc: power manager
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.11.8     14:41:34   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __SYSTEM_SYSSEVER_POWERMANAGER_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "DriverInclude.h"
#include "interrupt.h"
#include "PowerManager.h"
#include "Bsp.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef void(*ExecFunPtr)(void) __irq;
#define _ATTR_VECTT_PMU_             __attribute__((section("pmu_vect")))
#define _ATTR_PMU_CODE_              __attribute__((section("pmucode")))
#define _ATTR_PMU_DATA_              __attribute__((section("pmudata")))
#define _ATTR_PMU_BSS_              __attribute__((section("pmudata"),zero_init))

typedef enum
{
    CHARGE_NO,  //power is not changed.
    CHARGE_BATT,//power is changed.
    CHARGE_FULL //battery charge full.
}CHARGE_STATE_t;

typedef enum
{
    CHARGE_CURRENT_LOW = 0,
    CHARGE_CURRENT_HIGH,

}Charge_Current_Value;

#define BATTERY_BUF_SIZE            32
#define BATT_TOTAL_STEPS            5

#define BATT_USB_FULL_VALUE         4100
#define BATT_FULL_VALUE             4000
#define BATT_LEVEL2                 3800
#define BATT_LEVEL1                 3600
#define BATT_LEVEL0                 3500
#define BATT_EMPTY_VALUE            3350

#define CHARGE_SAMPLE_TIME          (5 * 100)           //采样间隔时间单位10mS
#define CHARGE_CHECKFULL_COUNT      (20)                //检测充电电压大于 阈值的次数
#define CHARGE_FULL_DELAY           (1*15*60*100/CHARGE_SAMPLE_TIME)    //小电流充电时间

#define RKNANOD_L                   0
#define RKNANOD_G                   1
#define RKNANOD_N                   2
#define RKNANOD_CHIP_TYPE           RKNANOD_N

#define BATT_POWEROFF_CNT           20
#define BATT_POWEROFF_VALUE         BATT_EMPTY_VALUE

#define BATTERY_VALUE_SETP          20
/************************/

#ifdef NOSCREEN_USE_LED //led display
extern int8 redLed_state;
//extern int8 greenled_state;
#endif

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
CRU_CLK_INFO    g_chipClk;
uint64          g_APPList;
uint64          g_APPDisableList;
uint64          g_APPEnableList;
uint8           FreqDisableFlag = 0;
uint8           FreqDisableCnt = 0;
static pSemaphore FreqOperSem;

pTimer  PowerTimer;
/*Battery*/
HDC     ADCHandler;
uint8   ChargeCurrent = 0;
uint16  BatteryCnt;
uint16  BatteryBuffer[BATTERY_BUF_SIZE];
uint32  lastChargeTimeMs = 0;
uint32  chargeFullCheckCnt = 0;
UINT32  ChargeFullFlag;

BATT_INFO    gBattery;

uint16 Batt_Level[BATT_TOTAL_STEPS] =
{
    BATT_EMPTY_VALUE,
    BATT_LEVEL0,
    BATT_LEVEL1,

    BATT_LEVEL2,
    BATT_FULL_VALUE
};
/*********/

extern UINT32 Image$$IDLE_STACK$$ZI$$Limit;
_ATTR_VECTT_PMU_
ExecFunPtr pmusram_table[NUM_INTERRUPTS] = {

    /* Core Fixed interrupts start here...*/

    (ExecFunPtr)&Image$$IDLE_STACK$$ZI$$Limit,
    (ExecFunPtr)NULL,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,

    //RKNano Process Intterupt
    IntDefaultHandler,                                          //00 int_dma
    IntDefaultHandler,                                          //01 uart
    IntDefaultHandler,                                          //02 sd mmc
    IntDefaultHandler,                                          //03 pwm1
    IntDefaultHandler,                                          //04 pwm2
    IntDefaultHandler,                                          //05 imdct36
    IntDefaultHandler,                                          //06 synthesize
    IntDefaultHandler,                                          //07 usb
    IntDefaultHandler,                                          //08 i2c
    IntDefaultHandler,                                          //09 i2s
    IntDefaultHandler,                                          //10 gpio
    IntDefaultHandler,                                          //11 spi
    IntDefaultHandler,                                          //12 pwm0
    IntDefaultHandler,                                          //13 timer
    IntDefaultHandler,                                          //14 sar-adc
    IntDefaultHandler,                                          //15 reserved
    IntDefaultHandler,                                          //16 reserved
    IntDefaultHandler,                                          //17 reserved
    IntDefaultHandler,                                          //18 rtc
    IntDefaultHandler,                                          //19 reserved
    IntDefaultHandler,                                          //20 ool_bat_snsen
    IntDefaultHandler,                                          //21 reserved
    IntDefaultHandler,                                          //22 ool_pause
    IntDefaultHandler,                                          //23 ool_PlayOn
    IntDefaultHandler,                                          //24 pwr_5v_ready     Adapter supply detected
    IntDefaultHandler,                                          //25 pwr_uvlo_vdd      Vbat under 3.2V indicator
    IntDefaultHandler,                                          //26 pwr_uvp              DC-DC under voltage alarm
    IntDefaultHandler,                                          //27 pwr_ovp              DC-DC over voltage alarm
    IntDefaultHandler,                                          //28 pwr_ot                DC-DC over temperature alarm
    IntDefaultHandler,                                          //29 pwr_oc                DC-DC over current alarm
    IntDefaultHandler,                                          //30 pwr_charge         Charge state changing indicator
    IntDefaultHandler,                                          //31 reserved
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler
};


const FREQ_APP_TABLE g_CruAPPTabel[FREQ_APP_MAX] =
{
#ifdef CODEC_24BIT
//     ID,                   sys_hclk,      cal_hclk,
    {FREQ_IDLE,              0,          0       },
    {FREQ_MIN,               500000,     500000  },
    {FREQ_INIT,              96000000,   0       },
    {FREQ_BLON,              24000000,   0       },
    {FREQ_AUDIO_INIT,        48000000,   48000000},
    {FREQ_SBC,               48000000,   60000000},
    {FREQ_MP3,               24000000,   24000000},
    {FREQ_MP3H,              36000000,   36000000},
    {FREQ_XXX,               60000000,   60000000},
    {FREQ_XXXH,              100000000,  200000000},
    {FREQ_WAV,               88000000,   88000000},
    {FREQ_AAC,               80000000,   80000000},
    {FREQ_APE,               150000000,  300000000},
    {FREQ_FLAC,              150000000,  150000000},
    {FREQ_OGG,               80000000,   120000000},
    {FREQ_NOGG,              120000000,  260000000},
    {FREQ_HOGG,              180000000,  180000000},
    {FREQ_EHOGG,             260000000,  260000000},
    {FREQ_HAPE,              80000000,   400000000},
    {FREQ_HFLAC,             80000000,   220000000},
    {FREQ_HALAC,             80000000,   400000000},
    {FREQ_HWAV,              80000000,   400000000},
    {FREQ_EQ_L,              36000000,   0},
    {FREQ_EQ,                72000000,   0},
    {FREQ_EQ_H,              140000000,  0},
    {FREQ_JPG,               250000000,  0},
    {FREQ_FM,                24000000,   0},
    {FREQ_RECORDWAV,         150000000,   300000000},
    {FREQ_RECORDAMR,         150000000,   500000000},
    {FREQ_RECORDXXX,         96000000,    192000000},
    {FREQ_FMAUTOSEARCH,      24000000,   0},
    {FREQ_MEDIAUPDATA,       100000000,  0},
    {FREQ_USB,               500000000,  0}, //USB 会锁频
    {FREQ_BEEP,              24000000,   0},
    {FREQ_DC_FILTER_LOW,     24000000,   0},
    {FREQ_DC_FILTER_HIGH,    24000000,   0},
    {FREQ_MEDIA_INIT,        100000000,  0},
    {FREQ_AVI,               100000000,  0},
    {FREQ_WIFI,              100000000,  0},
    {FREQ_DLNA,              100000000,  0},
    {FREQ_BT,                500000000,  0},
    {FREQ_RKI6000,           24000000,  0},
    {FREQ_5,                 200000000,  0},
    {FREQ_AMR,               88000000,  88000000},
    {FREQ_MAX,               200000000,  400000000},

#else
//     ID,                   sys_hclk,      cal_hclk,
    {FREQ_IDLE,              0,          0      },
    {FREQ_MIN,               2000000,    1000000},
    {FREQ_INIT,              96000000,   0      },
    {FREQ_BLON,              24000000,   0      },
    {FREQ_AUDIO_INIT,        48000000,   48000000},
    {FREQ_SBC,               48000000,   60000000},
    {FREQ_MP3,               24000000,   24000000},
    {FREQ_MP3H,              36000000,   36000000},
    {FREQ_XXX,               60000000,   60000000},
    {FREQ_XXXH,              100000000,  200000000},
    {FREQ_WAV,               88000000,   88000000},
    {FREQ_AAC,               80000000,   80000000},
    {FREQ_APE,               150000000,  150000000},
    {FREQ_FLAC,              150000000,  150000000},
    {FREQ_OGG,               80000000,   120000000},
    {FREQ_NOGG,              120000000,  260000000},
    {FREQ_HOGG,              180000000,  180000000},
    {FREQ_EHOGG,             260000000,  260000000},
    {FREQ_HAPE,              220000000,  200000000},
    {FREQ_HFLAC,             220000000,  220000000},
    {FREQ_HALAC,             220000000,  400000000},
    {FREQ_HWAV,              220000000,  400000000},
    {FREQ_EQ_L,              36000000,   0},
    {FREQ_EQ,                72000000,   0},
    {FREQ_EQ_H,              140000000,  0},
    {FREQ_JPG,               250000000,  0},
    {FREQ_FM,                24000000,   0},
    {FREQ_RECORDWAV,       96000000,   192000000},
    {FREQ_RECORDAMR,       96000000,   192000000},
    {FREQ_RECORDXXX,         48000000,    192000000},
    {FREQ_FMAUTOSEARCH,      24000000,   0},
    {FREQ_MEDIAUPDATA,       100000000,  0},
    {FREQ_USB,               500000000,  0}, //USB 会锁频
    {FREQ_BEEP,              24000000,   0},
    {FREQ_DC_FILTER_LOW,     24000000,   0},
    {FREQ_DC_FILTER_HIGH,    24000000,   0},
    {FREQ_MEDIA_INIT,        100000000,  0},
    {FREQ_AVI,               100000000,  0},
    {FREQ_WIFI,              100000000,  0},
    {FREQ_DLNA,              100000000,  0},
    {FREQ_BT,                100000000,  0},
    {FREQ_RKI6000,           24000000,  0},
    {FREQ_5,                 200000000,  0},
    {FREQ_AMR,               88000000,  88000000},
    {FREQ_MAX,               200000000,  400000000},
    #endif
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
void PowerTimerIsr(void);
void PmuSuspend(void);
void PmuResume(void);
int32 FREQ_Control_Init(void);
int32 FREQStopAPP(eFREQ_APP appId);
int32 FREQStartAPP(eFREQ_APP appId);
int32 FREQSetFreq(uint64 appList);
void FREQSetARMFreq(FREQ_APP_TABLE *FreqTab, chip_freq_t *pChipFreq);
void PmuResume();
void BatteryManagerInit();
void BatteryManagerDeInit();
#if 1
UINT16 Battery_GetChargeState(void);
uint32 RealBattValue(UINT16 adc);
void   BatteryInit(void);
uint32 ChargeFullCheck(void);
uint32 Charge_State_Get(void);
void   Charge_Current_Set(Charge_Current_Value Value);
Charge_Current_Value Charger_Current_Get(void);
#endif



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: CheckSystemIdle1
** Input:void
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.7.26
** Time: 16:45:11
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_COMMON_
COMMON API rk_err_t CheckSystemIdle(void)
{
    if(gSysConfig.SysIdleStatus)
    {
        return RK_SUCCESS;
    }
    else
    {
        return RK_ERROR;
    }
}

/*******************************************************************************
** Name: ClearSytemIdle1
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2016.7.26
** Time: 16:17:58
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_COMMON_
COMMON API void ClearSytemIdle(void)
{
    if(gSysConfig.SysIdleStatus == 0)
    {
        gSysConfig.PMTime = 0;
    }
    else if(gSysConfig.SysIdleStatus)
    {
        gSysConfig.SysIdle = 0;
    }
}

/*******************************************************************************
** Name: ChargeDisable
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2016.7.21
** Time: 18:46:55
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_COMMON_
COMMON API void ChargeDisable(void)
{
    gBattery.Enable_Charge = 0;
}

/*******************************************************************************
** Name: ChargeEnable
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2016.7.21
** Time: 18:45:04
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_COMMON_
COMMON API void ChargeEnable(void)
{
    gBattery.Enable_Charge = 1;/**/
}

/*******************************************************************************
** Name: BatteryUpdateLed
** Input:void
** Return: CHARGE_STATE_t
** Owner:hu.jiang
** Date: 2016.11.10
** Time: 20:50:09
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_COMMON_
COMMON API void BatteryUpdateLed(CHARGE_STATE_t state)
{
#ifdef NOSCREEN_USE_LED
	int8 led_flash = MAINTASK_LED1;
	int8 led_off = MAINTASK_LED2;
	
	if(state == CHARGE_BATT)//charging
	{
		led_flash = MAINTASK_LED2;
		led_off = MAINTASK_LED1;
	}
	else if(state == CHARGE_NO)//low power
	{
		led_flash = MAINTASK_LED1;
		led_off = MAINTASK_LED2;
	}
	MainTask_SetLED (led_off,MAINTASK_LED_OFF);//led off
	
	redLed_state++;
	if ((redLed_state == 1)||(redLed_state == 3))//led flash
	{
		MainTask_SetLED (led_flash,MAINTASK_LED_OFF);
	}
	if ((redLed_state == 2)||(redLed_state == 4))
	{
		MainTask_SetLED (led_flash,MAINTASK_LED_ON);
		if (redLed_state == 4)
		{
			redLed_state = 0;
		}
	}
#endif
}

/*******************************************************************************
** Name: Battery_GetLevel
** Input:void
** Return: uint32
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_COMMON_
COMMON API uint32 Battery_GetLevel(void)
{
    UINT32 batt_adcval;
    UINT32 ChargeState;
    UINT32 i, alarm_state;
    uint16 ADCVal;
	uint8 battery_update_led = 0;

    ADCDev_Read(ADCHandler,ADC_CHANEL_BATTERY, 1, 1);

    ADCDev_GetAdcBufData(ADCHandler,(uint16 *)&ADCVal, 1, ADC_CHANEL_BATTERY);
    batt_adcval = ADCVal;

    //printf ("\n------batt_adcval=%d\n",batt_adcval);
    gBattery.Batt_Value   = (UINT32)RealBattValue(batt_adcval);
    ChargeState = Battery_GetChargeState();
    //printf ("\n------Batt_Value=%d  ChargeState=%d\n",gBattery.Batt_Value,ChargeState);
    switch (ChargeState)
    {
        case CHARGE_BATT: //充电
        {
            //printf ("CHARGE_BATT = %d\n",ChargeState);
            for (i = 0; i < BATT_TOTAL_STEPS; i++)
            {
               if ((gBattery.Batt_Value) < Batt_Level[i])
               {
                   if(i==0)
                       gBattery.Batt_Level = i;
                   else
                       gBattery.Batt_Level = i-1;
                   break;
               }
            }
            if (i == BATT_TOTAL_STEPS)
            {
                gBattery.Batt_Level = i-1;
            }
			battery_update_led = 1;
			BatteryUpdateLed(CHARGE_BATT);
        }
        break;

        case CHARGE_FULL: //满电
        {
            printf ("CHARGE_FULL = %d\n",ChargeState);
            gBattery.Batt_Level = (BATT_TOTAL_STEPS - 1);
        }
        break;

        case CHARGE_NO:  //未充电
        {
            //printf ("CHARGE_NO = %d\n",ChargeState);
            if (gBattery.Batt_Value < BATT_POWEROFF_VALUE)//low power
            {
                gBattery.Batt_LowCnt++;

                if(gBattery.Batt_LowCnt > BATT_POWEROFF_CNT)
                {
                    if(FALSE==gBattery.Batt_LowPower)
                    {
                        MainTask_SysEventCallBack(MAINTASK_SHUTDOWN, NULL);//low power
                        gBattery.Batt_LowPower = TRUE;
                        gBattery.Batt_Level = 0;
						BatteryUpdateLed(CHARGE_NO);
                    }
                }

                return gBattery.Batt_Level;
            }

            gBattery.Batt_LowCnt = 0;

            for (i = 0; i < BATT_TOTAL_STEPS; i++)
            {
               if ((gBattery.Batt_Value) < Batt_Level[i])
               {
                   if(i==0)
                       gBattery.Batt_Level = i;
                   else
                       gBattery.Batt_Level = i-1;
                   break;
               }
            }
            if (i == BATT_TOTAL_STEPS)
            {
                gBattery.Batt_Level = i-1;
            }
        }
        break;

        default:
            break;
    }

	if(0 == battery_update_led)
		PnPSever();
    return gBattery.Batt_Level;
}

/*******************************************************************************
** Name: SetSysFreq
** Input:uint32 nMhz
** Return: void
** Owner:aaron.sun
** Date: 2015.11.8
** Time: 14:55:16
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_COMMON_
COMMON API void SetSysFreq(uint32 nMhz)
{
     uint32 i;
     PLL_ARG_t PllArg;
     memset(&PllArg, 0, sizeof(PLL_ARG_t));

     if(nMhz > 24)
     {
         Get_PLL_Post_Div(nMhz * 1000 * 1000, &PllArg);
         Get_PLL_Pre_Div(PllArg.VCO, &PllArg);
     }
     else
     {
        if(24 % nMhz)
        {
            for(i = 0; i < 24; i++)
            {
                if((24 % i) == 0)
                {
                    if(i > nMhz)
                    {
                        nMhz = i;
                        break;
                    }
                }
            }
        }

     }

     PllArg.cal_core_div = 1;
     PllArg.cal_stclk_div = 1;
     PllArg.sys_core_div = 1;
     PllArg.sys_stclk_div = 1;
     PllArg.pclk_logic_div = 0;
     PllArg.div_con_24m = 24 / nMhz;

     chip_freq.pll = nMhz * 1000 * 1000;
     chip_freq.hclk_sys_core = nMhz * 1000 * 1000;
     chip_freq.pclk_logic_pre = chip_freq.hclk_sys_core / (1 << PllArg.pclk_logic_div);

     SetPllFreq(nMhz * 1000 * 1000, &PllArg);

     SysTickPeriodSet(10);


}

/*******************************************************************************
** Name: FREQ_Enable
** Input:void
** Return: int32
** Owner:aaron.sun
** Date: 2015.11.8
** Time: 14:54:23
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_COMMON_
COMMON API int32 FREQ_Enable(void)
{
    uint32 i;

    rkos_semaphore_take(FreqOperSem, MAX_DELAY);

    if(FreqDisableCnt >= 1)
    {
        FreqDisableCnt--;
    }


    if(FreqDisableCnt == 0)
    {
        FreqDisableFlag = 0;
        for (i = 0; i < 64; i++)
        {
            if(g_APPDisableList & ((uint64)0x01<< i))
            {
                g_APPDisableList &= ~((uint64)0x01 << i);

                if(!(g_APPEnableList & ((uint64)0x01<< i)))
                {
                    FREQStopAPP(i);
                }
                else
                {
                    g_APPEnableList &= ~((uint64)0x01 << i);
                }
            }
            else if(g_APPEnableList & ((uint64)0x01<< i))
            {
                g_APPEnableList &= ~((uint64)0x01 << i);
                FREQStartAPP(i);
            }
        }
    }

    rkos_semaphore_give(FreqOperSem);
}
/*******************************************************************************
** Name: FREQ_Disable
** Input:void
** Return: int32
** Owner:aaron.sun
** Date: 2015.11.8
** Time: 14:53:39
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_COMMON_
COMMON API int32 FREQ_Disable(void)
{
    rkos_semaphore_take(FreqOperSem, MAX_DELAY);
    FreqDisableFlag = 1;
    FreqDisableCnt++;
    rkos_semaphore_give(FreqOperSem);
}

/*******************************************************************************
** Name: FREQ_ExitModule
** Input:eFREQ_APP modulename
** Return: int32
** Owner:aaron.sun
** Date: 2015.11.8
** Time: 14:50:09
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_COMMON_
COMMON API int32 FREQ_ExitModule(eFREQ_APP modulename)
{
    rkos_semaphore_take(FreqOperSem, MAX_DELAY);

    if (FreqDisableFlag == 0)
    {
        FREQStopAPP(modulename);
    }
    else
    {
         g_APPDisableList |= ((uint64)0x01<< modulename);
    }

    rkos_semaphore_give(FreqOperSem);

    return 0;
}

/*******************************************************************************
** Name: FREQ_EnterModule
** Input:eFREQ_APP modulename
** Return: int32
** Owner:aaron.sun
** Date: 2015.11.8
** Time: 14:48:54
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_COMMON_
COMMON API int32 FREQ_EnterModule(eFREQ_APP modulename)
{
    rkos_semaphore_take(FreqOperSem, MAX_DELAY);

    if (FreqDisableFlag == 0)
    {
        FREQStartAPP(modulename);
    }
    else
    {
         g_APPEnableList |= ((uint64)0x01<< modulename);
    }

    rkos_semaphore_give(FreqOperSem);

    return(0);
}

/*******************************************************************************
** Name: FREQ_Resume
** Input:void
** Return: int32
** Owner:aaron.sun
** Date: 2015.11.8
** Time: 14:48:54
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_COMMON_
COMMON API int32 FREQ_Resume(void)
{
    FREQSetFreq(g_APPList);
    return(0);
}


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: PowerTimerIsr
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.11.8
** Time: 17:02:06
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_COMMON_
COMMON FUN void PowerTimerIsr(void)
{
    UINT32 rate;
    uint32 battery_level;

    gSysConfig.PMTime += PM_TIME;

    rkos_start_timer(PowerTimer);
	
#ifndef _DISABLE_ENTER_IDLEMODE//jjjhhh 20161109
    {
        if(gSysConfig.PMTime == gSysConfig.SysIdle1EventTime)
        {
            rk_printf("system enter idle1");
            #ifdef _USE_GUI_
            Lcd_BL_Off(hLcd);
            #endif
            gSysConfig.SysIdleStatus = 1;
            gSysConfig.SysIdle = 1;
        }
        else if((gSysConfig.SysIdleStatus)
            &&(gSysConfig.SysIdle == 0))//SYSTEM RESUME
        {
            FREQ_EnterModule(FREQ_BLON);

            FwResume();

            #ifndef _IDLE_DEBUG_
            if(UartHDC == NULL)
            {
                UartHDC = RKDev_Open(DEV_CLASS_UART, 0, NOT_CARE);
            }
            #endif

            rk_printf("system return from idle%d",gSysConfig.SysIdleStatus);

            #ifdef _FS_
            if(FileSysHDC == NULL)
            {
                FileSysHDC = RKDev_Open(DEV_CLASS_FILE, 0, NOT_CARE);
            }
            #endif

            if(hKey == NULL)
            {
                hKey = RKDev_Open(DEV_CLASS_KEY, 0, NOT_CARE);
            }

            if(ADCHandler == NULL)
            {
                ADCHandler = RKDev_Open(DEV_CLASS_ADC, 0, NOT_CARE);
            }

            DBResume();

            #ifdef _USE_GUI_
            Lcd_BL_On(hLcd);
            #endif

            if(MainTask_GetStatus(MAINTASK_WIFI_SUSPEND))
            {
                MainTask_SysEventCallBack(MAINTASK_RESUME_WIFI, NULL);
            }

            gSysConfig.PMTime = 0;
            gSysConfig.SysIdleStatus = 0;
        }
        else if(gSysConfig.SysIdleStatus == 2)
        {
            RKTaskIdleTick();
            DeviceTask_DevIdleTick();
            if(TaskTotalCnt == (TaskTotalSuspendCnt + 4))
            {
                rk_printf("system enter idle2.8");
                gSysConfig.SysIdleStatus = 4;
                DBSuspend();

                #ifdef _FS_
                RKDev_Close(FileSysHDC);
                FileSysHDC = NULL;
                #endif

                RKDev_Close(hKey);
                hKey = NULL;
                RKDev_Close(ADCHandler);
                ADCHandler = NULL;

                #ifndef _IDLE_DEBUG_
                RKDev_Close(UartHDC);
                UartHDC = NULL;
                #endif
            }
        }
        else if(gSysConfig.SysIdleStatus == 4)
        {
            RKTaskIdleTick();
            DeviceTask_DevIdleTick();
            #ifdef _SPI_BOOT_
            #ifndef _IDLE_DEBUG_
            if(DevTotalCnt == (DevTotalSuspendCnt + 4))
            #else
            if(DevTotalCnt == (DevTotalSuspendCnt + 5))
            #endif
            #endif

            #ifdef _EMMC_BOOT_
            #ifndef _IDLE_DEBUG_
            if(DevTotalCnt == (DevTotalSuspendCnt + 5))
            #else
            if(DevTotalCnt == (DevTotalSuspendCnt + 6))
            #endif
            #endif
            {
                gSysConfig.SysIdleStatus = 5;
                FwSuspend();
                rk_printf("system enter idle2.9");
            }
        }
        else if(gSysConfig.SysIdleStatus == 5)
        {
            RKTaskIdleTick();
            DeviceTask_DevIdleTick();
            if((TaskTotalCnt == (TaskTotalSuspendCnt + 4)))
            {
                #ifndef _IDLE_DEBUG_
                if(DevTotalCnt == (DevTotalSuspendCnt + 1))
                #else
                if(DevTotalCnt == (DevTotalSuspendCnt + 2))
                #endif
                {
                    gSysConfig.SysIdleStatus = 3;
                    rk_printf("system enter idle3");
                    FREQ_ExitModule(FREQ_BLON);
                }
            }
        }
        else if(gSysConfig.SysIdleStatus == 3)
        {

        }
        else if(gSysConfig.PMTime > (gSysConfig.SysIdle1EventTime + gSysConfig.SysIdle2EventTime))
        {
            gSysConfig.SysIdleStatus = 2;
            rk_printf("system enter idle2");
        }
    }

    if(gSysConfig.SysIdleStatus < 3)
    {
        battery_level = Battery_GetLevel();
        if (gSysConfig.battery_level != battery_level)
        {
            gSysConfig.battery_level = battery_level;
            MainTask_SetTopIcon(MAINTASK_BATTERY);
        }
		
		PnPSever(); //called in Battery_GetLevel() jjjhhh----20161110
        
    }
    else if(((gSysConfig.PMTime % 10000) == 0) && (gSysConfig.SysIdleStatus == 3))
    {
        #if 0
        FwResume();
        if(UartHDC == NULL)
        {
            UartHDC = RKDev_Open(DEV_CLASS_UART, 0, NOT_CARE);
        }

        if(ADCHandler == NULL)
        {
            ADCHandler = RKDev_Open(DEV_CLASS_ADC, 0, NOT_CARE);
        }

        Battery_GetLevel();

        RKDev_Close(ADCHandler);
        ADCHandler = NULL;
        RKDev_Close(UartHDC);
        UartHDC = NULL;

        FwSuspend();
        #endif
    }
#else//#ifndef _DISABLE_ENTER_IDLEMODE
	battery_level = Battery_GetLevel();
	if (gSysConfig.battery_level != battery_level)//update battery level
	{
		gSysConfig.battery_level = battery_level;
		MainTask_SetTopIcon(MAINTASK_BATTERY);
	}
	//PnPSever(); //called in Battery_GetLevel() jjjhhh----20161110
#endif
}
/*******************************************************************************
** Name: PmuSuspend
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.11.8
** Time: 14:58:04
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_COMMON_
COMMON FUN void PmuSuspend(void)
{
    #if 0
    Grf->GRF_INTER_CON0 = FORCE_JTAG_MASK << 16 | FORCE_JTAG_DIS ;
    Grf->GPIO_IO0MUX[0] = 0xffff0000;
    Grf->GPIO_IO0MUX[1] = 0xffff0000;
    Grf->GPIO_IO0MUX[2] = 0xffff0000;
    Grf->GPIO_IO0MUX[3] = 0xffff0000;

    Grf->GPIO_IO1MUX[0] = 0xffff0000;
    Grf->GPIO_IO1MUX[1] = 0xffff0000;
    Grf->GPIO_IO1MUX[2] = 0xffff0000;
    Grf->GPIO_IO1MUX[3] = 0xffff0000;

    Grf->GPIO_IO2MUX[0] = 0xffff0000;
    Grf->GPIO_IO2MUX[1] = 0xffff0000;
    Grf->GPIO_IO2MUX[2] = 0xffff0000;
    Grf->GPIO_IO2MUX[3] = 0xffff0000;


    Gpio_SetPortDirec(GPIO_CH0,0x00000000);
    Gpio_SetPortDirec(GPIO_CH1,0x00000000);
    Gpio_SetPortDirec(GPIO_CH2,0x00000000);

    {
        //pull disable
        Grf->GPIO_IO0PULL[0] = 0xffffffff;
        Grf->GPIO_IO0PULL[1] = 0xffffffff;
        Grf->GPIO_IO0PULL[2] = 0xffffffff;
        Grf->GPIO_IO0PULL[3] = 0xffffffff;

        Grf->GPIO_IO1PULL[0] = 0xffffffff;
        Grf->GPIO_IO1PULL[1] = 0xffffffff;
        Grf->GPIO_IO1PULL[2] = 0xffffffff;
        Grf->GPIO_IO1PULL[3] = 0xffffffff;

        Grf->GPIO_IO2PULL[0] = 0xffffffff;
        Grf->GPIO_IO2PULL[1] = 0xffffffff;
        Grf->GPIO_IO2PULL[2] = 0xffffffff;
        Grf->GPIO_IO2PULL[3] = 0xffffffff;
    }

    //Power On
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin6,IOMUX_GPIO2A6_IO);
    Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin6,GPIO_OUT);
    Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin6,GPIO_HIGH);

    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin7,IOMUX_GPIO2A7_PMU_IDEL);

    Grf->GRF_UOC_CON1 = 0x000f0005;
    Grf->GRF_UOC_CON2 = 0x00010001;
    Grf->GRF_USBPHY_CON6 = 0x00040000;
    Grf->GRF_USBPHY_CON7 = 0x80008000;
    ScuClockGateCtr(CLK_USBPHY_GATE, 0);
    ScuClockGateCtr(HCLK_USBC_GATE, 0);

    ScuClockGateCtr(CLK_ACODEC_GATE, 0);      //ACODEC gating open
    ScuClockGateCtr(PCLK_ACODEC_GATE, 0);     //PCLK ACODEC gating open

    Grf_Set_ADC_Vref_PowerOn(GRF_VREF_PWOFF);
    AdcPowerDown();

    PmuPdLogicPowerDown(1);

    #if 0
    Grf_PVTM_Set_Cal_Cnt(1000);
    Grf_PVTM_OSC_Enable(1);
    Grf_PVTM_Fun_Start(1);
    Grf_PVTM_Set_Divider(127, 1);

    PmuSelLowFreq(PMU_LOW_FREQ_CLK_SEL_PVTM);
    Pmu_Reg->PMU_SOFTRST_CON = 0x00010001;
    Pmu_Reg->PMU_SOFT_CON = 0x00010001;
    Pmu_Reg->PMU_SOFT_CON |= PMU_OSC_POWER_DOWN_EN;

    #else

    Grf_PVTM_Set_Cal_Cnt(1000);
    Grf_PVTM_OSC_Enable(1);
    Grf_PVTM_Fun_Start(1);
    Grf_PVTM_Set_Divider(127, 1);

    PmuGpioPosedegePlusWakeUpEnable(GPIOPortA_Pin5);

    PmuSelLowFreq(PMU_LOW_FREQ_CLK_SEL_PVTM);
    PmuSetPowerMode(PMU_POWER_MODE_POWER_DOWN_VD_LOGIC
                    | PMU_POWER_MODE_USE_LOW_FREQ_EN
                    | PMU_POWER_MODE_OSC_POWER_DOWN_EN
                    | PMU_POWER_MODE_PLL_POWER_DOWN_EN
                    | PMU_POWER_MODE_EN);
    #endif

    DelayMs(1000);
    __WFI();
    while(1);
    #endif

}
/*******************************************************************************
** Name: PmuResume
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.11.8
** Time: 14:56:31
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_COMMON_
COMMON FUN void PmuResume(void)
{
#if 0
    UINT32 KeyTemp;

    //initialization for global variable.
    SysVariableInit();

    #ifdef _FILE_DEBUG_
    SysDebugHookInitMem();
    #endif

    //PLL setting,disable interrupt and PWM.
    SysCpuInit();
    IntMasterDisable(); /*disable int for sysinit*/

    //ADC initializtion, low power check,LCD initialization, Codec initialization.
    BoardInit();

    //interruption initialization.
    SysIntrruptInit();

    //system tick initialization.
    SysTickInit();

    //Flash initial,get system parameter, file system initial
    MediaInit();

    //文件系统初始化之后才能初始化调试文件
    #ifdef _FILE_DEBUG_
    SysDebugHookInit();
    #endif

    //module initialization.
    ModuleInit();

    SysServiceInit();

    while(1);
    #endif
}

/*******************************************************************************
** Name: FREQStopAPP
** Input:eFREQ_APP appId
** Return: int32
** Owner:aaron.sun
** Date: 2015.11.8
** Time: 14:47:54
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_COMMON_
COMMON FUN int32 FREQStopAPP(eFREQ_APP appId)
{
    if(appId >= FREQ_APP_MAX)
    {
        return (-1);
    }
    if(g_APPList & ((uint64)0x01<< appId))
    {
        g_APPList &= ~((uint64)0x01<< appId);
        FREQSetFreq(g_APPList);
    }
    else//this application had exit.
    {
        return(-1);
    }
    return(0);
}
/*******************************************************************************
** Name: FREQStartAPP
** Input:eFREQ_APP appId
** Return: int32
** Owner:aaron.sun
** Date: 2015.11.8
** Time: 14:46:58
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_COMMON_
COMMON FUN int32 FREQStartAPP(eFREQ_APP appId)
{
    if(g_APPList & ((uint64)0x01<< appId))//this application had enter.
    {
        return(0);
    }
    else
    {
        g_APPList |= ((uint64)0x01<< appId);
        FREQSetFreq(g_APPList);
        return(0);
    }

}

/*******************************************************************************
** Name: FREQSetFreq
** Input:uint64 appList
** Return: int32
** Owner:aaron.sun
** Date: 2015.11.8
** Time: 14:46:04
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_COMMON_
COMMON FUN int32 FREQSetFreq(uint64 appList)
{
    uint32 i;
    uint32 FreqIdleDisable = 0;
    FREQ_APP_TABLE FreqAppTabl;

    memset(&FreqAppTabl, 0,sizeof(FREQ_APP_TABLE));

    if(appList & (((uint64)0x01) << FREQ_IDLE))
    {
        appList = ((uint64)0x01) << FREQ_IDLE;
        FreqIdleDisable = 1;

    }
    else if(appList == 0)
    {
        appList = ((uint64)0x01) << FREQ_MIN;
        FreqIdleDisable = 0;
    }

    for(i = 0; i < FREQ_APP_MAX; i++)
    {
        if(appList & ((uint64)0x01 << i))
        {
            FreqAppTabl.syshclk  += g_CruAPPTabel[i].syshclk;
            FreqAppTabl.calhclk  += g_CruAPPTabel[i].calhclk;
        }
    }

    //syshclk min:mix
    if((FreqAppTabl.syshclk < g_CruAPPTabel[FREQ_MIN].syshclk) && (FreqIdleDisable == 0))
    {
        FreqAppTabl.syshclk = g_CruAPPTabel[FREQ_MIN].syshclk;
    }
    if(FreqAppTabl.syshclk > g_CruAPPTabel[FREQ_MAX].syshclk)
    {
        FreqAppTabl.syshclk = g_CruAPPTabel[FREQ_MAX].syshclk;
    }

    //calhclk min:mix
    if((FreqAppTabl.calhclk < g_CruAPPTabel[FREQ_MIN].calhclk) && (FreqIdleDisable == 0))
    {
        FreqAppTabl.calhclk = g_CruAPPTabel[FREQ_MIN].calhclk;
    }
    if(FreqAppTabl.calhclk > g_CruAPPTabel[FREQ_MAX].calhclk)
    {
        FreqAppTabl.calhclk = g_CruAPPTabel[FREQ_MAX].calhclk;
    }

    rkos_enter_critical();
    FREQSetARMFreq(&FreqAppTabl, &chip_freq);
    rkos_exit_critical();

    rk_printf("pll=%d, shclk=%d, stck=%d, splk=%d, hhclk=%d, htck=%d",
          chip_freq.pll, chip_freq.hclk_sys_core, chip_freq.stclk_sys_core, chip_freq.pclk_logic_pre,
          chip_freq.hclk_cal_core, chip_freq.stclk_cal_core);

    return(0);
}

/*******************************************************************************
** Name: FREQSetARMFreq
** Input:FREQ_APP_TABLE *FreqTab, chip_freq_t *pChipFreq
** Return: void
** Owner:aaron.sun
** Date: 2015.11.8
** Time: 14:44:41
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_COMMON_
COMMON FUN void FREQSetARMFreq(FREQ_APP_TABLE *FreqTab, chip_freq_t *pChipFreq)
{
    uint32 i,j,k;
    uint32 pllclk;
    uint32 syshclk_div;
    uint32 sysstclk_div;
    uint32 pclk_dkv;
    uint32 calhclk_div;
    uint32 calstclk_div;
    uint32 osc_24m_div;
    PLL_ARG_t PllArg;

    if ((FreqTab->syshclk <= 24000000) && (FreqTab->calhclk <= 24000000))
    {
        pllclk = 24000000;

        if (FreqTab->syshclk == 0)
        {
            FreqTab->syshclk = 24000000 / 64;
        }

        if (FreqTab->calhclk == 0)
        {
            FreqTab->calhclk = 24000000 / 64;
        }

        if (FreqTab->syshclk >= FreqTab->calhclk)
        {
            osc_24m_div = 24000000 / FreqTab->syshclk;
            pllclk = 24000000 / osc_24m_div;
        }
        else if (FreqTab->syshclk < FreqTab->calhclk)
        {
            osc_24m_div = 24000000 / FreqTab->calhclk;
            pllclk = 24000000 / osc_24m_div;
        }

        syshclk_div = pllclk / FreqTab->syshclk;
        calhclk_div = pllclk / FreqTab->calhclk;
    }
    else
    {
        if (FreqTab->syshclk == 0)
        {
            FreqTab->syshclk = FreqTab->calhclk;
        }
        else if (FreqTab->calhclk == 0)
        {
            FreqTab->calhclk = FreqTab->syshclk;
        }
        else if (FreqTab->syshclk > FreqTab->calhclk)
        {
            for (i = 1; i < 9; i++)
            {
                pllclk = FreqTab->calhclk * i;
                if (pllclk > 1600000000)
                {
                    pllclk = FreqTab->calhclk * (i - 1);
                    if (pllclk < FreqTab->syshclk)
                    {
                        pllclk = FreqTab->syshclk;
                    }
                    syshclk_div = pllclk / FreqTab->syshclk;
                    pllclk      = FreqTab->syshclk * syshclk_div;     //PLL 以系统频率对齐设定
                    calhclk_div = pllclk / FreqTab->calhclk;
                    if (calhclk_div > 8)
                    {
                        calhclk_div = 8;
                    }
                    break;
                }
                if ((pllclk % FreqTab->syshclk) == 0)
                {
                    calhclk_div = i;
                    syshclk_div = pllclk / FreqTab->syshclk;
                    break;
                }
            }

            if (i >= 9)
            {
                pllclk      = FreqTab->calhclk * 8;
                if (pllclk < FreqTab->syshclk)
                {
                    pllclk = FreqTab->syshclk;
                }
                syshclk_div = pllclk / FreqTab->syshclk;
                pllclk      = FreqTab->syshclk * syshclk_div;     //PLL 以系统频率对齐设定
                calhclk_div = pllclk / FreqTab->calhclk;
                if (calhclk_div > 8)
                {
                    calhclk_div = 8;
                }
            }
        }
        else // if (FreqTab->syshclk < FreqTab->calhclk)
        {
            for (i = 1; i < 9; i++)
            {
                pllclk = FreqTab->syshclk * i;
                if (pllclk > 1600000000)
                {
                    pllclk      = FreqTab->syshclk * (i - 1);
                    if (pllclk < FreqTab->calhclk)
                    {
                        pllclk = FreqTab->calhclk;
                    }
                    calhclk_div = pllclk / FreqTab->calhclk;
                    pllclk      = FreqTab->calhclk * calhclk_div;
                    syshclk_div = pllclk / FreqTab->syshclk;
                    if (pllclk / syshclk_div > 300000000)
                    {
                        syshclk_div++;
                    }
                    break;
                }
                if ((pllclk % FreqTab->calhclk) == 0)
                {
                    syshclk_div = i;
                    calhclk_div = pllclk / FreqTab->calhclk;
                    if (pllclk / syshclk_div > 300000000)
                    {
                        syshclk_div++;
                    }
                    if (syshclk_div > 8)
                    {
                        syshclk_div = 8;
                    }
                    break;
                }
            }
            if (i >= 9)
            {
                pllclk      = FreqTab->syshclk * 8;
                if (pllclk < FreqTab->calhclk)
                {
                    pllclk = FreqTab->calhclk;
                }
                calhclk_div = pllclk / FreqTab->calhclk;
                pllclk      = FreqTab->calhclk * calhclk_div;
                syshclk_div = pllclk / FreqTab->syshclk;
                if (pllclk / syshclk_div > 300000000)
                {
                    syshclk_div++;
                }
                if (syshclk_div > 8)
                {
                    syshclk_div = 8;
                }
            }
        }
    }

    //if(pllclk != pChipFreq->pll)
    {
        pChipFreq->pll = pllclk;

        if (pChipFreq->pll > 24000000)
        {
            Get_PLL_Post_Div(pChipFreq->pll, &PllArg);
            Get_PLL_Pre_Div(PllArg.VCO, &PllArg);
        }
        else
        {
            memset(&PllArg, 0, sizeof(PLL_ARG_t));
            PllArg.div_con_24m = osc_24m_div;
        }

        //sys core hclk/fclk
        PllArg.sys_core_div = syshclk_div;
        pChipFreq->hclk_sys_core = pChipFreq->pll / syshclk_div;
        pChipFreq->fclk_sys_core  = pChipFreq->hclk_sys_core;

        //sys core stclk
        PllArg.sys_stclk_div = 1;

        pChipFreq->stclk_sys_core = pChipFreq->hclk_sys_core / PllArg.sys_stclk_div;
        //sys core pclk

        #if 0
        if(pChipFreq->hclk_sys_core >= 200000000)
        {
            PllArg.pclk_logic_div = 3;
        }
        else if(pChipFreq->hclk_sys_core >= 100000000)
        {
            PllArg.pclk_logic_div = 1;
        }
        else
        {
            PllArg.pclk_logic_div = 0;
        }
        #else
        PllArg.pclk_logic_div = 1;
        #endif
                //DEBUG("PllArg.pclk_logic_div = %d", PllArg.pclk_logic_div);
        pChipFreq->pclk_logic_pre = pChipFreq->hclk_sys_core / (1 << PllArg.pclk_logic_div);

        //cal core hclk/fclk
        PllArg.cal_core_div = calhclk_div;
        pChipFreq->hclk_cal_core = pChipFreq->pll / calhclk_div;
        pChipFreq->fclk_cal_core = pChipFreq->hclk_cal_core;
        //cal core stclk
        PllArg.cal_stclk_div = 1;
        if (PllArg.cal_stclk_div == 0) PllArg.cal_stclk_div = 1;

        pChipFreq->stclk_cal_core = pChipFreq->hclk_cal_core / PllArg.cal_stclk_div;
        SetPllFreq(pllclk, &PllArg);
        SetHclkLogicPreFreq();
        SetPclkPmuPreFreq();
        SetHclkPmuPreFreq();
        if(pChipFreq->pclk_logic_pre < (pChipFreq->saradc_clk * 2))
        {
            SetSarAdcFreq(pChipFreq->pclk_logic_pre / 2);        // 1M
        }
        else if(pChipFreq->pclk_logic_pre >= (2 * 1000 * 1000))
        {
            SetSarAdcFreq(1 * 1000 * 1000);
        }

        {
            #ifdef __DRIVER_BCORE_BCOREDEVICE_C__
            rk_err_t ret;
            ret = RKDeviceFind(DEV_CLASS_BCORE, 0);
            if(ret > 0)
            {
                if(BcoreDev_IsRuning((HDC)ret) == RK_SUCCESS)
                {
                    memcpy((void *)0x01020000, &chip_freq, sizeof(chip_freq_t));
                }
            }
            #endif
        }

        if(hWdt != NULL)
        {
            WDTDev_Feed(hWdt);
        }

        SysTickPeriodSet(10);
    }

}


/*******************************************************************************
** Name: Charge_State_Get
** Input:void
** Return: uint32
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_COMMON_
COMMON FUN uint32 Charge_State_Get(void)
{
    uint32 ret = 0;

    //Charge OK Get
    #if(RKNANOD_CHIP_TYPE != RKNANOD_N)
    ret = Gpio_GetPinLevel(GPIO_CH2, GPIOPortA_Pin1);
    #endif

    return ret;
}

/*******************************************************************************
** Name: Charger_Current_Get
** Input:void
** Return: Charge_Current_Value
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_COMMON_
COMMON FUN Charge_Current_Value Charger_Current_Get(void)
{
    uint32 ret = 0;
    //Reserved for charge current get

    ret = ChargeCurrent;

    return ret;
}

/*******************************************************************************
** Name: Charge_Current_Set
** Input:Charge_Current_Value
** Return: void
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_COMMON_
COMMON FUN void Charge_Current_Set(Charge_Current_Value Value)
{
    //Reserved for charge current set

    ChargeCurrent = Value;
    if (Value == CHARGE_CURRENT_LOW)
    {
        #if(RKNANOD_CHIP_TYPE != RKNANOD_N)
        Gpio_SetPinLevel(GPIO_CH0,GPIOPortB_Pin4,GPIO_LOW);
        #endif
    }
    else
    {
        #if(RKNANOD_CHIP_TYPE != RKNANOD_N)
        Gpio_SetPinLevel(GPIO_CH0,GPIOPortB_Pin4,GPIO_HIGH);
        #endif
    }
}


/*******************************************************************************
** Name: ChargeFullCheck
** Input:void
** Return: uint32
** Owner:  wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_COMMON_
COMMON FUN uint32 ChargeFullCheck(void)
{
    UINT32 batt_state = CHARGE_BATT;
    uint32 ChargeTimeMs = SysTickCounter;

    if (ChargeFullFlag == 0)
    {
        if ((ChargeTimeMs - lastChargeTimeMs) > CHARGE_SAMPLE_TIME) //间隔5秒
        {
            lastChargeTimeMs = ChargeTimeMs;

            if ((gBattery.Batt_Value > BATT_USB_FULL_VALUE))
            {
                chargeFullCheckCnt++;

                if (chargeFullCheckCnt == CHARGE_CHECKFULL_COUNT)
                {
                    switch (Charger_Current_Get())
                    {
                        case CHARGE_CURRENT_HIGH: //目前永远不执行
                            chargeFullCheckCnt = 0;
                            Charge_Current_Set(CHARGE_CURRENT_LOW);
                            printf("Charge Current set is Low !\n");
                            break;

                        case CHARGE_CURRENT_LOW://满电或者快满电
                            printf("Charge will stop after 15 min!\n");
                            break;
                        default:
                            break;
                    }
                }
                if (chargeFullCheckCnt == CHARGE_FULL_DELAY)
                {
                    DEBUG("Battery ADC charge Full!");
                    batt_state =  CHARGE_FULL;
                }
            }
            else
            {
                if (chargeFullCheckCnt < CHARGE_CHECKFULL_COUNT)
                {
                    chargeFullCheckCnt = 0;
                }
            }
        }

        if (Charge_State_Get()) //目前永远不运行
        {
            printf("Battery PMU charge Full!");
            batt_state =  CHARGE_FULL;
        }

        if (batt_state == CHARGE_FULL)
        {
            if (ChargeFullFlag == 0)
            {
                ChargeFullFlag = 1;
                printf("Charge Full....\n");
            }
        }
    }
    else
    {
        batt_state = CHARGE_FULL;
    }

    return batt_state;
}


/*******************************************************************************
** Name: Battery_GetChargeState
** Input:void
** Return: UINT16
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_COMMON_
COMMON FUN UINT16 Battery_GetChargeState(void)
{
    UINT32 i;
    UINT32 battvalue;
    UINT32 batt_state = CHARGE_NO;
    uint16 ADCVal;

    if (gBattery.Enable_Charge)
    {
        if (gBattery.IsBatt_Charge != TRUE)
        {
            gBattery.IsBatt_Charge = TRUE;
            MainTask_DeleteTopIcon(MAINTASK_BATTERY);
            MainTask_SetTopIcon(MAINTASK_CHARGE);
            return CHARGE_BATT;
        }
        batt_state = ChargeFullCheck(); //充电状态检查是否满电
    }
    else
    {
        ChargeFullFlag = 0;
        batt_state  = CHARGE_NO;

        if(TRUE == gBattery.IsBatt_Charge)
        {
            for(i = 0; i < 50; i++) //循环50此获取稳定电压值
            {
                #if 0
                battlevel = GetAdcData(ADC_CHANEL_BATTERY);
                #else
                ADCDev_Read(ADCHandler,ADC_CHANEL_BATTERY, 1, 1);
                ADCDev_GetAdcBufData(ADCHandler,(uint16 *)&ADCVal, 1, ADC_CHANEL_BATTERY);
                battvalue = ADCVal;
                #endif
                battvalue = (UINT32)RealBattValue(battvalue);

                gBattery.Batt_Value += battvalue;
                gBattery.Batt_Value >>= 1;

            }

            for(gBattery.Batt_Level = 0; gBattery.Batt_Level < BATT_TOTAL_STEPS - 1; gBattery.Batt_Level++)
            {
                if (gBattery.Batt_Value < Batt_Level[gBattery.Batt_Level])
                {
                    break;
                }
            }

            if(gBattery.Batt_Level > 0)
            {
                gBattery.Batt_Level = gBattery.Batt_Level - 1;
            }

            if(gBattery.Batt_Value > BATT_FULL_VALUE)
            {
                gBattery.Batt_Level = BATT_TOTAL_STEPS - 1;
                batt_state = CHARGE_FULL;
            }


            gBattery.IsBatt_Charge = FALSE;
            gBattery.IsBattChange  = TRUE;//电压改变状态，现在还未使用

            MainTask_DeleteTopIcon(MAINTASK_CHARGE);
            MainTask_SetTopIcon(MAINTASK_BATTERY);

        }
    }

    return batt_state;

}

/*******************************************************************************
** Name: RealBattValue
** Input:void
** Return: void
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_COMMON_
COMMON FUN uint32 RealBattValue(UINT16 adc)
{
    int32  delta;
    int32  SetVol;
    uint32 i, buffer_max;
    uint32 temp;

    temp = 2500;                          //ADC Ref = 2500mV
    temp = ((adc << 1) * temp >> 10);//hj

    BatteryBuffer[BatteryCnt%BATTERY_BUF_SIZE] = temp;
    BatteryCnt++;

    buffer_max = BATTERY_BUF_SIZE;
    if (BatteryCnt < BATTERY_BUF_SIZE)
    {
        buffer_max = BatteryCnt;
    }

    temp = 0;
    for (i = 0; i < buffer_max; i++)
    {
        temp += (uint32)(BatteryBuffer[i]);
    }
    temp = temp/buffer_max;

    return (temp);
}

/*******************************************************************************
** Name: BatteryInit
** Input:void
** Return: void
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_COMMON_
COMMON FUN void BatteryInit(void)
{
    UINT32 battlevel;
    UINT32 i,j;
    uint32 temp;
    UINT8  AdcSamplingCh;
    uint16 ADCVal;

    BatteryCnt = 0;

    for(i =  0; i < 18; i++)
    {
        ADCDev_Read(ADCHandler,ADC_CHANEL_BATTERY, 1, 1);
        ADCDev_GetAdcBufData(ADCHandler,(uint16 *)&ADCVal, 1, ADC_CHANEL_BATTERY);
        battlevel = (UINT32)RealBattValue(ADCVal);
        if (i >= 10)
        {
            gBattery.Batt_Value += battlevel;
        }
    }

    gBattery.Batt_Value >>= 3; //取平均值(8个数据相加)

    for (gBattery.Batt_Level = 0; gBattery.Batt_Level < BATT_TOTAL_STEPS - 1; gBattery.Batt_Level++)
    {
        if (gBattery.Batt_Value < Batt_Level[gBattery.Batt_Level])
        {
            break;
        }
    }

    if((gBattery.Batt_Value <= BATT_EMPTY_VALUE+10)&(!Grf_CheckVbus()))
    {
        printf("\nLow Power, BatteryVal = %d\n", gBattery.Batt_Value);
        DelayMs(200);
        System_Power_On(0);       //power down
        while(1);
    }

    if(gBattery.Batt_Level>0) gBattery.Batt_Level = gBattery.Batt_Level-1 ;

    if((gBattery.Batt_Value>BATT_FULL_VALUE) == TRUE)
        gBattery.Batt_Level = BATT_TOTAL_STEPS - 1;

    ChargeFullFlag     = 0;
    chargeFullCheckCnt = 0;
    gBattery.Enable_Charge = 0;
    gBattery.IsBatt_Charge = 0;
    lastChargeTimeMs   = SysTickCounter;

}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: BatteryManagerDeInit
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2016.4.12
** Time: 18:14:26
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_INIT_
INIT API void BatteryManagerDeInit(void)
{
    RKDev_Close(ADCHandler);
    RKDev_Close(hLcd);
}
/*******************************************************************************
** Name: PowerManagerStart
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.11.13
** Time: 9:48:55
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_INIT_
INIT API void PowerManagerStart(void)
{
    rkos_start_timer(PowerTimer);
    BatteryManagerInit();
}

/*******************************************************************************
** Name: PowerManagerEnd
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2016.3.16
** Time: 16:49:24
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_INIT_
INIT API void PowerManagerEnd(void)
{
    rkos_stop_timer(PowerTimer);
    rkos_delete_timer(PowerTimer);
    BatteryManagerDeInit();
}


/*******************************************************************************
** Name: PowerManagerInit
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.11.8
** Time: 16:43:40
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_INIT_
INIT API void PowerManagerInit(void)
{
    FREQ_Control_Init();
    PowerTimer = rkos_create_timer(PM_TIME, -1, NULL, PowerTimerIsr);
}

/*******************************************************************************
** Name: BatteryManagerInit
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.11.8
** Time: 16:43:40
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_INIT_
INIT API void BatteryManagerInit(void)
{
    rk_err_t ret;

    /*ADC*/
    ADCHandler = RKDev_Open(DEV_CLASS_ADC, 0, NOT_CARE);
    if (ADCHandler == NULL)
    {
        printf("BATTERY:ADC Open fail\n");
    }

#ifdef _USE_GUI_
    hLcd = RKDev_Open(DEV_CLASS_LCD, 0, NOT_CARE);
    Lcd_BL_On(hLcd);
#endif

    BatteryInit();

    gSysConfig.PMTime = 0;
    gSysConfig.SysIdle = 0;
    gSysConfig.SysIdleStatus = 0;
    gSysConfig.SysIdle2EventTime = 500;


}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: FREQ_Control_Init
** Input:void
** Return: int32
** Owner:aaron.sun
** Date: 2015.11.8
** Time: 14:52:40
*******************************************************************************/
_SYSTEM_SYSSEVER_POWERMANAGER_INIT_
INIT FUN int32 FREQ_Control_Init(void)
{
    FreqDisableFlag = 0;
    FreqDisableCnt = 0;
    FreqOperSem = rkos_semaphore_create(1, 1);
}



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
