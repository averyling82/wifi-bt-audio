/*
********************************************************************************************
*
*                Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\Example\TaskFormat.c
* Owner: Aaron.sun
* Date: 2014.5.16
* Time: 17:05:40
* Desc: Task Format
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    Aaron.sun     2014.5.16     17:05:40   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __APP_EXAMPLE_TASKFORMAT_C__

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
typedef  struct _TEST_RESP_QUEUE
{
    uint32 cmd;
    uint32 status;

}TEST_RESP_QUEUE;
typedef  struct _TEST_ASK_QUEUE
{
    uint32 cmd;

}TEST_ASK_QUEUE;
typedef  struct _TEST_TASK_DATA_BLOCK
{
    pQueue  TestAskQueue;
    pQueue  TestRespQueue;

}TEST_TASK_DATA_BLOCK;

#define _APP_EXAMPLE_TASKFORMAT_READ_  __attribute__((section("app_example_taskformat_read")))
#define _APP_EXAMPLE_TASKFORMAT_WRITE_ __attribute__((section("app_example_taskformat_write")))
#define _APP_EXAMPLE_TASKFORMAT_INIT_  __attribute__((section("app_example_taskformat_init")))
#define _APP_EXAMPLE_TASKFORMAT_SHELL_  __attribute__((section("app_example_taskformat_shell")))


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static TEST_TASK_DATA_BLOCK * gpstTESTDataBlock;



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
rk_err_t TestTaskShellTest(HDC dev, uint8 * pstr);
rk_err_t TestTaskShellDel(HDC dev, uint8 * pstr);
rk_err_t TestTaskShellMc(HDC dev, uint8 * pstr);
rk_err_t TestTaskShellPcb(HDC dev, uint8 * pstr);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: TestTask_Enter
** Input:void
** Return: void
** Owner:Aaron.sun
** Date: 2014.5.16
** Time: 18:01:49
*******************************************************************************/
_APP_EXAMPLE_TASKFORMAT_READ_
COMMON API void TestTask_Enter(void)
{

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(write) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



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
** Name: TestTask_DevInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.16
** Time: 18:01:49
*******************************************************************************/
_APP_EXAMPLE_TASKFORMAT_INIT_
INIT API rk_err_t TestTask_DevInit(void *pvParameters)
{

}
/*******************************************************************************
** Name: TestTask_Init
** Input:void *pvParameters
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.16
** Time: 18:01:49
*******************************************************************************/
_APP_EXAMPLE_TASKFORMAT_INIT_
INIT API rk_err_t TestTask_Init(void *pvParameters)
{

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/


#ifdef _TEST_TASK_SHELL_
_APP_EXAMPLE_TASKFORMAT_SHELL_
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
** Name: TestTask_Shell
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.16
** Time: 18:01:49
*******************************************************************************/
_APP_EXAMPLE_TASKFORMAT_SHELL_
SHELL API rk_err_t TestTask_Shell(HDC dev, uint8 * pstr)
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
** Name: TestTaskShellTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.16
** Time: 18:01:49
*******************************************************************************/
_APP_EXAMPLE_TASKFORMAT_SHELL_
SHELL FUN rk_err_t TestTaskShellTest(HDC dev, uint8 * pstr)
{

}
/*******************************************************************************
** Name: TestTaskShellDel
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.16
** Time: 18:01:49
*******************************************************************************/
_APP_EXAMPLE_TASKFORMAT_SHELL_
SHELL FUN rk_err_t TestTaskShellDel(HDC dev, uint8 * pstr)
{

}
/*******************************************************************************
** Name: TestTaskShellMc
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.16
** Time: 18:01:49
*******************************************************************************/
_APP_EXAMPLE_TASKFORMAT_SHELL_
SHELL FUN rk_err_t TestTaskShellMc(HDC dev, uint8 * pstr)
{

}
/*******************************************************************************
** Name: TestTaskShellPcb
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.16
** Time: 18:01:49
*******************************************************************************/
_APP_EXAMPLE_TASKFORMAT_SHELL_
SHELL FUN rk_err_t TestTaskShellPcb(HDC dev, uint8 * pstr)
{

}



#endif
#endif

