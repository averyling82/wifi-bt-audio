/*
********************************************************************************************
*
*                  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                               All rights reserved.
*
* FileName: lib\rockcodec.h
* Owner: Aaron.sun
* Date: 2015.5.19
* Time: 20:01:00
* Desc:
* History:
*    <author>     <date>       <time>     <version>       <Desc>
*   Aaron.sun     2015.5.19     20:01:00   1.0
********************************************************************************************
*/

#ifndef __LIB_ROCKCODEC_H__
#define __LIB_ROCKCODEC_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define _LIB_ROCKCODEC_COMMON_  __attribute__((section("lib_rockcodec_common")))
#define _LIB_ROCKCODEC_INIT_  __attribute__((section("lib_rockcodec_init")))
#define _LIB_ROCKCODEC_SHELL_  __attribute__((section("lib_rockcodec_shell")))

#if (CODEC_CONFIG == CODEC_ROCKC)

#ifndef _RKCODEC_H
#define _RKCODEC_H
//reg offset define
#define ACODEC_VCTL                 ((UINT32)(0x0040))
#define ACODEC_VCTIME               ((UINT32)(0x0044))
#define ACODEC_LPST                 ((UINT32)(0x0048))
#define ACODEC_LPT                  ((UINT32)(0x004C))
#define ACODEC_SRST                 ((UINT32)(0x0054))
#define ACODEC_DIGEN                ((UINT32)(0x0058))
#define ACODEC_CLKE                 ((UINT32)(0x0060))
#define ACODEC_RTCFG0               ((UINT32)(0x0080))
#define ACODEC_RTCFG1               ((UINT32)(0x0084))
#define ACODEC_RTCFG2               ((UINT32)(0x0088))
#define ACODEC_ADCCFG0              ((UINT32)(0x00C0))
#define ACODEC_ADCCFG1              ((UINT32)(0x00C4))
#define ACODEC_ADCVCTLL             ((UINT32)(0x00C8))
#define ACODEC_ADCVCTLR             ((UINT32)(0x00CC))
#define ACODEC_ADCSR                ((UINT32)(0x00D0))
#define ACODEC_ALC0                 ((UINT32)(0x00D4))
#define ACODEC_ALC1                 ((UINT32)(0x00D8))
#define ACODEC_ALC2                 ((UINT32)(0x00DC))
#define ACODEC_ADCNG                ((UINT32)(0x00E0))
#define ACODEC_ADCNGST              ((UINT32)(0x00E4))
#define ACODEC_ADCHPF               ((UINT32)(0x00E8))
#define ACODEC_ADCVSTL              ((UINT32)(0x00EC))
#define ACODEC_ADCVSTR              ((UINT32)(0x00F0))
#define ACODEC_DACACFG0             ((UINT32)(0x0100))
#define ACODEC_DACACFG1             ((UINT32)(0x0104))
#define ACODEC_DACACFG2             ((UINT32)(0x0108))
#define ACODEC_DACPOPD              ((UINT32)(0x0140))
#define ACODEC_DACST                ((UINT32)(0x0144))
#define ACODEC_DACVCTLL             ((UINT32)(0x0148))
#define ACODEC_DACVCTLR             ((UINT32)(0x014C))
#define ACODEC_DACSR                ((UINT32)(0x0150))
#define ACODEC_LMT0                 ((UINT32)(0x0154))
#define ACODEC_LMT1                 ((UINT32)(0x0158))
#define ACODEC_LMT2                 ((UINT32)(0x015C))
#define ACODEC_DACMUTE              ((UINT32)(0x0160))
#define ACODEC_MIXCTRL              ((UINT32)(0x0164))
#define ACODEC_DACVSTL              ((UINT32)(0x0168))
#define ACODEC_DACVSTR              ((UINT32)(0x016C))
#define ACODEC_LICFG0               ((UINT32)(0x0180))
#define ACODEC_LICFG1               ((UINT32)(0x0184))
#define ACODEC_LICFG2               ((UINT32)(0x0188))
#define ACODEC_LICFG3               ((UINT32)(0x018C))
#define ACODEC_LICFG4               ((UINT32)(0x0190))
#define ACODEC_LILMT0               ((UINT32)(0x0198))
#define ACODEC_LILMT1               ((UINT32)(0x019C))
#define ACODEC_LILMT2               ((UINT32)(0x01A0))
#define ACODEC_ADCNGLMTCFG          ((UINT32)(0x01A4))
#define ACODEC_ADCNGLMTST           ((UINT32)(0x01A8))
#define ACODEC_HPLOCFG0             ((UINT32)(0x01C0))
#define ACODEC_HPLOCFG1             ((UINT32)(0x01C4))
#define ACODEC_HPLOCFG2             ((UINT32)(0x01C8))
#define ACODEC_HPLOCFG3             ((UINT32)(0x01CC))
#define ACODEC_HPLOCFG4             ((UINT32)(0x01D0))
#define ACODEC_HPLOCFG5             ((UINT32)(0x01D4))
#define ACODEC_PLLCFG0              ((UINT32)(0x0200))
#define ACODEC_PLLCFG1              ((UINT32)(0x0204))
#define ACODEC_PLLCFG2              ((UINT32)(0x0208))
#define ACODEC_PLLCFG3              ((UINT32)(0x020C))
#define ACODEC_PLLCFG4              ((UINT32)(0x0210))
#define ACODEC_PLLCFG5              ((UINT32)(0x0214))
#define ACODEC_I2SCKM               ((UINT32)(0x0240))
#define ACODEC_I2SRXCR0             ((UINT32)(0x0244))
#define ACODEC_I2SRXCR1             ((UINT32)(0x0248))
#define ACODEC_I2SRXCR2             ((UINT32)(0x024C))
#define ACODEC_I2SRXCMD             ((UINT32)(0x0250))
#define ACODEC_I2STXCR0             ((UINT32)(0x0260))
#define ACODEC_I2STXCR1             ((UINT32)(0x0264))
#define ACODEC_I2STXCR2             ((UINT32)(0x0268))
#define ACODEC_I2STXCR3             ((UINT32)(0x026C))
#define ACODEC_I2STXCMD             ((UINT32)(0x0270))
#define ACODEC_TMCFG0               ((UINT32)(0x0300))

//reg Value define
//ACODEC_VCTL(0x0040) ****
#define ADCBYPS_DISABLE      ((UINT32)(0x00 << 7))
#define ADCBYPS_ENABLE       ((UINT32)(0x01 << 7))

#define DACBYPS_DISABLE      ((UINT32)(0x00 << 6))
#define DACBYPS_ENABLE       ((UINT32)(0x01 << 6))

#define ADCFADE_IMMEDIATELY      ((UINT32)(0x00 << 5))
#define ADCFADE_AS_ADCCZDT       ((UINT32)(0x01 << 5))

#define DACFADE_IMMEDIATELY      ((UINT32)(0x00 << 4))
#define DACFADE_AS_DACCZDT       ((UINT32)(0x01 << 4))

#define ADCCZDT_0       ((UINT32)(0x00 << 1))
#define ADCCZDT_1       ((UINT32)(0x01 << 1))

#define DACCZDT_0       ((UINT32)(0x00 << 0))
#define DACCZDT_1       ((UINT32)(0x01 << 0))
//ACODEC_VCTIME(0x0044) ****

//ACODEC_LPST(0x0048) ****
#define LPDET_NO_VAILD      ((UINT32)(0x00 << 0))
#define LPDET_VAILD         ((UINT32)(0x01 << 0))
//ACODEC_LPT(0x004c) ****

//ACODEC_SRST(0x0054) ****
#define SOFT_RESET          ((UINT32)(0x01 << 0))
//ACODEC_DIGEN(0x0058) ****
#define ADC_EN_DISABLE      ((UINT32)(0x00 << 5))
#define ADC_EN_ENABLE       ((UINT32)(0x01 << 5))

#define I2STX_EN_DISABLE    ((UINT32)(0x00 << 4))
#define I2STX_EN_ENABLE     ((UINT32)(0x01 << 4))

#define DAC_EN_DISABLE      ((UINT32)(0x00 << 1))
#define DAC_EN_ENABLE       ((UINT32)(0x01 << 1))

#define I2SRX_EN_DISABLE    ((UINT32)(0x00 << 0))
#define I2SRX_EN_ENABLE     ((UINT32)(0x01 << 0))
//ACODEC_CLKE(0x0060) ****
#define ADC_CKE_DISABLE      ((UINT32)(0x00 << 5))
#define ADC_CKE_ENABLE       ((UINT32)(0x01 << 5))

#define I2STX_CKE_DISABLE    ((UINT32)(0x00 << 4))
#define I2STX_CKE_ENABLE     ((UINT32)(0x01 << 4))

#define DAC_CKE_DISABLE      ((UINT32)(0x00 << 1))
#define DAC_CKE_ENABLE       ((UINT32)(0x01 << 1))

#define I2SRX_CKE_DISABLE    ((UINT32)(0x00 << 0))
#define I2SRX_CKE_ENABLE     ((UINT32)(0x01 << 0))
//ACODEC_RTCFG0(0x0080) ****
#define HALF_ADC_BUF_DISABLE      ((UINT32)(0x00 << 7))
#define HALF_ADC_BUF_ENABLE       ((UINT32)(0x01 << 7))

#define HALF_VAG_BUF_DISABLE      ((UINT32)(0x00 << 6))
#define HALF_VAG_BUF_ENABLE       ((UINT32)(0x01 << 6))

#define IBIAS_ANA_SEL_100       ((UINT32)(0x00 << 4))
#define IBIAS_ANA_SEL_80        ((UINT32)(0x01 << 4))
#define IBIAS_ANA_SEL_120       ((UINT32)(0x02 << 4))
#define IBIAS_ANA_SEL_140       ((UINT32)(0x03 << 4))
//ACODEC_RTCFG1(0x0084) ****
#define BOOST_ADC_BUF_DISABLE      ((UINT32)(0x00 << 7))
#define BOOST_ADC_BUF_ENABLE       ((UINT32)(0x01 << 7))

#define BOOST_VAG_BUF_DISABLE      ((UINT32)(0x00 << 6))
#define BOOST_VAG_BUF_ENABLE       ((UINT32)(0x01 << 6))

#define REF_ADC_SEL_12V       ((UINT32)(0x00 << 5))
#define REF_ADC_SEL_15V       ((UINT32)(0x01 << 5))
//ACODEC_RTCFG2(0x0088) ****
#define IBIAS_PWD_ON            ((UINT32)(0x00 << 2))
#define IBIAS_PWD_DOWN          ((UINT32)(0x01 << 2))

#define VAG_BUF_PWD_ON            ((UINT32)(0x00 << 1))
#define VAG_BUF_PWD_DOWN          ((UINT32)(0x01 << 1))

#define REF_PWD_ON            ((UINT32)(0x00 << 0))
#define REF_PWD_DOWN          ((UINT32)(0x01 << 0))
//ACODEC_ADCCFG0(0x00c0) ****
#define ADC_L_PWD_ON            ((UINT32)(0x00 << 7))
#define ADC_L_PWD_DOWN          ((UINT32)(0x01 << 7))

#define ADC_R_PWD_ON            ((UINT32)(0x00 << 6))
#define ADC_R_PWD_DOWN          ((UINT32)(0x01 << 6))

#define ADC_DEM_EN_DISABLE      ((UINT32)(0x00 << 4))
#define ADC_DEM_EN_ENABLE       ((UINT32)(0x01 << 4))

#define ADC_DITH_OFF_DISABLE        ((UINT32)(0x00 << 3))
#define ADC_DITH_OFF_ENABLE         ((UINT32)(0x01 << 3))

//ACODEC_ADCCFG1(0x00c4) ****
#define ADC_MUTE_L_DISABLE         ((UINT32)(0x00 << 7))
#define ADC_MUTE_L_ENABLE          ((UINT32)(0x01 << 7))

#define ADC_MUTE_R_DISABLE         ((UINT32)(0x00 << 6))
#define ADC_MUTE_R_ENABLE          ((UINT32)(0x01 << 6))

#define ADC_ATTN_ALLIBIAS_DISABLE  ((UINT32)(0x00 << 5))
#define ADC_ATTN_ALLIBIAS_ENABLE   ((UINT32)(0x01 << 5))

#define ADC_ATTN_OPBIAS_DISABLE    ((UINT32)(0x00 << 4))
#define ADC_ATTN_OPBIAS_ENABLE     ((UINT32)(0x01 << 4))

#define ADC_DLY_INC_DISABLE        ((UINT32)(0x00 << 3))
#define ADC_DLY_INC_ENABLE         ((UINT32)(0x01 << 3))

#define ADC_OVERLAP_INC_DISABLE    ((UINT32)(0x00 << 2))
#define ADC_OVERLAP_INC_ENABLE     ((UINT32)(0x01 << 2))

#define ADC_BOOST_OPAMP_DISABLE    ((UINT32)(0x00 << 1))
#define ADC_BOOST_OPAMP_ENABLE     ((UINT32)(0x01 << 1))

#define ADC_BOOST_VAGOP_DISABLE    ((UINT32)(0x00 << 0))
#define ADC_BOOST_VAGOP_ENABLE     ((UINT32)(0x01 << 0))
//ACODEC_ADCVCTLL(0x00c8) ****

//ACODEC_ADCVCTLR(0x00cc) ****

//ACODEC_ADCSR(0x00d0) ****

//ACODEC_ALC0(0x00d4) ****
#define ALCL_DISABLE         ((UINT32)(0x00 << 7))
#define ALCL_ENABLE          ((UINT32)(0x01 << 7))

#define ALCR_DISABLE         ((UINT32)(0x00 << 6))
#define ALCR_ENABLE          ((UINT32)(0x01 << 6))
//ACODEC_ALC1(0x00d8) ****

//ACODEC_ALC2(0x00dc) ****
#define ALCMAX_0dB              ((UINT32)(0x00 << 4))
#define ALCMAX_F3dB             ((UINT32)(0x01 << 4))
#define ALCMAX_F6dB             ((UINT32)(0x02 << 4))
#define ALCMAX_F9dB             ((UINT32)(0x03 << 4))
#define ALCMAX_F12dB            ((UINT32)(0x04 << 4))
#define ALCMAX_F18dB            ((UINT32)(0x05 << 4))
#define ALCMAX_F24dB            ((UINT32)(0x06 << 4))
#define ALCMAX_F30dB            ((UINT32)(0x07 << 4))

#define ALCMIN_0dB              ((UINT32)(0x00 << 0))
#define ALCMIN_F3dB             ((UINT32)(0x01 << 0))
#define ALCMIN_F6dB             ((UINT32)(0x02 << 0))
#define ALCMIN_F9dB             ((UINT32)(0x03 << 0))
#define ALCMIN_F12dB            ((UINT32)(0x04 << 0))
#define ALCMIN_F18dB            ((UINT32)(0x05 << 0))
#define ALCMIN_F24dB            ((UINT32)(0x06 << 0))
#define ALCMIN_F30dB            ((UINT32)(0x07 << 0))
//ACODEC_ADCNG(0x00e0) ****
#define NGCHL_EITHER_CH         ((UINT32)(0x00 << 7))
#define NGCHL_BOTH_CH           ((UINT32)(0x01 << 7))

#define NGEN_DISABLE         ((UINT32)(0x00 << 6))
#define NGEN_ENABLE          ((UINT32)(0x01 << 6))

#define NGBOOST_NORMAL         ((UINT32)(0x00 << 5))
#define NGBOOST_BOOST          ((UINT32)(0x01 << 5))

#define NGDLY_2048          ((UINT32)(0x00 << 0))
#define NGDLY_4096          ((UINT32)(0x01 << 0))
#define NGDLY_8192          ((UINT32)(0x02 << 0))
#define NGDLY_16384         ((UINT32)(0x03 << 0))
//ACODEC_ADCNGST(0x00e4) ****

//ACODEC_ADCVSTL(0x00ec) ****
#define HPFLE_DISABLE         ((UINT32)(0x00 << 7))
#define HPFLE_ENABLE          ((UINT32)(0x01 << 7))

#define HPFRE_DISABLE         ((UINT32)(0x00 << 6))
#define HPFRE_ENABLE          ((UINT32)(0x01 << 6))

#define HPF_CF_3_79Hz  ((UINT32)(0x00 << 4))
#define HPF_CF_60Hz    ((UINT32)(0x01 << 4))
#define HPF_CF_243Hz   ((UINT32)(0x02 << 4))
#define HPF_CF_493Hz   ((UINT32)(0x03 << 4))
//ACODEC_ADCVSTL(0x00ec) ****

//ACODEC_ADCVSTR(0x00f0) ****

//ACODEC_DACACFG0(0x0100) ****
#define DAC_AMP_INC2DB_DISABLE         ((UINT32)(0x00 << 7))
#define DAC_AMP_INC2DB_ENABLE          ((UINT32)(0x01 << 7))

#define DAC_MAX_OUT_DISABLE         ((UINT32)(0x00 << 5))
#define DAC_MAX_OUT_ENABLE          ((UINT32)(0x01 << 5))

#define INC_DAC_RSTB_DISABLE         ((UINT32)(0x00 << 4))
#define INC_DAC_RSTB_ENABLE          ((UINT32)(0x01 << 4))
//ACODEC_DACACFG1(0x0104) ****
#define INC_DAC_SWITCH_DISABLE         ((UINT32)(0x00 << 2))
#define INC_DAC_SWITCH_ENABLE          ((UINT32)(0x01 << 2))

#define STOP_DAC_RSTB_DISABLE         ((UINT32)(0x00 << 1))
#define STOP_DAC_RSTB_ENABLE          ((UINT32)(0x01 << 1))

#define STOP_DAC_SW_DISABLE         ((UINT32)(0x00 << 0))
#define STOP_DAC_SW_ENABLE          ((UINT32)(0x01 << 0))
//ACODEC_DACACFG2(0x0108) ****
#define PWD_DACBIAS_DOWN        ((UINT32)(0x01 << 2))
#define PWD_DACBIAS_ON          ((UINT32)(0x00 << 2))

#define PWD_DACL_DOWN           ((UINT32)(0x01 << 1))
#define PWD_DACL_ON             ((UINT32)(0x00 << 1))

#define PWD_DACR_DOWN           ((UINT32)(0x01 << 0))
#define PWD_DACR_ON             ((UINT32)(0x00 << 0))
//ACODEC_DACPOPD(0x0140) ****
#define ATPCE_DISABLE           ((UINT32)(0x00 << 7))
#define ATPCE_ENABLE            ((UINT32)(0x01 << 7))

#define HPSEL_LINE2DAC          ((UINT32)(0x00 << 6))
#define HPSEL_HP2DAC            ((UINT32)(0x01 << 6))

#define SMTPO_DOWN           ((UINT32)(0x00 << 5))
#define SMTPO_ON             ((UINT32)(0x01 << 5))

#define ANTIPOP_DISABLE         ((UINT32)(0x00 << 4))
#define ANTIPOP_ENABLE          ((UINT32)(0x01 << 4))

#define DACATPO_DOWN         ((UINT32)(0x00 << 0))
#define DACATPO_ON           ((UINT32)(0x01 << 0))

//ACODEC_DACST(0x0144) ****

//ACODEC_DACVCTLL(0x0148) ****

//ACODEC_DACVCTLR(0x014c) ****

//ACODEC_DACSR(0x0150) ****

//ACODEC_LMT0(0x0154) ****
#define LMTEN_DISABLE          ((UINT32)(0x00 << 7))
#define LMTEN_ENABLE           ((UINT32)(0x01 << 7))

#define LMTCHL_COMBINATIONAL   ((UINT32)(0x00 << 6))
#define LMTEN_INDEPENTLY       ((UINT32)(0x01 << 6))
//ACODEC_LMT1(0x0158) ****

//ACODEC_LMT2(0x015c) ****

//ACODEC_DACMUTE(0x0160) ****
#define DACMTE_DISABLE          ((UINT32)(0x00 << 0))
#define DACMTE_ENABLE           ((UINT32)(0x01 << 0))
//ACODEC_MIXCTRL(0x0164) ****
#define MIXE_DISABLE          ((UINT32)(0x00 << 0))
#define MIXE_ENABLE           ((UINT32)(0x01 << 0))
//ACODEC_DACVSTL(0x0168) ****

//ACODEC_DACVSTR(0x016c) ****

//ACODEC_LICFG0(0x0180) ****
#define MICBIAS_SEL15V               ((UINT32)(0x00 << 4))
#define MICBIAS_SEL18V               ((UINT32)(0x01 << 4))
#define MICBIAS_SEL20V               ((UINT32)(0x02 << 4))
#define MICBIAS_SEL22V               ((UINT32)(0x03 << 4))
#define MICBIAS_SEL25V               ((UINT32)(0x04 << 4))
#define MICBIAS_SEL28V               ((UINT32)(0x05 << 4))
#define MICBIAS_SEL30V               ((UINT32)(0x06 << 4))
#define MICBIAS_SEL33V               ((UINT32)(0x07 << 4))
#define MICBIAS_SEL_MASK             ((UINT32)(0x07 << 4))

#define MIC_L_BOOST_0DB                ((UINT32)(0x00 << 2))
#define MIC_L_BOOST_10DB               ((UINT32)(0x01 << 2))
#define MIC_L_BOOST_20DB               ((UINT32)(0x02 << 2))
#define MIC_L_BOOST_30DB               ((UINT32)(0x03 << 2))

#define MIC_R_BOOST_0DB                ((UINT32)(0x00 << 0))
#define MIC_R_BOOST_10DB               ((UINT32)(0x01 << 0))
#define MIC_R_BOOST_20DB               ((UINT32)(0x02 << 0))
#define MIC_R_BOOST_30DB               ((UINT32)(0x03 << 0))
//ACODEC_LICFG1(0x0184) ****
#define MIC_L_DIFF_EN_DISABLE          ((UINT32)(0x00 << 5))
#define MIC_L_DIFF_EN_ENABLE           ((UINT32)(0x01 << 5))

#define MIC_R_DIFF_EN_DISABLE          ((UINT32)(0x00 << 4))
#define MIC_R_DIFF_EN_ENABLE           ((UINT32)(0x01 << 4))

#define MUX_L_IN_SEL_LINE_1            ((UINT32)(0x00 << 2))
#define MUX_L_IN_SEL_LINE_2            ((UINT32)(0x01 << 2))
#define MUX_L_IN_SEL_MIC               ((UINT32)(0x02 << 2))

#define MUX_R_IN_SEL_LINE_1            ((UINT32)(0x00 << 0))
#define MUX_R_IN_SEL_LINE_2            ((UINT32)(0x01 << 0))
#define MUX_R_IN_SEL_MIC               ((UINT32)(0x02 << 0))
//ACODEC_LICFG2(0x0188) ****

//ACODEC_LICFG3(0x018c) ****
#define CHOP_SEL_200K           ((UINT32)(0x00 << 2))
#define CHOP_SEL_400K           ((UINT32)(0x01 << 2))
#define CHOP_SEL_800K           ((UINT32)(0x02 << 2))

#define MIC_CHOP_EN_DISABLE          ((UINT32)(0x00 << 1))
#define MIC_CHOP_EN_ENABLE           ((UINT32)(0x01 << 1))

#define MUX_CHOP_EN_DISABLE          ((UINT32)(0x00 << 1))
#define MUX_CHOP_EN_ENABLE           ((UINT32)(0x01 << 1))
//ACODEC_LICFG4(0x0190) ****
#define MIC_L_PD_DOWN        ((UINT32)(0x01 << 5))
#define MIC_L_PD_ON          ((UINT32)(0x00 << 5))

#define MIC_R_PD_DOWN        ((UINT32)(0x01 << 4))
#define MIC_R_PD_ON          ((UINT32)(0x00 << 4))

#define MUX_L_PD_DOWN        ((UINT32)(0x01 << 1))
#define MUX_L_PD_ON          ((UINT32)(0x00 << 1))

#define MUX_R_PD_DOWN        ((UINT32)(0x01 << 0))
#define MUX_R_PD_ON          ((UINT32)(0x00 << 0))
//ACODEC_LILMT0(0x0198) ****
#define LMT_EN_DISABLE          ((UINT32)(0x00 << 0))
#define LMT_EN_ENABLE           ((UINT32)(0x01 << 0))
//ACODEC_LILMT1(0x019c) ****

//ACODEC_LILMT2(0x01a0) ****

//ACODEC_ADCNGLMTCFG(0x01a4) ****
#define NGCHL_LI_INDIVIDUAL     ((UINT32)(0x00 << 7))
#define NGCHL_LI_BOTH           ((UINT32)(0x01 << 7))

#define NGEN_LI_DISABLE          ((UINT32)(0x00 << 6))
#define NGEN_LI_ENABLE           ((UINT32)(0x01 << 6))

#define NGBOOST_LI_NORMAL          ((UINT32)(0x00 << 5))
#define NGBOOST_LI_BOOST           ((UINT32)(0x01 << 5))

#define NGDLY_LI_2048           ((UINT32)(0x00 << 0))
#define NGDLY_LI_4096           ((UINT32)(0x01 << 0))
#define NGDLY_LI_8192           ((UINT32)(0x02 << 0))
#define NGDLY_LI_16384          ((UINT32)(0x03 << 0))
//ACODEC_ADCNGLMTST(0x01a8) ****
#define NGACT_LI_ACTIVE     ((UINT32)(0x01 << 0))
#define NGACT_LI_NO_ACTIVE    ((UINT32)(0x00 << 0))
//ACODEC_HPLOCFG0(0x01c0) ****
#define INC_LO_AMP_0DB          ((UINT32)(0x00 << 2))
#define INC_LO_AMP_3DB          ((UINT32)(0x01 << 2))
#define INC_LO_AMP_6DB          ((UINT32)(0x02 << 2))
#define INC_LO_AMP_9DB          ((UINT32)(0x03 << 2))

#define LO_VAG_RISE_SLOW_ENABLE     ((UINT32)(0x01 << 1))
#define LO_VAG_RISE_SLOW_DISABLE    ((UINT32)(0x00 << 1))

#define LO_OUT_VAG_ENABLE     ((UINT32)(0x01 << 0))
#define LO_OUT_VAG_DISABLE    ((UINT32)(0x00 << 0))
//ACODEC_HPLOCFG1(0x01c4) ****
#define INC_HP_AMP_0DB          ((UINT32)(0x00 << 3))
#define INC_HP_AMP_3DB          ((UINT32)(0x01 << 3))
#define INC_HP_AMP_6DB          ((UINT32)(0x02 << 3))
#define INC_HP_AMP_9DB          ((UINT32)(0x03 << 3))

#define HP_TWO_STAGE_ENABLE     ((UINT32)(0x01 << 2))
#define HP_TWO_STAGE_DISABLE    ((UINT32)(0x00 << 2))

#define INC_OC_RANGE_ENABLE     ((UINT32)(0x01 << 1))
#define INC_OC_RANGE_DISABLE    ((UINT32)(0x00 << 1))

#define HP_OPAMP_HALF_BIAS_ENABLE     ((UINT32)(0x01 << 0))
#define HP_OPAMP_HALF_BIAS_DISABLE    ((UINT32)(0x00 << 0))
//ACODEC_HPLOCFG2(0x01c8) ****
#define HP_LN_OC_ENABLE           ((UINT32)(0x01 << 5))
#define HP_LN_OC_DISABLE          ((UINT32)(0x00 << 5))

#define HP_LP_OC_ENABLE           ((UINT32)(0x01 << 4))
#define HP_LP_OC_DISABLE          ((UINT32)(0x00 << 4))

#define HP_RN_OC_ENABLE           ((UINT32)(0x01 << 3))
#define HP_RN_OC_DISABLE          ((UINT32)(0x00 << 3))

#define HP_RP_OC_ENABLE           ((UINT32)(0x01 << 2))
#define HP_RP_OC_DISABLE          ((UINT32)(0x00 << 2))

#define HP_VGND_N_OC_ENABLE       ((UINT32)(0x01 << 1))
#define HP_VGND_N_OC_DISABLE      ((UINT32)(0x00 << 1))

#define HP_VGND_P_OC_ENABLE       ((UINT32)(0x01 << 0))
#define HP_VGND_P_OC_DISABLE      ((UINT32)(0x00 << 0))
//ACODEC_HPLOCFG3(0x01cc) ****
#define PWD_HP_OSTG_DOWN        ((UINT32)(0x01 << 3))
#define PWD_HP_OSTG_ON          ((UINT32)(0x00 << 3))

#define PWD_HP_VGND_DOWN        ((UINT32)(0x01 << 2))
#define PWD_HP_VGND_ON          ((UINT32)(0x00 << 2))

#define PWD_HP_BUF_DOWN               ((UINT32)(0x01 << 1))
#define PWD_HP_BUF_ON                 ((UINT32)(0x00 << 1))

#define HP_SHORT_OUT_ENABLE           ((UINT32)(0x01 << 0))
#define HP_SHORT_OUT_DISABLE          ((UINT32)(0x00 << 0))
//ACODEC_HPLOCFG4(0x01d0) ****
#define HP_ANTIPOP_ENABLE     ((UINT32)(0x01 << 4))
#define HP_ANTIPOP_DISABLE    ((UINT32)(0x00 << 4))

//ACODEC_HPLOCFG5(0x01d4) ****
#define PWD_LO_OSTG_ON      ((UINT32)(0x00 << 6))
#define PWD_LO_OSTG_DOWN    ((UINT32)(0x01 << 6))

#define PWD_LO_BUF_ON      ((UINT32)(0x00 << 5))
#define PWD_LO_BUF_DOWN    ((UINT32)(0x01 << 5))

#define LO_ANTIPOP_ENABLE     ((UINT32)(0x01 << 4))
#define LO_ANTIPOP_DISABLE    ((UINT32)(0x00 << 4))

//ACODEC_PLLCFG0(0x0200) ****
#define PLL_CLKIN_SEL_M_CLOCK   ((UINT32)(0x00 << 3))
#define PLL_CLKIN_SEL_M2_CLOCK  ((UINT32)(0x01 << 3))
#define PLL_CLKIN_SEL_S_CLOCK   ((UINT32)(0x02 << 3))
#define PLL_CLKIN_SEL_I2S_CLOCK ((UINT32)(0x03 << 3))

#define PLL_OUTDIV_ENABLE       ((UINT32)(0x01 << 2))
#define PLL_OUTDIV_DISABLE      ((UINT32)(0x00 << 2))
//ACODEC_PLLCFG1(0x0204) ****

//ACODEC_PLLCFG2(0x0208) ****
/*
    PLL_POSDIV_H8
*/
//ACODEC_PLLCFG3(0x020c) ****
/*
    PLL_PREDIV
*/
//ACODEC_PLLCFG4(0x0210) ****
/*
    PLL_OUTDIV
    PLL_CLK_DIV
*/
//ACODEC_PLLCFG5(0x0214) ****
#define PLL_PW_DOWN             ((UINT32)(0x01 << 0))
#define PLL_PW_UP               ((UINT32)(0x00 << 0))

#define PLL_TEST_ENABLE         ((UINT32)(0x01 << 1))

#define APLL_RESET               ((UINT32)(0x01 << 2))
#define APLL_RELEASE_RESET      ((UINT32)(0x00 << 2))
//ACODEC_I2SCKM(0x0240) ****
#define I2S_MST_MASTER          ((UINT32)(0x01 << 0))
#define I2S_MST_SLAVE           ((UINT32)(0x00 << 0))

#define SCK_P_ENABLE            ((UINT32)(0x01 << 1))
#define SCK_P_DISABLE           ((UINT32)(0x00 << 1))

#define SCK_EN_ENABLE           ((UINT32)(0x01 << 2))
#define SCK_EN_DISABLE          ((UINT32)(0x00 << 2))

//ACODEC_I2SRXCR0(0x0244)
#define SCKD_RX_64_DIV          ((UINT32)(0x00 << 1))
#define SCKD_RX_128_DIV         ((UINT32)(0x01 << 1))
#define SCKD_RX_256_DIV         ((UINT32)(0x02 << 1))

#define RXRL_P_NORMAL           ((UINT32)(0x00 << 0))
#define RXRL_P_INVERTED         ((UINT32)(0x01 << 0))
//ACODEC_I2SRXCR1(0x0248)
#define TFS_RX_I2S_MODE         ((UINT32)(0x00 << 6))
#define TFS_RX_PCM_MODE         ((UINT32)(0x01 << 6))

#define PBM_RX_BUS_MODE_DELAY0  ((UINT32)(0x00 << 4))
#define PBM_RX_BUS_MODE_DELAY1  ((UINT32)(0x01 << 4))
#define PBM_RX_BUS_MODE_DELAY2  ((UINT32)(0x02 << 4))
#define PBM_RX_BUS_MODE_DELAY3  ((UINT32)(0x03 << 4))

#define IBM_RX_BUS_MODE_NORMAL  ((UINT32)(0x00 << 2))
#define IBM_RX_BUS_MODE_LEFT    ((UINT32)(0x01 << 2))
#define IBM_RX_BUS_MODE_RIGHT   ((UINT32)(0x02 << 2))

#define EXRL_RX_NORMAL          ((UINT32)(0x00 << 1))
#define EXRL_RX_EXCHANGE        ((UINT32)(0x01 << 1))

#define LSB_RX_LSB              ((UINT32)(0x01 << 0))
#define LSB_RX_MSB              ((UINT32)(0x00 << 0))
//ACODEC_I2SRXCR2(0x024c)
#define VDW_RX_WIDTH_24BIT     ((UINT32)(0x17 << 0))
#define VDW_RX_WIDTH_16BIT     ((UINT32)(0x0F << 0))

//ACODEC_I2SRXCMD(0x0250)
#define RX_CLEAR               ((UINT32)(0x01 << 0))
#define RX_START               ((UINT32)(0x01 << 1))
#define RX_STOP                ((UINT32)(0x00 << 1))
//ACODEC_I2STXCR0(0x0260)
#define SCKD_TX_64_DIV          ((UINT32)(0x00 << 1))
#define SCKD_TX_128_DIV         ((UINT32)(0x01 << 1))
#define SCKD_TX_256_DIV         ((UINT32)(0x02 << 1))

#define TXRL_P_NORMAL           ((UINT32)(0x00 << 0))
#define TXRL_P_INVERTED         ((UINT32)(0x01 << 0))
//ACODEC_I2STXCR1(0x0264)
#define TFS_TX_I2S_MODE         ((UINT32)(0x00 << 6))
#define TFS_TX_PCM_MODE         ((UINT32)(0x01 << 6))

#define PBM_TX_BUS_MODE_DELAY0  ((UINT32)(0x00 << 4))
#define PBM_TX_BUS_MODE_DELAY1  ((UINT32)(0x01 << 4))
#define PBM_TX_BUS_MODE_DELAY2  ((UINT32)(0x02 << 4))

#define IBM_TX_BUS_MODE_NORMAL  ((UINT32)(0x00 << 2))
#define IBM_TX_BUS_MODE_LEFT    ((UINT32)(0x01 << 2))
#define IBM_TX_BUS_MODE_RIGHT   ((UINT32)(0x02 << 2))

#define EXRL_TX_NORMAL          ((UINT32)(0x00 << 1))
#define EXRL_TX_EXCHANGE        ((UINT32)(0x01 << 1))

#define LSB_TX_LSB              ((UINT32)(0x01 << 0))
#define LSB_TX_MSB              ((UINT32)(0x00 << 0))
//ACODEC_I2STXCR2(0x0268)
#define VDW_TX_WIDTH_24BIT     ((UINT32)(0x17 << 0))
#define VDW_TX_WIDTH_16BIT     ((UINT32)(0x0F << 0))

//ACODEC_I2STXCR3(0x026c)

//ACODEC_I2STXCMD(0x026c)
#define TX_CLEAR               ((UINT32)(0x01 << 0))

#define TX_START               ((UINT32)(0x01 << 1))
#define TX_STOP                ((UINT32)(0x00 << 1))

//ACODEC_TMCFG0(0x0270)
#define BURN_IN_MODE_ENBALE    ((UINT32)(0x01 << 0))
#define BURN_IN_MODE_DISBALE   ((UINT32)(0x00 << 0))

#define ATE_TEST_MODE_ENBALE   ((UINT32)(0x01 << 1))
#define ATE_TEST_MODE_DISBALE  ((UINT32)(0x00 << 1))

typedef enum
{
    ACodec_I2S_NORMAL_MODE = IBM_TX_BUS_MODE_NORMAL,
    ACodec_I2S_LEFT_MODE = IBM_TX_BUS_MODE_LEFT,
    ACodec_I2S_RIGHT_MODE = IBM_TX_BUS_MODE_RIGHT,
}ACodecI2S_BUS_MODE_t;

typedef enum
{
    ACodec_I2S_BUS_MODE = 0,
    ACodec_PCM_BUS_MODE
}ACodecI2S_PCM_SEL_t;

typedef enum
{
    ACodec_I2S_SLAVE_MODE = I2S_MST_SLAVE,
    ACodec_I2S_MASTER_MODE = I2S_MST_MASTER,
}ACodecI2S_mode_t;

typedef enum
{
    ACodec_I2S_FORMAT = TFS_TX_I2S_MODE,
    ACodec_PCM_FORMAT  = TFS_TX_PCM_MODE,
}eACodecI2sFormat_t;

typedef enum
{
    Pll_Target_Freq_40960,
    Pll_Target_Freq_56448,
    Pll_Target_Freq_61440,
    Pll_Target_Freq_NULL,
}eACodecPll_Target_Freq;

typedef enum
{
    LO_INC_0dB = INC_LO_AMP_0DB,
    LO_INC_3dB = INC_LO_AMP_3DB,
    LO_INC_6dB = INC_LO_AMP_6DB,
    LO_INC_9dB = INC_LO_AMP_9DB,
}eACodecLo_AMP;

typedef enum
{
    HP_INC_0dB = INC_HP_AMP_0DB,
    HP_INC_3dB = INC_HP_AMP_3DB,
    HP_INC_6dB = INC_HP_AMP_6DB,
    HP_INC_9dB = INC_HP_AMP_9DB,
}eACodecHp_AMP;

typedef enum
{
    CodecOut_Sel_HP,
    CodecOut_Sel_LINE,
}CodecOut_sel_t;

typedef enum
{
    Codecin_Sel_MIC1_MONO,
    Codecin_Sel_MIC2_MONO,
    Codecin_Sel_MIC_STERO,
    Codecin_Sel_LINE1,
    Codecin_Sel_LINE2,
}CodecIn_sel_t;

typedef enum
{
    MicBias_sel_15V = MICBIAS_SEL15V,
    MicBias_sel_18V = MICBIAS_SEL18V,
    MicBias_sel_20V = MICBIAS_SEL20V,
    MicBias_sel_22V = MICBIAS_SEL22V,
    MicBias_sel_25V = MICBIAS_SEL25V,
    MicBias_sel_28V = MICBIAS_SEL28V,
    MicBias_sel_30V = MICBIAS_SEL30V,
    MicBias_sel_33V = MICBIAS_SEL33V,
}CodecMicBias_sel_t;

typedef enum
{
    CodecMIC_Normal_Mode,
    CodecMIC_Diff_Mode,
}CodecMic_Mode_t;

typedef enum
{
    // Vailid Data width = n+1 bit
    ACodec_I2S_DATA_WIDTH16 = VDW_TX_WIDTH_16BIT,
    ACodec_I2S_DATA_WIDTH24  = VDW_TX_WIDTH_24BIT,
}eACodecI2sDATA_WIDTH_t;
typedef enum _F_SOURCE_ID
{
    F_SOURCE_2048KHz,
    F_SOURCE_3072KHz,
    F_SOURCE_4096KHz,
    F_SOURCE_6000KHz,
    F_SOURCE_6144KHz,
    F_SOURCE_12000KHz,
    F_SOURCE_12288KHz,
    F_SOURCE_24000KHz,

    F_SOURCE_MAX
}eF_SOURCE_ID;

#define ACODEC_ADDR     ACODEC_BASE
/*
********************************************************************************
*
*                         End of rockcedec.h
*
********************************************************************************
*/

#endif  //#ifndef _RKCODEC_H




/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/




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


#endif  //#if (CODEC_CONFIG == CODEC_ROCKC)

#endif
