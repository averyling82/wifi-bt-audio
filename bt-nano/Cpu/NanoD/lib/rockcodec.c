/*
********************************************************************************************
*
*                  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                               All rights reserved.
*
* FileName: lib\rockcodec.c
* Owner: Aaron.sun
* Date: 2015.5.19
* Time: 20:00:49
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*   Aaron.sun     2015.5.19     20:00:49   1.0
********************************************************************************************
*/
#include "BspConfig.h"
#ifdef __DRIVER_ROCKCODEC_ROCKCODECDEVICE_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "typedef.h"
#include "DriverInclude.h"

#if (CODEC_CONFIG == CODEC_ROCKC)

#include "rockcodec.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define  MAX_OUTLEVEL            10

#define ReadAcodecReg(addr)                     (*(volatile uint32 *)(addr))
#define WriteAcodecReg(addr, data)              (*(volatile uint32 *)(addr) = data)

#define SetAcodecRegData(addr, databit)         WriteAcodecReg(addr, ReadAcodecReg(addr)|(databit))
#define ClrAcodecRegData(addr, databit)         WriteAcodecReg(addr, ReadAcodecReg(addr)&(~databit))

#define SetAcodecRegBit(addr,bit)               WriteAcodecReg(addr,(ReadAcodecReg(addr)|(1<<bit)))
#define ClrAcodecRegBit(addr,bit)               WriteAcodecReg(addr,(ReadAcodecReg(addr)&(~(1<<bit))))
#define GetAcodecRegBit(addr,bit)               (ReadAcodecReg(addr)&(1<<bit))

#define MaskAcodecRegBits(addr, y, z)           WriteAcodecReg(addr, (ReadAcodecReg(addr)&(~y))|(z))


typedef struct tagAPLL_APP
{
    uint32 F_source;    //KHz
    uint32 F_target;    //KHz
    uint32 PLL_POSDIV_L3;
    uint32 PLL_POSDIV_H8;
    uint32 PLL_PREDIV_BIT;
    uint32 PLL_OUTDIV;
}ACodecPLL_APP,*pACodecPLL_APP;

static const ACodecPLL_APP ACodecpllTable_61440[8]=
{
    {2048  ,61440 , 0  , 45, 2,  6},
    {3072  ,61440 , 0  , 45, 3,  6},
    {4096  ,61440 , 0  , 45, 4,  6},
    {6000  ,61440 , 0  , 64, 10, 5},
    {6144  ,61440 , 0  , 45, 6,  6},
    {12000 ,61440 , 0  , 96, 25, 6},
    {12288 ,61440 , 0  , 45, 12, 6},
    {24000 ,61440 , 0  , 48, 25, 10},

};

static const ACodecPLL_APP ACodecpllTable_56448[8]=
{
    {2048  ,56448 , 3  , 41, 2,  6},
    {3072  ,56448 , 1  , 55, 4,  6},
    {4096  ,56448 , 3  , 41, 4,  6},
    {6000  ,56448 , 0  , 147,25, 5},
    {6144  ,56448 , 1  , 55, 8,  6},
    {12000 ,56448 , 4  , 73, 25, 5},
    {12288 ,56448 , 1  , 55, 16, 6},
    {24000 ,56448 , 6  , 36, 25, 3},

};

static const ACodecPLL_APP ACodecpllTable_40960[8]=
{
    {2048  ,40960 , 4   , 22, 1,  9},
    {3072  ,40960 , 0   , 45, 3,  9},
    {4096  ,40960 , 4   , 22, 2,  9},
    {6000  ,40960 , 0   , 192,25, 9},
    {6144  ,40960 , 0   , 40, 6,  9},
    {12000 ,40960 , 0   , 96, 25, 9},
    {12288 ,40960 , 0   , 45, 12, 9},
    {24000 ,40960 , 0   , 96, 50, 8},

};
#endif

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
ACodecI2S_mode_t Acodec_I2S_MODE;
eACodecPll_Target_Freq pll_old_target;
CodecMode_en_t Codecmode_Bak;
CodecFS_en_t CodecFS_Bak;
CodecPower_Status_t Codec_Power_Status;
uint32 Acodec_suspend_En;


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
void ACodec_ReadReg_printf(uint32 regaddr);
void ACodec_po_dac_lo(void);
void ACodec_get_dac_vol(void);
void ACodec_pd_dac_lo();
void ACodec_po_dac_hp();
void ACodec_pd_dac_hp();
void ACodec_ADC2DAC_MIX(CodecMIX_Mode_t MIX_en);
bool ACodec_Get_DAC_MTST(void);
void ACodec_DACUnMute(void);
void ACodec_DACMute(void);
uint8 ACodec_Get_DAC_DigVol_L(void);
uint8 ACodec_Get_DAC_DigVol_R(void);
uint8 ACodec_Set_DAC_DigVol(uint8 vol);
uint8 ACodec_Get_ADC_DigVol_L(void);
uint8 ACodec_Get_ADC_DigVol_R(void);
uint8 ACodec_Set_ADC_DigVol(uint8 vol);
uint8 ACodec_Set_MIC_AnaVol(uint8 vol);
uint8 ACodec_Set_ADCMUX_Vol(uint8 vol);
uint32 ACodec_Get_SRT_TIME(CodecFS_en_t CodecFS);
void ACodec_Set_HP_AMP(eACodecHp_AMP vol);
void ACodec_Set_LO_AMP(eACodecLo_AMP vol);
void ACodec_I2s_SetSampleRate(uint32 Sck_div, CodecFS_en_t CodecFS);
void ACodec_Soft_Reset(void);
void ACodec_PLL_Set(eF_SOURCE_ID F_SOURCE,eACodecPll_Target_Freq pll_target);
void ACodec_I2S_RX_Start(void);
void ACodec_I2S_RX_Stop(void);
void ACodec_I2S_RX_Init(eACodecI2sFormat_t BUS_FORMAT,
              eACodecI2sDATA_WIDTH_t I2S_Data_width,
              ACodecI2S_BUS_MODE_t I2S_Bus_mode);
void ACodec_I2S_TX_Start(void);
void ACodec_I2S_TX_Stop(void);
void ACodec_I2S_TX_Init(eACodecI2sFormat_t BUS_FORMAT,
                        eACodecI2sDATA_WIDTH_t I2S_Data_width,
                        ACodecI2S_BUS_MODE_t I2S_Bus_mode);
void ACodec_Set_LineIn_Limiter();
void ACodec_Set_DAC_Limiter();
void ACodec_Set_ADC_HighPassFilter();
void ACodec_Set_ADC_NoiseGate();
void ACodec_Set_ADC_ALC();
void ACodec_Exit_ADC_Mode();
void ACodec_Set_MICLI_Mode(CodecIn_sel_t In_Mode,CodecMicBias_sel_t Mic_Bias,CodecMic_Mode_t MIC_Mode);
void ACodec_Set_ADC_Mode(CodecIn_sel_t In_Mode, CodecFS_en_t CodecFS);
void ACodec_WriteReg(uint32 regaddr,uint32 data);
void ACodec_Exit_DAC_Mode();
void ACodec_Set_DAC_Mode(CodecOut_sel_t OUT_Mode,CodecFS_en_t CodecFS);
void ACodec_Set_HPLO_Mode(CodecOut_sel_t OUT_Mode);
void ACodec_Set_I2S_Mode(eACodecI2sFormat_t BUS_FORMAT,
              eACodecI2sDATA_WIDTH_t I2S_Data_width,
              ACodecI2S_BUS_MODE_t I2S_Bus_mode,
              ACodecI2S_mode_t I2S_mode);

void ACodec_Set_I2S_RX_Mode(eACodecI2sFormat_t BUS_FORMAT,
              eACodecI2sDATA_WIDTH_t I2S_Data_width,
              ACodecI2S_BUS_MODE_t I2S_Bus_mode,
              ACodecI2S_mode_t I2S_mode);

void Codec_PowerOnInitial(void);
void Codec_SetMode(CodecMode_en_t Codecmode, CodecFS_en_t CodecFS);
void Codec_ExitMode(CodecMode_en_t Codecmode);
void Codec_SetSampleRate(CodecFS_en_t CodecFS);
//void Codec_SetVolumet(unsigned int Volume);
void Codec_DACMute(void);
void Codec_DACUnMute(void);
void Codec_ADCPOWER_UP(void);
void Codec_ADCPOWER_DOWN(void);
void Codec_ADC_MUTE(void);
void Codec_ADC_UnMute(void);
void Codec_MUX_Power_down(void);
void Codec_MUX_Power_up(void);

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: ACodec_pd_dac_lo_test
** Input:
** Return:
** Owner:hj
** Date: 2014.11.11
** Time: 15:10:53
*******************************************************************************/
uint8 ACodec_get_over_current_value(void)
{
   uint8 config;
   config = ReadAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG2);
   return config;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: ACodec_get_dac_vol
** Input:void
** Return: void
** Owner:cjh
** Date: 2015.5.20
** Time: 10:39:16
*******************************************************************************/
//_LIB_ROCKCODEC_READ_
void ACodec_get_dac_vol(void)
{
    uint8 L1,L2,R1,R2;

    while (1)
    {
        L1 = ReadAcodecReg(ACODEC_BASE+ACODEC_DACVSTL);
        R1 = ReadAcodecReg(ACODEC_BASE+ACODEC_DACVSTR);

        rkos_sleep(5);

        L2 = ReadAcodecReg(ACODEC_BASE+ACODEC_DACVSTL);
        R2 = ReadAcodecReg(ACODEC_BASE+ACODEC_DACVSTR);

        if ((L1 == L2) && (R1 == R2))
            break;
    }
}

/*******************************************************************************
** Name: ACodec_Get_DAC_MTST
** Input:void
** Return: MUTE status
*******************************************************************************/
bool ACodec_Get_DAC_MTST(void)
{
    uint8 config;
    config = ReadAcodecReg(ACODEC_BASE+ACODEC_DACST);
    if (((config >> 4) & 0x1) == 0x1)
        return TRUE;
    else
        return FALSE;
}
/*******************************************************************************
** Name: ACodec_Get_DAC_DigVol_L
** Input:void
** Return: DigVol_L vol
*******************************************************************************/
uint8 ACodec_Get_DAC_DigVol_L(void)
{
    uint8 config;
    config = ReadAcodecReg(ACODEC_BASE+ACODEC_DACVSTL);
    return config;
}
/*******************************************************************************
** Name: ACodec_Get_DAC_DigVol_R
** Input:void
** Return: DigVol_R vol
*******************************************************************************/
uint8 ACodec_Get_DAC_DigVol_R(void)
{
    uint8 config;
    config = ReadAcodecReg(ACODEC_BASE+ACODEC_DACVSTR);
    return config;

}

/*******************************************************************************
** Name: ACodec_Get_ADC_DigVol_L
** Input:void
** Return: ADC_DigVol_L
*******************************************************************************/
uint8 ACodec_Get_ADC_DigVol_L(void)
{
    uint8 config;
    config = ReadAcodecReg(ACODEC_BASE+ACODEC_ADCVSTL);
    return config;
}
/*******************************************************************************
** Name: ACodec_Get_ADC_DigVol_R
** Input:void
** Return: ADC_DigVol_R
*******************************************************************************/
uint8 ACodec_Get_ADC_DigVol_R(void)
{
    uint8 config;
    config = ReadAcodecReg(ACODEC_BASE+ACODEC_ADCVSTR);
    return config;

}

/*******************************************************************************
** Name: ACodec_Get_SRT_TIME
** Input:pll_target
** Return: void
*******************************************************************************/
uint32 ACodec_Get_SRT_TIME(CodecFS_en_t CodecFS)
{
    uint32 DACSRT_TIME = 0;
    switch (CodecFS)
    {
        case CodecFS_8000Hz:
        case CodecFS_11025Hz:
        case CodecFS_12KHz:
            DACSRT_TIME = 0;
            break;

        case CodecFS_16KHz:
        case CodecFS_22050Hz:
        case CodecFS_24KHz:
            DACSRT_TIME = 1;
            break;

        case CodecFS_32KHz:
        case CodecFS_44100Hz:
        case CodecFS_48KHz:
            DACSRT_TIME = 2;
            break;
        case CodecFS_64KHz:
        case CodecFS_88200Hz:
        case CodecFS_96KHz:
            DACSRT_TIME = 3;
            break;

        case CodecFS_128KHz:
        case CodecFS_1764KHz:
        case CodecFS_192KHz:
            DACSRT_TIME = 4;
            break;
        default:
            break;
    }
    return DACSRT_TIME;

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
--------------------------------------------------------------------------------
  Function name : void Codec_SetMode(CodecMode_en_t Codecmode)
  Author        : yangwenjie
  Description   :

  Input         : Codecmode：

  Return        : null

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  Note:      if exit from application, like FM or MIC , please set codec to standby mode
--------------------------------------------------------------------------------
*/
void Codec_SetMode(CodecMode_en_t Codecmode, CodecFS_en_t CodecFS)
{

    Codecmode_Bak = Codecmode;
    CodecFS_Bak = CodecFS;

    switch (Codecmode)
    {
        case Codec_DACoutHP:
            ACodec_Set_DAC_Mode(CodecOut_Sel_HP,CodecFS);
            break;
        case Codec_DACoutLINE:
            ACodec_Set_DAC_Mode(CodecOut_Sel_LINE,CodecFS);
            break;

        case Codec_Line1ADC:
            ACodec_Set_ADC_Mode(Codecin_Sel_LINE1,CodecFS);
            ACodec_ADC2DAC_MIX(CodecMIX_DISABLE);
            ACodec_Set_ADCMUX_Vol(5);
            ACodec_Set_ADC_DigVol(0);
            ACodec_Set_ADC_HighPassFilter();
            break;

        case Codec_Line1in:
            ACodec_Set_ADC_Mode(Codecin_Sel_LINE1,CodecFS);
            ACodec_ADC2DAC_MIX(CodecMIX_ENABLE);
            ACodec_Set_ADC_HighPassFilter();
            break;

        case Codec_Line2ADC:
            ACodec_Set_ADC_Mode(Codecin_Sel_LINE2,CodecFS);
            ACodec_ADC2DAC_MIX(CodecMIX_DISABLE);
            ACodec_Set_ADCMUX_Vol(5);
            ACodec_Set_ADC_DigVol(0);
            ACodec_Set_ADC_HighPassFilter();
            break;

        case Codec_Line2in:
            ACodec_Set_ADC_Mode(Codecin_Sel_LINE2,CodecFS);
            ACodec_ADC2DAC_MIX(CodecMIX_ENABLE);
            ACodec_Set_ADC_HighPassFilter();
            break;

        case Codec_MicStero:
            ACodec_Set_ADC_Mode(Codecin_Sel_MIC_STERO,CodecFS);
            ACodec_ADC2DAC_MIX(CodecMIX_DISABLE);
            ACodec_Set_MIC_AnaVol(2);
            ACodec_Set_ADCMUX_Vol(5);
            ACodec_Set_ADC_DigVol(0);
            ACodec_Set_ADC_HighPassFilter();
            break;

        case Codec_Mic1Mono:
            ACodec_Set_ADC_Mode(Codecin_Sel_MIC1_MONO,CodecFS);
            ACodec_ADC2DAC_MIX(CodecMIX_DISABLE);
            ACodec_Set_MIC_AnaVol(2);
            ACodec_Set_ADCMUX_Vol(5);
            ACodec_Set_ADC_DigVol(0);
            ACodec_Set_ADC_HighPassFilter();
            break;

         case Codec_Mic2Mono:
            ACodec_Set_ADC_Mode(Codecin_Sel_MIC2_MONO,CodecFS);
            ACodec_ADC2DAC_MIX(CodecMIX_DISABLE);
            ACodec_Set_MIC_AnaVol(2);
            ACodec_Set_ADCMUX_Vol(5);
            ACodec_Set_ADC_DigVol(0);
            ACodec_Set_ADC_HighPassFilter();
            break;

        default:

            break;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void Codec_SetMode(CodecMode_en_t Codecmode)
  Author        : yangwenjie
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  note:      if exit from application, like FM or MIC , please set codec to standby mode
--------------------------------------------------------------------------------
*/
void Codec_ExitMode(CodecMode_en_t Codecmode)
{

    switch (Codecmode)
    {
        case Codec_DACoutHP:
            ACodec_Exit_DAC_Mode();
            break;
        case Codec_DACoutLINE:
            ACodec_Exit_DAC_Mode();
            break;

        case Codec_Line1ADC:
            ACodec_Exit_ADC_Mode();
            break;

        case Codec_Line1in:
            ACodec_Exit_ADC_Mode();
            ACodec_ADC2DAC_MIX(CodecMIX_DISABLE);
            break;
        case Codec_Line2ADC:
            ACodec_Exit_ADC_Mode();
            break;
        case Codec_Line2in:
            ACodec_Exit_ADC_Mode();
            ACodec_ADC2DAC_MIX(CodecMIX_DISABLE);
            break;
        case Codec_MicStero:
        case Codec_Mic1Mono:
        case Codec_Mic2Mono:
            ACodec_Exit_ADC_Mode();
            break;
        default:

            break;
    }

}

/*
--------------------------------------------------------------------------------
  Function name : void Codec_SetSampleRate(CodecFS_en_t CodecFS)
  Author        : yangwenjie
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  Note:
--------------------------------------------------------------------------------
*/
_LIB_ROCKCODEC_COMMON_
void Codec_SetSampleRate(CodecFS_en_t CodecFS)
{
    uint32 Sck_div,FS_value;
    FS_value = CodecFS;
    //printf("CodecFS =%d\n",CodecFS);
    switch (CodecFS)
    {
        case CodecFS_8000Hz:
        case CodecFS_16KHz:
        case CodecFS_32KHz:
        case CodecFS_64KHz:
        case CodecFS_128KHz:
            ACodec_PLL_Set(F_SOURCE_24000KHz,Pll_Target_Freq_40960);
            Sck_div = (128000 / FS_value) - 1 ;
            ACodec_I2s_SetSampleRate(Sck_div, CodecFS);
            break;

        case CodecFS_12KHz:
        case CodecFS_24KHz:
        case CodecFS_48KHz:
        case CodecFS_96KHz:
        case CodecFS_192KHz:
            ACodec_PLL_Set(F_SOURCE_24000KHz,Pll_Target_Freq_61440);
            Sck_div = (192000 / FS_value) - 1;
            //printf("CodecFS =%d Sck_div=%d F_SOURCE_24000KHz=%d Pll_Target_Freq_61440=%d\n",CodecFS,Sck_div,F_SOURCE_24000KHz,Pll_Target_Freq_61440);
            ACodec_I2s_SetSampleRate(Sck_div, CodecFS);
            break;
        case CodecFS_11025Hz:
        case CodecFS_22050Hz:
        case CodecFS_44100Hz:
        case CodecFS_88200Hz:
        case CodecFS_1764KHz:
            ACodec_PLL_Set(F_SOURCE_24000KHz,Pll_Target_Freq_56448);
            Sck_div = (176400 / FS_value) - 1;
            ACodec_I2s_SetSampleRate(Sck_div, CodecFS);
            break;

        default:

            break;

    }
}

/*
--------------------------------------------------------------------------------
  Function name : void Codec_SetVolumet(unsigned int Volume)
  Author        : yangwenjie
  Description   : codec control volume

  Input         : Volume

  Return        : null

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  Note:      volume = 0 mean mute,
--------------------------------------------------------------------------------
*/
/*
_LIB_ROCKCODEC_COMMON_
void Codec_SetVolumet(unsigned int Volume)
{
   uint32 VolumeMode = EQ_NOR;

    rk_printf("Volume = %d",Volume);

    if (Volume == 0)
    {
        Codec_DACMute();
    }
    else
    {
        if(ACodec_Get_DAC_MTST())
        {
            Codec_DACUnMute();
        }
        #if 0
        if (TRUE == ThreadCheck(pMainThread, &MusicThread))
        {
            RKEffect          *pEffect = &pAudio->EffectCtl;
            VolumeMode = pEffect->Mode;
        }
        #endif
        switch (VolumeMode)
        {
            case EQ_NOR:
                if(ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
                {
                    rk_printf("HPAMPVol = %d",ACodec_LineOutVol[Volume].HP_AMPVol);
                    ACodec_Set_HP_AMP(ACodec_LineOutVol[Volume].HP_AMPVol);
                    rk_printf("DacDigVol = %d",ACodec_LineOutVol[Volume].Dac_DigVol);
                    ACodec_Set_DAC_DigVol(ACodec_LineOutVol[Volume].Dac_DigVol);
                }
                else
                {
                    rk_printf("HPAMPVol = %d",ACodec_HPoutVol[Volume].HP_AMPVol);
                    ACodec_Set_HP_AMP(ACodec_HPoutVol[Volume].HP_AMPVol);

                    rk_printf("DacDigVol = %d",ACodec_HPoutVol[Volume].Dac_DigVol);
                    ACodec_Set_DAC_DigVol(ACodec_HPoutVol[Volume].Dac_DigVol);
                }
                break;
            case EQ_POP:
            case EQ_HEAVY:
                if(VOLTAB_CONFIG == VOL_Europe)
                {
                    ACodec_Set_DAC_DigVol(CodecConfig_Europe[Volume].DacDigVol);
                }
                else
                {
                    ACodec_Set_DAC_DigVol(CodecConfig_General[Volume].DacDigVol);
                }
                break;
            case EQ_JAZZ:
            case EQ_UNIQUE:
                if(VOLTAB_CONFIG == VOL_Europe)
                {
                   ACodec_Set_DAC_DigVol(CodecConfig_Europe[Volume].DacDigVol);
                }
                else
                {
                    ACodec_Set_DAC_DigVol(CodecConfig_General[Volume].DacDigVol);
                }
                break;

            case EQ_USER:
                if(VOLTAB_CONFIG == VOL_Europe)
                {
                    ACodec_Set_DAC_DigVol(CodecConfig_Europe[Volume].DacDigVol);
                }
                else
                {
                    ACodec_Set_DAC_DigVol(CodecConfig_General[Volume].DacDigVol);
                }
                break;

            case EQ_BASS:
                if(VOLTAB_CONFIG == VOL_Europe)
                {
                    ACodec_Set_DAC_DigVol(CodecConfig_Europe[Volume].DacDigVol);
                }
                else
                {
                    ACodec_Set_DAC_DigVol(CodecConfig_General[Volume].DacDigVol);
                }

                break;

            default:
                break;
        }
    }

    DelayUs(10);
}
*/
/*
--------------------------------------------------------------------------------
  Function name : void Codec_DACMute(void)
  Author        : yangwenjie
  Description   : set codec mute

  Input         : null

  Return        : null

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  Note:      this function only used when DAC working
--------------------------------------------------------------------------------
*/
_LIB_ROCKCODEC_COMMON_
void Codec_DACMute(void)
{
    ACodec_DACMute();
}

/*
--------------------------------------------------------------------------------
  Function name : void Codec_DACUnMute(void)
  Author        : yangwenjie
  Description   : set codec exit from mute.

  Input         :

  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  note:      this function only used when DAC working
--------------------------------------------------------------------------------
*/
_LIB_ROCKCODEC_COMMON_
void Codec_DACUnMute(void)
{
    ACodec_DACUnMute();
}

_LIB_ROCKCODEC_COMMON_
void Codec_ADCPOWER_UP(void)
{
    WriteAcodecReg(ACODEC_BASE+ACODEC_ADCCFG0,0x10);
    rk_printf("Codec_ADCPOWER_UP = 0x%x",ReadAcodecReg(ACODEC_BASE+ACODEC_ADCCFG0));
}
/*
--------------------------------------------------------------------------------
  Function name : void Codec_DACUnMute(void)
  Author        : yangwenjie
  Description   : set codec exit from mute.

  Input         :

  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  note:      this function only used when DAC working
--------------------------------------------------------------------------------
*/
_LIB_ROCKCODEC_COMMON_
void Codec_ADCPOWER_DOWN(void)
{
    WriteAcodecReg(ACODEC_BASE+ACODEC_ADCCFG0,0xD8);
    rk_printf("Codec_ADCPOWER_DOWN = 0x%x",ReadAcodecReg(ACODEC_BASE+ACODEC_ADCCFG0));

}
/*
--------------------------------------------------------------------------------
  Function name : void Codec_DACUnMute(void)
  Author        : yangwenjie
  Description   : set codec exit from mute.

  Input         :

  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  note:      this function only used when DAC working
--------------------------------------------------------------------------------
*/
_LIB_ROCKCODEC_COMMON_
void Codec_ADC_MUTE(void)
{
    //WriteAcodecReg(ACODEC_BASE+ACODEC_ADCCFG1, 0xc0);
    WriteAcodecReg(ACODEC_BASE+ACODEC_DACACFG2, 0x07);
    rk_printf("ACODEC_DACACFG2 = 0x%x",ReadAcodecReg(ACODEC_BASE+ACODEC_DACACFG2));

}
/*
--------------------------------------------------------------------------------
  Function name : void Codec_DACUnMute(void)
  Author        : yangwenjie
  Description   : set codec exit from mute.

  Input         :

  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  note:      this function only used when DAC working
--------------------------------------------------------------------------------
*/
_LIB_ROCKCODEC_COMMON_
void Codec_ADC_UnMute(void)
{
    WriteAcodecReg(ACODEC_BASE+ACODEC_DACACFG2, 0x00);
    rk_printf("ACODEC_DACACFG2 = 0x%x",ReadAcodecReg(ACODEC_BASE+ACODEC_DACACFG2));

}
/*
--------------------------------------------------------------------------------
  Function name : void Codec_DACUnMute(void)
  Author        : yangwenjie
  Description   : set codec exit from mute.

  Input         :

  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  note:      this function only used when DAC working
--------------------------------------------------------------------------------
*/
_LIB_ROCKCODEC_COMMON_
void Codec_MUX_Power_down(void)
{
    WriteAcodecReg(ACODEC_BASE+ACODEC_LICFG4,0x33);
    //WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG1,0x05);
    //WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG0,0x01);
    rk_printf("Codec_MUX_Power_down = 0x%x",ReadAcodecReg(ACODEC_BASE+ACODEC_LICFG4));

}
/*
--------------------------------------------------------------------------------
  Function name : void Codec_DACUnMute(void)
  Author        : yangwenjie
  Description   : set codec exit from mute.

  Input         :

  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  note:      this function only used when DAC working
--------------------------------------------------------------------------------
*/
_LIB_ROCKCODEC_COMMON_
void Codec_MUX_Power_up(void)
{
    WriteAcodecReg(ACODEC_BASE+ACODEC_LICFG4,0x30);
    //WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG1,0x01);
    //WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG0,0x00);

    rk_printf("Codec_MUX_Power_up = 0x%x",ReadAcodecReg(ACODEC_BASE+ACODEC_LICFG4));

}

/*******************************************************************************
** Name: ACodec_Set_HPLO_Mode
** Input:pll_target
** Return: void
*******************************************************************************/
void ACodec_Set_HPLO_Mode(CodecOut_sel_t OUT_Mode)
{
    uint32 config;

    switch (OUT_Mode)
    {
        case CodecOut_Sel_HP:

            break;

        case CodecOut_Sel_LINE:

            break;

        default:
            break;
    }
}

/*******************************************************************************
** Name: ACodec_Set_I2S_RX_Mode
** Input:HDC dev,  CodecFS_en_t CodecFS
** Return: rk_err_t
** Owner: cjh
** Date: 2016.4.14
** Time: 14:18:09
*******************************************************************************/
void ACodec_Set_I2S_RX_Mode(eACodecI2sFormat_t BUS_FORMAT,
                         eACodecI2sDATA_WIDTH_t I2S_Data_width,
                         ACodecI2S_BUS_MODE_t I2S_Bus_mode,
                         ACodecI2S_mode_t I2S_mode)
{
    Acodec_I2S_MODE = I2S_mode;
    ACodec_I2S_RX_Init(BUS_FORMAT,I2S_Data_width,I2S_Bus_mode);

    MaskAcodecRegBits(ACODEC_BASE+ACODEC_I2SCKM,(I2S_MST_MASTER) | (SCK_EN_DISABLE),
                      I2S_mode | SCK_EN_ENABLE);

    //WriteAcodecReg(ACODEC_BASE+ACODEC_I2SCKM,I2S_mode | SCK_EN_ENABLE);
}

/*******************************************************************************
** Name: ACodec_I2S_Init
** Input:
** Return: void
*******************************************************************************/
//_ATTR_SYS_INIT_CODE_
void ACodec_Set_I2S_Mode(eACodecI2sFormat_t BUS_FORMAT,
                         eACodecI2sDATA_WIDTH_t I2S_Data_width,
                         ACodecI2S_BUS_MODE_t I2S_Bus_mode,
                         ACodecI2S_mode_t I2S_mode)
{
    Acodec_I2S_MODE = I2S_mode;
    ACodec_I2S_TX_Init(BUS_FORMAT,I2S_Data_width,I2S_Bus_mode);
    //ACodec_I2S_RX_Init(BUS_FORMAT,I2S_Data_width,I2S_Bus_mode);

    MaskAcodecRegBits(ACODEC_BASE+ACODEC_I2SCKM,(I2S_MST_MASTER) | (SCK_EN_DISABLE),
                      I2S_mode | SCK_EN_ENABLE);

    //WriteAcodecReg(ACODEC_BASE+ACODEC_I2SCKM,I2S_mode | SCK_EN_ENABLE);
}

/*******************************************************************************
** Name: ACodec_Set_DAC_Mode
** Input:pll_target
** Return: void
*******************************************************************************/
void ACodec_Set_DAC_Mode(CodecOut_sel_t OUT_Mode,CodecFS_en_t CodecFS)
{
    uint32 config = 0;
    //uint32 DACSRT_NUM;
    uint32 timeout = 200000;

    WriteAcodecReg(ACODEC_BASE+ACODEC_DACMUTE,DACMTE_ENABLE);    //DAC mute

    config |= DACBYPS_DISABLE; //DACBYPS DISABLE
    config |= DACFADE_AS_DACCZDT; //DACFADE = 1
    config |= DACCZDT_1; //volume adjusts only when audio waveform crosses zero or volume-control time-limit condition meets

    MaskAcodecRegBits(ACODEC_BASE+ACODEC_VCTL,DACBYPS_ENABLE,config);
    config = 0;

    //VCTIME NO CONFIG

    //DACSRT_NUM = ACodec_Get_SRT_TIME(CodecFS);

    //WriteAcodecReg(ACODEC_BASE+ACODEC_DACSR,DACSRT_NUM);

    //DAC L/R power on
    //enable dac clk ;enbale i2s_rx clk
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_CLKE,DAC_CKE_DISABLE | I2SRX_CKE_DISABLE,
                                                DAC_CKE_ENABLE | I2SRX_CKE_ENABLE);


    //enbale i2s_rx
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_DIGEN,I2SRX_EN_DISABLE,I2SRX_EN_ENABLE);

    ACodec_I2S_RX_Start();

    rkos_sleep(1);

    //enbale dac
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_DIGEN,DAC_EN_DISABLE,DAC_EN_ENABLE);
    rkos_sleep(1);


    MaskAcodecRegBits(ACODEC_BASE+ACODEC_HPLOCFG1,(0x1 << 0),0x1);

    MaskAcodecRegBits(ACODEC_BASE+ACODEC_RTCFG0,(0xff << 0),0x1);
    rkos_sleep(1);

    MaskAcodecRegBits(ACODEC_BASE+ACODEC_RTCFG1,(0x7 << 0),0x1);
    rkos_sleep(1);

    Codec_Power_Status = Codec_Power_on;
    //power on analog part of DAC
    if(OUT_Mode == CodecOut_Sel_HP)
    {
        ACodec_po_dac_hp();
    }
    else if(OUT_Mode == CodecOut_Sel_LINE)
    {
        ACodec_po_dac_lo();
    }
    rkos_sleep(2);
}

/*******************************************************************************
** Name: ACodec_Set_ADC_Mode
** Input:pll_target
** Return: void
*******************************************************************************/
void ACodec_Set_ADC_Mode(CodecIn_sel_t In_Mode, CodecFS_en_t CodecFS)
{
    uint32 config = 0;
    //uint32 ADCSRT_NUM;
    uint32 timeout = 200000;
    //mute the left/right channel of ADC
    //WriteAcodecReg(ACODEC_BASE+ACODEC_ADCCFG1,0xc0);
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_ADCCFG1,ADC_MUTE_L_DISABLE | ADC_MUTE_R_DISABLE,
                                                    ADC_MUTE_L_ENABLE | ADC_MUTE_R_ENABLE);

    ACodec_Set_MICLI_Mode(In_Mode,MicBias_sel_20V,CodecMIC_Diff_Mode); //mic/line config


    config |= ADCBYPS_DISABLE; //ADCBYPS DISABLE
    config |= ADCFADE_AS_ADCCZDT; //ADCFADE = 1
    config |= ADCCZDT_1; //volume adjusts only when audio waveform crosses zero or volume-control time-limit condition meets

    MaskAcodecRegBits(ACODEC_BASE+ACODEC_VCTL,ADCBYPS_ENABLE,config);
    config = 0;

    //VCTIME NO CONFIG

    //ADCSR SET
    //ADCSRT_NUM = ACodec_Get_SRT_TIME(CodecFS);
    //WriteAcodecReg(ACODEC_BASE+ACODEC_ADCSR,ADCSRT_NUM);

    //ADC L/R power on
    if(In_Mode == Codecin_Sel_MIC1_MONO)
    {
        WriteAcodecReg(ACODEC_BASE+ACODEC_ADCCFG0,0x18 | ADC_R_PWD_DOWN);
    }
    else if(In_Mode == Codecin_Sel_MIC2_MONO)
    {
        WriteAcodecReg(ACODEC_BASE+ACODEC_ADCCFG0,0x18 | ADC_L_PWD_DOWN);
    }
    else
    {
        WriteAcodecReg(ACODEC_BASE+ACODEC_ADCCFG0,0x18);
    }

    //enable ADC and i2s tx clk
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_CLKE,ADC_CKE_DISABLE | I2STX_CKE_DISABLE,
                                                    ADC_CKE_ENABLE | I2STX_CKE_ENABLE);

    //enbale i2s_tx
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_DIGEN, I2STX_EN_DISABLE , I2STX_EN_ENABLE );

    // tx transfer start
    ACodec_I2S_TX_Start();

    //wait 1ms
    //DelayMs(1);

    //enable adc
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_DIGEN, ADC_EN_DISABLE , ADC_EN_ENABLE );

    //wait 1ms
    rkos_sleep(1);

    //unmute the left/right channel of ADC
    if(In_Mode == Codecin_Sel_MIC1_MONO)
    {
        WriteAcodecReg(ACODEC_BASE+ACODEC_ADCCFG1, 0x00 | ADC_MUTE_R_ENABLE);
    }
    else if(In_Mode == Codecin_Sel_MIC2_MONO)
    {
        WriteAcodecReg(ACODEC_BASE+ACODEC_ADCCFG1, 0x00 | ADC_MUTE_L_ENABLE);
    }
    else
    {
    WriteAcodecReg(ACODEC_BASE+ACODEC_ADCCFG1, 0x00);
    }
    rkos_sleep(1);
    //SET ALC
    ACodec_Set_ADC_ALC();
}



/*******************************************************************************
** Name: ACodec_Exit_DAC_Mode
** Input:pll_target
** Return: void
*******************************************************************************/
void ACodec_Exit_DAC_Mode()
{
    //auto power down
    if (ACODEC_OUT_CONFIG == ACODEC_OUT_HP)
    {
        ACodec_pd_dac_hp();
    }
    else if (ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
    {
        ACodec_pd_dac_lo();
    }

    //disable dac
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_DIGEN,DAC_EN_ENABLE,DAC_EN_DISABLE);
    //printf("ACODEC_DIGEN = 0x%x",ReadAcodecReg(ACODEC_BASE+ACODEC_DIGEN));

    //disable I2S RX
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_DIGEN,I2SRX_EN_ENABLE,I2SRX_EN_DISABLE);
    //printf("ACODEC_DIGEN = 0x%x",ReadAcodecReg(ACODEC_BASE+ACODEC_DIGEN));

    ACodec_I2S_RX_Stop();

    //disable dac clk ;disable i2s_rx clk
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_CLKE,I2SRX_CKE_ENABLE,I2SRX_CKE_DISABLE);
    //printf("ACODEC_CLKE = 0x%x",ReadAcodecReg(ACODEC_BASE+ACODEC_CLKE));

    //DelayMs(1500);
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_CLKE,DAC_CKE_ENABLE,DAC_CKE_DISABLE);
    //printf("ACODEC_CLKE = 0x%x",ReadAcodecReg(ACODEC_BASE+ACODEC_CLKE));

}

/*******************************************************************************
** Name: ACodec_Exit_ADC_Mode
** Input:
** Return: void
*******************************************************************************/
void ACodec_Exit_ADC_Mode()
{
    uint32 config = 0;
    uint32 ADCSRT_NUM;
    uint32 timeout = 200000;

    MaskAcodecRegBits(ACODEC_BASE+ACODEC_ADCCFG1,ADC_MUTE_L_DISABLE | ADC_MUTE_R_DISABLE,
                                                    ADC_MUTE_L_ENABLE | ADC_MUTE_R_ENABLE);
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_CLKE,ADC_CKE_ENABLE | I2STX_CKE_ENABLE,
                                                ADC_CKE_DISABLE | I2STX_CKE_DISABLE);
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_DIGEN , I2STX_EN_ENABLE, I2STX_EN_DISABLE );
    ACodec_I2S_TX_Stop();
    rkos_sleep(1);
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_DIGEN , ADC_EN_ENABLE, ADC_EN_DISABLE);
    Codec_ADCPOWER_DOWN();
    //WriteAcodecReg(ACODEC_BASE+ACODEC_ADCCFG0,0xD8);
    rkos_sleep(1);
    ACodec_Set_ADC_ALC();
}

/*******************************************************************************
** Name: ACodec_Set_ADC_Mode
** Input:
** Return: void
*******************************************************************************/
void ACodec_Set_MICLI_Mode(CodecIn_sel_t In_Mode,CodecMicBias_sel_t Mic_Bias,CodecMic_Mode_t MIC_Mode)
{
    uint32 config;

    switch (In_Mode)
    {
        case Codecin_Sel_MIC1_MONO:
            if(MIC_Mode == CodecMIC_Normal_Mode)
            {
                WriteAcodecReg(ACODEC_BASE+ACODEC_LICFG1,MUX_L_IN_SEL_MIC);
            }
            else
            {
                WriteAcodecReg(ACODEC_BASE+ACODEC_LICFG1,MUX_L_IN_SEL_MIC | MIC_L_DIFF_EN_ENABLE);
            }
            WriteAcodecReg(ACODEC_BASE+ACODEC_LICFG4,MIC_L_PD_ON  | MUX_L_PD_ON );
            MaskAcodecRegBits(ACODEC_BASE+ACODEC_LICFG0,MICBIAS_SEL_MASK,Mic_Bias);
            break;
        case Codecin_Sel_MIC2_MONO:
            if(MIC_Mode == CodecMIC_Normal_Mode)
            {
                WriteAcodecReg(ACODEC_BASE+ACODEC_LICFG1,MUX_R_IN_SEL_MIC);
            }
            else
            {
                WriteAcodecReg(ACODEC_BASE+ACODEC_LICFG1,MUX_R_IN_SEL_MIC | MIC_R_DIFF_EN_ENABLE);
            }
            WriteAcodecReg(ACODEC_BASE+ACODEC_LICFG4,MIC_R_PD_ON | MUX_R_PD_ON);
            MaskAcodecRegBits(ACODEC_BASE+ACODEC_LICFG0,MICBIAS_SEL_MASK,Mic_Bias);
            break;
        case Codecin_Sel_MIC_STERO:
            if(MIC_Mode == CodecMIC_Normal_Mode)
            {
                WriteAcodecReg(ACODEC_BASE+ACODEC_LICFG1,MUX_L_IN_SEL_MIC | MUX_R_IN_SEL_MIC);
            }
            else
            {
                WriteAcodecReg(ACODEC_BASE+ACODEC_LICFG1,MUX_L_IN_SEL_MIC | MUX_R_IN_SEL_MIC | MIC_R_DIFF_EN_ENABLE | MIC_L_DIFF_EN_ENABLE);
            }
            WriteAcodecReg(ACODEC_BASE+ACODEC_LICFG4,MIC_L_PD_ON | MIC_R_PD_ON | MUX_L_PD_ON | MUX_R_PD_ON);
            //select the output voltage of MIC bias
            MaskAcodecRegBits(ACODEC_BASE+ACODEC_LICFG0,MICBIAS_SEL_MASK,Mic_Bias);
            break;

        case Codecin_Sel_LINE1:
            WriteAcodecReg(ACODEC_BASE+ACODEC_LICFG1,MUX_L_IN_SEL_LINE_1 | MUX_R_IN_SEL_LINE_1);
            WriteAcodecReg(ACODEC_BASE+ACODEC_LICFG4,MUX_L_PD_ON | MUX_R_PD_ON | MIC_L_PD_DOWN | MIC_R_PD_DOWN);
            //ACodec_Set_ADCMUX_Vol(12);

            break;
        case Codecin_Sel_LINE2:
            WriteAcodecReg(ACODEC_BASE+ACODEC_LICFG1,MUX_L_IN_SEL_LINE_2 | MUX_R_IN_SEL_LINE_2);
            WriteAcodecReg(ACODEC_BASE+ACODEC_LICFG4,MUX_L_PD_ON | MUX_R_PD_ON | MIC_L_PD_DOWN | MIC_R_PD_DOWN);

            break;
        default:
            break;
    }
}

/*******************************************************************************
** Name: ACodec_Set_LineIn_Limiter
** Input:
** Return: void
*******************************************************************************/
void ACodec_Set_LineIn_Limiter()
{

}
/*******************************************************************************
** Name: ACodec_Set_DAC_Limiter
** Input:
** Return: void
*******************************************************************************/
void ACodec_Set_DAC_Limiter()
{

}
/*******************************************************************************
** Name: ACodec_Set_ADC_HighPassFilter
** Input:
** Return: void
*******************************************************************************/
void ACodec_Set_ADC_HighPassFilter()
{
    WriteAcodecReg(ACODEC_BASE+ACODEC_ADCHPF,0XC0);
}
/*******************************************************************************
** Name: ACodec_Set_ADC_NoiseGate
** Input:
** Return: void
*******************************************************************************/
void ACodec_Set_ADC_NoiseGate()
{
    WriteAcodecReg(ACODEC_BASE+ACODEC_ADCNG,0XE0);
}
/*******************************************************************************
** Name: ACodec_Set_ADC_ALC
** Input:
** Return: void
*******************************************************************************/
void ACodec_Set_ADC_ALC()
{

}

/*******************************************************************************
** Name: ACodec_I2S_RX_Stop
** Input:void
** Return: void
*******************************************************************************/
//_ATTR_SYS_INIT_CODE_
void ACodec_I2S_RX_Stop(void)
{
    WriteAcodecReg(ACODEC_BASE+ACODEC_I2SRXCMD,RX_STOP);
}

/*******************************************************************************
** Name: ACodec_I2S_TX_Start
** Input:void
** Return: void
*******************************************************************************/
//_ATTR_SYS_INIT_CODE_
void ACodec_I2S_TX_Start(void)
{
    WriteAcodecReg(ACODEC_BASE+ACODEC_I2STXCMD,TX_START);
}

/*******************************************************************************
** Name: ACodec_I2S_TX_Stop
** Input:void
** Return: void
*******************************************************************************/
//_ATTR_SYS_INIT_CODE_
void ACodec_I2S_TX_Stop(void)
{
    WriteAcodecReg(ACODEC_BASE+ACODEC_I2STXCMD,TX_STOP);
}

/*******************************************************************************
** Name: ACodec_PLL_Set
** Input:pll_target
** Return: void
*******************************************************************************/
//_ATTR_SYS_INIT_CODE_
void ACodec_PLL_Set(eF_SOURCE_ID F_SOURCE,eACodecPll_Target_Freq pll_target)
{
    uint32 MHz,POSDIV_L3,POSDIV_H8,PREDIV_BIT,OUTDIV;
    uint32 config,i;

    if (F_SOURCE >= F_SOURCE_MAX)
    {
        return;
    }

    if(pll_old_target == pll_target)
    {
       return;  //如果PLL一样就不需要重新设置，只需要配置SCK_DIV即可
    }

    if(ACodec_I2S_MASTER_MODE == Acodec_I2S_MODE)
    {
        switch (pll_target)
        {
            case Pll_Target_Freq_40960:
                POSDIV_L3 = ACodecpllTable_40960[F_SOURCE].PLL_POSDIV_L3;
                POSDIV_H8 = ACodecpllTable_40960[F_SOURCE].PLL_POSDIV_H8;
                PREDIV_BIT = ACodecpllTable_40960[F_SOURCE].PLL_PREDIV_BIT;
                OUTDIV = ACodecpllTable_40960[F_SOURCE].PLL_OUTDIV;
                break;

            case Pll_Target_Freq_56448:
                POSDIV_L3 = ACodecpllTable_56448[F_SOURCE].PLL_POSDIV_L3;
                POSDIV_H8 = ACodecpllTable_56448[F_SOURCE].PLL_POSDIV_H8;
                PREDIV_BIT = ACodecpllTable_56448[F_SOURCE].PLL_PREDIV_BIT;
                OUTDIV = ACodecpllTable_56448[F_SOURCE].PLL_OUTDIV;
                break;

            case Pll_Target_Freq_61440:
                POSDIV_L3 = ACodecpllTable_61440[F_SOURCE].PLL_POSDIV_L3;
                POSDIV_H8 = ACodecpllTable_61440[F_SOURCE].PLL_POSDIV_H8;
                PREDIV_BIT = ACodecpllTable_61440[F_SOURCE].PLL_PREDIV_BIT;
                OUTDIV = ACodecpllTable_61440[F_SOURCE].PLL_OUTDIV;
                break;

            default:
                break;
        }
        pll_old_target = pll_target;

        //disable out vco
        MaskAcodecRegBits(ACODEC_BASE+ACODEC_PLLCFG0,PLL_OUTDIV_ENABLE,PLL_OUTDIV_DISABLE);

        //PLL RESET pwd down
        MaskAcodecRegBits(ACODEC_BASE+ACODEC_PLLCFG5,APLL_RELEASE_RESET,APLL_RESET);
        //MaskAcodecRegBits(ACODEC_BASE+ACODEC_PLLCFG5,PLL_PW_UP,PLL_PW_DOWN);

        if(F_SOURCE == F_SOURCE_24000KHz)
        {
            //config = ReadAcodecReg(ACODEC_BASE+ACODEC_PLLCFG0);
            //Select 24M as the source clock of PLL
            MaskAcodecRegBits(ACODEC_BASE+ACODEC_PLLCFG0,(0x03 << 3),PLL_CLKIN_SEL_S_CLOCK);

            WriteAcodecReg(ACODEC_BASE+ACODEC_PLLCFG1,POSDIV_L3);
            WriteAcodecReg(ACODEC_BASE+ACODEC_PLLCFG2,POSDIV_H8);
            WriteAcodecReg(ACODEC_BASE+ACODEC_PLLCFG3,PREDIV_BIT);
            MaskAcodecRegBits(ACODEC_BASE+ACODEC_PLLCFG4,(0xf << 4),(OUTDIV << 4));

            //PLL power on

            MaskAcodecRegBits(ACODEC_BASE+ACODEC_PLLCFG5,PLL_PW_DOWN,PLL_PW_UP);
            MaskAcodecRegBits(ACODEC_BASE+ACODEC_PLLCFG5,APLL_RESET,APLL_RELEASE_RESET);


            //config = ReadAcodecReg(ACODEC_BASE+ACODEC_PLLCFG0);
            rkos_sleep(5);
            //enable out vco
            MaskAcodecRegBits(ACODEC_BASE+ACODEC_PLLCFG0,PLL_OUTDIV_DISABLE,PLL_OUTDIV_ENABLE);
        }

    }
    else
    {
        //考虑到I2S无法分出全部的采样率，codec推荐做主，做从暂时不考虑
    }
}

/*******************************************************************************
** Name: ACodec_I2S_RX_Start
** Input:void
** Return: void
*******************************************************************************/
//_ATTR_SYS_INIT_CODE_
void ACodec_I2S_RX_Start(void)
{
    WriteAcodecReg(ACODEC_BASE+ACODEC_I2SRXCMD,RX_START);
}

/*******************************************************************************
** Name: ACodec_I2s_SetSampleRate
** Input:Sck_div
** Return: void
*******************************************************************************/
void ACodec_I2s_SetSampleRate(uint32 Sck_div, CodecFS_en_t CodecFS)
{
    uint32 config;
    uint32 SRT_NUM;

    //config = ReadAcodecReg(ACODEC_BASE+ACODEC_I2SCKM);
    //WriteAcodecReg(ACODEC_BASE+ACODEC_I2SCKM,config | (Sck_div << 4));
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_I2SCKM,(0xF << 4),(Sck_div << 4));

    //ADCSR SET
    SRT_NUM = ACodec_Get_SRT_TIME(CodecFS);
    WriteAcodecReg(ACODEC_BASE+ACODEC_DACSR,SRT_NUM);

    //ADCSR SET
    //ADCSRT_NUM = ACodec_Get_SRT_TIME(CodecFS);
    WriteAcodecReg(ACODEC_BASE+ACODEC_ADCSR,SRT_NUM);

}

/*******************************************************************************
** Name: ACodec_PLL_Set
** Input:pll_target
** Return: void
*******************************************************************************/
//_ATTR_SYS_INIT_CODE_
void ACodec_Soft_Reset(void)
{
    uint32 timeout = 200000;
    WriteAcodecReg(ACODEC_BASE+ACODEC_SRST,SOFT_RESET);

    while (ReadAcodecReg(ACODEC_BASE+ACODEC_SRST))
    {
        if (--timeout == 0)
        {
            rk_printf("ACodec_Soft_Reset fail");
            break;
        }
    }
}

/*******************************************************************************
** Name: ACodec_Set_HP_Gain
** Input:vol 1db/step: (0-0xff)0dB - -15dB
** Return:
*******************************************************************************/
uint8 ACodec_Set_HP_Gain(uint8 vol)
{
    //printf("ACodec_Set_HP_Gain = %d\n",vol);
    WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG4, vol | HP_ANTIPOP_ENABLE);
    return vol;
}

/*******************************************************************************
** Name: ACodec_Set_LO_Gain
** Input:vol 1db/step: (0-0xff)0dB - -15dB
** Return:
*******************************************************************************/
uint8 ACodec_Set_LO_Gain(uint8 vol)
{
    WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5, vol | LO_ANTIPOP_ENABLE);
    return vol;
}


/*******************************************************************************
** Name: ACodec_Set_HP_AMP
** Input:vol
** Return: void
*******************************************************************************/
void ACodec_Set_HP_AMP(eACodecHp_AMP vol)
{
    uint32 config;
    //printf("ACodec_Set_HP_Gain = %d",vol);
    if(vol != 0)
    {
        MaskAcodecRegBits(ACODEC_BASE+ACODEC_HPLOCFG4, HP_ANTIPOP_ENABLE, HP_ANTIPOP_DISABLE);
    }
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_HPLOCFG1,(0x3 << 3),vol << 3);
}

/*******************************************************************************
** Name: ACodec_Set_LO_AMP
** Input:vol
** Return: void
*******************************************************************************/
void ACodec_Set_LO_AMP(eACodecLo_AMP vol)
{
    uint32 config;
    if(vol != 0)
    {
        MaskAcodecRegBits(ACODEC_BASE+ACODEC_HPLOCFG4, LO_ANTIPOP_DISABLE, LO_ANTIPOP_ENABLE);
    }
    MaskAcodecRegBits(ACODEC_BASE+ACODEC_HPLOCFG0,(0x3 << 2),vol << 2);
}

/*******************************************************************************
** Name: ACodec_Set_MIC_AnaVol
** Input:   0-3
         2'b00: 0dB
         2'b01: 10dB
         2'b10: 20dB
         2'b11: 30dB
** Return: void
** Owner:
** Date: 2015.5.20
** Time: 10:47:23
*******************************************************************************/
uint8 ACodec_Set_MIC_AnaVol(uint8 vol)
{
    uint8 vol_l,vol_r;
    uint32 config;

    if (vol > 3)
        return 0;

    vol_r = vol << 0;
    vol_l = vol << 2;

    MaskAcodecRegBits(ACODEC_BASE+ACODEC_LICFG0,0xF,vol_r | vol_l);
    return vol;

}

/*******************************************************************************
** Name: ACodec_Set_ADCMUX_Vol
** Input: uint8 vol
** Return: void
** Owner:
** Date: 2015.5.20
** Time: 10:47:23
*******************************************************************************/
uint8 ACodec_Set_ADCMUX_Vol(uint8 vol)
{
    uint8 vol_l,vol_r;

    if (vol > 15)
        return 0;

    vol_r = vol << 0;
    vol_l = vol << 4;

    WriteAcodecReg(ACODEC_BASE+ACODEC_LICFG2,vol_r | vol_l);

    return vol;

}

/*******************************************************************************
** Name: ACodec_Set_DAC_DigVol
** Input: uint8 vol
** Return: void
** Owner:
** Date: 2015.5.20
** Time: 10:47:23
*******************************************************************************/
uint8  ACodec_Set_DAC_DigVol(uint8 vol)
{
    WriteAcodecReg(ACODEC_BASE+ACODEC_DACVCTLL,vol);    //DAC left channel volume
    WriteAcodecReg(ACODEC_BASE+ACODEC_DACVCTLR,vol);    //DAC right channel volume
    return vol;
}

/*******************************************************************************
** Name: ACodec_Set_ADC_DigVol
** Input: uint8 vol
** Return: void
** Owner:
** Date: 2015.5.20
** Time: 10:47:23
*******************************************************************************/
uint8 ACodec_Set_ADC_DigVol(uint8 vol)
{
    uint32 config;
    WriteAcodecReg(ACODEC_BASE+ACODEC_ADCVCTLL,vol);    //ADC left channel volume
    WriteAcodecReg(ACODEC_BASE+ACODEC_ADCVCTLR,vol);    //ADC right channel volume
    return vol;

}

/*******************************************************************************
** Name: ACodec_DACUnMute
** Input: void
** Return: void
** Owner:
** Date: 2015.5.20
** Time: 10:47:23
*******************************************************************************/
void ACodec_DACUnMute(void)
{
    WriteAcodecReg(ACODEC_BASE+ACODEC_DACMUTE,DACMTE_DISABLE);    //DAC Unmute
}

/*******************************************************************************
** Name: ACodec_DACMute
** Input: void
** Return: void
** Owner:
** Date: 2015.5.20
** Time: 10:47:23
*******************************************************************************/
void ACodec_DACMute(void)
{
    WriteAcodecReg(ACODEC_BASE+ACODEC_DACMUTE,DACMTE_ENABLE);    //DAC mute
}
/*******************************************************************************
** Name: ACodec_ADC2DAC_MIX
** Input:CodecMIX_Mode_t MIX_en
** Return: void
** Owner:
** Date: 2015.5.20
** Time: 10:47:23
*******************************************************************************/
void ACodec_ADC2DAC_MIX(CodecMIX_Mode_t MIX_en)
{
    if (MIX_en == CodecMIX_ENABLE)
    {
        WriteAcodecReg(ACODEC_BASE+ACODEC_MIXCTRL,MIXE_ENABLE);    //MIX
    }
    else
    {
        WriteAcodecReg(ACODEC_BASE+ACODEC_MIXCTRL,MIXE_DISABLE);    //NO MIX
    }

}

/*******************************************************************************
** Name: ACodec_WriteReg
** Input:uint32 regaddr,uint32 data
** Return: void
** Owner:
** Date: 2015.5.20
** Time: 10:47:23
*******************************************************************************/
void ACodec_WriteReg(uint32 regaddr,uint32 data)
{
    WriteAcodecReg(regaddr,data);
    ACodec_ReadReg_printf(regaddr);
}

/*******************************************************************************
** Name: ACodec_Set_DAC_Mode
** Input:pll_target
** Return: void
*******************************************************************************/
void ACodec_ReadReg_printf(uint32 regaddr)
{
    rk_printf("reg data = %d",ReadAcodecReg(regaddr));
}


/*******************************************************************************
** Name: ACodec_po_dac_lo
** Input:void
** Return: void
** Owner:cjh
** Date: 2015.5.20
** Time: 10:40:50
*******************************************************************************/
void ACodec_po_dac_lo(void)
{
    int i = 0;
    uint32 antipop_bit_tmp = 0;
    WriteAcodecReg(ACODEC_BASE+ACODEC_DACMUTE,DACMTE_ENABLE);    //DAC mute
    rkos_sleep(1);
    //ClrAcodecRegBit(ACODEC_BASE+ACODEC_HPLOCFG5, 5); // clr lo buf pwd
    //DelayMs(1);
    //ClrAcodecRegBit(ACODEC_BASE+ACODEC_HPLOCFG5, 6); // clr lo ostg pwd
    //DelayMs(1);
    WriteAcodecReg(ACODEC_BASE+ACODEC_DACACFG2, PWD_DACBIAS_ON | PWD_DACL_ON | PWD_DACR_ON); // power down dac ibias/l/r
    rkos_sleep(1);
    for (i = 0; i < 16; i++)
    {
        antipop_bit_tmp = ReadAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5);
        antipop_bit_tmp = (antipop_bit_tmp & 0xf0) | (0xf - i);
        WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5, antipop_bit_tmp | LO_ANTIPOP_ENABLE);
        rkos_sleep(1);
    }
    //rk_printf("ACODEC_HPLOCFG5 = %d",ReadAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5));
    WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5, LO_ANTIPOP_DISABLE);

    //rk_printf("ACODEC_HPLOCFG5 = %d",ReadAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5));
    rkos_sleep(1);
    WriteAcodecReg(ACODEC_BASE+ACODEC_DACMUTE, DACMTE_DISABLE); // unmute
    rkos_sleep(1);
}

/*******************************************************************************
** Name: ACodec_pd_dac_lo
** Input:void
** Return: void
** Owner:
** Date: 2015.5.20
** Time: 10:40:50
*******************************************************************************/
void ACodec_pd_dac_lo()
{
    uint32 i = 0;
    uint32 antipop_bit_tmp = 0;
    WriteAcodecReg(ACODEC_BASE+ACODEC_DACMUTE,DACMTE_ENABLE);    //DAC mute
    rkos_sleep(1);
    WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5, LO_ANTIPOP_ENABLE); // clr lo antipop_en
    rkos_sleep(1);
    rk_printf("ACODEC_HPLOCFG5 = %d",ReadAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5));
    for (i = 0; i < 16; i++) {
        antipop_bit_tmp = ReadAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5);
        antipop_bit_tmp =  i;
        WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5, antipop_bit_tmp | LO_ANTIPOP_ENABLE);
        rkos_sleep(1);
    }
    rk_printf("ACODEC_HPLOCFG5 = %d",ReadAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG5));

    WriteAcodecReg(ACODEC_BASE+ACODEC_DACACFG2, PWD_DACBIAS_DOWN | PWD_DACL_ON | PWD_DACR_ON); // power down dac ibias/l/r
    rkos_sleep(1);
}

/*******************************************************************************
** Name: ACodec_po_dac_hp
** Input:void
** Return: void
** Owner:
** Date: 2015.5.20
** Time: 10:40:50
*******************************************************************************/
void ACodec_po_dac_hp()
{
    int i = 0;
    uint32 antipop_bit_tmp = 0;

    WriteAcodecReg(ACODEC_BASE+ACODEC_DACACFG2, PWD_DACBIAS_ON | PWD_DACL_ON | PWD_DACR_ON); // power down dac ibias/l/r

    //rk_printf("ACODEC_DACACFG2 = %d",ReadAcodecReg(ACODEC_BASE+ACODEC_DACACFG2));

    for (i = 0; i < 16; i++)
    {
        antipop_bit_tmp = ReadAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG4);
        antipop_bit_tmp = (antipop_bit_tmp & 0xf0) | (0xf - i);
        WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG4, antipop_bit_tmp | HP_ANTIPOP_ENABLE);
        rkos_sleep(5);
    }

    MaskAcodecRegBits(ACODEC_BASE+ACODEC_HPLOCFG4, HP_ANTIPOP_ENABLE,HP_ANTIPOP_DISABLE); // set hp antipop disable

    //rk_printf("ACODEC_HPLOCFG4 = %d",ReadAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG4));

    WriteAcodecReg(ACODEC_BASE+ACODEC_DACMUTE,DACMTE_DISABLE);    //DAC unmute

    //select headphone as DAC output  DAC automatical power-on
    WriteAcodecReg(ACODEC_BASE+ACODEC_DACPOPD,ATPCE_ENABLE | HPSEL_HP2DAC | DACATPO_ON);
    //WriteAcodecReg(ACODEC_BASE+ACODEC_DACPOPD,ATPCE_DISABLE | HPSEL_HP2DAC | SMTPO_DOWN | ANTIPOP_DISABLE);
}

/*******************************************************************************
** Name: ACodec_pd_dac_hp
** Input:void
** Return: void
** Owner:
** Date: 2015.5.20
** Time: 10:40:50
*******************************************************************************/
void ACodec_pd_dac_hp()
{
    uint32 i = 0;
    uint32 antipop_bit_tmp = 0;

    //Codec_SetVolumet(1);    // Voice type
    //ACodec_Set_HP_AMP(INC_HP_AMP_0DB);
    //ACodec_Set_DAC_DigVol(254);
//    ACodec_get_dac_vol();   // check vol change is ok
//    DelayMs(10);

    WriteAcodecReg(ACODEC_BASE+ACODEC_DACMUTE,DACMTE_ENABLE);    //DAC mute

    MaskAcodecRegBits(ACODEC_BASE+ACODEC_HPLOCFG4, HP_ANTIPOP_DISABLE, HP_ANTIPOP_ENABLE); // set hp antipop en

    rk_printf("ACODEC_HPLOCFG4 = %d",ReadAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG4));
    for (i = 0; i < 16; i++)
    {
        antipop_bit_tmp = ReadAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG4);
        antipop_bit_tmp = (antipop_bit_tmp & 0xf0) | i;
        WriteAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG4, antipop_bit_tmp | HP_ANTIPOP_ENABLE);
        rkos_sleep(1);
    }
    rk_printf("ACODEC_HPLOCFG4 = %d",ReadAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG4));


    WriteAcodecReg(ACODEC_BASE+ACODEC_DACACFG2, PWD_DACBIAS_DOWN | PWD_DACL_ON | PWD_DACR_ON); // power down dac ibias/l/r
    rk_printf("ACODEC_DACACFG2 = %d",ReadAcodecReg(ACODEC_BASE+ACODEC_DACACFG2));

}

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
/*
--------------------------------------------------------------------------------
  Function name : void Codec_PowerOnInitial(void)
  Author        : yangwenjie
  Description   : Codec power on initial

  Input         :

  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  Note:
--------------------------------------------------------------------------------
*/
//_ATTR_SYS_INIT_CODE_
void Codec_PowerOnInitial(void)
{
    uint32 i;

    Acodec_I2S_MODE = ACodec_I2S_MASTER_MODE;
    pll_old_target = Pll_Target_Freq_NULL;
    Acodec_suspend_En = 0;
    Codec_Power_Status = Codec_Power_null;

    ACodec_Soft_Reset();

    MaskAcodecRegBits(ACODEC_BASE+ACODEC_PLLCFG5,PLL_PW_DOWN,PLL_PW_UP);

    WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG2,REF_PWD_ON | VAG_BUF_PWD_DOWN | IBIAS_PWD_DOWN);
    //DelayMs(200);
    rkos_sleep(2);
    WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG2,REF_PWD_ON | VAG_BUF_PWD_ON | IBIAS_PWD_DOWN);
    WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG2,REF_PWD_ON | VAG_BUF_PWD_ON | IBIAS_PWD_ON);
    WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG1,0x7);
    //printf("ACODEC_RTCFG1 = %d\n",ReadAcodecReg(ACODEC_BASE+ACODEC_RTCFG1));

    rkos_sleep(5);


    WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG2,REF_PWD_DOWN | VAG_BUF_PWD_DOWN | IBIAS_PWD_DOWN);
    rkos_sleep(200);
    WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG2,REF_PWD_ON | VAG_BUF_PWD_DOWN | IBIAS_PWD_DOWN);
    rkos_sleep(200);

    WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG2,REF_PWD_ON | VAG_BUF_PWD_ON | IBIAS_PWD_DOWN);
    WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG2,REF_PWD_ON | VAG_BUF_PWD_ON | IBIAS_PWD_ON);

    WriteAcodecReg(ACODEC_BASE+ACODEC_RTCFG1,0x1);
    //rk_printf("ACODEC_RTCFG1 = %d\n",ReadAcodecReg(ACODEC_BASE+ACODEC_RTCFG1));
    WriteAcodecReg(ACODEC_BASE+ACODEC_DACPOPD,ATPCE_DISABLE | SMTPO_DOWN | ANTIPOP_DISABLE);
    //rk_printf("ACODEC_HPLOCFG4 bit6 headphone output = %d\n",ReadAcodecReg(ACODEC_BASE+ACODEC_HPLOCFG3));
    //MaskAcodecRegBits(ACODEC_BASE+ACODEC_HPLOCFG1,(0x1 << 1),0x2);

}

/*******************************************************************************
** Name: ACodec_I2S_RX_Init
** Input:
** Return: void
*******************************************************************************/
//_ATTR_SYS_INIT_CODE_
void ACodec_I2S_RX_Init(eACodecI2sFormat_t BUS_FORMAT,
                        eACodecI2sDATA_WIDTH_t I2S_Data_width,
                        ACodecI2S_BUS_MODE_t I2S_Bus_mode)
{
    uint32 timeout = 20000;
    uint32 cofig_data = 0;

    ACodec_I2S_RX_Stop();

    WriteAcodecReg(ACODEC_BASE+ACODEC_I2SRXCR2,I2S_Data_width);

    cofig_data = LSB_RX_MSB | EXRL_RX_NORMAL | PBM_RX_BUS_MODE_DELAY0 | BUS_FORMAT | I2S_Bus_mode;

    WriteAcodecReg(ACODEC_BASE+ACODEC_I2SRXCR1,cofig_data);
    cofig_data = 0;

    cofig_data = RXRL_P_NORMAL | SCKD_RX_64_DIV;

    WriteAcodecReg(ACODEC_BASE+ACODEC_I2SRXCR0,cofig_data);
    cofig_data = 0;


}


/*******************************************************************************
** Name: ACodec_I2S_TX_Init
** Input:
** Return: void
*******************************************************************************/
//_ATTR_SYS_INIT_CODE_
void ACodec_I2S_TX_Init(eACodecI2sFormat_t BUS_FORMAT,
                        eACodecI2sDATA_WIDTH_t I2S_Data_width,
                        ACodecI2S_BUS_MODE_t I2S_Bus_mode)
{
    uint32 timeout = 20000;
    uint32 cofig_data = 0;

    ACodec_I2S_TX_Stop();

    WriteAcodecReg(ACODEC_BASE+ACODEC_I2STXCR3,0);  //RCNVT_TX = 0

    WriteAcodecReg(ACODEC_BASE+ACODEC_I2STXCR2,I2S_Data_width);  //24bit/16bit

    cofig_data = LSB_TX_MSB | EXRL_TX_NORMAL | PBM_TX_BUS_MODE_DELAY0 | BUS_FORMAT | I2S_Bus_mode;

    WriteAcodecReg(ACODEC_BASE+ACODEC_I2STXCR1,cofig_data);
    cofig_data = 0;

    cofig_data = TXRL_P_NORMAL | SCKD_TX_64_DIV;

    WriteAcodecReg(ACODEC_BASE+ACODEC_I2STXCR0,cofig_data);
    cofig_data = 0;

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#endif

