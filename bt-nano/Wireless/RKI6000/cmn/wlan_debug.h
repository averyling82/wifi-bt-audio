#ifndef __WLAN_DEBUG_H__
#define __WLAN_DEBUG_H__

#define WLAN_MODULE_NAME "WLAN_DRV"

#define _drv_err_           1
#define _drv_warning_       2
#define _drv_info_          3
#define _drv_dump_          4
#define _drv_debug_         5

#define     WLAN_DBG_XMIT       BIT(0)
#define     WLAN_DBG_RECV       BIT(1)
#define     WLAN_DBG_MLME       BIT(2)
#define     WLAN_DBG_WPAS       BIT(3)
#define     WLAN_DBG_WORK       BIT(4)
#define     WLAN_DBG_IO             BIT(5)
#define     WLAN_DBG_SEC            BIT(6)
#define   WLAN_DBG_AP           BIT(7)
#define     WLAN_DBG_INIT           BIT(8)
#define     WLAN_DBG_IOCTL      BIT(9)
#define     WLAN_DBG_PWR            BIT(10)
#define     WLAN_DBG_INT            BIT(11)
#define     WLAN_DBG_OSAL       BIT(12)
#define     WLAN_DBG_MP         BIT(13)
#define   WLAN_DBG_DCM          BIT(14)
#define WLAN_DBG_HAL            BIT(15)

#define dbgdump(...)
#define DBG_PRINT(_Comp, _Level, Fmt) do{}while(0)
#define WLAN_ASSERT(_Exp,Fmt) do{}while(0)
#define _func_enter_ {}
#define _func_exit_ {}
#define _func_line_ {}

extern uint32 GlobalDebugLevel;
extern uint32 GlobalDebugComponents;

#ifdef CONFIG_DEBUG_SUPPORT

#undef dbgdump
#define dbgdump(...)     do {\
    DEBUG_PRINT(__VA_ARGS__);\
}while(0)

#undef DBG_PRINT
#define DBG_PRINT(_Comp, _Level, Fmt)\
do {\
    if((_Comp & GlobalDebugComponents) && (_Level <= GlobalDebugLevel)) {\
        dbgdump Fmt;                        \
    }\
}while(0)

#undef WLAN_ASSERT
#define WLAN_ASSERT(_Exp,Fmt)               \
do {                                    \
    if(!(_Exp))                         \
    {                                   \
        dbgdump("%s: ", WLAN_MODULE_NAME);          \
        dbgdump Fmt;                    \
    }                                   \
} while (0)

#undef  _func_enter_
#define _func_enter_ \
do {    \
    if (GlobalDebugLevel >= _drv_debug_) \
    {                                                                   \
        dbgdump("\n%s()\n", __WLAN_FUNC__);\
    }       \
} while(0)

#undef  _func_exit_
#define _func_exit_ \
do {    \
    if (GlobalDebugLevel >= _drv_debug_) \
    {                                                                   \
        dbgdump("\n%s() exit\n", __WLAN_FUNC__); \
    }   \
} while(0)

#undef  _func_line_
#define _func_line_ \
do {    \
    if (GlobalDebugLevel >= _drv_debug_) \
    {                                                            \
        dbgdump("\n%s() line:%d\n", __WLAN_FUNC__, __LINE__); \
    }   \
} while(0)

#endif /* CONFIG_DEBUG_SUPPORT */

#define DUMP_RX_PKT     0
#define DUMP_TX_PKT     0
#define DUMP_SEC_KEY        1
#define DUMP_PSK_EAPOL      0
#define DUMP_WPA_IE     0

//#define DUMP_DBG          1

#endif

