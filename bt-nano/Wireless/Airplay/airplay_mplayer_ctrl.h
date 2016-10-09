#ifndef _AIRPLAY_CONTROLLER_H_
#define _AIRPLAY_CONTROLLER_H_

typedef struct airplay_ctrl_s
{
    int (*init)(void);
    int (*deinit)(void);
    int (*set_volume)(int volume);
} airplay_ctrl_t;

#define AIRPLAY_CMD_DECSTART 0
#define AIRPLAY_CMD_STOP     1

int Airplay_MplayerCtrl_Init(void);
int Airplay_MplayerCtrl_Deinit(void);
void Airplay_SendCmd(int cmd);
int Airplay_WriteData(unsigned char * pbuf, unsigned int size);
int Airplay_SetFifoSize(int TotalSize);
int Airplay_FlusData(void);
#endif
