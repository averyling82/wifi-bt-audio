/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Cpu\NanoD\lib\hw_mp3_syn.c
* Owner: WJR
* Date: 2014.11.10
* Time: 17:37:32
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    WJR     2014.11.10     17:37:32   1.0
********************************************************************************************
*/
#include "BspConfig.h"
#ifdef __CPU_NANOD_LIB_HW_MP3_SYN_C__


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "interrupt.h"
#include "hw_mp3_syn.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _CPU_NANOD_LIB_HW_MP3_SYN_READ_  __attribute__((section("cpu_nanod_lib_hw_mp3_syn_read")))
#define _CPU_NANOD_LIB_HW_MP3_SYN_WRITE_ __attribute__((section("cpu_nanod_lib_hw_mp3_syn_write")))
#define _CPU_NANOD_LIB_HW_MP3_SYN_INIT_  __attribute__((section("cpu_nanod_lib_hw_mp3_syn_init")))
#define _CPU_NANOD_LIB_HW_MP3_SYN_SHELL_  __attribute__((section("cpu_nanod_lib_hw_mp3_syn_shell")))

#define UINT32 unsigned long

//用于开关中断
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/

struct mad_frame * synth_hw_frame;

mad_fixed_t (*synth_hw_sbsample)[36][32];      //input

unsigned int synth_hw_ch;

volatile unsigned int synth_hw_ns;

unsigned int synth_hw_s;

unsigned int synth_hw_phase;

volatile int synth_hw_busy;

int synth_hw_pcmout[32];                              //output

static volatile int dummy;

unsigned int synth_end_ch_number;

volatile int mp3_frame_cnt;

volatile int is_synthing;


volatile int synth_finish = 0;
volatile int synth_auto_output_addr;    //表示在做完子带合成运算后，自动将结果存放到哪里

long synth_out_samples_limit = 1152;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern volatile int synth_hw_busy;      //硬件子带合成模块忙标志
extern long synth_out_samples_limit;
extern long synth_out_samples_limit_change_flag ;
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
void mad_synth_mute(struct mad_synth *synth);
void synth_output(void);
void synth_fouth_frame_hw(struct mad_frame * frame, mad_fixed_t (fouth_frame_sbsample)[36][32], unsigned int ch, unsigned int ns);
void start_next_synth(void);
int DMA1ToSynth(unsigned long pSrc);
int DMA1FromSynth(unsigned long pDst);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: mp3_wait_synth
** Input:void
** Return: void
** Owner:WJR
** Date: 2014.11.10
** Time: 16:28:36
*******************************************************************************/
_CPU_NANOD_LIB_HW_MP3_SYN_READ_
COMMON API void mp3_wait_synth(void)
{
    long synth_e_cnt = 0;
    while (is_synthing==1)
    {
        if (synth_end_ch_number==1)
        {
            if (synth_hw_frame->output_pos[0] == synth_hw_frame->output_pos[1])
            {
                synth_e_cnt ++;
                if (synth_e_cnt > 1024)
                {
                    synth_hw_frame->output_pos[0] = 0;
                    synth_hw_frame->output_pos[1] = 0;

                    // by vincent hsiung , 6-17
                    synth_hw_frame->phase[0] = 0;
                    synth_hw_frame->phase[1] = 0;

                    is_synthing = 0;
                }
            }
        }
        /* for test */
        else
        {
            // mono
            synth_e_cnt ++ ;
            if ( synth_e_cnt > 20480 ) //enlager
            {
                synth_hw_frame->output_pos[0] = 0;
                synth_hw_frame->output_pos[1] = 0;

                synth_hw_frame->phase[0] = 0;
                synth_hw_frame->phase[1] = 0;

                is_synthing = 0;
            }
        }

    }
}
/*******************************************************************************
** Name: mad_synth_fouth_frame
** Input:struct mad_frame *frame, int whichch,int ns
** Return: void
** Owner:WJR
** Date: 2014.11.10
** Time: 16:02:59
*******************************************************************************/
_CPU_NANOD_LIB_HW_MP3_SYN_READ_
COMMON API void mad_synth_fouth_frame(struct mad_frame *frame, int whichch,int ns)
{
    synth_fouth_frame_hw(frame,(mad_fixed_t(*)[32])frame->sbsample,whichch, ns);

}


/*******************************************************************************
** Name: synth_handler
** Input:void
** Return: void
** Owner:WJR
** Date: 2014.11.10
** Time: 15:53:22
*******************************************************************************/
_CPU_NANOD_LIB_HW_MP3_SYN_READ_
COMMON API void synth_handler(void)
{
    SYNTH_EOIT_REG = 0;

    if (!DMA1FromSynth((unsigned long)(&synth_hw_pcmout[0])))      //if dma is busy, use memcpy
    {
        //memcpy(&synth_hw_pcmout[0],((unsigned long*)SYNTH_BASEADDR) ,32*sizeof(int));
        long *p = (long *)&synth_hw_pcmout[0];
        long *q = (long *)SYNTH_BASEADDR;
        p[ 0] = q[ 0];
        p[ 1] = q[ 1];
        p[ 2] = q[ 2];
        p[ 3] = q[ 3];
        p[ 4] = q[ 4];
        p[ 5] = q[ 5];
        p[ 6] = q[ 6];
        p[ 7] = q[ 7];
        p[ 8] = q[ 8];
        p[ 9] = q[ 9];
        p[10] = q[10];
        p[11] = q[11];
        p[12] = q[12];
        p[13] = q[13];
        p[14] = q[14];
        p[15] = q[15];
        p[16] = q[16];
        p[17] = q[17];
        p[18] = q[18];
        p[19] = q[19];
        p[20] = q[20];
        p[21] = q[21];
        p[22] = q[22];
        p[23] = q[23];
        p[24] = q[24];
        p[25] = q[25];
        p[26] = q[26];
        p[27] = q[27];
        p[28] = q[28];
        p[29] = q[29];
        p[30] = q[30];
        p[31] = q[31];
        synth_finish = 1;

        //synth_output();
        //do the next synth
       // start_next_synth();
    }
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: mad_synth_mute
** Input:struct mad_synth *synth
** Return: void
** Owner:WJR
** Date: 2014.11.10
** Time: 16:37:17
*******************************************************************************/
_CPU_NANOD_LIB_HW_MP3_SYN_READ_
COMMON FUN void mad_synth_mute(struct mad_synth *synth)
{
    unsigned int ch, s, v;

    for (ch = 0; ch < 2; ++ch)
    {
        for (s = 0; s < 16; ++s)
        {
            for (v = 0; v < 8; ++v)
            {

            }
        }
    }
}

/*******************************************************************************
** Name: synth_output
** Input:void
** Return: void
** Owner:WJR
** Date: 2014.11.10
** Time: 16:18:33
*******************************************************************************/
_CPU_NANOD_LIB_HW_MP3_SYN_READ_
COMMON FUN void synth_output(void)
{
    int i;
    short *start_pos;

    if (synth_hw_ch == 0)
        start_pos = &synth_hw_frame->output_ptrL[synth_hw_frame->output_pos[synth_hw_ch]*2] + synth_hw_ch;
    else
        start_pos = &synth_hw_frame->output_ptrR[synth_hw_frame->output_pos[synth_hw_ch]*2] + synth_hw_ch;

    {
        if (synth_end_ch_number==1)      //stereo
        {
            for (i = 0; i < 32; i++)
            {
                start_pos[i*2] = (short)(synth_hw_pcmout[i]);
            }
        }
        else                                    //mono
        {
            for (i = 0; i < 32; i++)
            {
                start_pos[i*2] = (short)synth_hw_pcmout[i];
                start_pos[i*2+1] = (short)synth_hw_pcmout[i];
            }
        }
    }

    synth_hw_frame->output_pos[synth_hw_ch]+=32;

}
/*******************************************************************************
** Name: synth_fouth_frame_hw
** Input:struct mad_frame * frame, mad_fixed_t (fouth_frame_sbsample)[36][32],unsigned int ch, unsigned int ns)
** Return: void
** Owner:WJR
** Date: 2014.11.10
** Time: 16:01:59
*******************************************************************************/
int test_data[32] = {
    0xd6da ,0xd6da ,0xd6da ,0xd6da ,0xd6da ,0xd6da ,
            0xd6da ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,
            0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,
            0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,
            0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,
            0x0 ,0x0 };
int test_out_data_r[32] = {
    0};
_CPU_NANOD_LIB_HW_MP3_SYN_READ_
COMMON FUN void synth_fouth_frame_hw(struct mad_frame * frame, mad_fixed_t (fouth_frame_sbsample)[36][32],
                                   unsigned int ch, unsigned int ns)
{
    int i;

    is_synthing = 1;
    synth_hw_busy = 1;
    synth_hw_s = 0;
    synth_hw_ch = ch;
    synth_hw_ns = ns;
    synth_hw_frame = frame;
    synth_hw_sbsample = (mad_fixed_t (*)[36][32])fouth_frame_sbsample;
    synth_hw_phase = frame->phase[ch];

    synth_end_ch_number = (frame->header.mode?1:0);

    if (synth_hw_s < synth_hw_ns)
    {
        SYNTH_CNFG_REG = (((15 - synth_hw_phase)<<1)|synth_hw_ch);      //config the phase & channel;
        //put the input data to the in-buffer
         if (!DMA1ToSynth((unsigned long)(&test_data)))
       // if (!DMA1ToSynth((unsigned long)(&(*synth_hw_sbsample)[synth_hw_s][0])))
        {
            //if dma is busy, use memcpy
            long *p = (long *)SYNTH_BASEADDR;
            long *q = (long *)&test_data;//(long *)&(*synth_hw_sbsample)[synth_hw_s][0];
            p[ 0] = q[ 0];
            p[ 1] = q[ 1];
            p[ 2] = q[ 2];
            p[ 3] = q[ 3];
            p[ 4] = q[ 4];
            p[ 5] = q[ 5];
            p[ 6] = q[ 6];
            p[ 7] = q[ 7];
            p[ 8] = q[ 8];
            p[ 9] = q[ 9];
            p[10] = q[10];
            p[11] = q[11];
            p[12] = q[12];
            p[13] = q[13];
            p[14] = q[14];
            p[15] = q[15];
            p[16] = q[16];
            p[17] = q[17];
            p[18] = q[18];
            p[19] = q[19];
            p[20] = q[20];
            p[21] = q[21];
            p[22] = q[22];
            p[23] = q[23];
            p[24] = q[24];
            p[25] = q[25];
            p[26] = q[26];
            p[27] = q[27];
            p[28] = q[28];
            p[29] = q[29];
            p[30] = q[30];
            p[31] = q[31];
        }
        synth_hw_phase = (synth_hw_phase + 1) % 16;

        synth_hw_s++;
    }
    else
    {
        synth_hw_frame->phase[synth_hw_ch] = synth_hw_phase;
        synth_hw_busy = 0;
    }
}
/*******************************************************************************
** Name: start_next_synth
** Input:void
** Return: void
** Owner:WJR
** Date: 2014.11.10
** Time: 15:58:50
*******************************************************************************/
_CPU_NANOD_LIB_HW_MP3_SYN_READ_
COMMON FUN void start_next_synth(void)
{
    if (synth_hw_s < synth_hw_ns)
    {
        SYNTH_CNFG_REG = (((15 - synth_hw_phase)<<1)|synth_hw_ch);      //config the phase & channel;
        //put the input data to the in-buffer
        if (!DMA1ToSynth((unsigned long)(&(*synth_hw_sbsample)[synth_hw_s][0])))      //if dma is busy, use memcpy
        {
            //memcpy(((void *)SYNTH_BASEADDR),&(*synth_hw_sbsample)[synth_hw_s][0],32*sizeof(int));
            long *p = (long *)SYNTH_BASEADDR;
            long *q = (long *)&(*synth_hw_sbsample)[synth_hw_s][0];
            p[ 0] = q[ 0];
            p[ 1] = q[ 1];
            p[ 2] = q[ 2];
            p[ 3] = q[ 3];
            p[ 4] = q[ 4];
            p[ 5] = q[ 5];
            p[ 6] = q[ 6];
            p[ 7] = q[ 7];
            p[ 8] = q[ 8];
            p[ 9] = q[ 9];
            p[10] = q[10];
            p[11] = q[11];
            p[12] = q[12];
            p[13] = q[13];
            p[14] = q[14];
            p[15] = q[15];
            p[16] = q[16];
            p[17] = q[17];
            p[18] = q[18];
            p[19] = q[19];
            p[20] = q[20];
            p[21] = q[21];
            p[22] = q[22];
            p[23] = q[23];
            p[24] = q[24];
            p[25] = q[25];
            p[26] = q[26];
            p[27] = q[27];
            p[28] = q[28];
            p[29] = q[29];
            p[30] = q[30];
            p[31] = q[31];
        }
        synth_hw_phase = (synth_hw_phase + 1) % 16;
        synth_hw_s++;
    }
    else
    {
        synth_hw_frame->phase[synth_hw_ch] = synth_hw_phase;
        synth_hw_busy = 0;
    }
}
/*******************************************************************************
** Name: DMA1ToSynth
** Input:unsigned long pSrc
** Return: int
** Owner:WJR
** Date: 2014.11.10
** Time: 15:46:17
*******************************************************************************/
_CPU_NANOD_LIB_HW_MP3_SYN_READ_
COMMON FUN int DMA1ToSynth(unsigned long pSrc)
{
    //for debug
    return false;
}
/*******************************************************************************
** Name: DMA1FromSynth
** Input:unsigned long pDst
** Return: int
** Owner:WJR
** Date: 2014.11.10
** Time: 15:45:22
*******************************************************************************/
_CPU_NANOD_LIB_HW_MP3_SYN_READ_
COMMON FUN int DMA1FromSynth(unsigned long pDst)
{

    //for debug
    return false;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(write) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(write) define
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

/*******************************************************************************
** Name: mad_synth_init
** Input:struct mad_synth *synth
** Return: void
** Owner:WJR
** Date: 2014.11.10
** Time: 16:35:46
*******************************************************************************/
_CPU_NANOD_LIB_HW_MP3_SYN_INIT_
INIT API void mad_synth_init(struct mad_synth *synth)
{

    mad_synth_mute(synth);

    synth->phase = 0;

    synth->pcm.samplerate = 0;
    synth->pcm.channels   = 0;
    synth->pcm.length     = 0;
}

/*******************************************************************************
** Name: Hw_syn_Init
** Input:void
** Return: void
** Owner:WJR
** Date: 2014.11.10
** Time: 16:54:15
*******************************************************************************/
_CPU_NANOD_LIB_HW_MP3_SYN_INIT_
INIT API void Hw_syn_Init(void)
{
    synth_finish = 0;

    /* Apr 25 */
    synth_hw_ch = 0;
    synth_hw_ns = 0;
    synth_hw_s = 0;
    synth_hw_phase = 0;
    synth_hw_busy = 0;

    SYNTH_EOIT_REG = 0;

    SYNTH_CTRL_REG = 1;                         //Enable SYNTH

}
/*******************************************************************************
** Name: Hw_syn_Exit
** Input:void
** Return: void
** Owner:WJR
** Date: 2014.11.10
** Time: 16:57:06
*******************************************************************************/
_CPU_NANOD_LIB_HW_MP3_SYN_INIT_
INIT API void Hw_syn_Exit(void)
{

    SYNTH_EOIT_REG = 0;
    SYNTH_CTRL_REG = 0;                         //Disable SYNTH
    synth_hw_busy  = 0;
}
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/


#ifdef _USE_SHELL_
#ifdef _SYN_SHELL_
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: hw_syn_shell
** Input:()
** Return: rk_err_t
** Owner:WJR
** Date: 2014.11.10
** Time: 17:57:44
*******************************************************************************/

_CPU_NANOD_LIB_HW_MP3_SYN_SHELL_
SHELL API rk_err_t hw_syn_shell()
{
    int whichch = 0;
    int ns = 18;
    mad_frame c_frame ;
    mad_frame *frame =&c_frame;
    IntEnable(INT_ID_SYNTH);
    IntPendingClear(INT_ID_SYNTH);
    IntRegister(INT_ID_SYNTH, synth_handler);
    Hw_syn_Init();
    //mp3_wait_synth();
    mad_synth_fouth_frame(frame, whichch,ns);
    while(synth_finish==0);
    Hw_syn_Exit();
    {
        int i;
        for (i = 0;i<32 ;i++)
        {
            if (synth_hw_pcmout[i] != test_out_data_r[i])
            {
                break;
            }
        }
        if (i == 32)
        {
            rk_print_string("\r\n syn test over\n");
            return  RK_SUCCESS;
        }
        else
        {
                                {
                    int i;
                    for(i = 0;i<32;i++)
                    {
                      printf(" 0x%0x,",synth_hw_pcmout[i]);
                      if((i+1)%6 == 0)
                        {
                          printf("\n");
                        }

                    }
                }
             rk_print_string("\r\n syn test error\n");
            return RK_ERROR  ;
        }
    }
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#endif
#endif
#endif

