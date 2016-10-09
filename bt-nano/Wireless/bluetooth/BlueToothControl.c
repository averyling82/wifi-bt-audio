/*
********************************************************************************************
*
*               Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: BlueToothControl.c
* Owner: wangping
* Date: 2015.10.15
* Time: 10:00:08
* Desc: bluetoothControl.c
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    wangping     2015.10.15     10:00:08   1.0
********************************************************************************************
*/

#define _IN_BLUETOOTH_

#include "bt_config.h"

#ifdef  _BLUETOOTH_
#ifdef  _A2DP_SINK_
#include "audio_file_access.h"
#include "BlueToothControl.h"
#include "Btbuffer.h"
//#include "bd_addr.h"
#include "BT_addr.h"
#include "BlueToothSysParam.h"
#include "rk_bt_Api.h"
#include "btHwControl.h"
#include "rk_bt_pbuf.h"

#if ((BT_CHIP_CONFIG == BT_CHIP_CC2564) || (BT_CHIP_CONFIG == BT_CHIP_CC2564B))
#include "btCC2564Control.h"
#endif

#if ((BT_CHIP_CONFIG == BT_CHIP_RTL8761AT) || (BT_CHIP_CONFIG == BT_CHIP_RTL8761ATV))
#include "btRtk8761aControl.h"
#endif

#if (BT_CHIP_CONFIG == BT_CHIP_AP6212)
#include "btAP6212Control.h"
#endif
//#include "lwbterr.h"
//#include "spp.h"
//#include "obex.h"
//#include "opp.h"




void BTPhoneInit(void);

_ATTR_BLUETOOTHCONTROL_DATA_ uint8  BTA2DPStatus = 0;
_ATTR_BLUETOOTHCONTROL_DATA_ uint8  BTCallStatus = 0;
_ATTR_BLUETOOTHCONTROL_DATA_ uint8  BTADStatus = 0;
_ATTR_BLUETOOTHCONTROL_DATA_ uint8  CC2564SleepConfigStatus = 0;
_ATTR_BLUETOOTHCONTROL_DATA_ uint8  BTAvcrpStatus =  AVCRP_STATE_PAUSE;
_ATTR_BLUETOOTHCONTROL_DATA_ uint32  BTState      = 1<< BT_CONNECTING;
_ATTR_BLUETOOTHCONTROL_DATA_ uint32  PowerEvent = 0;

_ATTR_BLUETOOTHCONTROL_DATA_ INT16   hOppFile = 0;
_ATTR_BLUETOOTHCONTROL_DATA_ uint32  OppObjectCurLen = 0;

_ATTR_BLUETOOTHCONTROL_DATA_ pQueue  bt_ctrl_queue = NULL;

_ATTR_BLUETOOTHCONTROL_DATA_ static HDC PAudio = NULL;
_ATTR_BLUETOOTHCONTROL_DATA_ static HDC hFifo = NULL;
_ATTR_BLUETOOTHCONTROL_DATA_ static HDC hFifoPhone = NULL;
_ATTR_BLUETOOTHCONTROL_DATA_ static uint8 bt_ctrl_task_exit = 0;
_ATTR_BLUETOOTHCONTROL_DATA_ static void* bt_ctrl_task_handle = 0;
#ifdef ENABLE_PAIR_TIMER
_ATTR_BLUETOOTHCONTROL_DATA_ pTimer blueToothWaitPairedTimer = NULL;
#endif

_ATTR_BLUETOOTHCONTROL_DATA_ static HDC hFileDev = NULL;
_ATTR_BLUETOOTHCONTROL_DATA_ static HDC pFileDev = NULL;
_ATTR_BLUETOOTHCONTROL_DATA_ pTimer blueToothAclDisConnectedTimer  = NULL;
_ATTR_BLUETOOTHCONTROL_DATA_ pTimer blueToochCheckTimer = NULL;

_ATTR_BLUETOOTHCONTROL_DATA_ uint8 bt_timeout_flag = 0;
_ATTR_BLUETOOTHCONTROL_DATA_ uint8 bt_timeout_retry = 0;
_ATTR_BLUETOOTHCONTROL_DATA_ uint16 stereoBuf[PHY_FRAME_MAX_SIZE];
typedef enum
{
    MSG_BLUETOOTH_DISPLAY_ALL = 0,
    MSG_BLUETOOTH_CONNECTING,
    MSG_BLUETOOTH_CONNECTED,
    MSG_BLUETOOTH_PAIR_ING,
    MSG_BLUETOOTH_PAIR_SUCCEED,
    MSG_BLUETOOTH_BUFFER_NODATA,

    MSG_BLUETOOTH_A2DP_START,
    MSG_BLUETOOTH_A2DP_PAUSE,
    MSG_BLUETOOTH_A2DP_PLAY,
    MSG_BLUETOOTH_SCO_CONNECTED,
    MSG_BLUETOOTH_SCO_DISCONNECTED,
    MSG_BLUETOOTH_ACL_CONNECTED,
    MSG_BLUETOOTH_ACL_DISCONNECTED,
    MSG_BLUETOOTH_CALL_STATUS_CHANGE,
    MSG_BLUETOOTH_CALL_SETUP_STATUS_CHANGE,

    MSG_BLUETOOTH_CALL_RING,

    MSG_BLUETOOTH_HFP_GET_PHONE_NO,

    MSG_BLUETOOTH_HFP_CONNECTED,
    MSG_BLUETOOTH_HFP_DISCONNECTED,
    MSG_BLUETOOTH_A2DP_CONNECTED,
    MSG_BLUETOOTH_A2DP_STREAMING,
    MSG_BLUETOOTH_A2DP_SUSPEND,
    MSG_BLUETOOTH_A2DP_CLOSE,
    MSG_BLUETOOTH_A2DP_ABORT,
    MSG_BLUETOOTH_GET_VAD,
    MSG_BLUETOOTH_AVRCP_CONNECTED,

    MSG_BLUETOOTH_OPP_CONNECTED,
    MSG_BLUETOOTH_OPP_CREATE_FILE,
    MSG_BLUETOOTH_OPP_CLOSE_FILE,
    MSG_BLUETOOTH_OPP_WRITE_FILE,
    MSG_BLUETOOTH_OPP_DELETE_FILE,
    MSG_BLUETOOTH_OPP_DISCONNECTED,
    MSG_BLUETOOTH_OPP_DISPFLAG_FILENAME,
    MSG_BLUETOOTH_OPP_DISPFLAG_STANDY,
    MSG_BLUETOOTH_OPP_DISPFLAG_SCHED,

    MSG_BLUETOOTH_DISPLAY_CALL_COMING,
    MSG_BLUETOOTH_DISPLAY_CALL_PLAY,
    MSG_BLUETOOTH_DISPLAY_CALL_STOP,
    MSG_BLUETOOTH_AVRCP_PLAY,
    MSG_BLUETOOTH_AVRCP_STOP,
    MSG_BLUETOOTH_AVRCP_PAUSE,
    MSG_BLUETOOTH_AVRCP_NXET,
    MSG_BLUETOOTH_AVRCP_PREV,
    MSG_BLUETOOTH_DISCONNECT_SUCCEED,
    MSG_BLUETOOTH_LINK_KEY_NOTIFY,
    MSG_BLUETOOTH_CONNECT_SUCCEED,
    MSG_BLUETOOTH_CONNECT_FAIL,
    MSG_BLUETOOTH_CONNECT_RETURN,
    MSG_BLUETOOTH_WIN_DISCONNECTED,
    MSG_BLUETOOTH_SCAN_WIN_DESTROY,


    MSG_PLAY_VOICE_WAIT_PAIR,
    MSG_PLAY_VOICE_NOTIFY_FINISH,
    MSG_PLAY_VOICE_NOTIFY_ACL_DISCONNECTED,

    MSG_BLUETOOTH_HCILL_SLEEP_IND,
    MSG_BLUETOOTH_HCILL_SLEEP_ACK,
    MSG_BLUETOOTH_HCILL_WAKE_UP_IND,
    MSG_BLUETOOTH_HCILL_WAKE_UP_ACK,
    MSG_BLUETOOTH_GET_WAKE_UP_INT,
    MSG_BLUETOOTH_NFC_INT_COMING,
    MSG_BLUETOOTH_GO_TO_WAIT_PAIR,

    MSG_BLUETOOTH_SPP_WORK,
    MSG_BLUETOOTH_SVAE_PAIR_DEV,

    MSG_CHECK_TIMER,
    //End

    MSG_EXIT_BLUETOOTH,
    MSG_ID_MAX  //max message number
} BT_MSG_ID;

static void SendMsg(uint32 cmd)
{
    uint32 cmd_temp = cmd;
    if(bt_ctrl_queue)
    {
        rkos_queue_send(bt_ctrl_queue, &cmd_temp, 0);
    }
}


#ifdef ENABLE_NFC
_ATTR_BLUETOOTHCONTROL_DATA_ uint32  BTNfcUnLockTick = 0;

_ATTR_BLUETOOTHCONTROL_CODE_
void blueToothNfcComing()
{
    if(!CHECK_BTSTATE(BTState,BT_NFC_INT_LOCK))
    {
        SendMsg(MSG_BLUETOOTH_NFC_INT_COMING);
        SET_BTSTATE(BTState,BT_NFC_INT_LOCK);
    }
    BTNfcUnLockTick = GetSysTick();

    //DEBUG("NFC COMING")   ;
}

#endif

_ATTR_BLUETOOTHCONTROL_CODE_
void blueToothConnectReqProc(struct bd_addr *bdaddr)
{

    uint8 * link_key;

    link_key = get_link_key(bdaddr);
    if(link_key == NULL)
    {
        hci_accept_connection_Request(bdaddr, 0x01);
    }
    else
    {
        hci_reject_connection_request(bdaddr, 0x0D);

    }
}

#ifdef _HFP_
_ATTR_BLUETOOTHCONTROL_CODE_
void blueTooth_hfp_event_handler(uint8 event_type, uint8 *parameter, uint16 param_size, err_t err)
{
    uint32 cmd = 0;
    BT_DEBUG("event_type=%d", event_type);
    switch(event_type)
    {
        case HFP_EVENT_CALL_STATUS_CHANGE:
            cmd = MSG_BLUETOOTH_CALL_STATUS_CHANGE;

            SendMsg(MSG_BLUETOOTH_CALL_STATUS_CHANGE);

            break;

        case HFP_EVENT_SETUP_STATUS_CHANGE:
            SendMsg(MSG_BLUETOOTH_CALL_SETUP_STATUS_CHANGE);
            break;

        case HFP_EVENT_CALL_RING:
            SendMsg(MSG_BLUETOOTH_CALL_RING);

            break;

        case HFP_EVENT_CONNECTED:
            SendMsg(MSG_BLUETOOTH_HFP_CONNECTED);

            break;

        case HFP_EVENT_DISCONNECTED:
            SendMsg(MSG_BLUETOOTH_HFP_DISCONNECTED);
            break;

        case HFP_EVENT_GET_PHONE_NUMBER:
            SendMsg(MSG_BLUETOOTH_HFP_GET_PHONE_NO);

            break;

        default:
            break;
    }
}

#endif

#ifdef _OPP_
_ATTR_BLUETOOTHCONTROL_CODE_
BOOLEAN opp_WriteFile (UINT8  *pWriteBuf, UINT32 ObjectCurLen, INT16 hOpp)
{
    UINT8  *pWriteBuffer;
    UINT32  uWriteBytes,nWriteBytes;
    UINT32  TempLength,CurLen = ObjectCurLen;
    pWriteBuffer = pWriteBuf;
    //printf("1");
    if (CurLen != 0)     //write 2 k data every time.
    {
        uWriteBytes = FileWrite(pWriteBuffer,CurLen ,hOpp);
        //printf("2");
        if (CurLen == uWriteBytes)
        {
            //printf("\n");
            OPP_CurrentLen += uWriteBytes;
            return TRUE;
        }
        else
        {
            printf("FileWrite err\n");
            return FALSE;
        }
    }
}
_ATTR_BLUETOOTHCONTROL_CODE_
void opp_WriteFileTail (UINT8  *pWriteBuf, UINT32 ObjectCurLen, INT16 hOpp)
{
    UINT16  i,j;
    UINT16  uHead;
    UINT8  *pWriteBuffer;
    UINT16  uWriteBytes;
    uint16  TempLength;
    pWriteBuffer = pWriteBuf;
    TempLength = ((ObjectCurLen + 511) / 512) * 512;
    j = ObjectCurLen;
    for (i = 0; i < TempLength - ObjectCurLen; i++)
    {
        pWriteBuffer[j++] = 0x00;
    }
    if(TempLength != 0)
    {
        uWriteBytes = FileWrite(pWriteBuffer,TempLength ,hOpp);
        if(uWriteBytes != TempLength)
        {
            printf("FileWrite err\n");
        }
        OPP_CurrentLen += uWriteBytes;

    }
}
_ATTR_BLUETOOTHCONTROL_CODE_
BOOLEAN blueTooth_opp_WriteFile (BOOLEAN FileTail)
{
    unsigned char Writebuf[2048] = {0,};
    unsigned short len,Curlen;
    UINT32  nSec,i,j;
    if(FileTail != TRUE)
    {
        nSec = OppObjectCurLen/512;
        if(nSec > 0)
        {
            len = nSec*512;
            if(OppObjectCurLen > 2048)
            {
                Curlen = RKBT_FRead(&Writebuf[0],2048,NULL);
                len = 2048;
            }
            else
            {
                Curlen = RKBT_FRead(&Writebuf[0],len,NULL);
            }
            DelayMs(10);
            if(len == Curlen)
            {
                if( TRUE == (opp_WriteFile(&Writebuf[0],Curlen,hOppFile)))
                {
                    OppObjectCurLen = OppObjectCurLen - Curlen;
                }
            }
        }
        else
        {
        }
    }
    else
    {
        //printf("Taillen = %d\n",OppObjectCurLen);
        i = OppObjectCurLen/2048;
        for(j=0; j < i; j++)
        {
            Curlen = RKBT_FRead(&Writebuf[0],2048,NULL);
            DelayMs(10);
            opp_WriteFile(&Writebuf[0],Curlen,hOppFile);
            OppObjectCurLen = OppObjectCurLen - Curlen;
            //printf("Taillen = %d\n",OppObjectCurLen);
        }
        //printf("lastTaillen = %d\n",OppObjectCurLen);
        Curlen = RKBT_FRead(&Writebuf[0],OppObjectCurLen,NULL);
        //printf("Curlen = %d\n",Curlen);
        DelayMs(10);
        if(Curlen == OppObjectCurLen)
        {
            opp_WriteFileTail(&Writebuf[0],Curlen,hOppFile);
        }
    }
}
_ATTR_BLUETOOTHCONTROL_CODE_
void blueTooth_opp_Service(uint32 cmd)
{
    switch(cmd)
    {
        case MSG_BLUETOOTH_OPP_CREATE_FILE:
            {
                printf("create file\n");
                UserIsrDisable();
                PMU_EnterModule(PMU_MAX);
                hOppFile = opp_CreateFileFromName();
                OPP_TotalLen = opp_GetFileLength();
                OPP_CurrentLen = 0;
                if(hOppFile != -1)
                {
                    //printf("file ok\n");
                    OBEX_Delay_Rsp(ERR_OK);
                    ClearMsg(MSG_BLUETOOTH_OPP_CREATE_FILE);
                    SendMsg(MSG_BLUETOOTH_OPP_DISPFLAG_FILENAME);
                }
                else
                {
                    printf("file ERR\n");
                    OBEX_Delay_Rsp(ERR_MEM);
                    ClearMsg(MSG_BLUETOOTH_OPP_CREATE_FILE);
                }
                PMU_ExitModule(PMU_MAX);
                UserIsrEnable(0);
            }
            break;

        case MSG_BLUETOOTH_OPP_WRITE_FILE:
            {
                if(CheckMsg(MSG_BLUETOOTH_OPP_CONNECTED))
                {
                    HciServeIsrDisable();
                    if(hOppFile != -1)
                        blueTooth_opp_WriteFile(FALSE);
                    SendMsg(MSG_BLUETOOTH_OPP_DISPFLAG_SCHED);
                    HciServeIsrEnable();
                }
            }
            break;

        case MSG_BLUETOOTH_OPP_CLOSE_FILE:
            {
                UserIsrDisable();
                if(hOppFile != -1)
                    blueTooth_opp_WriteFile(TRUE);
                bt_buf_reset();
                SendMsg(MSG_BLUETOOTH_OPP_DISPFLAG_SCHED);
                printf("FileClose\n");
                FileClose(hOppFile);
                hOppFile = -1;
                ClearMsg(MSG_BLUETOOTH_OPP_CLOSE_FILE);
                SendMsg(MSG_BLUETOOTH_OPP_DISPFLAG_STANDY);

                PMU_ExitModule(PMU_OPP);
                UserIsrEnable(0);
            }
            break;

    }
}
_ATTR_BLUETOOTHCONTROL_CODE_
void blueTooth_opp_event_handler(uint8 event_type, uint8 *parameter, uint16 param_size,err_t err)
{
    switch(event_type)
    {
        case OPP_CMD_CONNECT:
            OppObjectCurLen = 0;
            bt_buf_reset();

            SendMsg(MSG_BLUETOOTH_OPP_CONNECTED);
            break;
        case OPP_CMD_DISCONNECT:
            ClearMsg(MSG_BLUETOOTH_OPP_CONNECTED);
            SendMsg(MSG_BLUETOOTH_OPP_DISCONNECTED);
            break;
        case OPP_CMD_CREATE_FILE:
            printf("CREATE_FILE_MSG\n");
            SendMsg(MSG_BLUETOOTH_OPP_CREATE_FILE);
            break;
        case OPP_CMD_CLOSE_FILE:
            SendMsg(MSG_BLUETOOTH_OPP_CLOSE_FILE);
            break;
        case OPP_CMD_PUT:
            if(TRUE != CheckMsg(MSG_BLUETOOTH_OPP_CREATE_FILE))
                SendMsg(MSG_BLUETOOTH_OPP_WRITE_FILE);
            break;
        case OPP_CMD_ABORT:
            SendMsg(MSG_BLUETOOTH_OPP_DELETE_FILE);
            break;
        case OPP_CMD_SESSION:
            break;
        default:
            break;
    }
}
_ATTR_BLUETOOTHCONTROL_CODE_
void bt_objectdata_input(const unsigned char *s, unsigned int len)
{
    unsigned int free_len;
    {
        free_len = bt_buf_free_size();
        if(free_len < len)
        {
            return;
        }
        else
        {
            bt_buf_puts(s, len);
            OppObjectCurLen += len;
        }
    }
}
#endif

#ifdef _AVDTP_
_ATTR_BLUETOOTHCONTROL_CODE_
void blueTooth_avdtp_event_handler(uint8 event_type, uint8 *parameter, uint16 param_size,err_t err)
{
    switch(event_type)
    {
        case AVDTP_EVENT_CONNECTED:
            BT_DEBUG("AVDTP_EVENT_CONNECTED\n");
            SendMsg(MSG_BLUETOOTH_A2DP_CONNECTED);
            break;

        case AVDTP_EVENT_DISCONNECTED:

            break;

        case AVDTP_EVENT_STREAMING:
            {
                SendMsg(MSG_BLUETOOTH_A2DP_STREAMING);
#ifdef A2DP_ACC
                if(CurrentDecCodec == CODEC_AAC)
                {
                }
                else
#else
                bt_buf_reset();
#endif
                    BTAvcrpStatus = AVCRP_STATE_PLAY;
                if(!CHECK_BTSTATE(BTState, BT_CALL))
                {

                }
                BT_DEBUG("AVDTP_EVENT_STREAMING\n");

            }

            break;

        case AVDTP_EVENT_SUSPEND:
            {
                SendMsg(MSG_BLUETOOTH_A2DP_SUSPEND);

                BTAvcrpStatus = AVCRP_STATE_PAUSE;
                BT_DEBUG("AVDTP_EVENT_SUSPEND\n");
            }
            break;

        case AVDTP_EVENT_CLOSE:
            SendMsg(MSG_BLUETOOTH_A2DP_CLOSE);
            break;

        case AVDTP_EVENT_ABORT:
            SendMsg(MSG_BLUETOOTH_A2DP_ABORT);
            break;

        default:
            break;
    }
}
#endif

#ifdef _AVRCP_
_ATTR_BLUETOOTHCONTROL_CODE_
void blueTooth_avctp_event_handler(uint8 event_type, uint8 *parameter, uint16 param_size,err_t err)
{
    switch(event_type)
    {
        case AVCTP_EVENT_CONNECTED:
            SendMsg(MSG_BLUETOOTH_AVRCP_CONNECTED);
            break;

        case AVCTP_EVENT_DISCONNECTED:

            break;

        default:
            break;
    }
}
#endif


_ATTR_BLUETOOTHCONTROL_CODE_
void blueTooth_hci_app_event_handler(uint8 event_type, uint8 *parameter, uint16 param_size,err_t err)
{
    switch(event_type)
    {
        case HCI_APP_EVENT_ACL_CONNECTED:

            BT_DEBUG("HCI_APP_EVENT_ACL_CONNECTED\n");
            SendMsg(MSG_BLUETOOTH_ACL_CONNECTED);
            bt_timeout_flag = 0;
            bt_timeout_retry = _BT_TIMEOUT_RETRY_;


            break;

        case HCI_APP_EVENT_ACL_DISCONNECTED:
            {
                BT_DEBUG("HCI_APP_EVENT_ACL_DISCONNECTED\n");
                SendMsg(MSG_BLUETOOTH_ACL_DISCONNECTED);

                if(err == 0x13) //手机端主动断开
                {

                }
                else //其他原因断开，例如超过有效距离
                {
                    bt_timeout_flag = 1;
                    BT_DEBUG("acl disconnected err=0x%x\n", err);
                }

            }
            break;

        case HCI_APP_EVENT_SCO_CONNECTED:
            SendMsg(MSG_BLUETOOTH_SCO_CONNECTED);
            break;

        case HCI_APP_EVENT_SCO_DISCONNECTED:
            SendMsg(MSG_BLUETOOTH_SCO_DISCONNECTED);
            break;

        case HCI_APP_EVENT_BLE_CONNECTED:

            break;

        case HCI_APP_EVENT_BLE_DISCONNECTED:

            break;

        default:
            break;
    }
}

#ifdef _SPP_
_ATTR_BLUETOOTHCONTROL_CODE_
void blueTooth_spp_event_handler(uint8 event_type, uint8 *parameter, uint16 param_size,err_t err)
{

    switch(event_type)
    {
        case SPP_EVENT_CONNECTED:



            break;

        case SPP_EVENT_DISCONNECTED:

            break;

        default:
            break;
    }
}
#endif

_ATTR_BLUETOOTHCONTROL_CODE_
void blueToothPowerEvnetNotify(POWER_NOTIFICATION_t event)
{
    SET_BTSTATE(BTState,BT_GET_POWER_EVENT);

    PowerEvent = event;
}

_ATTR_BLUETOOTHCONTROL_CODE_
void bt_mediadata_input(const unsigned char *s, unsigned int len)
{
    unsigned int free_len;
    //BT_DEBUG("bt_mediadata_input\n");
#if 1
    if((!CHECK_BTSTATE(BTState,BT_CALL_PLAYING)) && (!CHECK_BTSTATE(BTState,BT_SCO_CONNECTED)) && (CHECK_BTSTATE(BTState,BT_A2DP_CONNECTED)) && (CHECK_BTSTATE(BTState,BT_PLAY_MUSIC))) //提示来电后不接受a2dp数据
    {
//        free_len = bt_buf_free_size();
//        BT_DEBUG("bt_mediadata_input\n");
//        if(free_len < len)
//        {
//            return;
//        }
//        else
//        {
//            bt_buf_puts(s, len);
//        }
        //BT_DEBUG("bt_mediadata_input\n");
        //printf("<\n");
        fifoDev_Write(hFifo, (uint8*)s, len, ASYNC_MODE, NULL);
        //printf(">\n");

    }
#endif
}

#ifdef _HFP_
_ATTR_BLUETOOTHCONTROL_CODE_
void bt_phonedata_input(const unsigned char *s, unsigned int len)
{
    if(CHECK_BTSTATE(BTState,BT_CALL_PLAYING) && CHECK_BTSTATE(BTState,BT_PLAY_CALL))
    {
        uint16 *monoBuf = NULL;
        int i, j;

        monoBuf = (uint16 *)s;

        for (i = 0, j = 0; i < len/2; ++i)
        {
            stereoBuf[j++] = monoBuf[i];
            stereoBuf[j++] = monoBuf[i];
        }

        fifoDev_Write(hFifoPhone, (uint8*)stereoBuf, len*2, ASYNC_MODE, NULL);
    }
}
#endif

#ifdef _SPP_
_ATTR_BLUETOOTHCONTROL_CODE_
void bt_spp_input(unsigned char *s, unsigned int len)
{
//    spp_send(s, len, NULL);
//  bt_spp_input_parse(s, len);
}

#endif


#ifdef ENABLE_PAIR_TIMER
#if 0
void blueToothWaitPairedTimerProc(void);
//pTimer blueToothWaitPairedTimer;
_ATTR_BLUETOOTHCONTROL_DATA_
SYSTICK_LIST blueToothWaitPairedTimer =
{
    NULL,
    0,
    PAIR_TIME_OUT,
    0,
    blueToothWaitPairedTimerProc,
};
#endif

_ATTR_BLUETOOTHCONTROL_CODE_
void blueToothWaitPairedTimerProc(void * handle)
{
#ifdef ENABLE_PAIR_TIMER
    //SystickTimerStop(&blueToothWaitPairedTimer);
    rkos_stop_timer(blueToothWaitPairedTimer);
#endif

    if(!CHECK_BTSTATE(BTState, BT_ACL_CONNECTED))
    {
#ifdef ENABLE_DEEP_SLEEP
        BlueToothHwSleepEnable();
#endif

        bt_discoverable_disable();
    }

}
#endif
//pTimer blueToothAclDisConnectedTimer;
#if 0
void blueToothAclDisConnectedTimerProc(void);
_ATTR_BLUETOOTHCONTROL_DATA_
SYSTICK_LIST blueToothAclDisConnectedTimer =
{
    NULL,
    0,
    600,
    0,
    blueToothAclDisConnectedTimerProc,
};
#endif

_ATTR_BLUETOOTHCONTROL_CODE_
void blueToothAclDisConnectedTimerProc(void * handle)
{

    //SystickTimerStop(&blueToothAclDisConnectedTimer);
    BT_DEBUG("AclDisConnectedTimerProc coming\n");
    rkos_stop_timer(blueToothAclDisConnectedTimer);
    if(!CHECK_BTSTATE(BTState,BT_ACL_CONNECTED))
    {
        SendMsg(MSG_PLAY_VOICE_NOTIFY_ACL_DISCONNECTED);
    }
}

_ATTR_BLUETOOTHCONTROL_CODE_
void blueToochCheckTimerProc(void)
{
    SendMsg(MSG_CHECK_TIMER);
}

/******************************************************************************
 * BlueToothControlStart -
 * DESCRIPTION: - avrcp ctrl
 *
 * Input: controlID
 * Output:void
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2015.10.15  written
 * Time : 14:45:15
 * --------------------
 ******************************************************************************/
_ATTR_BLUETOOTHCONTROL_CODE_
void BlueToothControlStart(uint32 controlID)
{
    int ret;
#ifdef _AVRCP_
    switch(controlID)
    {
        case BT_A2DP_PLAY_PAUSE:
            {
                ret = ct_pause();
                break;
            }
        case BT_A2DP_PLAY:
            {
                ret = ct_play();
                break;
            }

        case BT_A2DP_NEXT:
            {
                ret = ct_next();
                break;
            }
        case BT_A2DP_PREV:
            {
                ret = ct_previous();
                break;
            }
        case BT_HFP_PLAY:
            break;

        case BT_HFP_STOP:

            ret = ct_stop();
            break;

        case BT_HFP_REFUSE:

            break;

        case BT_HFP_LAST_RECALL:

            break;

        case BT_HFP_SWITCH:

            break;

        case BT_RESET_CONNECT_TABLE:

            break;

        default:
            break;
    }

#endif

}

_ATTR_BLUETOOTHCONTROL_CODE_
void BlueToothKeyPlayPause(void)
{
    if( CHECK_BTSTATE(BTState,BT_CALL) )
    {
#ifdef _HFP_
        if(CHECK_BTSTATE(BTState,BT_CALL_PLAYING))
        {
            hfp_Reject_Terminate_calling();
        }
        else
        {
            hfp_Answer_incoming_call();
        }
#endif
    }
    else
    {
#ifdef _AVDTP_
        if( CHECK_BTSTATE(BTState,BT_HFP_CONNECTED) ||    CHECK_BTSTATE(BTState,BT_A2DP_CONNECTED) )
        {
            if(CHECK_BTSTATE(BTState,BT_A2DP_CONNECTED))
            {
                if(CHECK_BTSTATE(BTState,BT_AVRCP_CONNECTED))
                {
                    if (AVCRP_STATE_PAUSE == BTAvcrpStatus)
                    {
                        //  AudioDecodeProc(MSG_AUDIO_RESUME, NULL);//未处于播放状态时调用会死机
                        BlueToothControlStart(BT_A2DP_PLAY);

                        BTAvcrpStatus = AVCRP_STATE_PLAY;
                    }
                    else if (AVCRP_STATE_PLAY == BTAvcrpStatus)
                    {
                        BTAvcrpStatus = AVCRP_STATE_PAUSE;
                        //AudioDecodeProc(MSG_AUDIO_PAUSE, NULL);
                        BlueToothControlStart(BT_A2DP_PLAY_PAUSE);
                    }
//                    SendMsg(MSG_MUSIC_DISPFLAG_STATUS);
                }
                else
                {
                    SET_BTSTATE(BTState, BT_WAIT_A2DP_AVRCP_CONNECTED);
                }
            }
            else
            {
                SET_BTSTATE(BTState, BT_WAIT_A2DP_AVRCP_CONNECTED);

                avdtp_connect((struct bd_addr *)&gSysConfig.BtConfig.LastConnectMac, NULL);
            }
        }
        else
        {
            SendMsg(MSG_BLUETOOTH_GO_TO_WAIT_PAIR);
        }
#endif
    }
}


/*
--------------------------------------------------------------------------------
  Function name :  void BlueToothControlInit(void *pArg)
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                   wanpping         2015/10/15         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
rk_err_t BlueToothControlInit(void *pvParameters, void *arg)
{
    POWER_ON_CONFIG power_on_config;
    char newmac[6]= {0x8c, 0xDF, 0x52, 0x20, 0x55, 0x57,};
#ifdef ENABLE_NFC
    char local_name[12] = {"nft_test"};
#else
    char local_name[12] = {"RKNanoD_Bt"};
#endif

    FREQ_EnterModule(FREQ_BT);


    //这里需要一个变量初始化函数
    BTA2DPStatus = BTAUDIO_STATE_STOP;
    BTCallStatus = BTAUDIO_STATE_STOP;
    BTADStatus = BT_AD_STATE_STOP;
    BTState      = 1<< BT_CONNECTING;
    CC2564SleepConfigStatus = 0;
    BTAvcrpStatus = AVCRP_STATE_PAUSE;
    bt_ctrl_task_exit = 0;
    BT_DEBUG();
    bt_ctrl_queue = rkos_queue_create(30, sizeof(int));
    if(bt_ctrl_queue == NULL)
    {
        goto err_queue_create;
    }
#ifdef ENABLE_PAIR_TIMER
    blueToothWaitPairedTimer = rkos_create_timer(PAIR_TIME_OUT/10, 0, NULL, blueToothWaitPairedTimerProc);
    if(blueToothWaitPairedTimer == NULL)
    {
        goto err_timer_create;
    }
#endif
    blueToothAclDisConnectedTimer  = rkos_create_timer(600/10, 0, NULL, blueToothAclDisConnectedTimerProc);
    if(blueToothAclDisConnectedTimer ==  NULL)
    {
#ifdef ENABLE_PAIR_TIMER
        //rkos_delete_timer(blueToothWaitPairedTimer);
        //blueToothWaitPairedTimer = NULL;
        goto err_AclDisConnectedTimer;
#endif
        return RK_ERROR;
    }

    BT_DEBUG();
    blueToochCheckTimer = rkos_create_timer(2, 1, NULL, blueToochCheckTimerProc);
    if(blueToochCheckTimer == NULL)
    {
#ifdef ENABLE_PAIR_TIMER
        rkos_delete_timer(blueToothWaitPairedTimer);
        blueToothWaitPairedTimer = NULL;
#endif
        //rkos_delete_timer(blueToothAclDisConnectedTimer);
        //blueToothAclDisConnectedTimer = NULL;
        goto err_blueToochCheckTimer;
        return RK_ERROR;

    }
    BT_DEBUG();
#ifdef ENABLE_NFC        //nfc
    Gpio_SetIntMode(GPIO_NFC_DET, IntrTypeFallingEdge);
    GpioIsrRegister(GPIO_NFC_DET, blueToothNfcComing);
    Gpio_EnableInt(GPIO_NFC_DET);
#endif
    //这里需要一个蓝牙串口初始化函数
    BT_DEBUG();
    memset(&power_on_config, 0, sizeof(POWER_ON_CONFIG));
    power_on_config.bt_init_flag = BT_INIT_FLAG;
    power_on_config.enable_dev_name = TRUE;
    //memcpy(power_on_config.dev_name, local_name, sizeof(local_name));
    GetBtAudioDeviceName(power_on_config.dev_name);

    power_on_config.enable_dev_mac = TRUE;
    memcpy(&power_on_config.bdaddr, newmac, 6);

    BT_DEBUG();
#if ((BT_CHIP_CONFIG == BT_CHIP_CC2564) || (BT_CHIP_CONFIG == BT_CHIP_CC2564B))
    if(BlueToothHwInit(&cc2564_hw_control) == RETURN_FAIL)
    {
        goto
        err_HwInit_fail;
    }
#endif

#if ((BT_CHIP_CONFIG == BT_CHIP_RTL8761AT) || (BT_CHIP_CONFIG == BT_CHIP_RTL8761ATV) )
    if(BlueToothHwInit(&rtk8761a_hw_control)== RETURN_FAIL)
    {
        goto
        err_HwInit_fail;

    }
#endif

#if (BT_CHIP_CONFIG == BT_CHIP_AP6212)
    if(BlueToothHwInit(&ap6212_hw_control)== RETURN_FAIL)
    {
        goto
        err_HwInit_fail;

    }
#endif
    BT_DEBUG();

    if(BlueToothHwPowerOn(&power_on_config)== RETURN_FAIL)
    {
        BT_DEBUG();
        goto err_powerOn_fail;
    }
    BT_DEBUG();
    BlueToothHwRegisterPowerNotification(blueToothPowerEvnetNotify);
    BT_DEBUG();
    bt_buf_reset();
    BT_DEBUG();
    hci_register_app_event_handler(blueTooth_hci_app_event_handler);
#ifdef _HFP_
    hfp_register_event_handler(blueTooth_hfp_event_handler);
    hci_sco_recv(bt_phonedata_input);
#endif

#ifdef _OPP_
    opp_register_event_handler(blueTooth_opp_event_handler);
    opp_object_data_recv(bt_objectdata_input);
#endif
#ifdef _AVDTP_
    avdtp_register_event_handler(blueTooth_avdtp_event_handler);
    avdtp_media_recv(bt_mediadata_input);
#endif

#ifdef _AVRCP_
    avctp_register_event_handler(blueTooth_avctp_event_handler);
#endif

#ifdef _SPP_
    spp_register_event_handler(blueTooth_spp_event_handler);
    spp_data_recv(bt_spp_input);
#endif

    SendMsg(MSG_BLUETOOTH_CONNECTING);
    SendMsg(MSG_PLAY_VOICE_WAIT_PAIR);

#ifdef ENABLE_PAIR_TIMER
    //SystickTimerStart(&blueToothWaitPairedTimer);
    rkos_start_timer(blueToothWaitPairedTimer);
#endif

    rkos_start_timer(blueToochCheckTimer);
    BT_DEBUG();



    return RK_SUCCESS;

err_HwInit_fail:
err_powerOn_fail:
    rkos_delete_timer(blueToochCheckTimer);
    blueToochCheckTimer = NULL;
err_blueToochCheckTimer:
    rkos_delete_timer(blueToothAclDisConnectedTimer);
    blueToothAclDisConnectedTimer = NULL;

err_AclDisConnectedTimer:
#ifdef ENABLE_PAIR_TIMER
    rkos_delete_timer(blueToothWaitPairedTimer);
    blueToothWaitPairedTimer = NULL;
#endif

err_timer_create:
    rkos_queue_delete(bt_ctrl_queue);
    bt_ctrl_queue = NULL;
err_queue_create:
    FREQ_ExitModule(FREQ_BT);

    return RK_ERROR;
}

_ATTR_BLUETOOTHCONTROL_CODE_
void rssi_result(struct bd_addr *bdaddr, int8 rssi)
{
    BT_DEBUG("rssi = %d\r\n", rssi);
}

_ATTR_BLUETOOTHCONTROL_CODE_
rk_err_t music_player_start(void)
{
    FIFO_DEV_ARG stFifoArg;
    rk_err_t ret;
    stFifoArg.BlockCnt = 20;
    stFifoArg.BlockSize = 1024;
    stFifoArg.UseFile = 0;
    stFifoArg.ObjectId = 1;
    stFifoArg.hReadFile = NULL;

    if(CheckSystemIdle() == RK_SUCCESS)
    {
        ClearSytemIdle();
    }

    BT_DEBUG("\n music_player_start");

    ret = DeviceTask_CreateDeviceList(DEVICE_LIST_FIFO, &stFifoArg, SYNC_MODE);
    if (ret != RK_SUCCESS)
    {
        BT_DEBUG("fifo dev create failure");
        return RK_ERROR;
    }

    BT_DEBUG("fifo dev create success");

    RK_TASK_AUDIOCONTROL_ARG pArg;
    pArg.ucSelPlayType = SOURCE_FROM_BT;
    pArg.FileNum = 1;
    pArg.pfAudioState = 0;
    RKTaskCreate(TASK_ID_AUDIOCONTROL, 0, &pArg, SYNC_MODE);
    BT_DEBUG();

    if(RKTaskFind(TASK_ID_MUSIC_PLAY_MENU, 0) != NULL)
    {
        MusicPlay_AudioCallBack(AUDIO_STATE_CREATE);
    }


    PAudio = RKDev_Open(DEV_CLASS_AUDIO,0,NOT_CARE);
    if ((PAudio == NULL) || (PAudio == (HDC)RK_ERROR) || (PAudio == (HDC)RK_PARA_ERR))
    {
        BT_DEBUG("Audio device open failure\n");

        return RK_ERROR;
    }

    hFifo = RKDev_Open(DEV_CLASS_FIFO,1,NOT_CARE);
    if ((hFifo == NULL) || (hFifo == (HDC)RK_ERROR) || (hFifo == (HDC)RK_PARA_ERR))
    {
        BT_DEBUG("Fifo device open failure\n");

        return RK_ERROR;
    }
    //fifoDev_SetTotalSize(hFifo, 0);
    fifoDev_SetTotalSize(hFifo, 1024*1024*1024);
    AudioControlTask_SendCmd(AUDIO_CMD_DECSTART, NULL, ASYNC_MODE);

    MainTask_SetStatus(MAINTASK_APP_BT_PLAYER, 1);
    SET_BTSTATE(BTState, BT_PLAY_MUSIC);

}

_ATTR_BLUETOOTHCONTROL_CODE_
void music_player_stop(void)
{
    FIFO_DEV_ARG stFifoArg;

    BT_DEBUG("music_player_stop\n");

    CLEAR_BTSTATE(BTState, BT_PLAY_MUSIC);
    MainTask_SetStatus(MAINTASK_APP_BT_PLAYER, 0);

    RKTaskDelete(TASK_ID_AUDIOCONTROL, 0, SYNC_MODE);
    BT_DEBUG();
    BT_DEBUG();

    RKDev_Close(PAudio);
    BT_DEBUG();
    RKDev_Close(hFifo);
    BT_DEBUG();

    stFifoArg.ObjectId = 1;
    DeviceTask_DeleteDeviceList(DEVICE_LIST_FIFO, &stFifoArg, SYNC_MODE);
}

_ATTR_BLUETOOTHCONTROL_CODE_
void music_player_reset(void)
{
    music_player_stop();
    music_player_start();
}

#ifdef _HFP_
_ATTR_BLUETOOTHCONTROL_CODE_
rk_err_t phone_music_player_start(void)
{
    FIFO_DEV_ARG stFifoArg;
    rk_err_t ret;
    FILE_ATTR stFileAttr;

    if(CheckSystemIdle() == RK_SUCCESS)
    {
        ClearSytemIdle();
    }

    BT_DEBUG("phone_music_player_start");
#if 0
    hFileDev = RKDev_Open(DEV_CLASS_FILE, 0, NOT_CARE);
    if ((hFileDev == NULL) || (hFileDev == (HDC)RK_ERROR) || (hFileDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("File device open failure");
    }
    stFileAttr.Path = L"C:\\";
    stFileAttr.FileName = L"pcm.buf";
    pFileDev = FileDev_OpenFile(hFileDev, NULL, READ_WRITE, &stFileAttr);
    if ((int)pFileDev <= 0)
    {
        ret = FileDev_CreateFile(hFileDev, NULL, &stFileAttr);
        if (ret != RK_SUCCESS)
        {
            rk_print_string("file create failure");
            RKDev_Close(hFileDev);
            while(1)
            {
                rkos_sleep(1000);
            }
        }
        pFileDev = FileDev_OpenFile(hFileDev, NULL, READ_WRITE, &stFileAttr);
        if ((int)pFileDev <= 0)
        {
            rk_print_string("file create failure");
            while(1)
            {
                rkos_sleep(1000);
            }
        }
    }
    stFifoArg.BlockCnt = 20;
    stFifoArg.BlockSize = 1024;
    stFifoArg.UseFile = 1;
    stFifoArg.ObjectId = 0;
#else
    stFifoArg.BlockCnt = 20;
    stFifoArg.BlockSize = 1024;
    stFifoArg.UseFile = 0;
    stFifoArg.ObjectId = 0;
    stFifoArg.hReadFile = NULL;
    ret = DeviceTask_CreateDeviceList(DEVICE_LIST_FIFO, &stFifoArg, SYNC_MODE);
    if (ret != RK_SUCCESS)
    {
        BT_DEBUG("fifo dev create failure");
        return RK_ERROR;
    }
    BT_DEBUG("fifo dev create success");
    hFifoPhone = RKDev_Open(DEV_CLASS_FIFO, 0, NOT_CARE);
    if ((hFifoPhone == NULL) || (hFifoPhone == (HDC)RK_ERROR) || (hFifoPhone == (HDC)RK_PARA_ERR))
    {
        BT_DEBUG("Fifo device open failure\n");
        return RK_ERROR;
    }
    ret = fifoDev_SetTotalSize(hFifoPhone, 1024*1024*1024);
    if (ret != RK_SUCCESS)
    {
        BT_DEBUG("fifo dev set size failure");
        return RK_ERROR;
    }
#endif
    RKTaskCreate(TASK_ID_MEDIA, 0, NULL, SYNC_MODE);
    RKTaskCreate(TASK_ID_BT_PHONE_VOICE, 0, NULL, SYNC_MODE);
    SET_BTSTATE(BTState, BT_PLAY_CALL);
    MainTask_SetStatus(MAINTASK_APP_BT_PLAYER, 1);
}
_ATTR_BLUETOOTHCONTROL_CODE_
void phone_music_player_stop(void)
{
    BT_DEBUG("phone_music_player_stop");
    FIFO_DEV_ARG stFifoArg;
    CLEAR_BTSTATE(BTState, BT_PLAY_CALL);
    MainTask_SetStatus(MAINTASK_APP_BT_PLAYER, 0);
    RKTaskDelete(TASK_ID_BT_PHONE_VOICE, 0, SYNC_MODE);
    BT_DEBUG("bt phone voice delete ok!\n");
    RKTaskDelete(TASK_ID_MEDIA, 0, SYNC_MODE);
    BT_DEBUG("media task delete ok!\n");
    RKDev_Close(hFifoPhone);
    stFifoArg.ObjectId = 0;
    DeviceTask_DeleteDeviceList(DEVICE_LIST_FIFO, &stFifoArg, SYNC_MODE);
}
#endif
#if 0//def BT_VOICENOTIFY
_ATTR_BLUETOOTHCONTROL_DATA_ uint16 voiceId = 0;
_ATTR_BLUETOOTHCONTROL_DATA_ static HDC hFifoVoiceNofity = NULL;
_ATTR_BLUETOOTHCONTROL_DATA_ static uint32 repeatTime = 0;
_ATTR_BLUETOOTHCONTROL_DATA_ uint8 Buf[4096];
_ATTR_BLUETOOTHCONTROL_DATA_ static void* btVoiceNofityHandle = NULL;
_ATTR_BLUETOOTHCONTROL_DATA_ static HDC hAudio = NULL;
_ATTR_BLUETOOTHCONTROL_DATA_ BOOL playStop = FALSE;
_ATTR_BLUETOOTHCONTROL_CODE_
void stopVoiceNotify(void)
{
#ifdef BT_VOICENOTIFY
#if 1
    if (playStop)
    {
        DEBUG("stopVoiceNotify\n");
        playStop = FALSE;
        AudioDev_RealseMainTrack(hAudio);
        RKDev_Close(hAudio);
        RKTaskDelete2(btVoiceNofityHandle);
        RKTaskDelete(TASK_ID_MEDIA, 0, SYNC_MODE);
    }
#else
    if (TRUE == ThreadCheck(pMainThread, &MusicThread))
    {
        Codec_DACMute();
        while(AudioPlayInfo.VolumeCnt > 1)
        {
            AudioPlayInfo.VolumeCnt--;
            Codec_SetVolumet(AudioPlayInfo.VolumeCnt);
            DelayMs(5);
        }
        if(CurrentCodec == CODEC_BT_VOCIE_NOTIFY)
            ThreadDelete(&pMainThread, &MusicThread);
        ClearMsg(MSG_PLAY_VOICE_NOTIFY_FINISH);
    }
#endif
#endif
}
_ATTR_BLUETOOTHCONTROL_CODE_
void voiceNotifyService(void *arg)
{
    SEGMENT_INFO_T stCodeInfo;
    rk_err_t ret;
    uint32 voiceLen, imageBase, offset = 0;
    FW_GetSegmentInfo(voiceId, &stCodeInfo);
    imageBase = stCodeInfo.CodeImageBase;
    voiceLen = stCodeInfo.CodeImageLength;
    DEBUG("image length=%d\n", voiceLen);
    while(1)
    {
        offset = 0;
        DEBUG("repeat\n");
        while(offset < voiceLen)
        {
            FW_ReadFirmwaveByByte(imageBase+offset, Buf, 1024);
            offset += 1024;
            {
                int16 *monoBuf = NULL;
                int32 i, j;
                monoBuf = (int16 *)(Buf);
                for (i = 0, j = 0; i < 512; ++i)
                {
                    stereoBuf[j++] = monoBuf[i];
                    stereoBuf[j++] = monoBuf[i];
                }
            }
            fifoDev_Write(hFifoPhone, (uint8*)stereoBuf, 2048, ASYNC_MODE, NULL);
        }
        if (repeatTime == PALY_TIME_ONCE)
            break;
    }
}
_ATTR_BLUETOOTHCONTROL_CODE_
void playVoiceNotify(uint16 VoiceId, uint16 mode, uint32 repeat_time)
{
#ifdef BT_VOICENOTIFY
    DEBUG("playVoiceNotify");
    stopVoiceNotify();
#if 1
    FIFO_DEV_ARG stFifoArg;
    rk_err_t ret;
    FILE_ATTR stFileAttr;
    voiceId = VoiceId;
    repeatTime = repeat_time;
    playStop = TRUE;
    stFifoArg.BlockCnt = 20;
    stFifoArg.BlockSize = 1024;
    stFifoArg.UseFile = 0;
    stFifoArg.ObjectId = 0;
    stFifoArg.hReadFile = NULL;
    ret = DeviceTask_CreateDeviceList(DEVICE_LIST_FIFO, &stFifoArg, SYNC_MODE);
    if (ret != RK_SUCCESS)
    {
        BT_DEBUG("fifo dev create failure");
        return;
    }
    BT_DEBUG("fifo dev create success");
    hFifoPhone = RKDev_Open(DEV_CLASS_FIFO, 0, NOT_CARE);
    if ((hFifoPhone == NULL) || (hFifoPhone == (HDC)RK_ERROR) || (hFifoPhone == (HDC)RK_PARA_ERR))
    {
        BT_DEBUG("Fifo device open failure\n");
        return;
    }
    fifoDev_SetTotalSize(hFifoPhone, 1024*1024*1024);
    RKTaskCreate(TASK_ID_MEDIA, 0, NULL, SYNC_MODE);
    btVoiceNofityHandle = RKTaskCreate2(voiceNotifyService, NULL, NULL, "bt_voice_notify",
                                        TASK_PRIORITY_BLUETOOTH_CTRL_STACK_SIZE, TASK_PRIORITY_MEDIA-1, NULL);
#else
    if (TRUE != ThreadCheck(pMainThread, &MusicThread))
    {
        AUDIO_THREAD_ARG AudioArg;
        ModuleOverlay(MODULE_ID_AUDIO_CONTROL, MODULE_OVERLAY_ALL);
        ClearMsg(MSG_PLAY_VOICE_NOTIFY_FINISH);
        AudioArg.pAudioOpsInit = BTVoiceNotifyInit;
        AudioArg.FileNum = VoiceId;
        ThreadCreat(&pMainThread, &MusicThread, &AudioArg);
        VoiceRepeatTime = (repeat_time == 0) ? 1 : repeat_time;
        if(mode == PLAY_MODE_BLOCK)
        {
            while(1)
            {
                if(CheckAndStopVoiceNotify() == TRUE)
                {
                    break;
                }
            }
        }
    }
#endif
#else
    SendMsg(MSG_PLAY_VOICE_NOTIFY_FINISH);
#endif
}
#endif
_ATTR_BLUETOOTHCONTROL_CODE_
void BlueToothControlService(void *arg)
{
    int ret = 0;
    uint16 avdtp_status;
    uint8  call_status;
    uint8  call_setup_status;
    uint32 cmd;

    if(BlueToothControlInit(NULL, NULL) != RK_SUCCESS)
    {
        BT_DEBUG("BlueToothControlInit fail\n");
        gSysConfig.BtOpened = -1;
        bt_ctrl_task_exit = 1;
        while(1)
        {

            rkos_delay(5000);

        }
    }

//   if(CHECK_BTSTATE(BTState,BT_ACL_CONNECTED))
//   {
//     bt_read_rssi((struct bd_addr *)&gSysConfig.BtConfig.LastConnectMac, rssi_result);
//      DelayMs(200);
//   }

    #if 0
    if( BTA2DPStatus == BTAUDIO_STATE_STOP)
    {
        music_player_start();
        BTA2DPStatus = BTAUDIO_STATE_PLAY;
    }
    #endif

    gSysConfig.BtOpened = 1;

#ifdef ENABLE_NFC
    if(CHECK_BTSTATE(BTState, BT_NFC_INT_LOCK))
    {
        uint32 systicknow;
        systicknow = GetSysTick();

        if(systicknow > BTNfcUnLockTick + 500)
        {
            CLEAR_BTSTATE(BTState, BT_NFC_INT_LOCK);
        }
    }

    if(TRUE == GetMsg(MSG_BLUETOOTH_NFC_INT_COMING))
    {
        if(BlueToothHwGetPowerStatus() == POWER_STATUS_SLEEP)
        {
            BlueToothHwWakeup(NULL);
            if(gSysConfig.BtConfig.btConnected)
            {
                SET_BTSTATE(BTState,BT_NFC_WAIT_CONNECTING);
            }
        }
        else
        {
            if(gSysConfig.BtConfig.btConnected)
            {
                bt_disconnect((struct bd_addr *)&gSysConfig.BtConfig.LastConnectMac);

                DelayMs(200);

                gSysConfig.BtConfig.btConnected = 0;
            }

            SendMsg(MSG_BLUETOOTH_GO_TO_WAIT_PAIR);
        }
    }
#endif
    BT_DEBUG("BlueToothControlService\n");
    while(1)
    {
        rkos_queue_receive(bt_ctrl_queue, &cmd, MAX_DELAY);
        switch(cmd)
        {
            case MSG_EXIT_BLUETOOTH:
                goto exit;
                break;
            case MSG_PLAY_VOICE_NOTIFY_ACL_DISCONNECTED:
                //playVoiceNotify(SEGMENT_ID_VOICE_WAIT_PAIR,PALY_MODE_NO_BLOCK, PALY_TIME_ONCE );
                break;

            case MSG_PLAY_VOICE_WAIT_PAIR:
                //playVoiceNotify(SEGMENT_ID_VOICE_WAIT_PAIR, PALY_MODE_NO_BLOCK, PALY_TIME_ONCE );
                break;

            case MSG_BLUETOOTH_ACL_CONNECTED:
                gSysConfig.BtConfig.btConnected = 1;  //by wp 2015-09-23

                SET_BTSTATE(BTState,BT_ACL_CONNECTED);
                BlueToothHwSleepDisable(NULL);
                bt_discoverable_disable();
                BT_DEBUG("MSG_BLUETOOTH_ACL_CONNECTED\n");
//        #ifdef ENABLE_PAIR_TIMER
//        SystickTimerReStart(&blueToothWaitPairedTimer);
//        #endif
                break;

            case MSG_BLUETOOTH_ACL_DISCONNECTED:
#if 0   //by wp 2015-09-23
                if (TRUE == ThreadCheck(pMainThread, &MusicThread))
                {
                    ThreadDelete(&pMainThread, &MusicThread);

                }
#endif
                if (BTA2DPStatus != BTAUDIO_STATE_STOP)
                {
                    music_player_stop();
                    BTA2DPStatus = BTAUDIO_STATE_STOP;
                }
                BTADStatus =   BT_AD_STATE_STOP;
                BTState      = 1<< BT_CONNECTING;
                BTAvcrpStatus = AVCRP_STATE_PAUSE;
                HciServeIsrDisable();
                bt_buf_reset();
                HciServeIsrEnable();

                SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
                SendMsg(MSG_BLUETOOTH_CONNECTING);

#ifdef   ENABLE_PAIR_TIMER
                rkos_start_timer(blueToothWaitPairedTimer);
#endif

                CLEAR_BTSTATE(BTState,BT_ACL_CONNECTED);

                gSysConfig.BtConfig.btConnected =0;  //by wp 2015-09-23
                bt_discoverable_enable();
                rkos_start_timer(blueToothAclDisConnectedTimer);
                BT_DEBUG("MSG_BLUETOOTH_ACL_DISCONNECTED\n");

                if (bt_timeout_flag && bt_timeout_retry > 0)
                {
                    bt_timeout_retry--;
                    bt_connect_last_dev();
                }

                //苹果手机连接过程中会中途断开重连
                break;
#ifdef _AVRCP_
            case MSG_BLUETOOTH_AVRCP_CONNECTED:
                if(CHECK_BTSTATE(BTState,BT_WAIT_A2DP_AVRCP_CONNECTED))
                {
                    CLEAR_BTSTATE(BTState,BT_WAIT_A2DP_AVRCP_CONNECTED);
                    BlueToothControlStart(BT_A2DP_PLAY);
                    bt_buf_reset();
                    if(BTA2DPStatus == BTAUDIO_STATE_PLAY)
                    {
                        BTA2DPStatus = BTAUDIO_STATE_PAUSE;
                    }
                }
                SET_BTSTATE(BTState,BT_AVRCP_CONNECTED);
                break;
#endif

#ifdef _HFP_
            case MSG_BLUETOOTH_CALL_STATUS_CHANGE:
                {
                    call_status = hfp_get_call_status();
                    BT_DEBUG("call status=%d", call_status);
                    switch(call_status)
                    {
                        case HFP_NO_CALL:
                            {
                                if(CHECK_BTSTATE(BTState,BT_SCO_CONNECTED))
                                {
                                    SendMsg(MSG_BLUETOOTH_DISPLAY_CALL_STOP);
                                }
                                if(hfp_get_In_Band_Ringing_status() == HFP_SERVICE_NO_In_Band)
                                {
                                    //stopVoiceNotify();
                                }

                                CLEAR_BTSTATE(BTState,BT_CALL);
                                CLEAR_BTSTATE(BTState,BT_CALL_PLAYING);

                                if(CHECK_BTSTATE(BTState,BT_SCO_MUSIC_PLAY_HOLD))
                                {
                                    CLEAR_BTSTATE(BTState, BT_SCO_MUSIC_PLAY_HOLD);
                                    //SET_BTSTATE(BTState, BT_SCO_MUSIC_PLAY);
                                    SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
                                    SendMsg(MSG_BLUETOOTH_A2DP_START);

                                    if (BTCallStatus != BTAUDIO_STATE_STOP)
                                    {
                                        phone_music_player_stop();
                                        BTCallStatus = BTAUDIO_STATE_STOP;
                                    }
                                }

                                //电话挂断时，sco链路不会同时断开，例如在对方先挂断的时候，还是继续播放嘟嘟声

                            }
                            break;

                        case HFP_CALL_PLAYING:
                            {
                                if(hfp_get_In_Band_Ringing_status() == HFP_SERVICE_NO_In_Band)
                                {
                                    //stopVoiceNotify(); //wp 20150929
                                }

                                SET_BTSTATE(BTState,BT_CALL_PLAYING);

                                if(BTA2DPStatus != BTAUDIO_STATE_STOP)
                                {
                                    BTA2DPStatus = BTAUDIO_STATE_STOP;
                                    music_player_stop();
                                }

                                if(CHECK_BTSTATE(BTState,BT_SCO_MUSIC_PLAY))
                                {
                                    CLEAR_BTSTATE(BTState, BT_SCO_MUSIC_PLAY);
                                    SET_BTSTATE(BTState, BT_SCO_MUSIC_PLAY_HOLD);

                                    if (BTCallStatus != BTAUDIO_STATE_STOP)
                                    {
                                        phone_music_player_stop();
                                        BTCallStatus = BTAUDIO_STATE_STOP;
                                    }
                                }
                                else
                                {
                                    HciServeIsrDisable();
                                    bt_buf_reset();
                                    HciServeIsrEnable();
                                }
                                SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
                                SendMsg(MSG_BLUETOOTH_DISPLAY_CALL_PLAY);
                            }
                            break;

                        default:
                            break;
                    }
                }
                break;

            case MSG_BLUETOOTH_CALL_SETUP_STATUS_CHANGE:
                {
                    call_setup_status = hfp_get_callsetup_status();
                    BT_DEBUG("call_setup_status=%d", call_setup_status);

                    switch(call_setup_status)
                    {
                        case HFP_CALL_NO_SET_UP:
                            {
                                if(!CHECK_BTSTATE(BTState,BT_CALL_PLAYING) && CHECK_BTSTATE(BTState, BT_CALL))  //电话接听后，来电状态也会清0
                                {
                                    //stopVoiceNotify();//wp 20150929
                                    SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
                                    SendMsg(MSG_BLUETOOTH_CONNECTED);
                                    CLEAR_BTSTATE(BTState,BT_CALL);
                                    if(hfp_get_In_Band_Ringing_status() == HFP_SERVICE_NO_In_Band)
                                    {
                                        if(CHECK_BTSTATE(BTState,BT_A2DP_PLAYING_HOLD))
                                        {
                                            CLEAR_BTSTATE(BTState,BT_A2DP_PLAYING_HOLD);
                                            SET_BTSTATE(BTState,BT_A2DP_PLAYING); //代表A2DP播放被Call打断
                                        }
                                    }
                                }
                            }
                            break;

                        case HFP_CALL_INCOMING_SET_UP:
                            {
                                if(BTA2DPStatus != BTAUDIO_STATE_STOP)
                                {
                                    BTA2DPStatus = BTAUDIO_STATE_STOP;
                                    music_player_stop();
                                }

                                if(!CHECK_BTSTATE(BTState, BT_CALL))
                                {
                                    SET_BTSTATE(BTState,BT_CALL);

                                    if(CHECK_BTSTATE(BTState,BT_A2DP_PLAYING))
                                    {
                                        CLEAR_BTSTATE(BTState,BT_A2DP_PLAYING);
                                        SET_BTSTATE(BTState,BT_A2DP_PLAYING_HOLD); //代表A2DP播放被Call打断
                                    }
                                    HciServeIsrDisable();
                                    bt_buf_reset();
                                    HciServeIsrEnable();
                                }

                                if(hfp_get_In_Band_Ringing_status() == HFP_SERVICE_NO_In_Band)
                                {
                                    //playVoiceNotify(VOICE_ID_CALL_COMING, PALY_MODE_NO_BLOCK, PALY_TIME_ALWAYS);
                                }

                                SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
                                SendMsg(MSG_BLUETOOTH_DISPLAY_CALL_COMING);
                            }

                            break;

                        case HFP_CALL_OUTGOING_SET_UP:
                            break;

                        case HFP_CALL_OUTGOING_ALERT:
                            break;

                        default:
                            break;
                    }
                }
                break;

            case MSG_BLUETOOTH_CALL_RING:
                break;

            case MSG_BLUETOOTH_HFP_CONNECTED:
                {
                    SET_BTSTATE(BTState,BT_HFP_CONNECTED);
                    if(!CHECK_BTSTATE(BTState,BT_A2DP_CONNECTED))
                    {
                        //playVoiceNotify(VOICE_ID_PAIRED, PALY_MODE_NO_BLOCK, PALY_TIME_ONCE );
                    }

                    SendMsg(MSG_BLUETOOTH_CONNECTED);
                }
                break;

            case MSG_BLUETOOTH_HFP_GET_PHONE_NO:
                {
                    char phonenum[32];
                    if(NULL == hsp_hfp_get_phone_num(phonenum,32))
                    {

                    }
                    else
                    {
                        //DisplayTestString(0,0, phonenum);

                        BT_DEBUG("phonenum = %s\r\n", phonenum);
                        //获取到来电号码
                    }
                }
                break;

            case MSG_BLUETOOTH_SCO_CONNECTED: //SCO的建立，跟CALL没有关系， 不支持A2DP设备 ，音乐将从sco走
                {
                    SET_BTSTATE(BTState,BT_SCO_CONNECTED);
                    BT_DEBUG("MSG_BLUETOOTH_SCO_CONNECTED");
                    if(BTA2DPStatus != BTAUDIO_STATE_STOP)
                    {
                        BTA2DPStatus = BTAUDIO_STATE_STOP;
                        music_player_stop();
                    }

                    if(CHECK_BTSTATE(BTState,BT_A2DP_PLAYING))
                    {
                        CLEAR_BTSTATE(BTState,BT_A2DP_PLAYING);
                        SET_BTSTATE(BTState,BT_A2DP_PLAYING_HOLD); //代表A2DP播放被Call打断
                    }

                    HciServeIsrDisable();
                    bt_buf_reset();
                    HciServeIsrEnable();
                }
                break;

            case MSG_BLUETOOTH_SCO_DISCONNECTED:
                {
                    if(CHECK_BTSTATE(BTState,BT_SCO_CONNECTED))
                    {
                        if(BTCallStatus != BTAUDIO_STATE_STOP)
                        {
                            BTCallStatus = BTAUDIO_STATE_STOP;
                            phone_music_player_stop();
                        }

                        if(CHECK_BTSTATE(BTState,BT_A2DP_PLAYING_HOLD))
                        {
                            CLEAR_BTSTATE(BTState,BT_A2DP_PLAYING_HOLD);
                            SET_BTSTATE(BTState,BT_A2DP_PLAYING); //代表A2DP播放被Call打断
                        }

                        HciServeIsrDisable();
                        bt_buf_reset();
                        HciServeIsrEnable();

                        CLEAR_BTSTATE(BTState,BT_SCO_CONNECTED);
                        CLEAR_BTSTATE(BTState,BT_SCO_MUSIC_PLAY);
                        //stopVoiceNotify(); //wp 20150929
                        SendMsg(MSG_BLUETOOTH_DISPLAY_ALL);
                        SendMsg(MSG_BLUETOOTH_CONNECTED);
                    }
                }
                break;

#endif


#ifdef _AVDTP_
            case MSG_BLUETOOTH_A2DP_CONNECTED:
                {
                    BT_DEBUG("MSG_BLUETOOTH_A2DP_CONNECTED\n");
                    SET_BTSTATE(BTState, BT_A2DP_CONNECTED);
                    if(!CHECK_BTSTATE(BTState,BT_HFP_CONNECTED))
                    {
                        //playVoiceNotify(VOICE_ID_PAIRED, PALY_MODE_NO_BLOCK, PALY_TIME_ONCE );//wp 20150929
                    }
                    SendMsg(MSG_BLUETOOTH_CONNECTED);
                }
                break;

            case MSG_BLUETOOTH_A2DP_STREAMING:
                {
                    if(CHECK_BTSTATE(BTState,BT_CALL))
                    {
                        SET_BTSTATE(BTState, BT_A2DP_RINGING);
                    }

                    //音频流接收，立刻关掉sleep
                    SET_BTSTATE(BTState, BT_A2DP_STREAMING);
                }
                break;

            case MSG_BLUETOOTH_A2DP_CLOSE:
                {
                    if(BTA2DPStatus != BTAUDIO_STATE_STOP)
                    {
                        BTA2DPStatus = BTAUDIO_STATE_STOP;
                        music_player_stop();// by yh 2016-03-28
                    }

                    CLEAR_BTSTATE(BTState, BT_A2DP_STREAMING);
                    CLEAR_BTSTATE(BTState, BT_A2DP_PLAYING);
                    CLEAR_BTSTATE(BTState, BT_A2DP_CONNECTED);
                    bt_buf_reset();
                }
                break;

            case MSG_BLUETOOTH_A2DP_ABORT:
                {
                    if(BTA2DPStatus != BTAUDIO_STATE_STOP)
                    {
                        BTA2DPStatus = BTAUDIO_STATE_STOP;
                        music_player_stop();// by yh 2016-03-28
                    }

                    CLEAR_BTSTATE(BTState, BT_A2DP_STREAMING);
                    CLEAR_BTSTATE(BTState, BT_A2DP_PLAYING);
                    CLEAR_BTSTATE(BTState, BT_A2DP_CONNECTED);
                    bt_buf_reset();
                }
                break;

            case MSG_BLUETOOTH_A2DP_SUSPEND:
                {

                    CLEAR_BTSTATE(BTState, BT_A2DP_STREAMING);
                }
                break;

            case MSG_BLUETOOTH_GET_VAD:
                {
                    BTA2DPStatus = BTAUDIO_STATE_PAUSE;
                    //AudioPause();
                    //printf("MSG_BLUETOOTH_GET_VAD\n");
                    bt_buf_reset();
                }
                break;
#endif

            case MSG_CHECK_TIMER:
                bt_time_serve();
#ifdef _HFP_
                if(CHECK_BTSTATE(BTState, BT_SCO_CONNECTED))
                {
                    if( BTCallStatus == BTAUDIO_STATE_STOP)
                    {
                        phone_music_player_start();
                        BTCallStatus = BTAUDIO_STATE_PLAY;
                        if(!CHECK_BTSTATE(BTState,BT_CALL) && !CHECK_BTSTATE(BTState,BT_CALL_PLAYING))
                        {
                            SET_BTSTATE(BTState,BT_SCO_MUSIC_PLAY);
                            SendMsg(MSG_BLUETOOTH_A2DP_START);
                        }
                    }
                }
#endif

#ifdef _AVDTP_
                if(CHECK_BTSTATE(BTState, BT_A2DP_CONNECTED) && (!CHECK_BTSTATE(BTState, BT_CALL_PLAYING))
                        && (!CHECK_BTSTATE(BTState, BT_SCO_CONNECTED)) && (!CHECK_BTSTATE(BTState, BT_CALL)))
                {
                    if( BTA2DPStatus == BTAUDIO_STATE_STOP)
                    {
                        music_player_start();
                        //phone_music_player_start();
                        BTA2DPStatus = BTAUDIO_STATE_PLAY;
                        SET_BTSTATE(BTState, BT_A2DP_PLAYING);
                        SendMsg(MSG_BLUETOOTH_A2DP_START);
                    }
                }
#endif
                BlueToothHwService();

                if(CHECK_BTSTATE(BTState,BT_GET_POWER_EVENT))
                {
                    CLEAR_BTSTATE(BTState,BT_GET_POWER_EVENT);

                    switch(PowerEvent)
                    {

                        case POWER_WILL_WAKE_UP_CFM:

                            CLEAR_BTSTATE(BTState, BT_IN_SLEEP);
#ifdef ENABLE_DEEP_SLEEP

                            BlueToothHwSleepDisable();
#endif
                            phybusif_send_waitlist();

                            if(CHECK_BTSTATE(BTState, BT_WAIT_PALY_NOTIFY))
                            {
                                CLEAR_BTSTATE(BTState, BT_WAIT_PALY_NOTIFY);
                                SendMsg(MSG_PLAY_VOICE_WAIT_PAIR);
                            }

#ifdef ENABLE_NFC
                            if(CHECK_BTSTATE(BTState, BT_NFC_WAIT_CONNECTING))
                            {
                                CLEAR_BTSTATE(BTState, BT_NFC_WAIT_CONNECTING);

                                if(gSysConfig.BtConfig.btConnected)
                                {
                                    bt_disconnect((struct bd_addr *)&gSysConfig.BtConfig.LastConnectMac);

                                    DelayMs(200);

                                    gSysConfig.BtConfig.btConnected = 0;
                                }

                                SendMsg(MSG_BLUETOOTH_GO_TO_WAIT_PAIR);
                            }
#endif
                            break;

                        case  POWER_WILL_SLEEP:
                            SET_BTSTATE(BTState, BT_IN_SLEEP);
                            // Os_idle();

                            break;

                        case POWER_WILL_WAKE_UP_IND:

                            CLEAR_BTSTATE(BTState, BT_IN_SLEEP);

                            break;
                    }
                }


                break;

#ifdef   ENABLE_PAIR_TIMER
            case MSG_BLUETOOTH_GO_TO_WAIT_PAIR:
                {
                    if(CHECK_BTSTATE(BTState, BT_IN_SLEEP))
                    {
                        SET_BTSTATE(BTState, BT_WAIT_PALY_NOTIFY);
                    }
                    else
                    {
                        SendMsg(MSG_PLAY_VOICE_WAIT_PAIR);
                    }
                    //BlueToothControlReInit();
#ifdef ENABLE_DEEP_SLEEP

                    BlueToothHwSleepDisable();

#endif

                    bt_discoverable_enable();

#ifdef ENABLE_PAIR_TIMER
                    //SystickTimerStart(&blueToothWaitPairedTimer);
                    rkos_start_timer(blueToothWaitPairedTimer);
#endif
                }
                break;
#endif
            default:
#ifdef _OPP_
                blueTooth_opp_Service();
#endif
                break;

        }

    }
exit:
    BlueToothControlDeInit();
    while(1)
    {
        rkos_delay(2000);
    }
}

/*
--------------------------------------------------------------------------------
  Function name :  void BlueToothControlDeInit(void)
  Author        :  wangping
  Description   :

  Input         :  null
  Return        :  TRUE/FALSE
  History       :  <author>         <time>         <version>
                    wangping        2015/10/15         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_BLUETOOTHCONTROL_CODE_
rk_err_t BlueToothControlDeInit(void)
{
    bt_disconnect((struct bd_addr *)&gSysConfig.BtConfig.LastConnectMac);
    rkos_delay(200);
    music_player_stop();
#ifdef _HFP_
    phone_music_player_stop();
#endif
    BT_DEBUG();
    BlueToothHwPowerOff();
#ifdef ENABLE_PAIR_TIMER
    rkos_stop_timer(blueToothWaitPairedTimer);
    rkos_delete_timer(blueToothWaitPairedTimer);
#endif
#ifdef ENABLE_NFC
    IntDisable(INT_ID26_GPIO);
    IntPendingClear(INT_ID26_GPIO);
    IntUnregister(INT_ID26_GPIO);
#endif
    rkos_stop_timer(blueToothAclDisConnectedTimer);
    rkos_delete_timer(blueToothAclDisConnectedTimer);
    rkos_stop_timer(blueToochCheckTimer);
    rkos_delete_timer(blueToochCheckTimer);
    rkos_queue_delete(bt_ctrl_queue);
    FREQ_ExitModule(FREQ_BT);
    bt_ctrl_task_exit = 1;
    bt_ctrl_queue = NULL;
    //FW_RemoveSegment(SEGMENT_ID_BLUETOOTH);
    return RK_SUCCESS;
}

_ATTR_BLUETOOTHCONTROL_CODE_
rk_err_t bluetooth_start(void)
{
    rk_err_t ret;

    FW_LoadSegment(SEGMENT_ID_BLUETOOTH_LWBT, SEGMENT_OVERLAY_ALL);

    gSysConfig.BtConfig.btCtrlTaskRun = 0;
    if(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) != NULL)
    {
        return RK_ERROR;
    }

    BT_DEBUG("bluetooth_start = %d\n",rkos_GetFreeHeapSize());
    //memory_leak_check_start();
    bt_ctrl_task_handle = RKTaskCreate2(BlueToothControlService, NULL,NULL,"bt_ctrl",
                                        TASK_PRIORITY_BLUETOOTH_CTRL_STACK_SIZE, TASK_PRIORITY_BLUETOOTH_CTRL,NULL);

    gSysConfig.BtConfig.btCtrlTaskRun = 1;
    return RK_SUCCESS;

}

_ATTR_BLUETOOTHCONTROL_CODE_
void bluetooth_stop(void)
{
    //BlueToothControlDeInit();
    if(gSysConfig.BtConfig.btCtrlTaskRun)
    {
        SendMsg(MSG_EXIT_BLUETOOTH);
        while (bt_ctrl_task_exit != 1)
        {
            rkos_delay(10);
        }
        bt_ctrl_task_exit = 0;
        if(bt_ctrl_task_handle)
        {
            RKTaskDelete2(bt_ctrl_task_handle);
            bt_ctrl_task_handle = NULL;
        }
        BT_DEBUG("bluetooth_stop = %d \n",rkos_GetFreeHeapSize());
        gSysConfig.BtConfig.btCtrlTaskRun = 0;
        gSysConfig.BtOpened = 0;
    }

    FW_RemoveSegment(SEGMENT_ID_BLUETOOTH_LWBT);
    //memory_leak_check_watch();
}

#ifdef _BLUETOOTH_SHELL_


_ATTR_BLUETOOTH_SHELL_
static SHELL_CMD ShellBlueToothName[] =
{
    "open",NULL,"NULL","NULL",
    "close",NULL,"NULL","NULL",
    "play",NULL,"NULL","NULL",
    "pause",NULL,"NULL","NULL",
    "next",NULL,"NULL","NULL",
    "stop",NULL,"NULL","NULL",
    "prev",NULL,"NULL","NULL",
    "\b",NULL,"NULL","NULL",
};

/*******************************************************************************
** Name: KeyDev_Shell
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2015.6.24
** Time: 15:07:41
*******************************************************************************/
_ATTR_BLUETOOTH_SHELL_
SHELL API rk_err_t BlueTooth_Shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret;
    uint8 Space;
    printf("BlueTooth_Shell\n");
    StrCnt = ShellItemExtract(pstr, &pItem, &Space);
    if (StrCnt == 0)
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellBlueToothName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;
    switch (i)
    {
        case 0x00:
            FW_LoadSegment(SEGMENT_ID_BLUETOOTH, SEGMENT_OVERLAY_ALL);
            FW_LoadSegment(SEGMENT_ID_BLUETOOTH_LWBT, SEGMENT_OVERLAY_ALL);
            bluetooth_start();
            break;

        case 0x01:
            bluetooth_stop();
            FW_RemoveSegment(SEGMENT_ID_BLUETOOTH);
            FW_RemoveSegment(SEGMENT_ID_BLUETOOTH_LWBT);
            break;

        case 0x02:
            // play
            ct_play();
            break;

        case 0x03:
            // pause
            ct_pause();
            break;

        case 0x04:
            // next
            ct_next();
            break;

        case 0x05:
            // stop
            ct_stop();
            break;

        case 0x06:
            // prev
            ct_previous();
            break;

        default:
            ret = RK_ERROR;
            break;
    }
    return ret;

}
#endif

#endif
#endif

/*
********************************************************************************
*
*                         End of BlueToothControl.c
*
********************************************************************************
*/




