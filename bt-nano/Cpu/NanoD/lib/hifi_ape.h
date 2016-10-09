/*
********************************************************************************************
*
*                  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                               All rights reserved.
*
* FileName: Cpu\NanoC\lib\hifi_ape.h
* Owner: WJR
* Date: 2014.11.28
* Time: 15:55:04
* Desc:
* History:
*    <author>     <date>       <time>     <version>       <Desc>
*    WJR     2014.11.28     15:55:04   1.0
********************************************************************************************
*/

#ifndef __CPU_NANOD_LIB_HIFI_APE_H__
#define __CPU_NANOD_LIB_HIFI_APE_H__

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
#define _CPU_NANOD_LIB_HIFI_APE_COMMON_  __attribute__((section("cpu_nanod_lib_hifi_ape_common")))
#define _CPU_NANOD_LIB_HIFI_APE_INIT_  __attribute__((section("cpu_nanod_lib_hifi_ape_init")))
#define _CPU_NANOD_LIB_HIFI_APE_SHELL_  __attribute__((section("cpu_nanod_lib_hifi_ape_shell")))

#define APE_VERSION_LESS_3980                (uint32)(0x0 << 31) //<3980的版本
#define APE_VERSION_EQUAL_OR_GREAT_3980      (uint32)(0x1 << 31) //>=3980的版本

#define APE_COMPRESS_LEVEL_1                 (uint32)(0x0)
#define APE_COMPRESS_LEVEL_2                 (uint32)(0x10<<22)
#define APE_COMPRESS_LEVEL_3                 (uint32)(0x40<<22)
#define APE_COMPRESS_LEVEL_4                 (uint32)((0x20<<22)|(0x1<<29))
#define APE_COMPRESS_LEVEL_5                 (uint32)((0x10<<22)|(0x2<<29))

#define APE_LPC_QUANT_256                        (uint32)(0xD << 12)//wjr
#define APE_LPC_QUANT_1280                       (uint32)(0xF << 8)
#define APE_LPC_QUANT_64                         (uint32)(0xB << 16)
#define APE_LPC_QUANT_16                         (uint32)(0xB << 16)
#define APE_LPC_QUANT_32                         (uint32)(0xA << 16)

#define APE_CLC                                  (uint32)(0x1 << 21)
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
extern rk_err_t APE_Clear(UINT32 HifiId);
extern rk_err_t hifi_ape_shell(void);
extern rk_err_t APE_Set_CFG(UINT32 HifiId,int version,int com_level);



#endif

