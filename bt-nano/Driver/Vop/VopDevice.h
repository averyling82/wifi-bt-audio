/*
********************************************************************************************
*
*                  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                               All rights reserved.
*
* FileName: ..\Driver\Vop\VopDevice.h
* Owner: Benjo.lei
* Date: 2015.10.09
* Time: 14:43:48
* Desc:
* History:
*    <author>     <date>       <time>     <version>       <Desc>
*    Benjo.sun     2015.10.09     14:43:48   1.0
********************************************************************************************
*/

#ifndef __DRIVER_VOP_VOPDEVICE_H__
#define __DRIVER_VOP_VOPDEVICE_H__

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
#define _DRIVER_VOP_VOPDEVICE_COMMON_      __attribute__((section("driver_vop_vopdevice_common")))
#define _DRIVER_VOP_VOPDEVICE_INIT_      __attribute__((section("driver_vop_vopdevice_init")))
#define _DRIVER_VOP_VOPDEVICE_SHELL_      __attribute__((section("driver_vop_vopdevice_shell")))

#if defined(__arm__) && (__ARMCC_VERSION)
#define _DRIVER_VOP_VOPDEVICE_SHELL_DATA_      _DRIVER_VOP_VOPDEVICE_SHELL_
#elif defined(__arm__) && (__GNUC__)
#define _DRIVER_VOP_VOPDEVICE_SHELL_DATA_      __attribute__((section("driver_vop_vopdevice_shell_data")))
#else
#error Unknown compiling tools.
#endif

typedef enum VOP_DEV
{
    VOP_DEV0 = 0,   //0
    VOP_DEV_MAX
}eVOP_DEV;


typedef struct _VOP_DEV_ARG
{
    HDC hDma;
    uint32 Channel;
}VOP_DEV_ARG;


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
extern rk_err_t VopDevSetWidthHeight(HDC dev, uint32 Width, uint32 Height);
extern rk_size_t VopDev_Write(HDC dev, uint8 * buffer, uint32 size, uint32 data);

extern HDC VopDev_Create(uint32 DevID, void *arg);
extern rk_err_t VopDev_Delete(uint32 DevID, void * arg);

#ifdef _VOP_DEV_SHELL_
extern rk_err_t VopDev_Shell(HDC dev, uint8 * pstr);
#endif
#endif

