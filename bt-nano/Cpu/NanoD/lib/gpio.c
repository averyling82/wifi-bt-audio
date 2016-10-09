/*
********************************************************************************************
*
*                Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Cpu\NanoD\lib\gpio.c
* Owner: chad.Ma
* Date: 2014.12.10
* Time: 15:46:54
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    chad.Ma     2014.12.10     15:46:54   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __CPU_NANOD_LIB_GPIO_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "BSP.h"


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _CPU_NANOD_LIB_GPIO_READ_  __attribute__((section("cpu_nanod_lib_gpio_read")))
#define _CPU_NANOD_LIB_GPIO_WRITE_ __attribute__((section("cpu_nanod_lib_gpio_write")))
#define _CPU_NANOD_LIB_GPIO_INIT_  __attribute__((section("cpu_nanod_lib_gpio_init")))
#define _CPU_NANOD_LIB_GPIO_SHELL_  __attribute__((section("cpu_nanod_lib_gpio_shell")))


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/



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
rk_err_t GpioShellSpeedTest(void);
void GpioInt2(void);
void GpioInt1(void);
void GpioInt0(void);
rk_err_t GPIOShellHelp(uint8 * pstr);
rk_err_t GPIOShell(HDC dev,uint8 * pstr);
void GPIO_ISR(void);
rk_err_t GPIOBspShellHelp(uint8 * pstr);
rk_err_t GPIOBspShell_PinINTTest(uint8 * pstr);
rk_err_t GPIOBspShell_PinLevelTest(uint8 * pstr);
rk_err_t GPIOBspShell_PinDirTest(uint8 * pstr);
rk_err_t GPIOBspShell(uint8 * pstr);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/




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



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#ifdef _USE_SHELL_
#ifdef _GPIO_SHELL_

_CPU_NANOD_LIB_GPIO_SHELL_
static SHELL_CMD ShellGPIOName[] =
{
    "speed",GpioShellSpeedTest,"test gpio seep","gpio.speed\n",
    "bsp",NULL,"NULL","NULL",
    "\b",NULL,"NULL","NULL",
};

#ifdef SHELL_BSP
_CPU_NANOD_LIB_GPIO_SHELL_
static SHELL_CMD ShellGPIOBspName[] =
{
    "help",NULL,"NULL","NULL",
    "pullup",NULL,"NULL","NULL",
    "pulldown",NULL,"NULL","NULL",
    "in",NULL,"NULL","NULL",
    "out",NULL,"NULL","NULL",
    "riseint",NULL,"NULL","NULL",
    "fallint",NULL,"NULL","NULL",
    "lowint",NULL,"NULL","NULL",
    "highint",NULL,"NULL","NULL",
    "\b",NULL,"NULL","NULL",
};

_CPU_NANOD_LIB_GPIO_SHELL_ int gGPIOChannel;
_CPU_NANOD_LIB_GPIO_SHELL_ int gGPIOPinNum;
_CPU_NANOD_LIB_GPIO_SHELL_ int gGPIOPinIOMux;
_CPU_NANOD_LIB_GPIO_SHELL_ int gGPIOPinIntType;
_CPU_NANOD_LIB_GPIO_SHELL_ int gGPIO_intflag;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: GpioInt2
** Input:void
** Return: void
** Owner:chad.Ma
** Date: 2014.12.11
** Time: 14:04:46
*******************************************************************************/
_CPU_NANOD_LIB_GPIO_SHELL_
SHELL FUN void GpioInt2(void)
{
    GpioInt(GPIO_CH2);
}

/*******************************************************************************
** Name: GpioInt1
** Input:void
** Return: void
** Owner:chad.Ma
** Date: 2014.12.11
** Time: 14:04:27
*******************************************************************************/
_CPU_NANOD_LIB_GPIO_READ_
SHELL FUN void GpioInt1(void)
{
    GpioInt(GPIO_CH1);
}

/*******************************************************************************
** Name: GpioInt0
** Input:void
** Return: void
** Owner:chad.Ma
** Date: 2014.12.11
** Time: 14:03:42
*******************************************************************************/
_CPU_NANOD_LIB_GPIO_READ_
SHELL FUN void GpioInt0(void)
{
    GpioInt(GPIO_CH0);
}


/*******************************************************************************
** Name: GPIO_ISR
** Input:void
** Return: void
** Owner:chad.Ma
** Date: 2014.12.10
** Time: 17:07:42
*******************************************************************************/
_CPU_NANOD_LIB_GPIO_SHELL_
SHELL FUN void GPIO_ISR(void)
{
    gGPIO_intflag = 1;
    Gpio_DisableInt(gGPIOChannel,gGPIOPinNum);
    GPIOIsrUnRegister(gGPIOPinNum);
}

/*******************************************************************************
** Name: GPIOBspShell_PinINTTest
** Input:HDC dev, uint8 * pstr
** Return: rk
** Owner:chad.Ma
** Date: 2014.12.10
** Time: 16:08:22
*******************************************************************************/
_CPU_NANOD_LIB_GPIO_SHELL_
SHELL FUN rk_err_t GPIOBspShell_PinINTTest(uint8 * pstr)
{
#ifdef SHELL_HELP
	pstr--;
    if(pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if(StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("gpio int test : 该命令用来测试gpio某一管脚的四种中断形式.\r\n");
            return RK_SUCCESS;
        }
    }
#endif

    pstr++; //omit the '.' to next char
    gGPIOChannel = GPIO_CH0;
    gGPIOPinNum  = GPIOPortB_Pin2;
    gGPIOPinIOMux = IOMUX_GPIO0A0_IO;
    gGPIO_intflag = 0;

    switch(gGPIOChannel)
    {
        case GPIO_CH0:
            IntRegister(INT_ID_GPIO0 ,GpioInt0);
            IntPendingClear(INT_ID_GPIO0);
            IntEnable(INT_ID_GPIO0);
            break;
        case GPIO_CH1:
            IntRegister(INT_ID_GPIO1 ,GpioInt1);
            IntPendingClear(INT_ID_GPIO1);
            IntEnable(INT_ID_GPIO1);
            break;
        case GPIO_CH2:
            IntRegister(INT_ID_GPIO2 ,GpioInt2);
            IntPendingClear(INT_ID_GPIO2);
            IntEnable(INT_ID_GPIO2);
            break;

        default:
            break;
    }

    Gpio_EnableInt(gGPIOChannel,gGPIOPinNum);
    GpioIsrRegister(gGPIOPinNum, GPIO_ISR);

    if(StrCmpA(pstr,"rise",4) == 0)
    {
        gGPIOPinIntType = IntrTypeRisingEdge;
        GpioMuxSet(gGPIOChannel,gGPIOPinNum,gGPIOPinIOMux);

        Gpio_SetPinDirection(gGPIOChannel,gGPIOPinNum,GPIO_IN);
        GpioMuxSet(gGPIOChannel,GPIOPortA_Pin2,gGPIOPinIOMux);
        Gpio_SetPinDirection(gGPIOChannel,GPIOPortA_Pin4,GPIO_OUT);

        Gpio_SetPinLevel(gGPIOChannel,GPIOPortA_Pin4,GPIO_LOW);

        Gpio_SetIntMode(gGPIOChannel,gGPIOPinNum, gGPIOPinIntType);

        DelayMs(100);
        Gpio_SetPinLevel(gGPIOChannel,GPIOPortA_Pin4,GPIO_HIGH);

    }
    else if(StrCmpA(pstr,"fall",4) == 0)
    {
        gGPIOPinIntType = IntrTypeFallingEdge;
        GpioMuxSet(gGPIOChannel,gGPIOPinNum,gGPIOPinIOMux);
        Gpio_SetPinDirection(gGPIOChannel,gGPIOPinNum,GPIO_IN);
        GpioMuxSet(gGPIOChannel,GPIOPortA_Pin2,gGPIOPinIOMux);

        Gpio_SetPinDirection(gGPIOChannel,GPIOPortA_Pin4,GPIO_OUT);
        Gpio_SetPinLevel(gGPIOChannel,GPIOPortA_Pin4,GPIO_HIGH);

        Gpio_SetIntMode(gGPIOChannel,gGPIOPinNum, gGPIOPinIntType);

        DelayMs(100);
        Gpio_SetPinLevel(gGPIOChannel,GPIOPortA_Pin4,GPIO_LOW);
    }
    else if(StrCmpA(pstr,"low",3) == 0)
    {
        gGPIOPinIntType = IntrTypeLowLevel;
        GpioMuxSet(gGPIOChannel,gGPIOPinNum,gGPIOPinIOMux);
        Gpio_SetPinDirection(gGPIOChannel,gGPIOPinNum,GPIO_IN);
        Gpio_SetIntMode(gGPIOChannel,gGPIOPinNum, gGPIOPinIntType);

        Gpio_SetPinDirection(gGPIOChannel,GPIOPortA_Pin4,GPIO_OUT);
        Gpio_SetPinLevel(gGPIOChannel,GPIOPortA_Pin4,GPIO_HIGH);
        DelayMs(100);
        Gpio_SetPinLevel(gGPIOChannel,GPIOPortA_Pin4,GPIO_LOW);
    }
    else if(StrCmpA(pstr,"high",4) == 0)
    {
        gGPIOPinIntType = IntrTypeHighLevel;
        GpioMuxSet(gGPIOChannel,gGPIOPinNum,gGPIOPinIOMux);
        Gpio_SetPinDirection(gGPIOChannel,gGPIOPinNum,GPIO_IN);
        Gpio_SetIntMode(gGPIOChannel,gGPIOPinNum, gGPIOPinIntType);

        Gpio_SetPinDirection(gGPIOChannel,GPIOPortA_Pin4,GPIO_OUT);
        Gpio_SetPinLevel(gGPIOChannel,GPIOPortA_Pin4,GPIO_LOW);
        DelayMs(100);
        Gpio_SetPinLevel(gGPIOChannel,GPIOPortA_Pin4,GPIO_HIGH);
    }

    while(!gGPIO_intflag)
        ;


    switch(gGPIOChannel)
    {
        case GPIO_CH0:
            IntDisable(INT_ID_GPIO0);
            IntPendingClear(INT_ID_GPIO0);
            IntUnregister(INT_ID_GPIO0);
            break;

        case GPIO_CH1:
            IntDisable(INT_ID_GPIO1);
            IntPendingClear(INT_ID_GPIO1);
            IntUnregister(INT_ID_GPIO1);
            break;

        case GPIO_CH2:
            IntDisable(INT_ID_GPIO2);
            IntPendingClear(INT_ID_GPIO2);
            IntUnregister(INT_ID_GPIO2);
            break;
    }

    printf(" gpio %d had interrupted~~~ ",gGPIOPinNum);
    switch(gGPIOPinIntType)
    {
        case IntrTypeRisingEdge:
            printf(" interrupt type is:  riseing edge." );
            break;
        case IntrTypeFallingEdge:
            printf(" interrupt type is:  falling edge." );
            break;
        case IntrTypeLowLevel:
            printf(" interrupt type is:  low level." );
            break;
        case IntrTypeHighLevel:
            printf(" interrupt type is:  high level." );
            break;
    }


    return RK_SUCCESS;
}

/*******************************************************************************
** Name: GPIOBspShell_PinLevelTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_T
** Owner:chad.Ma
** Date: 2014.12.10
** Time: 15:59:25
*******************************************************************************/
_CPU_NANOD_LIB_GPIO_SHELL_
SHELL FUN rk_err_t GPIOBspShell_PinLevelTest(uint8 * pstr)
{
    int level;
#ifdef SHELL_HELP
	pstr--;
    if(pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if(StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("gpio pin level : 设置gpio某一管脚的电平高低.\r\n");
            return RK_SUCCESS;
        }
    }
#endif

    pstr++; //omit the '.' to next char
    gGPIOChannel = GPIO_CH0;
    gGPIOPinNum  = GPIOPortB_Pin2;
    gGPIOPinIOMux = IOMUX_GPIO0A0_IO;

    if(StrCmpA(pstr,"up",2) == 0)
    {
        level = GPIO_HIGH;
        GpioMuxSet(gGPIOChannel,gGPIOPinNum,gGPIOPinIOMux);
        Gpio_SetPinDirection(gGPIOChannel,gGPIOPinNum,GPIO_OUT);
        Gpio_SetPinLevel(gGPIOChannel,gGPIOPinNum,level);
        printf(" pull up gpio %d \n",gGPIOPinNum);
    }
    else if(StrCmpA(pstr,"down",4) == 0)
    {
        level = GPIO_LOW;
        GpioMuxSet(gGPIOChannel,gGPIOPinNum,gGPIOPinIOMux);
        Gpio_SetPinDirection(gGPIOChannel,gGPIOPinNum,GPIO_OUT);
        Gpio_SetPinLevel(gGPIOChannel,gGPIOPinNum,level);
        printf(" pull down gpio %d \n",gGPIOPinNum);
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: GPIOBspShell_PinDirTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.12.10
** Time: 15:58:07
*******************************************************************************/
_CPU_NANOD_LIB_GPIO_SHELL_
SHELL FUN rk_err_t GPIOBspShell_PinDirTest(uint8 * pstr)
{
    int direction;
#ifdef SHELL_HELP
	pstr--;
    if(pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if(StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("gpio pin direction : 设置gpio某一管脚的输入或输出方向.\r\n");
            return RK_SUCCESS;
        }
    }
#endif

    pstr++; //omit the '.' to next char
    gGPIOChannel = GPIO_CH0;
    gGPIOPinNum  = GPIOPortA_Pin0;
    gGPIOPinIOMux = IOMUX_GPIO0A0_IO;

    if(StrCmpA(pstr,"in",2) == 0)
    {
        direction = GPIO_IN;
        GpioMuxSet(gGPIOChannel,gGPIOPinNum,gGPIOPinIOMux);
        Gpio_SetPinDirection(gGPIOChannel,gGPIOPinNum,direction);
        printf("gpio %d direction set in~.\r\n",gGPIOPinNum);
    }

    if(StrCmpA(pstr,"out",3) == 0)
    {
        direction = GPIO_OUT;
        GpioMuxSet(gGPIOChannel,gGPIOPinNum,gGPIOPinIOMux);
        Gpio_SetPinDirection(gGPIOChannel,gGPIOPinNum,direction);
        printf("gpio %d direction set out~.\r\n",gGPIOPinNum);
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: GPIO_BSP_SHELL
** Input:HDC Dev,uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.12.10
** Time: 15:52:45
*******************************************************************************/
_CPU_NANOD_LIB_GPIO_SHELL_
SHELL FUN rk_err_t GPIOBspShell(uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret;
    uint8 Space;

    StrCnt = ShellItemExtract(pstr, &pItem, &Space);

    if (StrCnt == 0)
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellGPIOBspName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                          //remove '.',the point is the useful item

    uint8 ptemp[10];

    switch (i)
    {
        case 0x00:  //bsp help
            #ifdef SHELL_HELP
            ret = GPIOBspShellHelp(pItem);
            #endif

            break;

        case 0x01:  //pull up
            MemCpy(ptemp, "up", 2);
            ret = GPIOBspShell_PinLevelTest(ptemp);
            break;

        case 0x02:  //pull down
            MemCpy(ptemp, "down", 4);
            ret = GPIOBspShell_PinLevelTest(ptemp);

            break;

        case 0x03:  //gpio as in
            MemCpy(ptemp, "in", 2);
            ret = GPIOBspShell_PinDirTest(ptemp);
            break;

        case 0x04:  //gpio as out
            MemCpy(ptemp, "out", 3);
            ret = GPIOBspShell_PinDirTest(ptemp);
            break;

        case 0x05:  //gpio int test
            MemCpy(ptemp, "rise", 4);
            ret = GPIOBspShell_PinINTTest(ptemp);
            break;

        case 0x06:  //gpio int test
            MemCpy(ptemp, "fall", 4);
            ret = GPIOBspShell_PinINTTest(ptemp);
            break;

        case 0x07:  //gpio int test
            MemCpy(ptemp, "low", 3);
            ret = GPIOBspShell_PinINTTest(ptemp);
            break;

        case 0x08:  //gpio int test
            MemCpy(ptemp, "high", 4);
            ret = GPIOBspShell_PinINTTest(ptemp);
            break;

        default:
            ret = RK_ERROR;
            break;
    }
    return ret;
}

#endif

/*******************************************************************************
** Name: GpioShellSpeedTest
** Input:void
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.6.3
** Time: 16:26:16
*******************************************************************************/
_CPU_NANOD_LIB_GPIO_SHELL_
SHELL FUN rk_err_t GpioShellSpeedTest(void)
{
    uint32 flag;

    Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin1,IOMUX_GPIO1B1_IO);
    Gpio_SetPinDirection(GPIO_CH1,GPIOPortB_Pin1, GPIO_OUT);
    Grf_GPIO_SetPinPull(GPIO_CH1, GPIOPortB_Pin1, ENABLE);

    #if 0
    flag = 1;

    while(1)
    {
        if(flag)
        {
            Gpio_SetPinLevel(GPIO_CH1,GPIOPortB_Pin1, GPIO_HIGH);
        }
        else
        {
            Gpio_SetPinLevel(GPIO_CH1,GPIOPortB_Pin1, GPIO_LOW);
        }

        flag = 1 - flag;
    }
    #else
    {
        UINT8  pin;
        UINT32 GPIOPortData;
        GPIO_t* pgpio;

        pgpio = ((GPIO_t*)GPIO1_BASE);

        pin   = GPIOPortB_Pin1 % 32;

        while(1)
        {
            pgpio->PortData = pgpio->PortData ^ (1 << pin);
        }
    }
    #endif
}


/*******************************************************************************
** Name: GPIOShell
** Input:HDC dev,uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.12.11
** Time: 10:34:58
*******************************************************************************/
_CPU_NANOD_LIB_GPIO_SHELL_
SHELL FUN rk_err_t GPIOShell(void* dev,uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret;
    uint8 Space;

    StrCnt = ShellItemExtract(pstr, &pItem, &Space);

    if (StrCnt == 0)
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellGPIOName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;

    ShellHelpDesDisplay(dev, ShellGPIOName[i].CmdDes, pItem);
    if(ShellGPIOName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellGPIOName[i].ShellCmdParaseFun(dev, pItem);
    }

    return ret;

}


#endif

#define _IN_GPIO_
#include <typedef.h>
#include <DriverInclude.h>


const uint32 GPIO_GROUP[3] =
{
    GPIO0_BASE,
    GPIO1_BASE,
    GPIO2_BASE,
};

//#define UartReg              ((UART_REG*)UART_GROUP)

#define GPIOPORT(n)          ((GPIO_t*)(GPIO_GROUP[n]))



//#ifdef DRIVERLIB_IRAM
/*
--------------------------------------------------------------------------------
  Function name : GPIO_SetPinDirection(eGPIOPinNum_t GPIOPinNum, eGPIOPinDirection_t direction)
  Author        : anzhiguo
  Description   :

  Input         : GPIOPinNum --
                : direction --
  Return        :

  History:     <author>         <time>         <version>
             anzhiguo     2009-1-14         Ver1.0
  desc:        ORG
--------------------------------------------------------------------------------
*/
void Gpio_SetPinDirection(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum, eGPIOPinDirection_t direction)
{
    UINT8  pin;
    GPIO_t* pgpio;

    pgpio = GPIOPORT(gpioPort);

    pin    = GPIOPinNum % 32;

    if (direction)
    {
        pgpio->PortDir |= (1 << pin);
    }
    else
    {
        pgpio->PortDir &= ~(1 << pin);
    }
}

void Gpio_SetPortDirec(eGPIO_CHANNEL gpioChn,uint32 dirData)
{
    UINT8  pin;
    GPIO_t* pgpio;

    pgpio = GPIOPORT(gpioChn);

    pgpio->PortDir = dirData;
}

/*
--------------------------------------------------------------------------------
  Function name : GPIO_GetPinDirection(eGPIOPinNum_t GPIOPinNum)
  Author        : anzhiguo
  Description   :

  Input         : GPIOPinNum -- GPIO pin number

  Return        : not 0 -- out direction
                  0   -- in direction
  History:     <author>         <time>         <version>
             anzhiguo     2009-1-14         Ver1.0
  desc:        ORG
--------------------------------------------------------------------------------
*/
UINT8 Gpio_GetPinDirection(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum)
{
    UINT8  pin;
    GPIO_t* pgpio;

    pgpio = GPIOPORT(gpioPort);

    pin   = GPIOPinNum % 32;
    return (((pgpio->PortDir) & (0x1 << pin)) >> pin);

}

UINT8 Gpio_GetPortDirec(eGPIO_CHANNEL gpioChn,eGPIO_PORT gpioPort)
{
    UINT8  pin;
    GPIO_t* pgpio;
    uint32 portDir;
    uint32 pinDir;
    int i;

    pgpio = GPIOPORT(gpioChn);
    portDir = pgpio->PortDir;

//    printf("    0 -- IN , 1 -- OUT   \n");
    if( gpioPort == GPIO_PORTA)
    {
        for(i = 0; i < 8 ;i++)
        {
            pinDir = ((portDir & (0x1 << i)) >> i);
            //printf("gpio channel %d:PORT A pin %d direction is [%s] \n",gpioChn,i,(pinDir==0) ? "IN":"OUT");
        }
    }
    else if( gpioPort == GPIO_PORTB)
    {
        for(i = 8; i < 16 ;i++)
        {
            pinDir = ((portDir & (0x1 << i)) >> i);
            //printf("gpio channel %d:PORT B pin %d direction is [%s] \n",gpioChn,i,(pinDir==0) ? "IN":"OUT");
        }
    }
    else if( gpioPort == GPIO_PORTC)
    {
        for(i = 16; i < 24 ;i++)
        {
            pinDir = ((portDir & (0x1 << i)) >> i);
            //printf("gpio channel %d:PORT C pin %d direction is [%s]\n",gpioChn,i,(pinDir==0) ? "IN":"OUT");
        }
    }
    else if( gpioPort == GPIO_PORTD)
    {
        for(i = 24; i < 32 ;i++)
        {
            pinDir = ((portDir & (0x1 << i)) >> i);
            //printf("gpio channel %d:PORT D pin %d direction is %s \n",gpioChn,i,(pinDir==0) ? "IN":"OUT");
        }
    }

    return 0;
}

/*
--------------------------------------------------------------------------------
  Function name : GPIO_SetPinLevel(eGPIOPinNum_t GPIOPinNum, eGPIOPinLevel_t level)
  Author        : anzhiguo
  Description   :

  Input         : GPIOPinNum -- GPIO pin number
                  level -- 1 or 0(high or low)
  Return        :

  History:     <author>         <time>         <version>
             anzhiguo     2009-1-14         Ver1.0
  desc:      GPIOPinNum define the enum by need in future.
--------------------------------------------------------------------------------
*/
void Gpio_SetPinLevel(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum, eGPIOPinLevel_t level)
{
    UINT8  pin;
    UINT32 GPIOPortData;
    GPIO_t* pgpio;

    pgpio = GPIOPORT(gpioPort);

    pin   = GPIOPinNum % 32;

    if (level)
    {
        pgpio->PortData |= (1 << pin);
    }
    else
    {
        pgpio->PortData &= ~(1 << pin);
    }
}

void Gpio_SetChnLevel(eGPIO_CHANNEL gpioChn,eGPIOPinLevel_t level)
{
    GPIO_t* pgpio;

    pgpio = GPIOPORT(gpioChn);

    if (level)
    {
        pgpio->PortData = 0xffffffff;
    }
    else
    {
        pgpio->PortData = 0x00000000;
    }

}

void Gpio_SetPortLevel(eGPIO_CHANNEL gpioChn,eGPIO_PORT gpioPort,eGPIOPinLevel_t level)
{
    GPIO_t* pgpio;

    pgpio = GPIOPORT(gpioChn);

    if (level)
    {
        if( gpioPort == GPIO_PORTA)
            pgpio->PortData = 0xff;
        else if( gpioPort == GPIO_PORTB)
            pgpio->PortData |= 0xff << 8;
        else if( gpioPort == GPIO_PORTC)
            pgpio->PortData |= 0xff << 16;
        else if( gpioPort == GPIO_PORTD)
            pgpio->PortData |= 0xff << 24;
    }
    else
    {
        if( gpioPort == GPIO_PORTA)
            pgpio->PortData &= ~(0xff);
        else if( gpioPort == GPIO_PORTB)
            pgpio->PortData &= ~(0xff << 8);
        else if( gpioPort == GPIO_PORTC)
            pgpio->PortData &= ~(0xff << 16);
        else if( gpioPort == GPIO_PORTD)
            pgpio->PortData &= ~(0xff << 24);
    }
}

/*
--------------------------------------------------------------------------------
  Function name : GPIO_GetPinLevel(eGPIOPinNum_t GPIOPinNum)
  Author        : anzhiguo
  Description   :

  Input         : GPIOPinNum -- GPIO pin number

  Return        : no 0 is high, 0 is low

  History:     <author>         <time>         <version>
             anzhiguo     2009-1-14         Ver1.0
  desc:        ORG
--------------------------------------------------------------------------------
*/
UINT8 Gpio_GetPinLevel(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum)
{
    UINT8  pin;
    GPIO_t* pgpio;

    pgpio = GPIOPORT(gpioPort);

    //pin   = GPIOPinNum % 32;

    return (((pgpio->ExtPort) & (0x1 << pin)) >> pin);
}


UINT8 Gpio_GetPortLevel(eGPIO_CHANNEL gpioChn,eGPIO_PORT gpioPort)
{
    uint32 i;
    UINT8  pin;
    uint32 portLev;
    uint32 pinLev;
    GPIO_t* pgpio;

    pgpio = GPIOPORT(gpioChn);
    portLev = pgpio->ExtPort;

    printf(" \n  gpio channel %d = 0x%08x \n", gpioChn, portLev);

    return 0;
}

/*
--------------------------------------------------------------------------------
  Function name : GPIO_SetIntMode(eGPIOPinNum_t GPIOPinNum, GPIOIntrType_c type)
  Author        : anzhiguo
  Description   : set GPIO pin interrupt type

  Input         : GPIOPinNum -- GPIO pin
                : type --
  Return        : null

  note          :recommend use edge type.
  History:     <author>         <time>         <version>
             anzhiguo     2009-1-14         Ver1.0
  desc:        ORG
--------------------------------------------------------------------------------
*/
void Gpio_SetIntMode(eGPIO_CHANNEL gpioChn,eGPIOPinNum_t GPIOPinNum, GPIOIntrType_c type)
{
    GPIO_t* pgpio;

    pgpio = GPIOPORT(gpioChn);

    switch (type)
    {
        case IntrTypeRisingEdge://rise edge
            pgpio->PortIntType     |= (1 << GPIOPinNum);//edge trigger
            pgpio->PortIntPolarity |= (1 << GPIOPinNum);//level trigger
            break;

        case IntrTypeFallingEdge://down edge
            pgpio->PortIntType     |=  (1 << GPIOPinNum);//edge trigger
            pgpio->PortIntPolarity &= ~(1 << GPIOPinNum);//low level valid
            break;

        case IntrTypeHighLevel://high voltage
            pgpio->PortIntType     &= ~(1 << GPIOPinNum);//level trigger
            pgpio->PortIntPolarity |=  (1 << GPIOPinNum);//high leve valid
            break;

        case IntrTypeLowLevel://low level
            pgpio->PortIntType     &= ~(1 << GPIOPinNum);//level trigger
            pgpio->PortIntPolarity &= ~(1 << GPIOPinNum);//low level valid
            break;

        default:

            break;
    }
}

void Gpio_SetPortIntMode(eGPIO_CHANNEL gpioChn,eGPIO_PORT gpioPort, GPIOIntrType_c type)
{
    GPIO_t* pgpio;

    pgpio = GPIOPORT(gpioChn);

    pgpio->PortIntType      = 0;
    pgpio->PortIntPolarity  = 0;

    switch (type)
    {
        case IntrTypeRisingEdge://rise edge
        {
            if(gpioPort == GPIO_PORTA)
            {
                pgpio->PortIntType     |= 0x000000ff;//edge trigger
                pgpio->PortIntPolarity |= 0x000000ff;//level trigger
            }
            else if(gpioPort == GPIO_PORTB)
            {
                pgpio->PortIntType     |= 0x0000ff00;//edge trigger
                pgpio->PortIntPolarity |= 0x0000ff00;//level trigger
            }
            else if(gpioPort == GPIO_PORTC)
            {
                pgpio->PortIntType     |= 0x00ff0000;//edge trigger
                pgpio->PortIntPolarity |= 0x00ff0000;//level trigger
            }
            else if(gpioPort == GPIO_PORTD)
            {
                pgpio->PortIntType     |= 0xff000000;//edge trigger
                pgpio->PortIntPolarity |= 0xff000000;//level trigger
            }
        }
            break;

        case IntrTypeFallingEdge://down edge
        {
            if(gpioPort == GPIO_PORTA)
            {
                pgpio->PortIntType     |= 0x000000ff;//edge trigger
                pgpio->PortIntPolarity &= 0xffffff00;//level trigger
            }
            else if(gpioPort == GPIO_PORTB)
            {
                pgpio->PortIntType     |= 0x0000ff00;//edge trigger
                pgpio->PortIntPolarity &= 0xffff00ff;//level trigger
            }
            else if(gpioPort == GPIO_PORTC)
            {
                pgpio->PortIntType     |= 0x00ff0000;//edge trigger
                pgpio->PortIntPolarity &= 0xff00ffff;//level trigger
            }
            else if(gpioPort == GPIO_PORTD)
            {
                pgpio->PortIntType     |= 0xff000000;//edge trigger
                pgpio->PortIntPolarity &= 0x00ffffff;//level trigger
            }
        }
            break;

        case IntrTypeHighLevel://high voltage
        {
            if(gpioPort == GPIO_PORTA)
            {
                pgpio->PortIntType     &= 0xffffff00;//edge trigger
                pgpio->PortIntPolarity |= 0x000000ff;//level trigger
            }
            else if(gpioPort == GPIO_PORTB)
            {
                pgpio->PortIntType     &= 0xffff00ff;//edge trigger
                pgpio->PortIntPolarity |= 0x0000ff00;//level trigger
            }
            else if(gpioPort == GPIO_PORTC)
            {
                pgpio->PortIntType     &= 0xff00ffff;//edge trigger
                pgpio->PortIntPolarity |= 0x00ff0000;//level trigger
            }
            else if(gpioPort == GPIO_PORTD)
            {
                pgpio->PortIntType     &= 0x00ffffff;//edge trigger
                pgpio->PortIntPolarity |= 0xff000000;//level trigger
            }
        }
            break;

        case IntrTypeLowLevel://low level
        {
            if(gpioPort == GPIO_PORTA)
            {
                pgpio->PortIntType     &= 0xffffff00;//edge trigger
                pgpio->PortIntPolarity &= 0xffffff00 ;//level trigger
            }
            else if(gpioPort == GPIO_PORTB)
            {
                pgpio->PortIntType     &= 0xffff00ff;//edge trigger
                pgpio->PortIntPolarity &= 0xffff00ff;//level trigger
            }
            else if(gpioPort == GPIO_PORTC)
            {
                pgpio->PortIntType     &= 0xff00ffff;//edge trigger
                pgpio->PortIntPolarity &= 0xff00ffff;//level trigger
            }
            else if(gpioPort == GPIO_PORTD)
            {
                pgpio->PortIntType     &= 0x00ffffff;//edge trigger
                pgpio->PortIntPolarity &= 0x00ffffff;//level trigger
            }
        }
            break;

        default:
            break;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : GPIO_GetIntMode(eGPIOPinNum_t GPIOPinNum)
  Author        : anzhiguo
  Description   : get interrupt type:edge or level

  Input         : GPIOPinNum -- GPIO pin
                :
  Return        : 0:level

  History:     <author>         <time>         <version>
             anzhiguo     2009-1-14         Ver1.0
  desc:        ORG
--------------------------------------------------------------------------------
*/
UINT32 Gpio_GetIntMode(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum)
{
    GPIO_t* pgpio;

    pgpio = GPIOPORT(gpioPort);
    return ((pgpio->PortIntType) & (0x1 << GPIOPinNum));
}

/*
--------------------------------------------------------------------------------
  Function name : GPIO_ClearInt_Level(eGPIOPinNum_t GPIOPinNum)
  Author        : anzhiguo
  Description   : mask interrupt.

  Input         : GPIOPinNum -- GPIO pin
                :
  Return        : null

  History:     <author>         <time>         <version>
             anzhiguo     2009-1-14         Ver1.0
  desc:        ORG
  note          : after clear interrupt trigger type,must clear interrupt mask bit,if not,it will affect
                  next interrupt.
--------------------------------------------------------------------------------
*/
void Gpio_ClearInt_Level(eGPIO_CHANNEL gpioChn,eGPIOPinNum_t GPIOPinNum)
{
    GPIO_t* pgpio;

    pgpio = GPIOPORT(gpioChn);
    pgpio->PortIntMask |= (0x1 << GPIOPinNum);
}

void Gpio_ClearPortInt_Level(eGPIO_CHANNEL gpioChn,eGPIO_PORT gpioPort)
{
    GPIO_t* pgpio;

    pgpio = GPIOPORT(gpioChn);

    if( gpioPort == GPIO_PORTA)
    {
        pgpio->PortIntMask |= 0xff;
    }
    else if( gpioPort == GPIO_PORTB)
    {
        pgpio->PortIntMask |= 0xff00;
    }
    else if( gpioPort == GPIO_PORTC)
    {
        pgpio->PortIntMask |= 0xff0000;
    }
    else if( gpioPort == GPIO_PORTD)
    {
        pgpio->PortIntMask |= 0xff000000;
    }
}


/*
--------------------------------------------------------------------------------
  Function name : GPIO_EnableInt(eGPIOPinNum_t GPIOPinNum)
  Author        : anzhiguo
  Description   :

  Input         : GPIOPinNum -- GPIO pin

  Return        :

  History:     <author>         <time>         <version>
             anzhiguo     2009-1-14         Ver1.0
  desc:        ORG
--------------------------------------------------------------------------------
*/
void Gpio_EnableInt(eGPIO_CHANNEL gpioChn,eGPIOPinNum_t GPIOPinNum)
{
    GPIO_t* pgpio;

    pgpio = GPIOPORT(gpioChn);
    pgpio->PortIntEnable |= (1 << GPIOPinNum);
}

void Gpio_EnablePortInt(eGPIO_CHANNEL gpioChn,eGPIO_PORT gpioPort)
{
    GPIO_t* pgpio;

    pgpio = GPIOPORT(gpioChn);

    if( gpioPort == GPIO_PORTA)
    {
        pgpio->PortIntEnable |= 0xff;
    }
    else if( gpioPort == GPIO_PORTB)
    {
        pgpio->PortIntEnable |= 0xff00;
    }
    else if( gpioPort == GPIO_PORTC)
    {
        pgpio->PortIntEnable |= 0xff0000;
    }
    else if( gpioPort == GPIO_PORTD)
    {
        pgpio->PortIntEnable |= 0x0f000000;
    }
}
/*
--------------------------------------------------------------------------------
  Function name : GPIO_DisableInt(eGPIOPinNum_t GPIOPinNum)
  Author        : anzhiguo
  Description   :

  Input         : GPIOPinNum -- GPIO pin

  Return        : null

  History:     <author>         <time>         <version>
             anzhiguo     2009-1-14         Ver1.0
  desc:        ORG
--------------------------------------------------------------------------------
*/
void Gpio_DisableInt(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum)
{
    GPIO_t* pgpio;

    pgpio = GPIOPORT(gpioPort);
    pgpio->PortIntEnable &= ~(1 << GPIOPinNum);
}
//#endif
void Gpio_MaskInt(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum)
{
    GPIO_t* pgpio;

    pgpio = GPIOPORT(gpioPort);
    pgpio->PortIntMask |= (1 << GPIOPinNum);
}

void Gpio_MaskPortInt(eGPIO_CHANNEL gpioPort,uint32 data)
{
    GPIO_t* pgpio;

    pgpio = GPIOPORT(gpioPort);
    pgpio->PortIntMask = data;
}

void Gpio_UnMaskInt(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum)
{
    GPIO_t* pgpio;

    pgpio = GPIOPORT(gpioPort);
    pgpio->PortIntMask &= ~(1 << GPIOPinNum);
}

void Gpio_UnMaskPortInt(eGPIO_CHANNEL gpioPort,uint32 data)
{
    GPIO_t* pgpio;

    pgpio = GPIOPORT(gpioPort);
    pgpio->PortIntMask = data;
 }


void Gpio_ClearEdgeInt(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum)
{
    GPIO_t* pgpio;

    pgpio = GPIOPORT(gpioPort);
    pgpio->PortIntClear |= (1 << GPIOPinNum);
}

/*only when io port configured as input*/
UINT8 Gpio_GetExtPortLevel(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum)
{
    UINT8  pin;
    GPIO_t* pgpio;

    pgpio = GPIOPORT(gpioPort);

    pin   = GPIOPinNum % 32;

    return (((pgpio->ExtPort) & (0x1 << pin)) >> pin);
}


void Gpio_SetLevelSens_Sync(eGPIO_CHANNEL gpioPort,bool bSync )
{
    GPIO_t* pgpio;

    pgpio = GPIOPORT(gpioPort);

    if( bSync)
        pgpio->LevelSensSync = 1;//all level-sensitive interrupts being synchronized to pclk_intr
    else
        pgpio->LevelSensSync = 0;
}

void Gpio_EnableDebounce(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum)
{
    GPIO_t* pgpio;

    pgpio = GPIOPORT(gpioPort);
    pgpio->DebounceEnable |= (1 << GPIOPinNum);
}

/*
*/
pFunc GPIOIsrCallBack[GPIO_CH_MAX][GPIOPortD_Pin4]=
{
    /*channel 0*/
    //GPIO_A
    0,0,0,0,0,0,0,0,
    //GPIO_B
    0,0,0,0,0,0,0,0,
    //GPIO_C
    0,0,0,0,0,0,0,0,
    //GPIO_D
    0,0,0,0,

    /*channel 1*/
    //GPIO_A
    0,0,0,0,0,0,0,0,
    //GPIO_B
    0,0,0,0,0,0,0,0,
    //GPIO_C
    0,0,0,0,0,0,0,0,
    //GPIO_D
    0,0,0,0,

    /*channel 2*/
    //GPIO_A
    0,0,0,0,0,0,0,0,
    //GPIO_B
    0,0,0,0,0,0,0,0,
    //GPIO_C
    0,0,0,0,0,0,0,0,
    //GPIO_D
    0,0,0,0

};

/*
*/
int32 GpioIsrRegister(eGPIO_CHANNEL gpioChn,uint32 pin, pFunc CallBack)
{
    if (pin >= GPIOPortD_Pin4)
    {
        printf("GPIO beyond the range: pin = %d !\n" , pin);
        return ERROR;
    }

    GPIOIsrCallBack[gpioChn][pin] = (pFunc)CallBack;

    return OK;
}

int32 GPIOIsrUnRegister(eGPIO_CHANNEL gpioChn,uint32 pin)
{
    if (pin >= GPIOPortD_Pin4)
    {
        printf("GPIO beyond the range: pin = %d!\n", pin);
        return ERROR;
    }

    GPIOIsrCallBack[gpioChn][pin] = NULL;

    return OK;
}

/*
*/
void GpioInt(eGPIO_CHANNEL gpioPort)
{
    int ch = 0;
    uint32 IsEnable;
    uint32 GPIOPinBitTmp;
    pFunc GPIOCallBack;

    uint32 pin;
    uint32 rawStatus;
    pFunc  CallBack;
    GPIO_t* pgpio;

    pgpio = GPIOPORT(gpioPort);

    do
    {
        rawStatus = (uint32)(pgpio->PortIntStatus) & 0x0fffffff;
        pgpio->PortIntClear = rawStatus;

        pin = 0;
        do
        {
             if (rawStatus & (0x01ul << pin))
             {
                rawStatus &= ~(0x01ul << pin);
                CallBack = GPIOIsrCallBack[gpioPort][pin];
                if (CallBack)
                {
                    CallBack();
                }
             }
             pin++;

        }while(rawStatus & 0x0fffffff);

    }while((uint32)(pgpio->PortIntStatus) & 0x0fffffff);

}

void GpioInt2(void)
{
    GpioInt(GPIO_CH2);
}

void GpioInt1(void)
{
    GpioInt(GPIO_CH1);
}

void GpioInt0(void)
{
    GpioInt(GPIO_CH0);
}

/*
********************************************************************************
*
*                         End of Gpio.c
*
********************************************************************************
*/
#endif
#endif

