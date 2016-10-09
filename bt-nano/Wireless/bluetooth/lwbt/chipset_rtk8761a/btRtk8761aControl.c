/*
********************************************************************************************
*
*        Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Web\bluetooth\lwbt\chipset_rtk8761a\btRtk8761aControl.c
* Owner: wangping
* Date: 2015.10.15
* Time: 14:49:05
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    wangping     2015.10.15     14:49:05   1.0
********************************************************************************************
*/
#include "bt_config.h"



#ifdef  _BLUETOOTH_

#include "BlueToothControl.h"
#include "BlueToothSysParam.h"
#include "btHwControl.h"
#include "btRtk8761aControl.h"
//#include "lwbterr.h"
//#include "bt_pbuf.h"
#include "rk_bt_pbuf.h"
#include "rk_bt_Api.h"
//#include "bd_addr.h"


#if (BT_CHIP_CONFIG == BT_CHIP_RTL8761AT) || ((BT_CHIP_CONFIG == BT_CHIP_RTL8761ATV))

#define HCI_VENDER_DOWNLOAD_OCF     0x20    //mlc
#define PATCH_DATA_FIELD_MAX_SIZE     252


/*-----------------------------------------------------------------------------------*/
int32 Rtk8761aHwReset(void);
int32 Rtk8761aHwInit(void);
int32 Rtk8761aHwDownInitScript(void);
int32 Rtk8761aHwSetChipBaudrate(uint baudrate);

#define CMD_HEAD_LEN 4



typedef struct
{
    int(*power_event_notify)(POWER_NOTIFICATION_t event);
    pSemaphore cmd_complete_sem;
    uint8 cmd_complete_flag;
    uint8 Rtk8761aSleepConfigStatus;
    uint8 PowerStatus;
    uint8 needChangeMac;
    struct bd_addr bdaddr;

} _rtk8761aPcb_;

_ATTR_BLUETOOTHCONTROL_DATA_ _rtk8761aPcb_ rtk8761apcb;

#if 0
_ATTR_BLUETOOTHCONTROL_CODE_
int rtk8761a_init_complete(void *arg, struct hci_pcb *pcb, uint8 ogf,
                           uint8 ocf, uint8 result)
{
    rtk8761apcb.cmd_complete_flag = 1;
    if(result == 0x00) //HCI_SUCCESS
    {
        return 0;
    }

    return 1;
}

_ATTR_BLUETOOTHCONTROL_CODE_
int wait_rtk8761a_cmd_complete(uint timeoutms)
{
    timeoutms = timeoutms/10;
    while(--timeoutms)
    {
        if(rtk8761apcb.cmd_complete_flag)
        {
            rtk8761apcb.cmd_complete_flag = 0;
            return RETURN_OK;
        }

        rkos_delay(10);
    }

    return TIMEOUT;
}
#endif
_ATTR_BLUETOOTHCONTROL_CODE_
int rtk8761a_init_complete(void *arg, struct hci_pcb *pcb, uint8 ogf,
                           uint8 ocf, uint8 result)
{
    rkos_semaphore_give(rtk8761apcb.cmd_complete_sem);
    if(result == 0x00) //HCI_SUCCESS
    {
        return 0;
    }

    return 1;
}

_ATTR_BLUETOOTHCONTROL_CODE_
int wait_rtk8761a_cmd_complete(uint timeoutms)
{
    if(rkos_semaphore_take(rtk8761apcb.cmd_complete_sem, timeoutms/10) ==  RK_SUCCESS)
    {
        return 0;
    }
    return TIMEOUT;
}

_ATTR_BLUETOOTHCONTROL_CODE_
int32 rtk8761a_power_up(void)
{
    uint32 timeout = 400;

    //8761
#if BT_VCC_ON_GPIO_CH != 0xFF
    Grf_GpioMuxSet(BT_VCC_ON_GPIO_CH, BT_VCC_ON_GPIO_PIN,0);
    Gpio_SetPinDirection(BT_VCC_ON_GPIO_CH, BT_VCC_ON_GPIO_PIN, GPIO_OUT);
    Gpio_SetPinLevel(BT_VCC_ON_GPIO_CH,BT_VCC_ON_GPIO_PIN,GPIO_LOW);
#endif

    Grf_GpioMuxSet(BT_POWER_GPIO_CH, BT_POWER_GPIO_PIN,0);
    Gpio_SetPinDirection(BT_POWER_GPIO_CH, BT_POWER_GPIO_PIN, GPIO_OUT);

#if BT_VCC_ON_GPIO_CH != 0xFF
    Gpio_SetPinLevel(BT_VCC_ON_GPIO_CH,BT_VCC_ON_GPIO_PIN,GPIO_HIGH);
    rkos_delay(10);
#endif

    Gpio_SetPinLevel(BT_POWER_GPIO_CH,BT_POWER_GPIO_PIN,GPIO_LOW);
    rkos_delay(10);

    Gpio_SetPinLevel(BT_POWER_GPIO_CH,BT_POWER_GPIO_PIN,GPIO_HIGH);


    //Grf_GpioMuxSet(BT_POWER_GPIO_CH, BT_POWER_GPIO_PIN,0);
    //Gpio_SetPinDirection(BT_POWER_GPIO_CH, BT_POWER_GPIO_PIN, GPIO_OUT);
    //Gpio_SetPinLevel(BT_POWER_GPIO_CH,BT_POWER_GPIO_PIN,GPIO_LOW);

    rkos_delay(400);
    //DEBUG("rtk8761_power_up");

    while(timeout--)
    {
        rkos_delay(10);
        if (1 == Gpio_GetPinLevel(BT_HOST_RX_CH, BT_HOST_RX_PIN))
        {
            //SendMsg(MSG_BLUETOOTH_POWERON);
            return RETURN_OK;
        }
    }
    return RETURN_FAIL;
}

/*
--------------------------------------------------------------------------------
  Function name :  void BlueToothPowerOn(void )
  Author        :  zs
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                     zs            2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32  rtk8761a_power_down(void)
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
void rtk8761a_set_host_rts_leave(eGPIOPinLevel_t level)
{
    Gpio_SetPinLevel(BT_HOST_RTS_CH, BT_HOST_RTS_PIN,level);
}


_ATTR_BLUETOOTHCONTROL_CODE_
void rtk8761a_send_go_to_sleep_ack(void)
{

}

_ATTR_BLUETOOTHCONTROL_CODE_
void rtk8761a_send_wake_up_ind(void)
{

}

_ATTR_BLUETOOTHCONTROL_CODE_
void rtk8761a_send_wake_up_ack(void)
{

}


_ATTR_BLUETOOTHCONTROL_CODE_
void rtk8761a_wake_up_int()
{

}

/*
--------------------------------------------------------------------------------
  Function name :  int Rtk8761aHwPowerOn(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping        2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwPowerOn(void * config)
{
    int ret = RETURN_OK;
    int retry = 3;
//    Rtk8761aHwInit();
    HDC uart_handle;



#ifdef HCI_DATA_WATCH
    PMUSetARMFreq(59*2);
    //PMUSetARMFreq(140);
    PMU_Disable();

#endif
    UART_DEV_ARG stUartArg;
    stUartArg.dwBitWidth = UART_DATA_8B;
    stUartArg.dwBitRate = UART_BR_115200;
    stUartArg.Channel = BT_UART_CH;
    stUartArg.stopbit = UART_ONE_STOPBIT;
    stUartArg.parity = UART_EVEN_PARITY;
    BT_DEBUG("create uart start= %d\n",rkos_GetFreeHeapSize());
    RKDev_Create(DEV_CLASS_UART, BT_HCI_UART_ID, &stUartArg);
    uart_handle = RKDev_Open(DEV_CLASS_UART, BT_HCI_UART_ID, NOT_CARE);
    BT_DEBUG("create uart stop= %d\n",rkos_GetFreeHeapSize());
    BT_DEBUG("uart_handle = 0x%04x\n", uart_handle);

    rtk8761apcb.cmd_complete_sem = rkos_semaphore_create(1, 0);
    if(uart_handle == NULL)
    {
        return RETURN_FAIL;
    }
    rtk8761a_hw_control.user_data = uart_handle;
    if(rtk8761a_power_up() == RETURN_FAIL)
    {
        return RETURN_FAIL;
    }
    #if 1
    rkos_delay(50);
    //BT_DEBUG("phybusif_init start= %d\n",rkos_GetFreeHeapSize());
    if(phybusif_init(&rtk8761a_hw_control))
    {
        BT_DEBUG("\n H4/H5 init fail,check uart connect\n");
        //BT_DEBUG("phybusif_init stop= %d\n",rkos_GetFreeHeapSize());
        rkos_semaphore_delete(rtk8761apcb.cmd_complete_sem);
        rtk8761apcb.cmd_complete_sem = 0;
        RKDev_Close(rtk8761a_hw_control.user_data);
        RKDev_Delete(DEV_CLASS_UART, BT_HCI_UART_ID, NULL);
        //BT_DEBUG("phybusif_init fail= %d\n",rkos_GetFreeHeapSize());
        return RETURN_FAIL;
    }
    bt_init(0);
    #endif

    //rtk8761a_set_host_rts_leave(GPIO_LOW);

power_up:
    ret = Rtk8761aHwDownInitScript();
    if(ret != RETURN_OK)
    {
        while(1);//for test
        if(retry--)
        {

            rtk8761a_power_down();
            rkos_delay(10);
            rtk8761a_power_up();
            goto power_up;
        }
        else
        {
            return ret;
        }

    }
    BT_DEBUG();
    hci_cmd_complete(rtk8761a_init_complete);
    Rtk8761aHwReset();
    wait_rtk8761a_cmd_complete(50);
    rtk8761apcb.PowerStatus = POWER_STATUS_ON;

    return ret;

}


/*
--------------------------------------------------------------------------------
  Function name :  int Rtk8761aHwPowerOff(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping         2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwPowerOff(void * config)
{
    rtk8761a_power_down();
    rtk8761apcb.PowerStatus == POWER_STATUS_OFF;
    phybusif_deinit();
#ifdef HCI_DATA_WATCH
    PMU_Enable();
#endif
    rkos_semaphore_delete(rtk8761apcb.cmd_complete_sem);
    rtk8761apcb.cmd_complete_sem = 0;

    RKDev_Close(rtk8761a_hw_control.user_data);
    RKDev_Delete(DEV_CLASS_UART, BT_HCI_UART_ID, NULL);
}

/*
--------------------------------------------------------------------------------
  Function name :  int Rtk8761aHwReset(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping            2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwReset(void)
{
    hci_reset();
}

/*
--------------------------------------------------------------------------------
  Function name :  int Rtk8761aHwInit(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping            2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwInit(void)
{
    memset(&rtk8761apcb, 0, sizeof(_rtk8761aPcb_));

    return RETURN_OK;

}
/*******************************************************************************
 * Function :  Rtk8761aDownload_patchcode
 * Descr    :  download rtk8761a pathch code
 *
 * Input    :
 *
 * Output   :
 *
 * Return   : index
 *
 * Author   : MLC
 * Others   : 2014-2-25
 * Notes    : Download realtek patch code and config file together.
 *
 *******************************************************************************/
_ATTR_BLUETOOTHCONTROL_CODE_
int Rtk8761aChange_BaudRate()
{
    struct pbuf *p;

    uint8 patchcode_15M[4] = {0x02, 0x80, 0x92, 0x04};
    uint8 patchcode_9216[4] = {0x04, 0x50, 0xF7, 0x05};
    //uint8 patchcode[4] = {0x14, 0xC0, 0x52, 0x02};
    if((p = pbuf_alloc(PBUF_RAW, 8, PBUF_RAM)) == NULL)
    {
        //LWIP_DEBUGF(HCI_DEBUG, ("hci_download_patchcode: Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }

    /* Assembling command packet */
    ((uint8 *)p->payload)[0] = 0x01;
    ((uint8 *)p->payload)[1] = 0x17;
    ((uint8 *)p->payload)[2] = 0xfc;

    /// Set data struct.
    ((uint8 *)p->payload)[3] = 0x04;//length

#ifdef HCI_DATA_WATCH
    memcpy(&((uint8 *)p->payload)[4], patchcode_9216, 4);
#else
    memcpy(&((uint8 *)p->payload)[4], patchcode_15M, 4);
#endif

    /* Assembling cmd prameters */
    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return 1;
}
/*******************************************************************************
 * Function :  Rtk8761aDownload_patchcode
 * Descr    :  download rtk8761a pathch code
 *
 * Input    :
 *
 * Output   :
 *
 * Return   : index
 *
 * Author   : MLC
 * Others   : 2014-2-25
 * Notes    : Download realtek patch code and config file together.
 *
 *******************************************************************************/
_ATTR_BLUETOOTHCONTROL_CODE_
int Rtk8761aDownload_patchcode(uint8* patchcode,int patch_len, int index)
{
    struct pbuf *p;

    int cur_index = index;

    if((p = pbuf_alloc(PBUF_RAW, 3 + 2 + patch_len, PBUF_RAM)) == NULL)
    {
        //LWIP_DEBUGF(HCI_DEBUG, ("hci_download_patchcode: Could not allocate memory for pbuf\n"));
        return ERR_MEM;
    }

    /* Assembling command packet */
    ((uint8 *)p->payload)[0] = 0x01;
    ((uint8 *)p->payload)[1] = HCI_VENDER_DOWNLOAD_OCF;
    ((uint8 *)p->payload)[2] = 0xfc;

    /// Set data struct.
    ((uint8 *)p->payload)[3] = patch_len + 1;//add index
    ((uint8 *)p->payload)[4] = cur_index;

    if (NULL != patchcode)
    {
        memcpy(&((uint8 *)p->payload)[5], patchcode, patch_len);
    }
    /* Assembling cmd prameters */

    phybusif_output(p, p->tot_len);
    pbuf_free(p);

    return cur_index;
}


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

        if(len + 9 < PATCH_DATA_FIELD_MAX_SIZE ) //not exceed the patch code maximum data size
        {
            des[len]     = 0x3C;    //offset of BT Efuse.
            des[len + 1] = 0x00;
            des[len + 2] = 0x06;    //length of BDAdrr.
            memcpy(&des[len + 3],bd_add,sizeof(bd_add));

            *cur_len = len + 9;     //add 9 bytes bdaddr data not
        }
    }

}


/*
--------------------------------------------------------------------------------
  Function name :  int Rtk8761aHwDownInitScript(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping            2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
extern int  rtl_config_data;
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwDownInitScript(void)
{
    int offset = 0,i = 0;

    int downcnt;
    int ret;
    unsigned char *data = NULL;
    uint32 loadAddr;
    unsigned char loadbuf[252];
    int finish_flag = 0;
    uint8 iCurIndex = 0;
    uint8 iCurLen = 0;
    uint32 patchcodeLen;
    uint32 configFileLen = 0;   //size of config file
    int macSplitFlag =0; /*ÿ´ú±ímac·Ö²ð·ÖÔÚÁ½¸ö°üÖÐ*/
    int macleftcnt = 0;
    SEGMENT_INFO_T stCodeInfo;
    configFileLen = rtl_config_data;//sizeof( rtk_config_data );
    ret = FW_GetSegmentInfo(SEGMENT_ID_BT_FIRMAVE, &stCodeInfo);
    if (ret)
    {

    }
    loadAddr = stCodeInfo.CodeLoadBase;
    patchcodeLen = stCodeInfo.CodeImageLength;

    BT_DEBUG("configFileLen = %d, loadAddr = 0x%04x, patchcodeLen = %d\n", configFileLen,loadAddr,patchcodeLen);
    //ModuleOverlayLoadData(loadAddr + offset, (uint32)loadbuf, PATCH_DATA_FIELD_MAX_SIZE);
    FW_ReadFirmwaveByByte(loadAddr + offset, loadbuf, PATCH_DATA_FIELD_MAX_SIZE);
    data = loadbuf;
    downcnt = 0;

    //    if((data[0] != 0x68)
    //    &&(data[1] != 0x63)
    //    &&(data[2] != 0x65)
    //    &&(data[3] != 0x74)
    //    &&(data[4] != 0x6C)
    //    &&(data[5] != 0x61)
    //    &&(data[6] != 0x65)
    //    &&(data[7] != 0x52)
    //          )
    //    {
    // return;
    //    }

    hci_cmd_complete(rtk8761a_init_complete);
#if 1
    Rtk8761aChange_BaudRate();
    ret = wait_rtk8761a_cmd_complete(200);
    if(ret == TIMEOUT && !finish_flag)
    {
        return TIMEOUT;
    }
    else
    {

        rkos_delay(300);
#ifdef HCI_DATA_WATCH
        //UARTInit(59*2*1000*1000,921600,UART_DATA_8B,UART_ONE_STOPBIT,UART_PARITY_DISABLE);
        UartDev_SetBaudRate(rtk8761a_hw_control.user_data, 59*2*1000*1000);
#else
        //UARTInit(24*1000*1000,1500000,UART_DATA_8B,UART_ONE_STOPBIT,UART_PARITY_DISABLE);
        UartDev_SetBaudRate(rtk8761a_hw_control.user_data,1500000);
#endif
        BT_DEBUG("change baud ok\n");
    }
#endif
    while(downcnt  < patchcodeLen)
    {
        if(downcnt + PATCH_DATA_FIELD_MAX_SIZE >  patchcodeLen)

        {
            iCurIndex = iCurIndex | 0x80;
            //iCurLen = patchcodeLen - downcnt - 1;   //last packet length
            iCurLen = patchcodeLen - downcnt;   //last packet length
#ifdef BT_ENABLE_SET_MAC
            if(macSplitFlag)
            {
                for(i=0; i++; i< macleftcnt)
                {
                    loadbuf[i] = rtk8761apcb.bdaddr.addr[BD_ADDR_LEN-macleftcnt+i];
                }
            }
            else
            {
                for(i=0; i<BD_ADDR_LEN; i++)
                {
                    loadbuf[iCurLen-1-i] = rtk8761apcb.bdaddr.addr[BD_ADDR_LEN-1-i];
                }
            }

#endif
        }
        else if(downcnt + PATCH_DATA_FIELD_MAX_SIZE == patchcodeLen)
        {
            iCurIndex = iCurIndex | 0x80;
            iCurLen = PATCH_DATA_FIELD_MAX_SIZE;    //last packet length

#ifdef BT_ENABLE_SET_MAC
            {

                for(i=0; i++; i< BD_ADDR_LEN)
                {
                    loadbuf[PATCH_DATA_FIELD_MAX_SIZE-1-i] = rtk8761apcb.bdaddr.addr[BD_ADDR_LEN-1-i];
                }

            }

#endif
        }
        else
        {
            iCurIndex = iCurIndex & 0x7F;
            iCurLen = PATCH_DATA_FIELD_MAX_SIZE;
            if(patchcodeLen-(downcnt+PATCH_DATA_FIELD_MAX_SIZE) < PATCH_DATA_FIELD_MAX_SIZE)
            {
                /*×îºóÒ»¸ö°ü,±£´æµ¹Êý  µÚ¶þ°ü×îºó6¸ö×Ö½Ú*/
#ifdef BT_ENABLE_SET_MAC
                macleftcnt = patchcodeLen-(downcnt+PATCH_DATA_FIELD_MAX_SIZE);

                if(macleftcnt<6)
                {
                    for(i=0; i<6-macleftcnt; i++)
                        loadbuf[PATCH_DATA_FIELD_MAX_SIZE-1-i] = rtk8761apcb.bdaddr.addr[BD_ADDR_LEN-macleftcnt-1-i];

                    macSplitFlag =1; /*ÐèÒªÔÚÏÂÒ»¸ö°üÖÐ²¹ÉÏ ÆäËûµÄmac*/
                }


#endif
            }
        }

        if( (iCurIndex & 0x80) && configFileLen <= patchcodeLen - downcnt)
        {
            //last loaded buffer include the whole all config data.
            if(rtk8761apcb.needChangeMac)
            {
                //change the bt chip mac addr
                //Set_BDAdress(loadbuf,rtk8761apcb.bdaddr.addr,&iCurLen);
            }
            finish_flag = 1;
        }
        else
        {
            ///Other conditions may be complex.
        }

        Rtk8761aDownload_patchcode(data, iCurLen,iCurIndex );
        if(finish_flag)
        {
            rkos_delay(100);
#ifdef HCI_DATA_WATCH
            //UARTInit(59*2*1000*1000,921600,UART_DATA_8B,UART_ONE_STOPBIT,UART_PARITY_DISABLE);
            UartDev_SetBaudRate(rtk8761a_hw_control.user_data, 59*2*1000*1000);
#else
            //UARTInit(24*1000*1000,1500000,UART_DATA_8B,UART_ONE_STOPBIT,UART_PARITY_DISABLE);
            //UARTSetBaudRate(BT_HCI_UART_ID,24*1000*1000, 1500000);
            UartDev_SetFlowControl(rtk8761a_hw_control.user_data);
#endif

        }
        ret = wait_rtk8761a_cmd_complete(2000);
        if(ret == TIMEOUT && !finish_flag)
        {
            BT_DEBUG("time out\n");
            return TIMEOUT;
        }
        if( iCurIndex & 0x80 )  //patch code & configuration file have download successed
            break;

        downcnt += iCurLen;
        offset += iCurLen;
        //ModuleOverlayLoadData(loadAddr + offset, (uint32)(loadbuf), PATCH_DATA_FIELD_MAX_SIZE);
        FW_ReadFirmwaveByByte(loadAddr + offset, loadbuf, PATCH_DATA_FIELD_MAX_SIZE);
        iCurIndex ++;
    }
    BT_DEBUG("down script complete\n");
    rkos_delay(400);
    hci_cmd_complete(NULL);

    return RETURN_OK;
}


/*
--------------------------------------------------------------------------------
  Function name :  int Rtk8761aHwSleepEnable(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping            2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwSleepEnable(void * config)
{

    return RETURN_OK;
}

/*
--------------------------------------------------------------------------------
  Function name :  int Rtk8761aHwSleepDisable(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping            2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwSleepDisable(void * config)
{

    return RETURN_OK;
}

/*
--------------------------------------------------------------------------------
  Function name :  int Rtk8761aHwWakeup(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping            2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwWakeup(void * config)
{
    rtk8761a_set_host_rts_leave(GPIO_LOW);
    rtk8761a_send_wake_up_ind();

    return RETURN_OK;
}

/*
--------------------------------------------------------------------------------
  Function name :  int Rtk8761aHwGetPowerStatus(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping            2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwGetPowerStatus(void)
{
    return rtk8761apcb.PowerStatus;
}


/*
--------------------------------------------------------------------------------
  Function name :  int Rtk8761aHwSetChipBaudrate(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping            2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwSetChipBaudrate(uint baudrate)
{
    return RETURN_OK;
}


/*
--------------------------------------------------------------------------------
  Function name :  int BlueToothHwSetDevMac(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping            2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwSetDevMac(struct bd_addr *bdaddr)
{
    rtk8761apcb.needChangeMac = TRUE;
    memcpy(rtk8761apcb.bdaddr.addr,bdaddr->addr, 6);
    return RETURN_OK;
}


/*
--------------------------------------------------------------------------------
  Function name :  int Rtk8761aHwRegisterPowerNotification(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping            2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwRegisterPowerNotification(int(*func)(POWER_NOTIFICATION_t event))
{
    rtk8761apcb.power_event_notify = func;
    return RETURN_OK;
}



/*
--------------------------------------------------------------------------------
  Function name :  int Rtk8761aHwRegisterHwError(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping            2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwRegisterHwError(int(*func)(POWER_NOTIFICATION_t event))
{
    return RETURN_OK;
}

/*
--------------------------------------------------------------------------------
  Function name :  int BlueToothHwSetDevMac(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping            2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwSetUartRecAbility(UART_ABILITY_t Ability)
{
    if(Ability == UART_ABILITY_RECV_ENABLE)
    {
        rtk8761a_set_host_rts_leave(GPIO_LOW);
    }
    else if(Ability == UART_ABILITY_RECV_DISABLE)
    {
        rtk8761a_set_host_rts_leave(GPIO_HIGH);
    }

    return RETURN_OK;
}

/*
--------------------------------------------------------------------------------
  Function name :  int Rtk8761aHwHwService(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping            2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwService()
{
#ifdef ENABLE_DEEP_SLEEP
    if(TRUE == GetMsg(MSG_BLUETOOTH_HCILL_WAKE_UP_ACK))
    {
        Gpio_DisableInt(GPIO_HOST_CTS);
        rtk8761apcb.PowerStatus = POWER_STATUS_ON;

        if(rtk8761apcb.power_event_notify)
        {
            rtk8761apcb.power_event_notify(POWER_WILL_WAKE_UP_CFM);
        }
    }

    if(TRUE == GetMsg(MSG_BLUETOOTH_GET_WAKE_UP_INT))
    {
        rtk8761a_set_host_rts_leave(GPIO_LOW);
    }

    if(TRUE == GetMsg(MSG_BLUETOOTH_HCILL_SLEEP_IND))
    {
        rtk8761a_set_host_rts_leave(GPIO_HIGH);
        rtk8761a_send_go_to_sleep_ack();

        Gpio_EnableInt(GPIO_HOST_CTS);
        rtk8761apcb.PowerStatus = POWER_STATUS_SLEEP;

        if(rtk8761apcb.power_event_notify)
        {
            rtk8761apcb.power_event_notify(POWER_WILL_SLEEP);
        }

    }

    if(TRUE == GetMsg(MSG_BLUETOOTH_HCILL_WAKE_UP_IND))
    {
        rtk8761a_send_wake_up_ack();
        Gpio_DisableInt(GPIO_HOST_CTS);
        rtk8761apcb.PowerStatus = POWER_STATUS_ON;

        if(rtk8761apcb.power_event_notify)
        {
            rtk8761apcb.power_event_notify(POWER_WILL_WAKE_UP_IND);
        }
    }
#endif

    return RETURN_OK;
}


/*
--------------------------------------------------------------------------------
  Function name :  int Rtk8761aHwHwService(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping            2014/03/5        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHwVendorRecv(uint8 *data ,uint len)
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

/*
--------------------------------------------------------------------------------
  Function name :  int Rtk8761aHciDataRead(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping            2015/09/23        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHciDataRead(uint8 *data ,uint len)
{
    return UartDev_Read(rtk8761a_hw_control.user_data, data, len, 3);

    //BT_DEBUG("rtk8761a_hw_control.user_data = 0x%04x\n", rtk8761a_hw_control.user_data);
    //return RETURN_OK;

}


/*
--------------------------------------------------------------------------------
  Function name :  int Rtk8761aHciDataWrite(void )
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wangping            2015/09/23        Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
int32 Rtk8761aHciDataWrite(uint8 *data ,uint len)
{
    return UartDev_Write(rtk8761a_hw_control.user_data, data, len, SYNC_MODE, NULL);

    //return RETURN_OK;

}

_ATTR_BLUETOOTHCONTROL_DATA_ bt_hw_control_t rtk8761a_hw_control =
{
    Rtk8761aHwInit,
    Rtk8761aHwPowerOn,
    Rtk8761aHwPowerOff,
    NULL,
    NULL,
    NULL,
    NULL,
    Rtk8761aHwGetPowerStatus,
    NULL,
    Rtk8761aHwRegisterPowerNotification,
    Rtk8761aHwSetUartRecAbility,
    Rtk8761aHwSetDevMac,
    NULL,
    NULL,
    Rtk8761aHciDataWrite,
    Rtk8761aHciDataRead,
    NULL,

};

#endif    //(BT_CHIP_CONFIG == BT_CHIP_RTL8761)


#endif
