/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: rk-iot\Web\Lwip\utilities\ip_shell.c
* Owner: ipconfig.h
* Date: 2015.11.24
* Time: 15:38:28
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    yangh     2015.11.24     15:38:28   1.0
********************************************************************************************
*/


#include "BspConfig.h"
//#ifdef __IP_SHELL__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "../lwip-1.4.1/src/include/lwip/netif.h"
#include "../lwip-1.4.1/src/include/lwip/sockets.h"
#include "../lwip-1.4.1/src/include/ipv4/lwip/ip.h"
#include "dns.h"

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
rk_err_t IP_config_display(HDC dev, uint8 * pstr);


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
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



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/




static SHELL_CMD ShellIPConfigName[] =
{
    "config",IP_config_display,"display local ip information","ip.config",
    "ping",Ping_Shell,"ping remote ip","ip.ping [ip addr]",
    "conn",NULL,"NULL","NULL",
    "\b",NULL,"NULL","NULL",
};

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

rk_err_t IP_config_display(HDC dev, uint8 * pstr)
{
    struct netif *default_netif = NULL;
    ip_addr_t ip_addr = {0}, netmask = {0}, gw = {0};
    int i = 0;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    default_netif = netif_get_default();

    if (default_netif != NULL)
    {
        ip_addr = default_netif->ip_addr;
        netmask = default_netif->netmask;
        gw = default_netif->gw;

        rk_printf("\r\n");
        rk_printf("defalut Ethernet Adapter %s \r\n", default_netif->name);
        rk_printf("***************************************************************************\r\n");
        rk_printf(" IP address        : %s\r\n", ipaddr_ntoa(&ip_addr));
        rk_printf(" Netmask           : %s\r\n", ipaddr_ntoa(&netmask));
        rk_printf(" Default Gateway   : %s\r\n", ipaddr_ntoa(&gw));
        for(i = 0; i < DNS_MAX_SERVERS; i++)
        {
            ip_addr = dns_getserver(i);
            rk_printf(" dns sever%d        : %s\r\n", i, ipaddr_ntoa(&ip_addr));
        }

        rk_printf(" MAC Address       ");
        for (i = 0; i < default_netif->hwaddr_len; ++i)
        {
            if(i == (default_netif->hwaddr_len - 1))
            {
                printf("%02X", default_netif->hwaddr[i]);
            }
            else
            {
               printf("%02X:", default_netif->hwaddr[i]);
            }
        }
        rk_printf("\r\n");
        rk_printf("***************************************************************************\r\n");
    }

    for(default_netif = netif_list; default_netif != NULL; default_netif = default_netif->next)
    {
        ip_addr = default_netif->ip_addr;
        netmask = default_netif->netmask;
        gw = default_netif->gw;

        rk_printf("\r\n");
        rk_printf("%x Ethernet Adapter %s \r\n", default_netif,default_netif->name);
        rk_printf("***************************************************************************\r\n");
        rk_printf(" IP address        : %s\r\n", ipaddr_ntoa(&ip_addr));
        rk_printf(" Netmask           : %s\r\n", ipaddr_ntoa(&netmask));
        rk_printf(" Default Gateway   : %s\r\n", ipaddr_ntoa(&gw));
        for(i = 0; i < DNS_MAX_SERVERS; i++)
        {
            ip_addr = dns_getserver(i);
            rk_printf(" dns sever%d        : %s\r\n", i, ipaddr_ntoa(&ip_addr));
        }

        rk_printf(" MAC Address       ");
        for (i = 0; i < default_netif->hwaddr_len; ++i)
        {
            if(i == (default_netif->hwaddr_len - 1))
            {
                printf("%02X", default_netif->hwaddr[i]);
            }
            else
            {
               printf("%02X:", default_netif->hwaddr[i]);
            }
        }
        rk_printf("\r\n");
        rk_printf("***************************************************************************\r\n");
    }

}

rk_err_t IP_config_shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret;
    uint8 Space;

    StrCnt = ShellItemExtract(pstr, &pItem, &Space);
    if (StrCnt == 0)
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellIPConfigName, pItem, StrCnt);
    if (ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;

    if((MainTask_GetStatus(MAINTASK_WIFI_OPEN_OK) != 1)
        &&(MainTask_GetStatus(MAINTASK_WIFI_AP_OPEN_OK) != 1))
    {
        rk_printf("wifi not open");
        return RK_SUCCESS;
    }

    ShellHelpDesDisplay(dev, ShellIPConfigName[i].CmdDes, pItem);
    if(ShellIPConfigName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellIPConfigName[i].ShellCmdParaseFun(dev, pItem);
    }
    return ret;
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



//#endif
