#include "BspConfig.h"
#ifdef __WIFI_AIRPLAY_C__

#define NOT_INCLUDE_OTHER
#include "typedef.h"
#include "RKOS.h"
#include "Bsp.h"
#include "global.h"
#include "SysInfoSave.h"
#include "TaskPlugin.h"
#include "device.h"
#include "FIFODevice.h"
#include "FileDevice.h"
#include "DeviceManagerTask.h"
#include "AudioControlTask.h"
#include "airplay_mplayer_ctrl.h"

airplay_ctrl_t airplay_mplayer_ctrl;
HDC hAirplayFifo = NULL;

int Airplay_MplayerCtrl_Init(void)
{
    RK_TASK_AUDIOCONTROL_ARG pArg;
    FIFO_DEV_ARG stFifoArg;
    rk_err_t ret;

    #if 0
    stFifoArg.BlockCnt = 40;
    stFifoArg.BlockSize = 1024;
    stFifoArg.UseFile = 0;
    stFifoArg.ObjectId = 0;
    stFifoArg.hReadFile = NULL;

    printf("\n Airplay_MplayerCtrl_Init\n");
    #else

    FILE_ATTR stFileAttr;

    stFileAttr.Path = L"C:\\";
    stFileAttr.FileName = L"RkFifiDevice.buf";

    stFifoArg.hReadFile = FileDev_OpenFile(FileSysHDC, NULL, READ_WRITE, &stFileAttr);
    if ((int)stFifoArg.hReadFile <= 0)
    {
        ret = FileDev_CreateFile(FileSysHDC, NULL, &stFileAttr);
        if (ret != RK_SUCCESS)
        {
            rk_print_string("file create failure");
            while(1)
            {
                rkos_sleep(1000);
            }
        }

        stFifoArg.hReadFile = FileDev_OpenFile(FileSysHDC, NULL, READ_WRITE, &stFileAttr);
        if ((int)stFifoArg.hReadFile <= 0)
        {
            while(1)
            {
                rkos_sleep(1000);
            }
        }
    }


    stFifoArg.BlockCnt = 200000;
    //stFifoArg.BlockCnt = 50;
    stFifoArg.BlockSize = 1024;
    stFifoArg.UseFile = 1;
    stFifoArg.ObjectId = 0;

    #endif

    ret = DeviceTask_CreateDeviceList(DEVICE_LIST_FIFO, &stFifoArg, SYNC_MODE);
    if (ret != RK_SUCCESS)
    {
        rk_printf("airplay fifo dev create failure");
        return RK_ERROR;
    }

    pArg.ucSelPlayType = SOURCE_FROM_XXX;
    pArg.FileNum = 1;

    pArg.pfAudioState = NULL;

    RKTaskCreate(TASK_ID_AUDIOCONTROL, 0, &pArg, SYNC_MODE);

    hAirplayFifo = RKDev_Open(DEV_CLASS_FIFO,0,NOT_CARE);
    if ((hAirplayFifo == NULL) || (hAirplayFifo == (HDC)RK_ERROR) || (hAirplayFifo == (HDC)RK_PARA_ERR))
    {
        printf("Audio fifo device open failure\n");

        return RK_ERROR;
    }

    return 0;
}

int Airplay_MplayerCtrl_Deinit(void)
{
    FIFO_DEV_ARG stFifoArg;
    printf("\n Airplay_MplayerCtrl_Deinit\n");


    if(hAirplayFifo != NULL)
    {
       RKDev_Close(hAirplayFifo);
       hAirplayFifo = NULL;
    }

    stFifoArg.ObjectId = 0;
    RKTaskDelete(TASK_ID_AUDIOCONTROL, 0, SYNC_MODE);
    DeviceTask_DeleteDeviceList(DEVICE_LIST_FIFO, &stFifoArg, SYNC_MODE);

    return 0;
}

int Airplay_MplayerCtrl_Set_volume(int volume)
{
    printf("Airplay_MplayerCtrl_Set_volume = %d\n", volume);
    AudioControlTask_SendCmd(AUDIO_CMD_VOLUMESET, (void *)volume, ASYNC_MODE);
    return 0;
}

//int Airplay_Mplayer_Init(void)
//{
//    airplay_mplayer_ctrl.init = Airplay_MplayerCtrl_Init;
//    airplay_mplayer_ctrl.deinit = Airplay_MplayerCtrl_Deinit;
//    airplay_mplayer_ctrl.set_volume = Airplay_MplayerCtrl_Set_volume;
//}

void Airplay_SendCmd(int cmd)
{
    switch(cmd)
    {
        case AIRPLAY_CMD_DECSTART:
            AudioControlTask_SendCmd(AUDIO_CMD_DECSTART, NULL, ASYNC_MODE);
            break;

        case AIRPLAY_CMD_STOP:
            AudioControlTask_SendCmd(AUDIO_CMD_STOP, (void *)Audio_Stop_Force, SYNC_MODE);
            break;

        default:
            break;
    }
}

int Airplay_WriteData(unsigned char * pbuf, unsigned int size)
{
    int ret;
    ret = fifoDev_Write(hAirplayFifo, pbuf, size, SYNC_MODE, NULL);
    return ret;
}

int Airplay_FlusData(void)
{
    int ret;
    //ret = fifoDev_FlushBuf(hAirplayFifo);
    return ret;
}


int Airplay_SetFifoSize(int TotalSize)
{
    int ret;
    ret = fifoDev_SetTotalSize(hAirplayFifo, TotalSize);
    return ret;
}
#endif
