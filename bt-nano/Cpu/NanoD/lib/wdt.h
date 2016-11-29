/*
********************************************************************************************
*
*        Copyright (c): 2016 - 2016 + 5, Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Cpu\NanoD\lib\wdt.h
* Owner: aaron.sun
* Date: 2016.9.28
* Time: 10:22:55
* Version: 1.0
* Desc: wdt bsp api interface file
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2016.9.28     10:22:55   1.0
********************************************************************************************
*/


#ifndef __CPU_NANOD_LIB_WDT_H__
#define __CPU_NANOD_LIB_WDT_H__

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

#define _CPU_NANOD_LIB_WDT_COMMON_  __attribute__((section("cpu_nanod_lib_wdt_common")))
#define _CPU_NANOD_LIB_WDT_INIT_  __attribute__((section("cpu_nanod_lib_wdt_init")))
#define _CPU_NANOD_LIB_WDT_SHELL_  __attribute__((section("cpu_nanod_lib_wdt_shell")))

typedef enum WdtRespMode
{
    RESP_MODE_RESET,/*Generate a system reset */

    RESP_MODE_INT_RESET,/*First generate an interrupt and if it is not
                        cleared by the time a second timeout occurs then
                       generate a system reset*/

}WdtRespMode_t;


typedef enum WdtRstPluseLenth
{
/*This is used to select the number of pclk cycles
for which the system reset stays asserted.*/
    PCLK_CYCLES_2,
    PCLK_CYCLES_4,
    PCLK_CYCLES_8,
    PCLK_CYCLES_16,
    PCLK_CYCLES_32,
    PCLK_CYCLES_64,
    PCLK_CYCLES_128,
    PCLK_CYCLES_256,

}WdtRstPluseLenth_t;

typedef enum WdtTimeoutPeriod
{
/*This field is used to select the timeout period from
which the watchdog counter restarts. A change of the
timeout period takes effect only after the next
counter restart (kick).
The range of values available for a 32-bit watchdog*/

   PERIOD_RANGE_0X0000FFFF,
   PERIOD_RANGE_0X0001FFFF,
   PERIOD_RANGE_0X0003FFFF,
   PERIOD_RANGE_0X0007FFFF,

   PERIOD_RANGE_0X000FFFFF,
   PERIOD_RANGE_0X001FFFFF,
   PERIOD_RANGE_0X003FFFFF,
   PERIOD_RANGE_0X007FFFFF,

   PERIOD_RANGE_0X00FFFFFF,
   PERIOD_RANGE_0X01FFFFFF,
   PERIOD_RANGE_0X03FFFFFF,
   PERIOD_RANGE_0X07FFFFFF,

   PERIOD_RANGE_0X0FFFFFFF,
   PERIOD_RANGE_0X1FFFFFFF,
   PERIOD_RANGE_0X3FFFFFFF,
   PERIOD_RANGE_0X7FFFFFFF,


}WdtTimeoutPeriod_t;
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
extern rk_err_t WatchDogReload(void);
extern rk_size_t WatchDogGetStat(void);
extern rk_err_t WatchDogInit(WdtRespMode_t mode , WdtRstPluseLenth_t RstPluse, WdtTimeoutPeriod_t period);



#endif
