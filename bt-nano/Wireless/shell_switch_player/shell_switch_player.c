
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "BspConfig.h"
#ifdef __SHELL_SWITCH_PLAYER_C__

#include "RKOS.h"
#include "BSP.h"


#include "shell_switch_player.h"
#include "AudioControlTask.h"



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/


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
_SWITCH_PLAYER_SHELL_
static SHELL_CMD ShellSwitchPlayerName[] =
{
    "sd",NULL,"NULL","NULL",
	/*"dlna",NULL,"NULL","NULL",
	"bt",NULL,"NULL","NULL",
	"airplay",NULL,"NULL","NULL",
    "delete",NULL,"NULL","NULL",*/
    "\b",NULL,"NULL","NULL",
};


_SWITCH_PLAYER_SHELL_
rk_err_t SwitchPlayer_shell(HDC dev, uint8 * pstr)
{

    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret;
    uint8 Space;
#if 1
    StrCnt = ShellItemExtract(pstr,&pItem, &Space);
    if (StrCnt == 0)
    {
        printf("\n StrCnt = 0 \n");
        return RK_ERROR;
    }
    ret = ShellCheckCmd(ShellSwitchPlayerName, pItem, StrCnt);
    if (ret < 0)
    {
        printf("\n ret < 0 \n");
        return RK_ERROR;
    }
    i = (uint32)ret;
    pItem += StrCnt;
    pItem++;         //remove '.',the point is the useful item

    printf("SwitchPlayer_shell i=%d\n",i);
    switch (i)
    {
        case 0x00:
			SwitchToPlayer(SOURCE_FROM_FILE_BROWSER);
            break;

        case 0x01:
			SwitchToPlayer(SOURCE_FROM_HTTP);
            break;

		case 0x02:
			SwitchToPlayer(SOURCE_FROM_BT);
			break;

        case 0x03:
			SwitchToPlayer(SOURCE_FROM_XXX);
            break;
        default:
            ret = RK_ERROR;
            break;
    }
    ret = RK_SUCCESS;
#endif
    return RK_SUCCESS;
}

#endif


