/*
 * Copyright 2015, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/** @file
 *
 */
#include "BspConfig.h"
#ifdef _DRIVER_WIFI__
#define NOT_INCLUDE_OTHER

#include "wwd_debug.h"
#include "wwd_buffer_interface.h"
#include "platform/wwd_resource_interface.h"
#include "network/wwd_network_constants.h"
#include "wwd_bus_protocol_interface.h"
#include "../wwd_internal.h"   /* TODO: fix include dependency */
#include "../chip/43362A2/chip_constants.h"
#include "platform_toolchain.h"
#include "typedef.h"
#include "ModuleInfoTab.h"
#include "wiced_resource.h"


//#pragma arm section code = "ap6181wifiCode", rodata = "ap6181wifiCode", rwdata = "ap6181wifidata", zidata = "ap6181wifidata"




__attribute__((used,section("wifi_nvram")))const char wifi_nvram_image[] =
#ifdef _WIFI_AP6212 // ap6212
        "muxenab=0x10"                                 "\x00"
        "manfid=0x2d0"                                 "\x00"
        "prodid=0x0726"                                "\x00"
        "vendid=0x14e4"                                "\x00"
        "devid=0x43e2"                                 "\x00"
        "boardtype=0x0726"                             "\x00"
        "boardrev=0x1101"                              "\x00"
        "boardnum=22"                                  "\x00"
        "macaddr=00:90:4c:c5:12:38"                    "\x00"
        "sromrev=11"                                   "\x00"
        "boardflags=0x00404201"                        "\x00"
        "xtalfreq=26000"                               "\x00"
        "nocrc=1"                                      "\x00"
        "ag0=255"                                      "\x00"
        "aa2g=1"                                       "\x00"
        "ccode=ALL"                                    "\x00"
        "pa0itssit=0x20"                               "\x00"
        "extpagain2g=0"                                "\x00"
        "pa2ga0=-168,7161,-820"                        "\x00"
        "AvVmid_c0=0x0,0xc8"                           "\x00"
        "cckpwroffset0=5"                              "\x00"
        "maxp2ga0=90"                                  "\x00"
        "txpwrbckof=6"                                 "\x00"
        "cckbw202gpo=0x5555"                           "\x00"
        "legofdmbw202gpo=0x77777777"                   "\x00"
        "mcsbw202gpo=0xaaaaaaaa"                       "\x00"
        "ofdmdigfilttype=7"                            "\x00"
        "papdmode=2"                                   "\x00"
        "il0macaddr=00:90:4c:c5:12:38"                 "\x00"
        "wl0id=0x431b"                                 "\x00"
        "hostwake=0x40"                                "\x00"
        "hostrdy=0x41"                                 "\x00"
        "usbrdy=0x03"                                  "\x00"
        "usbrdydelay=100"                              "\x00"
        "deadman_to=0xffffffff"                        "\x00"
        "\x00\x00";
#endif

#ifdef _WIFI_5G_AP6234
        "manfid=0x2d0"                                                       "\x00"
        "prodid=0x0653"                                                      "\x00"
        "vendid=0x14e4"                                                      "\x00"
        "devid=0x4386"                                                       "\x00"
        "boardtype=0x0653"                                                   "\x00"
        "boardrev=0x1203"                                                    "\x00"
        "boardnum=22"                                                        "\x00"
        //NVRAM_GENERATED_MAC_ADDRESS                                          "\x00"
        "sromrev=3"                                                          "\x00"
        //#boardflags:
        //# bit 19 3tswitch:   2.4GHz FEM: SP3T switch share with BT
        //# bit 16     nopa:   no external pa
        //#            keep original 0x200
        "boardflags=0x0090201"                                               "\x00"
        "xtalfreq=37400"                                                     "\x00"
        "nocrc=1"                                                            "\x00"
        "ag0=255"                                                            "\x00"
        "aa2g=1"                                                             "\x00"
        //#"aa5g=1"                                                             "\x00"
        "ccode=ALL"                                                          "\x00"
        "pa0itssit=0x20"                                                     "\x00"
        //#PA parameters for 2.4GHz
        "pa0b0=6957"                                                         "\x00"
        "pa0b1=-808"                                                         "\x00"
        "pa0b2=-178"                                                         "\x00"
        "tssifloor2g=69"                                                     "\x00"
        //# rssi params for 2.4GHz
        "rssismf2g=0xf"                                                      "\x00"
        "rssismc2g=0x8"                                                      "\x00"
        "rssisav2g=0x1"                                                      "\x00"
        "cckPwrOffset=3"                                                     "\x00"
        //# rssi params for 5GHz
        "rssismf5g=0xf"                                                      "\x00"
        "rssismc5g=0x7"                                                      "\x00"
        //#rssisav5g=0x1
        "rssisav5g=0x3"                                                      "\x00"
        //#PA parameters for lower a-band
        "pa1lob0=5859"                                                       "\x00"
        "pa1lob1=-693"                                                       "\x00"
        "pa1lob2=-178"                                                       "\x00"
        "tssifloor5gl=77"                                                    "\x00"
        //#PA parameters for midband
        "pa1b0=5372"                                                         "\x00"
        "pa1b1=-671"                                                         "\x00"
        "pa1b2=-212"                                                         "\x00"
        "tssifloor5gm=77"                                                    "\x00"
        //#PA paramasdeters for high band
        "pa1hib0=5620"                                                       "\x00"
        "pa1hib1=-663"                                                       "\x00"
        "pa1hib2=-179"                                                       "\x00"
        "tssifloor5gh=74"                                                    "\x00"
        "rxpo5g=0"                                                           "\x00"
        "maxp2ga0=76"                                                      "\x00"
        //#  19.5dBm max; 18dBm target
        //#Per rate power back-offs for g band, in .5 dB steps. Set it once you have the right numbers.
        "cck2gpo=0x2222"                                                     "\x00"
        "ofdm2gpo=0x333333333"                                                "\x00"
        //# R54 16dBm; R48 17dBm; others 18dBm
        "mcs2gpo0=0x5555"                                                    "\x00"
        //# M0~ M4 17dBm
        "mcs2gpo1=0x5555"                                                    "\x00"
        //# M5M6 15dBm; M7 14.5dBm
        //#max power for 5G
        "maxp5ga0=68"                                                      "\x00"
        //# 16dBm target; 17.5dBm Max
        "maxp5gla0=68"                                                     "\x00"
        "maxp5gha0=68"                                                     "\x00"
        //#Per rate power back-offs for a band, in .5 dB steps. Set it once you have the right numbers.
        "ofdm5gpo=0x66666666"                                                "\x00"
        //# R54 13.5dBm
        "ofdm5glpo=0x66666666"                                               "\x00"
        "ofdm5ghpo=0x66666666"                                               "\x00"
        "mcs5gpo0=0x8888"                                                    "\x00"
        //# M0~M4 16dBm (1dB higher than ofdm)
        "mcs5gpo1=0x8888"                                                    "\x00"
        //# M5M6 13.5dBm; M7 12dBm
        "mcs5glpo0=0x7777"                                                   "\x00"
        "mcs5glpo1=0x7777"                                                   "\x00"
        "mcs5ghpo0=0x7777"                                                   "\x00"
        "mcs5ghpo1=0x7777"                                                   "\x00"
        //# Parameters for DAC2x mode and ALPF bypass
        //# RF SW Truth Table: ctrl0 for BT_TX; ctrl1 or 5G Tx; ctrl2 for 5G Rx; Ctrl3 for 2G Tx; Ctrl4 for 2G Rx
        "swctrlmap_2g=0x00080008,0x00100010,0x00080008,0x011010,0x11f"       "\x00"
        "swctrlmap_5g=0x00040004,0x00020002,0x00040004,0x011010,0x2fe"       "\x00"
        "gain=32"                                                            "\x00"
        "triso2g=8"                                                          "\x00"
        "triso5g=8"                                                          "\x00"
        //#tx parameters
        "loflag=0"                                                           "\x00"
        "iqlocalidx5g=40"                                                    "\x00"
        "dlocalidx5g=70"                                                     "\x00"
        "iqcalidx5g=50"                                                      "\x00"
        "lpbckmode5g=1"                                                      "\x00"
        "txiqlopapu5g=0"                                                     "\x00"
        "txiqlopapu2g=0"                                                     "\x00"
        "dlorange_lowlimit=5"                                                "\x00"
        "txalpfbyp=1"                                                        "\x00"
        "txalpfpu=1"                                                         "\x00"
        "dacrate2xen=1"                                                      "\x00"
        "papden2g=1"                                                         "\x00"
        "papden5g=1"                                                         "\x00"
        //#rx parameters
        "gain_settle_dly_2g=4"                                               "\x00"
        "gain_settle_dly_5g=4"                                               "\x00"
        "noise_cal_po_2g=-1"                                                 "\x00"
        "noise_cal_po_40_2g=-1"                                              "\x00"
        "noise_cal_high_gain_2g=73"                                          "\x00"
        "noise_cal_nf_substract_val_2g=346"                                  "\x00"
        "noise_cal_po_5g=-1"                                                 "\x00"
        "noise_cal_po_40_5g=-1"                                              "\x00"
        "noise_cal_high_gain_5g=73"                                          "\x00"
        "noise_cal_nf_substract_val_5g=346"                                  "\x00"
        "cckpapden=0"                                                        "\x00"
        //# Enable OOB interrupt: level trigger
        "muxenab=0x10"                                                       "\x00"
        //#CE 1.8.1
        "edonthd=-70"                                                        "\x00"
        "edoffthd=-76"                                                       "\x00"
        "\x00\x00";


#endif

#ifdef  _WIFI_AP6181     //∑Â÷µ”–960k 0612
        "muxenab=0x10"                                                       "\x00"
        "manfid=0x2d0"                                                       "\x00"
        "prodid=0x492"                                                       "\x00"
        "vendid=0x14e4"                                                      "\x00"
        "devid=0x4343"                                                       "\x00"
        "boardtype=0x05a0"                                                   "\x00"
        "boardrev=0x1301"                                                    "\x00" /*Board Revision is REV3.1*/
        "boardnum=777"                                                       "\x00"
        "xtalfreq=26000"                                                     "\x00"
        "boardflags=0xa00"                                                   "\x00"
        "sromrev=3"                                                          "\x00"
        "wl0id=0x431b"                                                       "\x00"
        "macaddr=02:0A:F7:d8:21:58"                                          "\x00"
        "aa2g=3"                                                             "\x00"
        "ag0=2"                                                              "\x00"
        "maxp2ga0=68"                                                        "\x00"
        "ofdm2gpo=0x44444444"                                                "\x00"
        "mcs2gpo0=0x3333"                                                    "\x00"
        "mcs2gpo1=0x6333"                                                    "\x00"
        "pa0maxpwr=80"                                                       "\x00"
        "pa0b0=0x133E"                                                       "\x00"
        "pa0b1=0xFDBA"                                                       "\x00"
        "pa0b2=0xFF53"                                                       "\x00"
        "pa0itssit=62"                                                       "\x00"
        "pa1itssit=62"                                                       "\x00"
        "temp_based_dutycy_en=1"                                             "\x00"
        "tx_duty_cycle_ofdm=100"                                             "\x00"
        "tx_duty_cycle_cck=100"                                              "\x00"
        "tx_ofdm_temp_0=115"                                                 "\x00"
        "tx_cck_temp_0=115"                                                  "\x00"
        "tx_ofdm_dutycy_0=40"                                                "\x00"
        "tx_cck_dutycy_0=40"                                                 "\x00"
        "tx_ofdm_temp_1=255"                                                 "\x00"
        "tx_cck_temp_1=255"                                                  "\x00"
        "tx_ofdm_dutycy_1=40"                                                "\x00"
        "tx_cck_dutycy_1=40"                                                 "\x00"
        "tx_tone_power_index=40"                                             "\x00"
        "tx_tone_power_index.fab.3=48"                                       "\x00"
        "cckPwrOffset=0"                                                     "\x00"
        "ccode=0"                                                            "\x00"
        "rssismf2g=0xa"                                                      "\x00"
        "rssismc2g=0x3"                                                      "\x00"
        "rssisav2g=0x7"                                                      "\x00"
        "triso2g=0"                                                          "\x00"
        "noise_cal_enable_2g=0"                                              "\x00"
        "noise_cal_po_2g=0"                                                  "\x00"
        "noise_cal_po_2g.fab.3=-2"                                           "\x00"
        "swctrlmap_2g=0x050c050c,0x030a030a,0x030a030a,0x0,0x1ff"            "\x00"
        "temp_add=29767"                                                     "\x00"
        "temp_mult=425"                                                      "\x00"
        "temp_q=10"                                                          "\x00"
        "initxidx2g=45"                                                      "\x00"
        "tssitime=1"                                                         "\x00"
        "rfreg033=0x19"                                                      "\x00"
        "rfreg033_cck=0x1f"                                                  "\x00"
        "cckPwrIdxCorr=-8"                                                   "\x00"
        "spuravoid_enable2g=1"                                               "\x00"
        "edonthd=-70"                                                        "\x00"
        "edoffthd=-76"                                                       "\x00"
        "\x00\x00";
#endif
#ifdef _WIFI_5G_AP6255
        "NVRAMRev=$Rev: 498373 $"                                             "\x00"
        "sromrev=11"                                                          "\x00"
        "vendid=0x14e4"                                                       "\x00"
        "devid=0x43ab"                                                        "\x00"
        "manfid=0x2d0"                                                        "\x00"
        "prodid=0x06e4"                                                       "\x00"
        "macaddr=00:90:4c:c5:12:38"                                           "\x00"
        "nocrc=1"                                                             "\x00"
        "boardtype=0x6e4"                                                     "\x00"
        "boardrev=0x1304"                                                     "\x00"
        "xtalfreq=37400"                                                      "\x00"
        "boardflags=0x00080201"                                               "\x00"
        "boardflags2=0x40000000"                                              "\x00"
        "boardflags3=0x48200100"                                              "\x00"
        "rxgains2gelnagaina0=0"                                               "\x00"
        "rxgains2gtrisoa0=0"                                                  "\x00"
        "rxgains2gtrelnabypa0=0"                                              "\x00"
        "rxgains5gelnagaina0=0"                                               "\x00"
        "rxgains5gtrisoa0=0"                                                  "\x00"
        "rxgains5gtrelnabypa0=0"                                              "\x00"
        "rxchain=1"                                                           "\x00"
        "txchain=1"                                                           "\x00"
        "aa2g=1"                                                              "\x00"
        "aa5g=1"                                                              "\x00"
        "tssipos5g=1"                                                         "\x00"
        "tssipos2g=1"                                                         "\x00"
        "femctrl=0"                                                           "\x00"
        "AvVmid_c0=0,157,1,126,1,126,1,126,1,126"                             "\x00"
        "pa2ga0=-112,6296,-662"                                               "\x00"
        "pa2ga1=-165,3699,-515"                                               "\x00"
        "pa5ga0=-143,6016,-683,-141,6013,-678,-137,5988,-670,-136,5982,-670"  "\x00"
        "pa5ga1=-161,3544,-499,-166,3543,-497,-169,3569,-497,-171,3598,-498"  "\x00"
        "itrsw=1"                                                             "\x00"
        "pdoffset2g40ma0=10"                                                  "\x00"
        "pdoffset40ma0=0xaaaa"                                                "\x00"
        "pdoffset80ma0=0xaaaa"                                                "\x00"
        "extpagain5g=2"                                                       "\x00"
        "extpagain2g=2"                                                       "\x00"
        "tworangetssi2g=1"                                                    "\x00"
        "tworangetssi5g=1"                                                    "\x00"
        "ltecxmux=0"                                                          "\x00"
        "ltecxpadnum=0x0504"                                                  "\x00"
        "ltecxfnsel=0x22"                                                     "\x00"
        "ltecxgcigpio=0x32"                                                   "\x00"
        "maxp2ga0=80"                                                         "\x00"
        "ofdmlrbw202gpo=0x0033"                                               "\x00"
        "dot11agofdmhrbw202gpo=0x3775"                                        "\x00"
        "mcsbw202gpo=0x99355533"                                              "\x00"
        "maxp5ga0=80,82,76,77"                                                "\x00"
        "mcsbw205glpo=0x99755000"                                             "\x00"
        "mcsbw205gmpo=0x9df55000"                                             "\x00"
        "mcsbw205ghpo=0x99855000"                                             "\x00"
        "mcsbw405glpo=0xb8555000"                                             "\x00"
        "mcsbw405gmpo=0xed955000"                                             "\x00"
        "mcsbw405ghpo=0xd9755000"                                             "\x00"
        "mcsbw805glpo=0xc8555000"                                             "\x00"
        "mcsbw805gmpo=0xe9555000"                                             "\x00"
        "mcsbw805ghpo=0xd9555000"                                             "\x00"
        "swctrlmap_2g=0x00040004,0x00020002,0x00040004,0x010a02,0x1ff"        "\x00"
        "swctrlmap_5g=0x00100010,0x00200020,0x00100010,0x010a02,0x2f4"        "\x00"
        "swctrlmapext_5g=0x00000000,0x00000000,0x00000000,0x000000,0x000"     "\x00"
        "swctrlmapext_2g=0x00000000,0x00000000,0x00000000,0x000000,0x000"     "\x00"
        "vcodivmode=1"                                                        "\x00"
        "deadman_to=481500000"                                                "\x00"
        "ed_thresh2g=-47"                                                     "\x00"
        "ed_thresh5g=-47"                                                     "\x00"
        "muxenab=0x10"                                                        "\x00"
        "edonthd=-70"                                                         "\x00"
        "edoffthd=-76"                                                        "\x00"
        "\x00\x00";
#endif

#if 0
    "manfid=0x2d0"                                                       "\x00"
    "prodid=0x492"                                                       "\x00"
    "vendid=0x14e4"                                                      "\x00"
    "devid=0x4343"                                                       "\x00"
    "boardtype=0x05a0"                                                   "\x00"
    "boardrev=0x1301"                                                    "\x00" /*Board Revision is REV3.1*/
    "boardnum=777"                                                       "\x00"
    "xtalfreq=26000"                                                     "\x00"
    "boardflags=0xa00"                                                   "\x00"
    "sromrev=3"                                                          "\x00"
    "wl0id=0x431b"                                                       "\x00"
    "macaddr=02:0A:F7:d8:21:58"                                          "\x00"
    "aa2g=3"                                                             "\x00"
    "ag0=2"                                                              "\x00"
    "maxp2ga0=68"                                                        "\x00"
    "cck2gpo=0x55555555"                                                 "\x00"
    "ofdm2gpo=0x88888888"                                                "\x00"
    "mcs2gpo0=0x8888"                                                    "\x00"
    "mcs2gpo1=0x8888"                                                    "\x00"
    "pa0maxpwr=80"                                                       "\x00"
    "pa0b0=0x133E"                                                       "\x00"
    "pa0b1=0xFDBA"                                                       "\x00"
    "pa0b2=0xFF53"                                                       "\x00"
    "pa0itssit=62"                                                       "\x00"
    "pa1itssit=62"                                                       "\x00"
    "temp_based_dutycy_en=1"                                             "\x00"
    "tx_duty_cycle_ofdm=100"                                             "\x00"
    "tx_duty_cycle_cck=100"                                              "\x00"
    "tx_ofdm_temp_0=115"                                                 "\x00"
    "tx_cck_temp_0=115"                                                  "\x00"
    "tx_ofdm_dutycy_0=40"                                                "\x00"
    "tx_cck_dutycy_0=40"                                                 "\x00"
    "tx_ofdm_temp_1=255"                                                 "\x00"
    "tx_cck_temp_1=255"                                                  "\x00"
    "tx_ofdm_dutycy_1=40"                                                "\x00"
    "tx_cck_dutycy_1=40"                                                 "\x00"
    "tx_tone_power_index=40"                                             "\x00"
    "tx_tone_power_index.fab.3=48"                                       "\x00"
    "cckPwrOffset=0"                                                     "\x00"
    "ccode=0"                                                            "\x00"
    "rssismf2g=0xa"                                                      "\x00"
    "rssismc2g=0x3"                                                      "\x00"
    "rssisav2g=0x7"                                                      "\x00"
    "triso2g=0"                                                          "\x00"
    "noise_cal_enable_2g=0"                                              "\x00"
    "noise_cal_po_2g=0"                                                  "\x00"
    "noise_cal_po_2g.fab.3=-2"                                           "\x00"
    "swctrlmap_2g=0x050c050c,0x030a030a,0x030a030a,0x0,0x1ff"            "\x00"
    "temp_add=29767"                                                     "\x00"
    "temp_mult=425"                                                      "\x00"
    "temp_q=10"                                                          "\x00"
    "initxidx2g=45"                                                      "\x00"
    "tssitime=1"                                                         "\x00"
    "rfreg033=0x19"                                                      "\x00"
    "rfreg033_cck=0x1f"                                                  "\x00"
    "cckPwrIdxCorr=-8"                                                   "\x00"
    "spuravoid_enable2g=1"                                               "\x00"
    "\x00\x00";
#endif
#if 0
"cbuckout=1500"                                                      "\x00"
"sromrev=3"                                                          "\x00"
"boardtype=0x05a0"                                                   "\x00"
"boardrev=0x1203"                                                    "\x00"
"manfid=0x2d0"                                                       "\x00"
"prodid=0x492"                                                       "\x00"
"vendid=0x14e4"                                                      "\x00"
"devid=0x4343"                                                       "\x00"
"boardflags=0x200"                                                   "\x00"
"nocrc=1"                                                            "\x00"
"xtalfreq=26000"                                                     "\x00"
"boardnum=777"                                                       "\x00"
"macaddr=02:0A:F7:d8:21:58"                                          "\x00"
"aa2g=3"                                                             "\x00"
"ag0=0"                                                              "\x00"
"ccode=ww"                                                           "\x00"
"pa0b0= 0x13F9"                                                      "\x00"
"pa0b1= 0xFD93"                                                      "\x00"
"pa0b2= 0xFF4D"                                                      "\x00"
"rssismf2g=0xa"                                                      "\x00"
"rssismc2g=0x3"                                                      "\x00"
"rssisav2g=0x7"                                                      "\x00"
"maxp2ga0=0x46"                                                      "\x00"
"cck2gpo=0x0"                                                        "\x00"
"ofdm2gpo=0x22222222"                                                "\x00"
"mcs2gpo0=0x3333"                                                    "\x00"
"mcs2gpo1=0x6333"                                                    "\x00"
"wl0id=0x431b"                                                       "\x00"
"cckdigfilttype=22"                                                  "\x00"
"cckPwrOffset=5"                                                     "\x00"
"ofdmanalogfiltbw2g=3"                                               "\x00"
"rfreg033=0x19"                                                      "\x00"
"rfreg033_cck=0x1f"                                                  "\x00"
"noise_cal_enable_2g=0"                                              "\x00"
"pacalidx2g=10"                                                      "\x00"
"swctrlmap_2g=0x0c050c05,0x0a030a03,0x0a030a03,0x0,0x1ff"            "\x00"
"triso2g=1"                                                          "\x00"
"RAW1=4a 0b ff ff 20 04 d0 02 62 a9"                                 "\x00"
"otpimagesize=76"                                                    "\x00";
#endif
//#endif

#define DHD_SDALIGN    64
#define MEMBLOCK            (2048+1024)    /* Block size used for downloading of dongle image */


#if defined( WWD_DYNAMIC_NVRAM )
#define NVRAM_SIZE             dynamic_nvram_size
#define NVRAM_IMAGE_VARIABLE   dynamic_nvram_image
#else
#define NVRAM_SIZE             sizeof( wifi_nvram_image )
#define NVRAM_IMAGE_VARIABLE   wifi_nvram_image
#endif


#define INDIRECT_BUFFER_SIZE (512)

#define WWD_BUS_ROUND_UP_ALIGNMENT ( 64 )

#ifdef WWD_DIRECT_RESOURCES
//#define WWD_BUS_MAX_TRANSFER_SIZE     ( 16 * 1024 )
#define WWD_BUS_MAX_TRANSFER_SIZE  2048
#else /* ifdef WWD_DIRECT_RESOURCES */
#define WWD_BUS_MAX_TRANSFER_SIZE     ( 64 )
#endif /* ifdef WWD_DIRECT_RESOURCES */

static uint32_t backplane_window_current_base_address = 0;
static wwd_result_t download_resource( wwd_resource_t resource, uint32_t address );


extern const resource_hnd_t wifi_firmware_image;
extern const char wifi_nvram_image[];



void wwd_bus_init_backplane_window( void )
{
    backplane_window_current_base_address = 0;
}

wwd_result_t host_platform_resource_size( wwd_resource_t resource, uint32_t* size_out )
{
    if ( resource == WWD_RESOURCE_WLAN_FIRMWARE )
    {

#ifdef NO_WIFI_FIRMWARE
        wiced_assert("Request firmware in a no wifi firmware application", 0 == 1);
        *size_out = 0;
#else
#ifdef WIFI_FIRMWARE_IN_MULTI_APP
        wiced_app_t wifi_app;

        *size_out = 0;
        if ( wiced_waf_app_open( DCT_WIFI_FIRMWARE_INDEX, &wifi_app ) != WICED_SUCCESS )
        {
            return RESOURCE_UNSUPPORTED;
        }
        wiced_waf_app_get_size( &wifi_app, size_out );
#else
        *size_out = (uint32_t) resource_get_size( &wifi_firmware_image );
#endif
#endif

    }
    else
    {
        *size_out =  NVRAM_SIZE;
    }
    return WWD_SUCCESS;
}



#if defined( WWD_DIRECT_RESOURCES )
wwd_result_t host_platform_resource_read_direct( wwd_resource_t resource, const void** ptr_out )
{
    if ( resource == WWD_RESOURCE_WLAN_FIRMWARE )
    {
#ifndef NO_WIFI_FIRMWARE
        *ptr_out = wifi_firmware_image.val.mem.data;
#else
        wiced_assert("Request firmware in a no wifi firmware application", 0 == 1);
        *ptr_out = NULL;
#endif
    }
    else
    {
        *ptr_out = NVRAM_IMAGE_VARIABLE;
    }
    return WWD_SUCCESS;
}
#else /* ! defined( WWD_DIRECT_RESOURCES ) */
wwd_result_t host_platform_resource_read_indirect( wwd_resource_t resource, uint32_t offset, void* buffer, uint32_t buffer_size, uint32_t* size_out )
{
    if ( resource == WWD_RESOURCE_WLAN_FIRMWARE )
    {

#ifdef NO_WIFI_FIRMWARE
        wiced_assert("Request firmware in a no wifi firmware application", 0 == 1);
        return RESOURCE_UNSUPPORTED;
#else
#ifdef WIFI_FIRMWARE_IN_MULTI_APP
        wiced_app_t wifi_app;
        if ( wiced_waf_app_open( DCT_WIFI_FIRMWARE_INDEX, &wifi_app ) != WICED_SUCCESS )
        {
            return RESOURCE_UNSUPPORTED;
        }
        if ( wiced_waf_app_read_chunk( &wifi_app, offset, buffer, buffer_size ) == WICED_SUCCESS )
        {
            *size_out = buffer_size;
        }
        else
        {
            *size_out = 0;
        }
        return WWD_SUCCESS;
#else
        return resource_read( &wifi_firmware_image, offset, buffer_size, size_out, buffer );
#endif
#endif

    }
    else
    {
//rk del start
#if 0
        *size_out = MIN( buffer_size, NVRAM_SIZE - offset );
#endif
//rk del end
        memcpy( buffer, &NVRAM_IMAGE_VARIABLE[ offset ], *size_out );
        return WWD_SUCCESS;
    }
}
#endif /* if defined( WWD_DIRECT_RESOURCES ) */


WEAK wwd_result_t wwd_bus_write_wifi_firmware_image( void )
{
    //return download_resource( WWD_RESOURCE_WLAN_FIRMWARE, 0 );
    return download_resource( WWD_RESOURCE_WLAN_FIRMWARE, WLAN_MEM_BASE_ADDRESS );
}
#ifdef WWD_DIRECT_RESOURCES
WEAK wwd_result_t wwd_bus_write_wifi_nvram_image( void )
{
    wwd_result_t result;
    uint32_t image_size;

    /* Get the size of the variable image */
    host_platform_resource_size( WWD_RESOURCE_WLAN_NVRAM, &image_size );

    /* Round up the size of the image */
    image_size = ROUND_UP( image_size, WWD_BUS_ROUND_UP_ALIGNMENT );

    /* Write image */
    result = download_resource( WWD_RESOURCE_WLAN_NVRAM, CHIP_RAM_SIZE - 4 - image_size );
    if ( result != WWD_SUCCESS )
    {
        return result;
    }

    /* Write the variable image size at the end */
    image_size = ( ~( image_size / 4 ) << 16 ) | ( image_size / 4 );

    result = wwd_bus_write_backplane_value( (uint32_t) ( CHIP_RAM_SIZE - 4 ), 4, image_size );
    if ( result != WWD_SUCCESS )
    {
        return result;
    }
    return WWD_SUCCESS;
}
#else
wwd_result_t wwd_bus_write_wifi_nvram_image( void )
{
    wwd_result_t result;
    uint32_t image_size;
    SEGMENT_INFO_T stCodeInfo;
    int ret;
    ret = FW_GetSegmentInfo(SEGMENT_ID_WIFI_FIRMAVE_TWO, &stCodeInfo);
    if (ret)
    {
        result = WWD_BUFFER_ALLOC_FAIL;
        return  result;
    }
    /* Get the size of the variable image */
    //host_platform_resource_size( WWD_RESOURCE_WLAN_NVRAM, &image_size );
    image_size = stCodeInfo.CodeImageLength;
    /* Round up the size of the image */
    image_size = ROUND_UP( image_size, WWD_BUS_ROUND_UP_ALIGNMENT );
    /* Write image */
    result = download_resource( WWD_RESOURCE_WLAN_NVRAM, WLAN_MEM_BASE_ADDRESS + CHIP_RAM_SIZE - 4 - image_size );
    if ( result != WWD_SUCCESS )
    {
        return result;
    }

    /* Write the variable image size at the end */
    image_size = ( ~( image_size / 4 ) << 16 ) | ( image_size / 4 );

    result = wwd_bus_write_backplane_value( (uint32_t) ( WLAN_MEM_BASE_ADDRESS + CHIP_RAM_SIZE - 4 ), 4, image_size);
    if ( result != WWD_SUCCESS )
    {
        return result;
    }

    return WWD_SUCCESS;
}
#endif


#ifdef WWD_DIRECT_RESOURCES
static wwd_result_t download_resource( wwd_resource_t resource, uint32_t address )
{
    uint32_t transfer_progress;
    uint16_t transfer_size;
    wwd_result_t result;
    const uint8_t* image;
    //uint8_t recv_buf[2048] = {0};

    uint32_t image_size;
    host_platform_resource_size( resource, &image_size );

    host_platform_resource_read_direct( resource, (const void**)&image );

    for ( transfer_progress = 0; transfer_progress < image_size; transfer_progress += transfer_size, address += transfer_size, image += transfer_size )
    {
        /* Set the backplane window */
        if ( WWD_SUCCESS != ( result = wwd_bus_set_backplane_window( address ) ) )
        {
            return result;
        }
        transfer_size = (uint16_t) MIN( WWD_BUS_MAX_TRANSFER_SIZE, (int) ( image_size - transfer_progress ) );

        /* Round up the size of the chunk */
        transfer_size = (uint16_t) ROUND_UP( transfer_size, WWD_BUS_ROUND_UP_ALIGNMENT );

        if ( WWD_SUCCESS != ( result = wwd_bus_transfer_bytes( BUS_WRITE, BACKPLANE_FUNCTION, address & BACKPLANE_ADDRESS_MASK, transfer_size, (wwd_transfer_bytes_packet_t*) image ) ) )
        {
            return result;
        }

        //if(resource == WWD_RESOURCE_WLAN_NVRAM)
        //result = wwd_bus_transfer_bytes( BUS_READ, BACKPLANE_FUNCTION, address & BACKPLANE_ADDRESS_MASK, transfer_size, (wwd_transfer_bytes_packet_t*)recv_buf );

#if 0
        {
            /* TODO: THIS VERIFY CODE IS CURRENTLY BROKEN - ONLY CHECKS 64 BYTES, NOT 16KB */
            /* Verify download of image data */
            uint8_t tmpbuff[64];
            if ( WWD_SUCCESS != ( result = wwd_bus_transfer_bytes( BUS_READ, BACKPLANE_FUNCTION, address & BACKPLANE_ADDRESS_MASK, 64, (wwd_transfer_bytes_packet_t*)tmpbuff ) ) )
            {
                return result;
            }
            if ( 0 != memcmp( tmpbuff, image, (size_t) 64 ) )
            {
                /* Verify failed */
                WPRINT_WWD_ERROR(("Verify of firmware/NVRAM image failed"));
            }
        }
#endif /* if 0 */
    }
    return WWD_SUCCESS;
}

#else /*  defined( WWD_DIRECT_RESOURCES ) */
static wwd_result_t download_resource( wwd_resource_t resource, uint32_t address )
{
    uint32_t transfer_progress;
    uint16_t transfer_size;
    wwd_result_t result;
    const uint8_t* image;

    uint32_t image_size;

    uint8_t *memblock = NULL, *memptr;
    uint32_t page_addr;
    SEGMENT_INFO_T stCodeInfo;
    int fwsize;
    int offset;
    int ret;

    memptr = memblock = (uint8*)rkos_memory_malloc(MEMBLOCK + DHD_SDALIGN);
    if (memblock == NULL)
        return WWD_BUFFER_ALLOC_FAIL;
    if ((uint)memblock % DHD_SDALIGN)
        memptr += (DHD_SDALIGN - ((uint)memblock % DHD_SDALIGN));

    /* Download image */
    if (resource == WWD_RESOURCE_WLAN_FIRMWARE)
    {
        ret = FW_GetSegmentInfo(SEGMENT_ID_WIFI_FIRMAVE_ONE, &stCodeInfo);
        //ret = FW_GetSegmentInfo(SEGMENT_ID_BT_FIRMAVE, &stCodeInfo);

        if (ret)
        {
            result = WWD_BUFFER_ALLOC_FAIL;
            goto err;
        }
    }
    else if (resource == WWD_RESOURCE_WLAN_NVRAM)
    {
        ret = FW_GetSegmentInfo(SEGMENT_ID_WIFI_FIRMAVE_TWO, &stCodeInfo);
        if (ret)
        {
            result = WWD_BUFFER_ALLOC_FAIL;
            goto err;

        }

    }

    page_addr = stCodeInfo.CodeLoadBase;
    fwsize = stCodeInfo.CodeImageLength;
    offset = 0;

    image_size = fwsize;
    for ( transfer_progress = 0; transfer_progress < image_size; transfer_progress += transfer_size, address += transfer_size)
    {
        /* Set the backplane window */
        if ( WWD_SUCCESS != ( result = wwd_bus_set_backplane_window( address ) ) )
        {
            goto err;
        }
        transfer_size = (uint16_t) MIN( WWD_BUS_MAX_TRANSFER_SIZE, (int) ( image_size - transfer_progress ) );
        /* Round up the size of the chunk */
        transfer_size = (uint16_t) ROUND_UP( transfer_size, WWD_BUS_ROUND_UP_ALIGNMENT );

        FW_ReadFirmwaveByByte(page_addr + offset, memptr, transfer_size);

        if ( WWD_SUCCESS != ( result = wwd_bus_transfer_bytes( BUS_WRITE, BACKPLANE_FUNCTION, address & BACKPLANE_ADDRESS_MASK, transfer_size, (wwd_transfer_bytes_packet_t*)memptr ) ) )
        {
            goto err;
        }
#if 1

        unsigned long len = 0;
        uint8_t recv_buf[2048] = {0};
       // printf("transfer_size = %d\n", transfer_size);
        //debug_hex(memptr, transfer_size, 16);
        //if(resource == WWD_RESOURCE_WLAN_NVRAM)
        //{
        result = wwd_bus_transfer_bytes( BUS_READ, BACKPLANE_FUNCTION, address & BACKPLANE_ADDRESS_MASK, transfer_size, (wwd_transfer_bytes_packet_t*)recv_buf );
        if(result == WWD_SUCCESS)
        {
           // printf("read_addr = 0x%8x\n", address & BACKPLANE_ADDRESS_MASK);
            len = len+transfer_size;
            //printf("transfer_progress = %d\n", len);
            if( 0 != memcmp( recv_buf, memptr, 64))
            {
                debug_hex(recv_buf, transfer_size, 16);
                // debug_hex(memptr, transfer_size, 16);
                //printf("transfer_progress = %d\n", len);
            }
        }
        else
        {
            printf("fail\n");
        }
        //}
#endif
        offset += transfer_size;
    }
    if (memblock)
        rkos_memory_free(memblock);
    return WWD_SUCCESS;


err:
    if (memblock)
        rkos_memory_free(memblock);
    return result;

}
#endif


//Rk del start
#if 0
static wwd_result_t download_resource( wwd_resource_t resource, uint32_t address )
{
    uint32_t transfer_progress;

    uint32_t size;
    host_platform_resource_size( resource, &size );


    /* Transfer firmware image into the RAM */
    transfer_progress = 0;

    while ( transfer_progress < size )
    {
        wiced_buffer_t buffer;
        uint32_t buffer_size = INDIRECT_BUFFER_SIZE;
        uint8_t* packet;
        uint16_t transfer_size;
        uint32_t segment_size;
        wwd_result_t result;

        do
        {
            result = host_buffer_get( &buffer, WWD_NETWORK_TX, (unsigned short) ( buffer_size + sizeof(wwd_buffer_header_t) ), WICED_FALSE );
        }
        while ( ( result == WWD_BUFFER_UNAVAILABLE_PERMANENT ) && ( ( buffer_size >>= 1 ) > 1 ) );

        if ( result != WWD_SUCCESS )
        {
            WPRINT_WWD_ERROR(("Fatal error: download_resource cannot allocate buffer"));
            return 0;
        }
        packet = (uint8_t*) host_buffer_get_current_piece_data_pointer( buffer );

        host_platform_resource_read_indirect( resource, transfer_progress, packet + sizeof(wwd_buffer_header_t), buffer_size, &segment_size );

        for ( ; segment_size != 0; segment_size -= transfer_size, packet += transfer_size, transfer_progress += transfer_size, address += transfer_size )
        {
            transfer_size = (uint16_t) MIN( WWD_BUS_MAX_TRANSFER_SIZE, segment_size );
            result = wwd_bus_set_backplane_window( address );
            if ( result != WWD_SUCCESS )
            {
                host_buffer_release( buffer, WWD_NETWORK_TX );
                return result;
            }
            result = wwd_bus_transfer_bytes( BUS_WRITE, BACKPLANE_FUNCTION, ( address & BACKPLANE_ADDRESS_MASK ), transfer_size, (wwd_transfer_bytes_packet_t*) ( packet + sizeof(wwd_buffer_queue_ptr_t)) );
            if ( result != WWD_SUCCESS )
            {
                host_buffer_release( buffer, WWD_NETWORK_TX );
                return result;
            }
        }

        host_buffer_release( buffer, WWD_NETWORK_TX );
    }
    return WWD_SUCCESS;
}
#endif
//rk del end

/*
 * Update the backplane window registers
 */
WEAK wwd_result_t wwd_bus_set_backplane_window( uint32_t addr )
{
    wwd_result_t result = WWD_BUS_WRITE_REGISTER_ERROR;
    uint32_t base = addr & ( (uint32_t) ~BACKPLANE_ADDRESS_MASK );

    if ( base == backplane_window_current_base_address )
    {
        return WWD_SUCCESS;
    }
    if ( ( base & 0xFF000000 ) != ( backplane_window_current_base_address & 0xFF000000 ) )
    {
        if ( WWD_SUCCESS != ( result = wwd_bus_write_register_value( BACKPLANE_FUNCTION, SDIO_BACKPLANE_ADDRESS_HIGH, (uint8_t) 1, ( base >> 24 ) ) ) )
        {
            return result;
        }
    }
    if ( ( base & 0x0FF0000 ) != ( backplane_window_current_base_address & 0x00FF0000 ) )
    {
        if ( WWD_SUCCESS != ( result = wwd_bus_write_register_value( BACKPLANE_FUNCTION, SDIO_BACKPLANE_ADDRESS_MID, (uint8_t) 1, ( base >> 16 ) ) ) )
        {
            return result;
        }
    }
    if ( ( base & 0x0000FF00 ) != ( backplane_window_current_base_address & 0x0000FF00 ) )
    {
        if ( WWD_SUCCESS != ( result = wwd_bus_write_register_value( BACKPLANE_FUNCTION, SDIO_BACKPLANE_ADDRESS_LOW, (uint8_t) 1, ( base >> 8 ) ) ) )
        {
            return result;
        }
    }

    backplane_window_current_base_address = base;
    return WWD_SUCCESS;
}

//#pragma arm section code
#endif
