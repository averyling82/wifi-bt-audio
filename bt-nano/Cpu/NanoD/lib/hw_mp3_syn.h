/*
********************************************************************************************
*
*                  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                               All rights reserved.
*
* FileName: Cpu\NanoD\lib\hw_mp3_syn.h
* Owner: WJR
* Date: 2014.11.10
* Time: 15:43:39
* Desc:
* History:
*    <author>     <date>       <time>     <version>       <Desc>
*    WJR     2014.11.10     15:43:39   1.0
********************************************************************************************
*/

#ifndef _CPU_NANOD_LIB_HW_MP3_SYN_H__
#define _CPU_NANOD_LIB_HW_MP3_SYN_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "hw_memap.h"
//base------------------------------------------------------------------------------------
#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif




/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

//dma memory map---------------------------------------------------------------------------
//#define DMA_BASE        0x62040000






typedef  signed int mad_fixed_t;

enum mad_layer
{
    MAD_LAYER_I   = 1,            /* Layer I */
    MAD_LAYER_II  = 2,            /* Layer II */
    MAD_LAYER_III = 3            /* Layer III */
};

enum mad_mode
{
    MAD_MODE_SINGLE_CHANNEL = 0,        /* single channel */
    MAD_MODE_DUAL_CHANNEL      = 1,        /* dual channel */
    MAD_MODE_JOINT_STEREO      = 2,        /* joint (MS/intensity) stereo */
    MAD_MODE_STEREO      = 3        /* normal LR stereo */
};

enum mad_emphasis
{
    MAD_EMPHASIS_NONE      = 0,        /* no emphasis */
    MAD_EMPHASIS_50_15_US      = 1,        /* 50/15 microseconds emphasis */
    MAD_EMPHASIS_CCITT_J_17 = 3,        /* CCITT J.17 emphasis */
    MAD_EMPHASIS_RESERVED   = 2        /* unknown emphasis */
};

struct mad_header
{
    enum mad_layer layer;            /* audio layer (1, 2, or 3) */
    enum mad_mode mode;            /* channel mode (see above) */
    int mode_extension;            /* additional mode info */
    enum mad_emphasis emphasis;        /* de-emphasis to use (see above) */

    unsigned long bitrate;        /* stream bitrate (bps) */
    unsigned int samplerate;        /* sampling frequency (Hz) */

    unsigned short crc_check;        /* frame CRC accumulator */
    unsigned short crc_target;        /* final target CRC checksum */

    int flags;                /* flags (see below) */
    int private_bits;            /* private bits (see below) */

};
typedef struct mad_frame
{
    struct mad_header header;        /* MPEG audio header */

    int options;                /* decoding options (from stream) */

    mad_fixed_t (*sbsample)[36][32];    /* synthesis subband filter samples */

    mad_fixed_t (*overlap)[2*32*18];    /* Layer III block overlap data */

    short *output_ptrL;
    short *output_ptrR;
    int output_pos[2];    //two channels, current output position

    int phase[2];            //synthesis phase (0~15)
}mad_frame;
struct mad_pcm
{
    unsigned int samplerate;        /* sampling frequency (Hz) */
    unsigned short channels;        /* number of channels */
    unsigned short length;        /* number of samples per channel */
};

struct mad_synth
{
    unsigned int phase;            /* current processing phase */
    struct mad_pcm pcm;            /* PCM output */
};

enum
{
    MAD_PCM_CHANNEL_SINGLE = 0
};

enum
{
    MAD_PCM_CHANNEL_DUAL_1 = 0,
    MAD_PCM_CHANNEL_DUAL_2 = 1
};

enum
{
    MAD_PCM_CHANNEL_STEREO_LEFT  = 0,
    MAD_PCM_CHANNEL_STEREO_RIGHT = 1
};



//synth memory map--------------------------------------------------------------------------
//#define    SYNTH_BASEADDR    0x62044000
#define    SYNTH_BASEADDR     SYNTH_BASE//(0x60020000UL)

#define SYNTH_CTRL_REG    *((volatile unsigned long*)(SYNTH_BASEADDR + 0x0800))
#define SYNTH_CNFG_REG    *((volatile unsigned long*)(SYNTH_BASEADDR + 0x0804))
#define SYNTH_STAT_REG    *((volatile unsigned long*)(SYNTH_BASEADDR + 0x0808))
#define SYNTH_INTR_REG    *((volatile unsigned long*)(SYNTH_BASEADDR + 0x080C))
#define SYNTH_EOIT_REG    *((volatile unsigned long*)(SYNTH_BASEADDR + 0x0810))
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/




/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern rk_err_t hw_syn_shell();
extern void Hw_syn_Exit(void);
extern void mad_synth_init(struct mad_synth *synth);
extern void mp3_wait_synth(void);
extern void mad_synth_fouth_frame();
extern void synth_handler(void);



#endif

