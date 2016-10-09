/*
*********************************************************************************************************
*                                       NANO_OS The Real-Time Kernel
*                                         FUNCTIONS File for V0.X
*
*                                    (c) Copyright 2013, RockChip.Ltd
*                                          All Rights Reserved
*File    : BSP.C
* By     : Zhu Zhe
*Version : V0.x
*
*********************************************************************************************************
*/
#include "BspConfig.h"
#include "RKOS.h"
#include "SysTickHandler.h"


/*
*********************************************************************************************************
*                                        Macro Define
*********************************************************************************************************
*/


#define _BSP_EVK_V20_BSP2_READ_  __attribute__((section("bsp_evk_v20_bsp2_read")))
#define _BSP_EVK_V20_BSP2_WRITE_ __attribute__((section("bsp_evk_v20_bsp2_write")))
#define _BSP_EVK_V20_BSP2_INIT_  __attribute__((section("bsp_evk_v20_bsp2_init")))
#define _BSP_EVK_V20_BSP2_SHELL_  __attribute__((section("bsp_evk_v20_bsp2_shell")))


/*
*********************************************************************************************************
*                                        Variable Define
*********************************************************************************************************
*/
extern uint32 SysTickCounter2;
extern chip_freq_t chip_freq2;

extern void SysTickDisable2(void);
extern void SysTickClkSourceSet2(UINT32 Source);
extern void SysTickPeriodSet2(UINT32 mstick );
extern void SysTickEnable2(void);
extern void DelayMs2(UINT32 ms);



void debug_hex(char *cmd, int cmd_siz, int radix)
{

	int i;

	for (i=0; i<cmd_siz; i++) {
		if ((i>0)&&((i % 10) == 0))
			rk_print_string2("\r\n");

		if (radix == 10)
			rk_print_string2(" %d", cmd[i]);
		else
		 	rk_print_string2(" %02x", cmd[i]);
	}
	rk_print_string2("\r\n\r\n");

}

uint32 StrLenW2(uint16 * pstr)
{
    uint32 i = 0;
    while (*pstr++ != 0)
        i++;

    return i;
}

uint32 StrLenA2(uint8 * pstr)
{
    uint32 i = 0;
    while (*pstr++ != 0)
        i++;

    return i;
}

int32 StrCmpW2(uint16 * pstr1, uint16 * pstr2, uint32 len)
{
    uint32 len1, len2;

    if(len == 0)
    {
        len1 = StrLenW2(pstr1);
        len2 = StrLenW2(pstr2);

        if(len1 > len2)
        {
            return 1;
        }
        else if(len1 < len2)
        {
            return -1;
        }
        else
        {
            len = len1;
        }
    }

    while (len--)
    {
        if ((*pstr1 == 0) && (*pstr2 == 0))
        {
            return 0;
        }
        else if (*pstr1 == 0)
        {
            return -1;
        }
        else if (*pstr2 == 0)
        {
            return 1;
        }
        if (*pstr1 > * pstr2)
        {
            return 1;
        }
        else if (*pstr1 < * pstr2)
        {
            return -1;
        }
        pstr1++;
        pstr2++;
    }

    return 0;

}

void MemCpy2(uint8 * trg, uint8 * src, uint32 len)
{
    uint32 i;

    for(i = 0; i < len; i++)
    {
        *trg++ = *src++;
    }
}
void MemMov2(uint8 * trg, uint8 * src, uint32 len)
{
    uint32 i;

    for(i = 0; i < len; i++)
    {
        *trg++ = *src++;
    }
}
void MemSet2(uint8 * trg, int  data, uint32 len)
{
    uint32 i;
    for(i = 0; i < len; i++)
    {
        *trg++ = data;
    }
}

int32 StrCmpA2(uint8 * pstr1, uint8 * pstr2, uint32 len)
{
    uint32 len1, len2;

    if(len == 0)
    {
        len1 = StrLenA2(pstr1);
        len2 = StrLenA2(pstr2);

        if(len1 > len2)
        {
            return 1;
        }
        else if(len1 < len2)
        {
            return -1;
        }
        else
        {
            len = len1;
        }
    }


    while (len--)
    {
        if ((*pstr1 == 0) && (*pstr2 == 0))
        {
            return 0;
        }
        else if (*pstr1 == 0)
        {
            return -1;
        }
        else if (*pstr2 == 0)
        {
            return 1;
        }
        if (*pstr1 > * pstr2)
        {
            return 1;
        }
        else if (*pstr1 < * pstr2)
        {
            return -1;
        }
        pstr1++;
        pstr2++;
    }

    return 0;

}

uint32 StringtoNum2(uint8 * pstr)
{
    uint32 num = 0;
    int32 i;

    /*ptr save the fisrt char of pstr*/
    const uint8* ptr = pstr;

    /*if pstr start with '-' or '+' sign,move to the next charactor.*/
    if(*pstr == '-' || *pstr == '+')
        pstr++;

     while(*pstr != 0)
     {
        /*if pstr is none digal number ,break and return*/
        if(*pstr < '0' || *pstr > '9')
            break;
        /*current is digal ,calculate the value*/
        num = num * 10 + (*pstr - '0');
        pstr++;
     }

     if( *ptr == '-')
        num = -num;

    return num;
}

rk_size_t Ascii2Unicode2(uint8 * pbAscii, uint16 * pwUnicode, uint32 len)
{
    uint32 i;

    for (i = 0; i < len / 2; i++)
    {
        pwUnicode[i] = (uint16)pbAscii[i];
    }
    return i * 2;
}

rk_size_t Unicode2Ascii2(uint8 * pbAscii, uint16 * pwUnicode, uint32 len)
{
    uint32 i;
    uint8 * pbUnicode = (uint8 *)pwUnicode;

    for (i = 0; i < len;)
    {
        if ((*pbUnicode == 0) && (*(pbUnicode + 1) == 0))
        {
            pbAscii[i] = 0;
            break;
        }

        if (*pbUnicode != 0)
        {
            pbAscii[i] = *pbUnicode;
            i++;
        }

        if (*(pbUnicode + 1) != 0)
        {
            pbAscii[i] = *(pbUnicode + 1);
            i++;
        }
        pbUnicode += 2;
    }

    return i;
}

/*
*********************************************************************************************************
*                               void SysVariableInit(void)
* Description:  This function is to init the system value.
*
* Argument(s) : none
* Return(s)   : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
void SysVariableInit2(void)
{
    SysTickCounter2   = 0;
}

/*
*********************************************************************************************************
*                                     void SysInterruptInit(void)
* Description:  This function is to init the interrut .
*
* Argument(s) : none
* Return(s)   : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
void SysInterruptInit2(void)
{
    //set priority.
    IrqPriorityInit2();
    //......

    //register interrupt functions.
    IntRegister2(FAULT_ID15_SYSTICK, (void*)SysTickHandler2);


    //interrupt enable.

    IntEnable2(FAULT_ID15_SYSTICK);

    IntMasterEnable2();

}

/*
*********************************************************************************************************
*                                     void SysTickInit(void)
* Description:  This function is to init the systick.
*
* Argument(s) : none
* Return(s)   : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
void SysTickInit2(void)
{
    SysTickDisable2();

    SysTickClkSourceSet2(NVIC_SYSTICKCTRL_CLKIN);
    SysTickPeriodSet2(10);
    SysTickEnable2();
}


#ifdef _LOG_DEBUG_
extern void BBDebugOk(void);
#endif

__irq void BBSystemBIsr(void)
{
    uint32 Cmd;
    uint32 Data;

    MailBoxClearA2BInt(MAILBOX_ID_0,  MAILBOX_INT_0);
    Cmd = MailBoxReadA2BCmd(MAILBOX_ID_0, MAILBOX_CHANNEL_0);
    Data = MailBoxReadA2BData(MAILBOX_ID_0, MAILBOX_CHANNEL_0);
    switch(Cmd)
    {
        case MSGBOX_CMD_SYSTEM_PRINT_LOG_OK:
            #ifdef _LOG_DEBUG_
            BBDebugOk();
            #endif
            break;
    }

}


void BSP_Init2(void)
{
    IntMasterDisable2();
    SysVariableInit2();
    SysInterruptInit2();

    MailBoxEnableA2BInt(MAILBOX_ID_0, MAILBOX_INT_0);
    IntRegister2(INT_ID_MAILBOX0, (void*)BBSystemBIsr);
    IntPendingClear2(INT_ID_MAILBOX0);
    IntEnable2(INT_ID_MAILBOX0);

    #ifdef _LOG_DEBUG_
    DebugInit2();
    #endif

    //bb_printf1("chip_freq2.hclk_cal_core =%d!\n", chip_freq2.hclk_cal_core);
    //bb_printf1("chip_freq2.stclk_cal_core =%d!\n", chip_freq2.stclk_cal_core);
    //bb_printf1("chip_freq2.fclk_cal_core =%d!\n", chip_freq2.fclk_cal_core);
    //bb_printf1("chip_freq2.hclk_high_pre =%d!\n", chip_freq2.hclk_high_pre);
    SysTickInit2();

    IntDisable2(FAULT_ID15_SYSTICK);

}
