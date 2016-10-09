/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\FM\hw_fm5807.h
* Owner: cjh
* Date: 2016.3.18
* Time: 14:59:22
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    cjh     2016.3.18     14:59:22   1.0
********************************************************************************************
*/


#ifndef __DRIVER_FM_HW_FM5807_H__
#define __DRIVER_FM_HW_FM5807_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#undef  EXT
#ifdef _IN_HWFM5807_
#define EXT
#else
#define EXT extern
#endif

#define    RUNER_AREA_JA            0x0002      // 1Ϊja 0Ϊch   us
#define    RUNER_SEEK_DN            0x01      //
#define    RUNER_SEEK_UP            0x03      //
#define    TUNER_MONO               0x20            /* Stereo indication, 0: stereo reception, 1: mono  5800*/
#define    TUNER_MUTE               0x40               //  mute  5800

//------------------------------------------------------------------------------
#define    TUNER_HIGHT_IF           0x3e
#define    YUNER_LOW_IF             0x3e

#define    TUNER_ADD_W              0xc0            /* TEA5767HN I2C slave address write DEVID */
#define    TUNER_ADD_R              0xc1            /* TEA5767HN I2C slave address write DEVID */

#define    TUNER_REFFREQ            32768           /* tuner reference frequency, 32.768Khz    */

//------------------------------------------------------------------------------
/* bit mask value for the 1st data in write mode    */
//#define    TUNER_MUTE               0x80            /* 1: mute enabled, 0: not mute                     */
#define    TUNER_SM                 0x40            /* search mode, 1:search, 0:not search mode         */
#define    TUNER_PLLHIGH            0x3f            /* PLL[13:8] value                                  */

/* bit mask value for the 2nd data in write mode    */
#define    TUNER_PLLLOW             0xff            /* PLL[7:0] value                                   */

/* bit mask value for the 3rd data in write mode    */
#define    TUNER_SUD                0x80            /* search up/down, 1: search up, 0: search down     */
#define    TUNER_SSL                0x20            /* search stop level, datasheet pp.11, Table 12     */
#define    TUNER_HLSI               0x10            /* Hight/Low side injection, 1: high, 0: low        */
#define    TUNER_MS                 0x08            /* Mono/Stereo, 1: mono, 0:stereo                   */
#define    TUNER_ML                 0x04            /* Mute Left, 1: left ch. muted, forced mono        */
#define    TUNER_MR                 0x02            /* Mute right, 1: right ch. muted, forced mono      */
#define    TUNER_SWP1               0x01            /* Software programmable port1, 1: port1 is high    */

/* bit mask value for the 4th data in write mode    */
#define    TUNER_SWP2               0x80            /* Software programmable port2, 1: port2 is high    */
#define    TUNER_STBY               0x40            /* Standby, 1: forced to standby state              */
#define    TUNER_BL                 0x20            /* Band Limit, 1: Japan FM band, 0: US/Europe FM    */
#define    TUNER_XTAL               0x10            /* Select xtal, 1: 32.768kHz, 0: 13MHz              */
#define    TUNER_SMUTE              0x08            /* Soft Mute, 1: mute on, 0: mute off               */
#define    TUNER_HCC                0x04            /* High Cut Control, 1: on, 0: off                  */
#define    TUNER_SNC                0x02            /* Stereo noise cancelling, 1: on, 0: off           */
#define    TUNER_SI                 0x01            /* Search indicator, 1: SWPORT1 is ready flag       */

/* bit mask value for the 5th data in write mode    */
#define    TUNER_PLLREF             0x80            /* 1: 6.5MHz reference, 0: not enabled              */
#define    TUNER_DTC                0x40            /* 1: de-emphasis time constants is 75us, 0: 50us   */

/* bit mask value for the 1st data in read mode     */
#define    TUNER_RF                 0x80            /* Ready Flag, 1: a station has been found          */
#define    TUNER_BLF                0x40            /* Band Limit Flag, 1: band limit has been reached  */

/* bit mask value for the 3rd data in read mode     */
#define    TUNER_STEREO             0x80            /* Stereo indication, 1: stereo reception, 0: mono  */

/* bit mask value for the 4th data in read mode     */
#define    TUNER_LEV                0xf0            /* Level ADC output                                 */
#define    TUNER_CI                 0x0e            /* Chip identification, these bit must be set to 0  */
//------------------------------------------------------------------------------
#define    RFF                      0x10
#define    RFNE                     0x08
#define    TFE                      0x04
#define    TFNF                     0x02
#define    ACTIVITY                 0x01
#define    ENABLE                   0

#define    TUNER_SEEK                0x01

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _DRIVER_FM_HW_FM5807_COMMON_  __attribute__((section("driver_fm_hw_fm5807_common")))
#define _DRIVER_FM_HW_FM5807_INIT_  __attribute__((section("driver_fm_hw_fm5807_init")))
#define _DRIVER_FM_HW_FM5807_SHELL_  __attribute__((section("driver_fm_hw_fm5807_shell")))
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

extern RK_FM_DRIVER fm5807Driver;

#endif
