#ifndef __WEB_AIRPLAY_H__
#define __WEB_AIRPLAY_H__
#include "typedef.h"

//---------HJL----------------------------------
#define _AIRPLAY_TASK_INIT_
#define _AIRPLAY_SHELL_ __attribute__((section("AirplayShell")))
#if defined(__arm__) && defined(__ARMCC_VERSION)
#define _AIRPLAY_SHELL_DATA_ __attribute__((section("AirplayShell")))
#elif defined(__arm__) && defined(__GNUC__)
#define _AIRPLAY_SHELL_DATA_ __attribute__((section("AirplayShell_data")))
#else

#endif

typedef void * HDC;
#define airplay_shell XXX_shell
#define AIRPLAYTask_Enter XXXTask_Enter
#define AIRPLAYTask_DeInit XXXTask_DeInit
#define AIRPLAYTask_Init XXXTask_Init
#define TASK_ID_AIRPLAY TASK_ID_XXX
#define airplay_end XXX_end
#define airplay_init_state XXX_init_state

extern rk_err_t airplay_shell(HDC dev, uint8 * pstr);
extern rk_err_t AIRPLAYTask_Resume(void);
extern rk_err_t AIRPLAYTask_Suspend(void);
extern void AIRPLAYTask_Enter(void);
extern rk_err_t AIRPLAYTask_DeInit(void *pvParameters);
extern rk_err_t AIRPLAYTask_Init(void *pvParameters, void *arg);

#endif

