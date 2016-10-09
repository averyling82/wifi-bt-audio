/*
********************************************************************************************
*
*  Copyright (c):Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* FileName: Driver\Example\DeviceFormat.c
* Owner: Aaron.sun
* Date: 2014.5.16
* Time: 16:30:19
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    Aaron.sun     2014.5.16     16:30:19   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __DRIVER_EXAMPLE_DEVICEFORMAT_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "Bsp.h"



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef  struct _TEST_DEVICE_CLASS
{
	DEVICE_CLASS stTestDevice;
	pSemaphore osTestReadReqSem;
	pSemaphore osTestReadSem;
	pSemaphore osTestWriteReqSem;
	pSemaphore osTestWriteSem;
	pSemaphore osTestControlReqSem;

}TEST_DEVICE_CLASS;

#define _DRIVER_EXAMPLE_DEVICEFORMAT_COMMON_  __attribute__((section("driver_example_deviceformat_read")))
#define _DRIVER_EXAMPLE_DEVICEFORMAT_COMMON_ __attribute__((section("driver_example_deviceformat_write")))
#define _DRIVER_EXAMPLE_DEVICEFORMAT_INIT_  __attribute__((section("driver_example_deviceformat_init")))
#define _DRIVER_EXAMPLE_DEVICEFORMAT_SHELL_  __attribute__((section("driver_example_deviceformat_shell")))


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static TEST_DEVICE_CLASS * gpstTestDevISR;



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
rk_err_t TestDevShellTest(HDC dev, uint8 * pstr);
rk_err_t TestDevShellDel(HDC dev, uint8 * pstr);
rk_err_t TestDevShellMc(HDC dev, uint8 * pstr);
rk_err_t TestDevShellPcb(HDC dev, uint8 * pstr);
void TestDevIntIsr(void);
rk_err_t TestDevDelete(HDC dev);
rk_err_t TestDevDeInit(HDC dev);
rk_err_t TestDevInit(HDC dev);
rk_err_t TestDevResume(HDC dev);
rk_err_t TestDevSuspend(HDC dev);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: TestDev_Read
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.16
** Time: 16:51:29
*******************************************************************************/
_DRIVER_EXAMPLE_DEVICEFORMAT_COMMON_
COMMON API rk_err_t TestDev_Read(HDC dev)
{

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: TestDevIntIsr
** Input:void
** Return: void
** Owner:Aaron.sun
** Date: 2014.5.16
** Time: 16:51:29
*******************************************************************************/
_DRIVER_EXAMPLE_DEVICEFORMAT_COMMON_
COMMON FUN void TestDevIntIsr(void)
{

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(write) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: TestDev_Write
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.16
** Time: 16:51:29
*******************************************************************************/
_DRIVER_EXAMPLE_DEVICEFORMAT_COMMON_
COMMON API rk_err_t TestDev_Write(HDC dev)
{

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(write) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: TestDev_Create
** Input:void * arg
** Return: HDC
** Owner:Aaron.sun
** Date: 2014.5.16
** Time: 16:51:29
*******************************************************************************/
_DRIVER_EXAMPLE_DEVICEFORMAT_INIT_
INIT API HDC TestDev_Create(uint32 DevID, void * arg)
{

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: TestDevDelete
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.16
** Time: 16:51:29
*******************************************************************************/
_DRIVER_EXAMPLE_DEVICEFORMAT_INIT_
INIT FUN rk_err_t TestDevDelete(HDC dev)
{

}
/*******************************************************************************
** Name: TestDevDeInit
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.16
** Time: 16:51:29
*******************************************************************************/
_DRIVER_EXAMPLE_DEVICEFORMAT_INIT_
INIT FUN rk_err_t TestDevDeInit(HDC dev)
{

}
/*******************************************************************************
** Name: TestDevInit
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.16
** Time: 16:51:29
*******************************************************************************/
_DRIVER_EXAMPLE_DEVICEFORMAT_INIT_
INIT FUN rk_err_t TestDevInit(HDC dev)
{

}
/*******************************************************************************
** Name: TestDevResume
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.16
** Time: 16:51:29
*******************************************************************************/
_DRIVER_EXAMPLE_DEVICEFORMAT_INIT_
INIT FUN rk_err_t TestDevResume(HDC dev)
{

}
/*******************************************************************************
** Name: TestDevSuspend
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.16
** Time: 16:51:29
*******************************************************************************/
_DRIVER_EXAMPLE_DEVICEFORMAT_INIT_
INIT FUN rk_err_t TestDevSuspend(HDC dev)
{

}


#ifdef _TEST_SHELL_
_DRIVER_EXAMPLE_DEVICEFORMAT_SHELL_
static SHELL_CMD ShellTestName[] =
{
    "pcb",NULL,"NULL","NULL",
    "mc",NULL,"NULL","NULL",
    "del",NULL,"NULL","NULL",
    "test",NULL,"NULL","NULL",
    "help",NULL,"NULL","NULL",
    "\b",NULL,"NULL","NULL",
};

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: TestDev_Shell
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.16
** Time: 16:51:29
*******************************************************************************/
_DRIVER_EXAMPLE_DEVICEFORMAT_SHELL_
SHELL API rk_err_t TestDev_Shell(HDC dev, uint8 * pstr)
{

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: TestDevShellTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.16
** Time: 16:51:29
*******************************************************************************/
_DRIVER_EXAMPLE_DEVICEFORMAT_SHELL_
SHELL FUN rk_err_t TestDevShellTest(HDC dev, uint8 * pstr)
{

}
/*******************************************************************************
** Name: TestDevShellDel
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.16
** Time: 16:51:29
*******************************************************************************/
_DRIVER_EXAMPLE_DEVICEFORMAT_SHELL_
SHELL FUN rk_err_t TestDevShellDel(HDC dev, uint8 * pstr)
{

}
/*******************************************************************************
** Name: TestDevShellMc
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.16
** Time: 16:51:29
*******************************************************************************/
_DRIVER_EXAMPLE_DEVICEFORMAT_SHELL_
SHELL FUN rk_err_t TestDevShellMc(HDC dev, uint8 * pstr)
{

}
/*******************************************************************************
** Name: TestDevShellPcb
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.16
** Time: 16:51:29
*******************************************************************************/
_DRIVER_EXAMPLE_DEVICEFORMAT_SHELL_
SHELL FUN rk_err_t TestDevShellPcb(HDC dev, uint8 * pstr)
{

}



#endif
#endif

