/*
********************************************************************************************
*
*        Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: OS\FwAnalysis\FwUpdate.h
* Owner: aaron.sun
* Date: 2016.5.9
* Time: 17:38:44
* Version: 1.0
* Desc: fw update.h
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2016.5.9     17:38:44   1.0
********************************************************************************************
*/


#ifndef __OS_FWANALYSIS_FWUPDATE_H__
#define __OS_FWANALYSIS_FWUPDATE_H__

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

#define _OS_FWANALYSIS_FWUPDATE_COMMON_  __attribute__((section("os_fwanalysis_fwupdate_common")))
#define _OS_FWANALYSIS_FWUPDATE_INIT_  __attribute__((section("os_fwanalysis_fwupdate_init")))
#define _OS_FWANALYSIS_FWUPDATE_SHELL_  __attribute__((section("os_fwanalysis_fwupdate_shell")))
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
extern rk_err_t FwRecovery(void);
extern rk_err_t FwUpdate(uint16 * path, uint32 ForceUpate);
extern void FwCheck(void);

extern rk_err_t CheckOTAandUpdateFw(void);

#endif
