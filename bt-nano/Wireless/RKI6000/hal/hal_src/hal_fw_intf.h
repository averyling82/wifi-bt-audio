#ifndef __HAL_FW_INTF_H__
#define __HAL_FW_INTF_H__


//#define SET_CMD_XXX(__pCmd, __Value)          SET_BITS_TO_LE_1BYTE(__pCmd, 0, 8, __Value)
//#define GET_CMD_XXX(__pCmd)                   LE_BITS_TO_1BYTE(__pCmd, 0, 8)

/* ARP pkt */
#define eqMacAddr(a,b)                      ( ((a)[0]==(b)[0] && (a)[1]==(b)[1] && (a)[2]==(b)[2] && (a)[3]==(b)[3] && (a)[4]==(b)[4] && (a)[5]==(b)[5]) ? 1:0 )
#define cpMacAddr(des,src)                  ((des)[0]=(src)[0],(des)[1]=(src)[1],(des)[2]=(src)[2],(des)[3]=(src)[3],(des)[4]=(src)[4],(des)[5]=(src)[5])
#define cpIpAddr(des,src)                   ((des)[0]=(src)[0],(des)[1]=(src)[1],(des)[2]=(src)[2],(des)[3]=(src)[3])

#define SET_ARP_PKT_HW(__pHeader, __Value)                      WriteEF2Byte( ((uint8*)(__pHeader)) + 0, __Value)
#define SET_ARP_PKT_PROTOCOL(__pHeader, __Value)                WriteEF2Byte( ((uint8*)(__pHeader)) + 2, __Value)
#define SET_ARP_PKT_HW_ADDR_LEN(__pHeader, __Value)             WriteEF1Byte( ((uint8*)(__pHeader)) + 4, __Value)
#define SET_ARP_PKT_PROTOCOL_ADDR_LEN(__pHeader, __Value)   WriteEF1Byte( ((uint8*)(__pHeader)) + 5, __Value)
#define SET_ARP_PKT_OPERATION(__pHeader, __Value)           WriteEF2Byte( ((uint8*)(__pHeader)) + 6, __Value)
#define SET_ARP_PKT_SENDER_MAC_ADDR(__pHeader, _val)        cpMacAddr(((uint8*)(__pHeader))+8, (uint8*)(_val))
#define SET_ARP_PKT_SENDER_IP_ADDR(__pHeader, _val)             cpIpAddr(((uint8*)(__pHeader))+14, (uint8*)(_val))
#define SET_ARP_PKT_TARGET_MAC_ADDR(__pHeader, _val)        cpMacAddr(((uint8*)(__pHeader))+18, (uint8*)(_val))
#define SET_ARP_PKT_TARGET_IP_ADDR(__pHeader, _val)             cpIpAddr(((uint8*)(__pHeader))+24, (uint8*)(_val))


#define PageNum_128(_Len)       (uint32)(((_Len)>>7) + ((_Len)&0x7F ? 1:0))

typedef enum _HOST_CMD_
{
    //class 0: common
    CMD_DEBUG_COMMAND               = 0x00,
    CMD_MEDIA_STATUS_REPORT     = 0x01,
    CMD_DOWNLOAD_PAGE_NOTIFY        = 0x02,
    CMD_KEEP_ALIVE                      = 0x03,
    CMD_DISCONNECT_DETECT           = 0x04,
    CMD_SET_CHANNEL             = 0x05,

    //class 1: power save
    CMD_POWER_SAVE_CTRL             = 0x10,
    CMD_POWER_SAVE_PARAM_1      = 0x11,
    CMD_POWER_SAVE_PARAM_2      = 0x12,

    //class 2: dynamic machenism

    //class 3: wowlan
    CMD_WOWLAN_PARAM                = 0x30,
    CMD_WOWLAN_CTRL             = 0x31,
    CMD_SECURITY_TYPE_NOTIFY        = 0x32,
    CMD_DOWNLOAD_PAGE_NOTIFY_2  = 0x33,
    CMD_IP_INFO_NOTIFY_1            = 0x34,
    CMD_IP_INFO_NOTIFY_2            = 0x35,

} HOST_CMD, *PHOST_CMD;

typedef enum _FW_STATUS_ID
{
    STATUS_COMMAND_CONFIRM                      = 0x00,
    WAKEUP_PACKET_RX_INDICATE           = 0x01,

} FW_STATUS_ID, *PFW_STATUS_ID;

void
fill_and_issue_host_cmd(
    PADAPTER    Adapter,
    uint8   ElementID,
    uint32  CmdLen,
    uint8 * pCmdBuffer);

#ifdef CONFIG_WOWLAN_SUPPORT
void    hal_wowlan_ctrl(PADAPTER    Adapter, uint32 en_wowlan);
void    hal_wowlan_test(PADAPTER    Adapter);
#endif

#ifdef CONFIG_LPS_SUPPORT
void    hal_lps_ctrl(PADAPTER   Adapter, uint8 en_lps);
#endif

void
hal_keep_alive_ctrl(
    IN  PADAPTER    pAdapter,
    IN  boolean bFuncEn
);

void
hal_wlan_info_notify(
    IN PADAPTER Adapter,
    IN uint8    mstatus
);

#endif
