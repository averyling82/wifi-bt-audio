#include "bt_config.h"

#ifdef  _BLUETOOTH_

#include "BlueToothControl.h"
#include "BlueToothSysParam.h"
#include "btHwControl.h"
#include "btAP6212Control.h"
#include "rk_bt_pbuf.h"
#include "rk_bt_Api.h"
#include "lwbtdebug.h"
#include "lwbtopts.h"

#if (BT_CHIP_CONFIG == BT_CHIP_AP6212)

/*-----------------------------------------------------------------------------------*/
int32 AP6212HwReset(void);
int32 AP6212HwInit(void);
int32 AP6212HwDownInitScript(void);
int32 AP6212HwSetChipBaudrate(uint baudrate);
int32 AP6212HwSetDevMac(struct bd_addr *bdaddr);
static int32 AP6212SetMacAddr(void);
static int32 AP6212SetDataUart(void);

#define CMD_HEAD_LEN 4

typedef struct
{
    int(*power_event_notify)(POWER_NOTIFICATION_t event);
    pSemaphore cmd_complete_sem;
    uint8 cmd_complete_flag;
    uint8 AP6212SleepConfigStatus;
    uint8 PowerStatus;
    uint8 needChangeMac;
    struct bd_addr bdaddr;

} _ap6212Pcb_;

_ATTR_BLUETOOTHCONTROL_DATA_
_ap6212Pcb_ ap6212pcb;

_ATTR_BLUETOOTHCONTROL_DATA_
uint8 buffer[1024];

_ATTR_BLUETOOTHCONTROL_CODE_
int ap6212_init_complete(void *arg, struct hci_pcb *pcb, uint8 ogf,
                           uint8 ocf, uint8 result)
{
    rkos_semaphore_give(ap6212pcb.cmd_complete_sem);
    if(result == 0x00) //HCI_SUCCESS
    {
        return 0;
    }

    return 1;
}

_ATTR_BLUETOOTHCONTROL_CODE_
int wait_ap6212_cmd_complete(uint timeoutms)
{
    if(rkos_semaphore_take(ap6212pcb.cmd_complete_sem, timeoutms/10) ==  RK_SUCCESS)
    {
        return 0;
    }
    return TIMEOUT;
}

_ATTR_BLUETOOTHCONTROL_CODE_
int32 ap6212_power_up(void)
{
    uint32 timeout = 400;
    BT_DEBUG("POER UP");

#if BT_VCC_ON_GPIO_CH != 0xFF
    Grf_GpioMuxSet(BT_VCC_ON_GPIO_CH, BT_VCC_ON_GPIO_PIN,0);
    Gpio_SetPinDirection(BT_VCC_ON_GPIO_CH, BT_VCC_ON_GPIO_PIN, GPIO_OUT);
    Gpio_SetPinLevel(BT_VCC_ON_GPIO_CH,BT_VCC_ON_GPIO_PIN,GPIO_LOW);
    rkos_delay(10);
    Gpio_SetPinLevel(BT_VCC_ON_GPIO_CH,BT_VCC_ON_GPIO_PIN,GPIO_HIGH);

    rkos_delay(300);
#endif

    Grf_GpioMuxSet(BT_POWER_GPIO_CH, BT_POWER_GPIO_PIN,0);
    Gpio_SetPinDirection(BT_POWER_GPIO_CH, BT_POWER_GPIO_PIN, GPIO_OUT);
    Gpio_SetPinLevel(BT_POWER_GPIO_CH,BT_POWER_GPIO_PIN,GPIO_LOW);
    rkos_delay(10);
    Gpio_SetPinLevel(BT_POWER_GPIO_CH,BT_POWER_GPIO_PIN,GPIO_HIGH);

    return RETURN_OK;
}

_ATTR_BLUETOOTHCONTROL_CODE_
int32  ap6212_power_down(void)
{
    uint32 timeout = 20;

    Gpio_SetPinLevel(BT_POWER_GPIO_CH, BT_POWER_GPIO_PIN ,GPIO_LOW);

    rkos_delay(1100);
#if BT_VCC_ON_GPIO_CH != 0xFF
    Gpio_SetPinLevel(BT_VCC_ON_GPIO_CH,BT_VCC_ON_GPIO_PIN,GPIO_LOW);
#endif

    Gpio_SetPinDirection(BT_POWER_GPIO_CH, BT_POWER_GPIO_PIN, GPIO_IN);
#if BT_VCC_ON_GPIO_CH != 0xFF
    rkos_delay(100);
    Gpio_SetPinDirection(BT_VCC_ON_GPIO_CH, BT_VCC_ON_GPIO_PIN, GPIO_IN);
#endif
    return RETURN_FAIL;
}


_ATTR_BLUETOOTHCONTROL_CODE_
void ap6212_set_host_rts_leave(eGPIOPinLevel_t level)
{
    Gpio_SetPinLevel(BT_HOST_RTS_CH, BT_HOST_RTS_PIN,level);
}


_ATTR_BLUETOOTHCONTROL_CODE_
void ap6212_send_go_to_sleep_ack(void)
{

}

_ATTR_BLUETOOTHCONTROL_CODE_
void ap6212_send_wake_up_ind(void)
{

}

_ATTR_BLUETOOTHCONTROL_CODE_
void ap6212_send_wake_up_ack(void)
{

}

_ATTR_BLUETOOTHCONTROL_CODE_
int32 AP6212HwPowerOn(void * config)
{
    int ret = RETURN_OK;
    int retry = 3;
    HDC uart_handle;

    UART_DEV_ARG stUartArg;
    stUartArg.dwBitWidth = UART_DATA_8B;
    stUartArg.dwBitRate = UART_BR_115200;
    stUartArg.Channel = BT_UART_CH;
    stUartArg.stopbit = UART_ONE_STOPBIT;
    stUartArg.parity = UART_PARITY_DISABLE;
    BT_DEBUG("create uart start= %d\n",rkos_GetFreeHeapSize());
    RKDev_Create(DEV_CLASS_UART, BT_HCI_UART_ID, &stUartArg);
    uart_handle = RKDev_Open(DEV_CLASS_UART, BT_HCI_UART_ID, NOT_CARE);
    BT_DEBUG("create uart stop= %d\n",rkos_GetFreeHeapSize());
    BT_DEBUG("uart_handle = 0x%04x\n", uart_handle);

    ap6212pcb.cmd_complete_sem = rkos_semaphore_create(1, 0);
    if(uart_handle == NULL)
    {
        return RETURN_FAIL;
    }
    ap6212_hw_control.user_data = uart_handle;

    if(ap6212_power_up() == RETURN_FAIL)
    {
        return RETURN_FAIL;
    }
    rkos_delay(50);

    BT_DEBUG("phybusif_init start= %d\n",rkos_GetFreeHeapSize());
    if(phybusif_init(&ap6212_hw_control))
    {
        BT_DEBUG("\n H4/H5 init fail,check uart connect\n");
        //BT_DEBUG("phybusif_init stop= %d\n",rkos_GetFreeHeapSize());
        rkos_semaphore_delete(ap6212pcb.cmd_complete_sem);
        ap6212pcb.cmd_complete_sem = 0;
        RKDev_Close(ap6212_hw_control.user_data);
        RKDev_Delete(DEV_CLASS_UART, BT_HCI_UART_ID, NULL);
        //BT_DEBUG("phybusif_init fail= %d\n",rkos_GetFreeHeapSize());
        return RETURN_FAIL;
    }
    bt_init(0);

    //ap6212_set_host_rts_leave(GPIO_LOW);
    BT_DEBUG();
power_up:
    ret = AP6212HwDownInitScript();
    if(ret != RETURN_OK)
    {
        while(1);//for test
        if(retry--)
        {
            ap6212_power_down();
            rkos_delay(10);
            ap6212_power_up();
            goto power_up;
        }
        else
        {
            return ret;
        }
    }
    BT_DEBUG();

    AP6212Change_BaudRate();
    ret = wait_ap6212_cmd_complete(200);
    if(ret == TIMEOUT)
    {
        BT_DEBUG("change baud TIMEOUT");
        return TIMEOUT;
    }
    else
    {
        rkos_delay(300);
        UartDev_SetBaudRate(ap6212_hw_control.user_data,1500000);
        BT_DEBUG("change baud ok");
    }

    AP6212SetMacAddr();
    ret = wait_ap6212_cmd_complete(50);
    if(ret == TIMEOUT)
    {
        BT_DEBUG("set mac TIMEOUT");
        return TIMEOUT;
    }
    BT_DEBUG("set mac OK");

    AP6212SetDataUart();
    ret = wait_ap6212_cmd_complete(50);
    if(ret == TIMEOUT)
    {
        BT_DEBUG("set data uart TIMEOUT");
        return TIMEOUT;
    }
    BT_DEBUG("set data uart OK");

    hci_cmd_complete(NULL);
    ap6212pcb.PowerStatus = POWER_STATUS_ON;
    return ret;

}

_ATTR_BLUETOOTHCONTROL_CODE_
int32 AP6212HwPowerOff(void * config)
{
    ap6212_power_down();
    ap6212pcb.PowerStatus == POWER_STATUS_OFF;
    phybusif_deinit();
#ifdef HCI_DATA_WATCH
    PMU_Enable();
#endif
    rkos_semaphore_delete(ap6212pcb.cmd_complete_sem);
    ap6212pcb.cmd_complete_sem = 0;

    RKDev_Close(ap6212_hw_control.user_data);
    RKDev_Delete(DEV_CLASS_UART, BT_HCI_UART_ID, NULL);
}


_ATTR_BLUETOOTHCONTROL_CODE_
int32 AP6212HwReset(void)
{
    hci_reset();
}


_ATTR_BLUETOOTHCONTROL_CODE_
int32 AP6212HwInit(void)
{
    memset(&ap6212pcb, 0, sizeof(_ap6212Pcb_));

    return RETURN_OK;

}

_ATTR_BLUETOOTHCONTROL_CODE_
int AP6212Change_BaudRate()
{
    struct pbuf *p;

    uint8 patchcode[6] = {0x00, 0x00, 0x60, 0xe3, 0x16, 0x00};
    if((p = pbuf_alloc(PBUF_RAW, 10, PBUF_RAM)) == NULL)
    {
        LWIP_DEBUGF(HCI_DEBUG, ("hci_download_patchcode: Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }

    /* Assembling command packet */
    ((uint8 *)p->payload)[0] = 0x01;
    ((uint8 *)p->payload)[1] = 0x18;
    ((uint8 *)p->payload)[2] = 0xfc;

    /// Set data struct.
    ((uint8 *)p->payload)[3] = 0x06;//length

    memcpy((uint8 *)p->payload + 4, patchcode, 6);

    /* Assembling cmd prameters */
    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return 1;
}

_ATTR_BLUETOOTHCONTROL_CODE_
void AP6212Download_patchcode(uint8* patchcode,int patch_len)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, patch_len, PBUF_RAM)) == NULL)
    {
        return;
    }

    if (NULL != patchcode)
    {
        memcpy(p->payload, patchcode, patch_len);
    }

    phybusif_output(p, p->tot_len);
    pbuf_free(p);
}

_ATTR_BLUETOOTHCONTROL_CODE_
void AP6212Download_patchcode_cmd()
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, 4, PBUF_RAM)) == NULL)
    {
        LWIP_DEBUGF(HCI_DEBUG, ("hci_download_patchcode: Could not allocate memory for pbuf\n"));
        return;
    }

    /* Assembling command packet */
    ((uint8 *)p->payload)[0] = 0x01;
    ((uint8 *)p->payload)[1] = 0x2e;
    ((uint8 *)p->payload)[2] = 0xfc;
    ((uint8 *)p->payload)[3] = 0;

    phybusif_output(p, p->tot_len);
    pbuf_free(p);
}


_ATTR_BLUETOOTHCONTROL_CODE_
static void Set_BDAdress(uint8* des,uint8* bd_add,uint8 *cur_len)
{
    int i;
    int len,pos;
    uint16 data_len = 0;

    if( des == NULL|| bd_add == NULL)
        return;

    pos = -1;
    len = *cur_len;

    for(i = 0; i < len; i++)
    {
        //config file's signature 0x55AB2387
        if( des[i] == 0x55
            && des[i+1] == 0xAB
            && des[i+2] == 0x23
            && des[i+3] == 0x87
          )
        {
            pos = i;        //config file is in last loadbuf data.
            break;
        }
    }

    if( pos == -1 ) //means not found config file start signature in this buffer .
    {
        DEBUG("Not found config file signature,need to get more data.");
    }
    else    //we get the config file's position in last load buffer data.
    {
        data_len = (des[pos + 4] | des[pos + 5] << 8);

        data_len += 9;
        des[pos + 4] = data_len &  0x00ff;
        des[pos + 5] = data_len >> 8 & 0xff;

        if(len + 9 < 252 ) //not exceed the patch code maximum data size
        {
            des[len]     = 0x3C;    //offset of BT Efuse.
            des[len + 1] = 0x00;
            des[len + 2] = 0x06;    //length of BDAdrr.
            memcpy(&des[len + 3],bd_add,sizeof(bd_add));

            *cur_len = len + 9;     //add 9 bytes bdaddr data not
        }
    }

}

_ATTR_BLUETOOTHCONTROL_CODE_
int32 AP6212HwDownInitScript(void)
{
    int offset = 0,i = 0;
    int ret;
    unsigned char *data = NULL;
    uint32 loadAddr;
    uint8 iCurLen = 0;
    uint32 patchcodeLen;
    SEGMENT_INFO_T stCodeInfo;

    ret = FW_GetSegmentInfo(SEGMENT_ID_BT_FIRMAVE, &stCodeInfo);
    loadAddr = stCodeInfo.CodeLoadBase;
    patchcodeLen = stCodeInfo.CodeImageLength;
    BT_DEBUG("loadAddr = 0x%04x, patchcodeLen = %d", loadAddr,patchcodeLen);

    hci_cmd_complete(ap6212_init_complete);
    UartDev_SetFlowControl(ap6212_hw_control.user_data);

    AP6212HwReset();
    ret = wait_ap6212_cmd_complete(200);
    if(ret == TIMEOUT)
    {
        BT_DEBUG("hci reset TIMEOUT");
        return TIMEOUT;
    }
    BT_DEBUG("hci reset ok");

    AP6212Change_BaudRate();
    ret = wait_ap6212_cmd_complete(200);
    if(ret == TIMEOUT)
    {
        BT_DEBUG("change baud TIMEOUT");
        return TIMEOUT;
    }
    else
    {
        rkos_delay(300);
        UartDev_SetBaudRate(ap6212_hw_control.user_data,1500000);
        BT_DEBUG("change baud ok");
    }


    AP6212Download_patchcode_cmd();
    ret = wait_ap6212_cmd_complete(200);
    if(ret == TIMEOUT)
    {
        BT_DEBUG("download patch code command TIMEOUT");
        return TIMEOUT;
    }
    BT_DEBUG("download patch code command ok");
    rkos_delay(200);
    while ((offset + 3 < patchcodeLen) && (FW_ReadFirmwaveByByte(loadAddr + offset, &buffer[1], 3) == RK_SUCCESS))
    {
        offset += 3;
        buffer[0] = 0x01;
        iCurLen = buffer[3];
        if (FW_ReadFirmwaveByByte(loadAddr + offset, &buffer[4], iCurLen) != RK_SUCCESS)
            return RETURN_FAIL;
        AP6212Download_patchcode(buffer, iCurLen + 4);
        ret = wait_ap6212_cmd_complete(2000);
        if(ret == TIMEOUT)
        {
            BT_DEBUG("download patch code TIMEOUT");
            return TIMEOUT;
        }
        offset += iCurLen;
    }
    BT_DEBUG("down script complete");
    rkos_delay(200);
    UartDev_SetBaudRate(ap6212_hw_control.user_data,115200);

    AP6212HwReset();
    ret = wait_ap6212_cmd_complete(200);
    if(ret == TIMEOUT)
    {
        BT_DEBUG("hci reset TIMEOUT");
        return TIMEOUT;
    }
    BT_DEBUG("hci reset ok");
    //hci_cmd_complete(NULL);
    return RETURN_OK;
}

_ATTR_BLUETOOTHCONTROL_CODE_
int32 AP6212HwWakeup(void * config)
{
    ap6212_set_host_rts_leave(GPIO_LOW);
    ap6212_send_wake_up_ind();

    return RETURN_OK;
}

_ATTR_BLUETOOTHCONTROL_CODE_
int32 AP6212HwGetPowerStatus(void)
{
    return ap6212pcb.PowerStatus;
}


_ATTR_BLUETOOTHCONTROL_CODE_
int32 AP6212HwSetChipBaudrate(uint baudrate)
{
    return RETURN_OK;
}

_ATTR_BLUETOOTHCONTROL_CODE_
static int32 AP6212SetMacAddr(void)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, 10, PBUF_RAM)) == NULL)
    {
        LWIP_DEBUGF(HCI_DEBUG, ("hci_download_patchcode: Could not allocate memory for pbuf\n"));
        return RETURN_FAIL;
    }

    /* Assembling command packet */
    ((uint8 *)p->payload)[0] = 0x01;
    ((uint8 *)p->payload)[1] = 0x01;
    ((uint8 *)p->payload)[2] = 0xfc;
    ((uint8 *)p->payload)[3] = 0x06;

    memcpy(((uint8 *)p->payload) + 4, ap6212pcb.bdaddr.addr, 6);

    phybusif_output(p, p->tot_len);
    pbuf_free(p);
    return RETURN_OK;
}

_ATTR_BLUETOOTHCONTROL_CODE_
static int32 AP6212SetDataUart(void)
{
    struct pbuf *p;

    if((p = pbuf_alloc(PBUF_RAW, 9, PBUF_RAM)) == NULL)
    {
        LWIP_DEBUGF(HCI_DEBUG, ("AP6212SetDataUart: Could not allocate memory for pbuf\n"));
        return RETURN_FAIL;
    }

    /* Assembling command packet */
    ((uint8 *)p->payload)[0] = 0x01;
    ((uint8 *)p->payload)[1] = 0x1c;
    ((uint8 *)p->payload)[2] = 0xfc;
    ((uint8 *)p->payload)[3] = 0x05;
    ((uint8 *)p->payload)[4] = 0x01;
    ((uint8 *)p->payload)[5] = 0x00;
    ((uint8 *)p->payload)[6] = 0x00;
    ((uint8 *)p->payload)[7] = 0x00;
    ((uint8 *)p->payload)[8] = 0x00;

   // memcpy(((uint8 *)p->payload) + 4, ap6212pcb.bdaddr.addr, 6);

    phybusif_output(p, p->tot_len);
    pbuf_free(p);
    return RETURN_OK;
}


_ATTR_BLUETOOTHCONTROL_CODE_
int32 AP6212HwSetDevMac(struct bd_addr *bdaddr)
{

    ap6212pcb.needChangeMac = TRUE;
    memcpy(ap6212pcb.bdaddr.addr,bdaddr->addr, 6);
    return RETURN_OK;
}


_ATTR_BLUETOOTHCONTROL_CODE_
int32 AP6212HwRegisterPowerNotification(int(*func)(POWER_NOTIFICATION_t event))
{
    ap6212pcb.power_event_notify = func;
    return RETURN_OK;
}


_ATTR_BLUETOOTHCONTROL_CODE_
int32 AP6212HwRegisterHwError(int(*func)(POWER_NOTIFICATION_t event))
{
    return RETURN_OK;
}

_ATTR_BLUETOOTHCONTROL_CODE_
int32 AP6212HwSetUartRecAbility(UART_ABILITY_t Ability)
{
    if(Ability == UART_ABILITY_RECV_ENABLE)
    {
        ap6212_set_host_rts_leave(GPIO_LOW);
    }
    else if(Ability == UART_ABILITY_RECV_DISABLE)
    {
        ap6212_set_host_rts_leave(GPIO_HIGH);
    }

    return RETURN_OK;
}

_ATTR_BLUETOOTHCONTROL_CODE_
int32 AP6212HwService()
{
#ifdef ENABLE_DEEP_SLEEP
    if(TRUE == GetMsg(MSG_BLUETOOTH_HCILL_WAKE_UP_ACK))
    {
        Gpio_DisableInt(GPIO_HOST_CTS);
        ap6212pcb.PowerStatus = POWER_STATUS_ON;

        if(ap6212pcb.power_event_notify)
        {
            ap6212pcb.power_event_notify(POWER_WILL_WAKE_UP_CFM);
        }
    }

    if(TRUE == GetMsg(MSG_BLUETOOTH_GET_WAKE_UP_INT))
    {
        ap6212_set_host_rts_leave(GPIO_LOW);
    }

    if(TRUE == GetMsg(MSG_BLUETOOTH_HCILL_SLEEP_IND))
    {
        ap6212_set_host_rts_leave(GPIO_HIGH);
        ap6212_send_go_to_sleep_ack();

        Gpio_EnableInt(GPIO_HOST_CTS);
        ap6212pcb.PowerStatus = POWER_STATUS_SLEEP;

        if(ap6212pcb.power_event_notify)
        {
            ap6212pcb.power_event_notify(POWER_WILL_SLEEP);
        }

    }

    if(TRUE == GetMsg(MSG_BLUETOOTH_HCILL_WAKE_UP_IND))
    {
        ap6212_send_wake_up_ack();
        Gpio_DisableInt(GPIO_HOST_CTS);
        ap6212pcb.PowerStatus = POWER_STATUS_ON;

        if(ap6212pcb.power_event_notify)
        {
            ap6212pcb.power_event_notify(POWER_WILL_WAKE_UP_IND);
        }
    }
#endif

    return RETURN_OK;
}

_ATTR_BLUETOOTHCONTROL_CODE_
int32 AP6212HwVendorRecv(uint8 *data ,uint len)
{
    uint8 c;

    c = *data;

    switch(c)
    {
        default:
            break;
    }
    return RETURN_OK;
}


_ATTR_BLUETOOTHCONTROL_CODE_
int32 AP6212HciDataRead(uint8 *data ,uint len)
{
    //return UartDev_Read(ap6212_hw_control.user_data, data, len, MAX_DELAY);
    return UartDev_Read(ap6212_hw_control.user_data, data, len, 3);
}

_ATTR_BLUETOOTHCONTROL_CODE_
int32 AP6212HciDataWrite(uint8 *data ,uint len)
{
    return UartDev_Write(ap6212_hw_control.user_data, data, len, SYNC_MODE, NULL);
}

_ATTR_BLUETOOTHCONTROL_DATA_
bt_hw_control_t ap6212_hw_control =
{
    AP6212HwInit,
    AP6212HwPowerOn,
    AP6212HwPowerOff,
    NULL,
    NULL,
    NULL,
    NULL,
    AP6212HwGetPowerStatus,
    NULL,
    AP6212HwRegisterPowerNotification,
    AP6212HwSetUartRecAbility,
    AP6212HwSetDevMac,
    NULL,
    NULL,
    AP6212HciDataWrite,
    AP6212HciDataRead,
    NULL,

};

#endif    //(BT_CHIP_CONFIG == BT_CHIP_AP6212)


#endif
