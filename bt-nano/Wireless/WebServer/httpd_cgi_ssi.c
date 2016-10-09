/**
  ******************************************************************************
  * @file    httpd_cg_ssi.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    31-July-2013
  * @brief   Webserver SSI and CGI handlers
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "FreeRTOS.h"
#include "RKOS.h"
#include "lwip/debug.h"
#include "httpd.h"
#include "lwip/tcp.h"
#include "fs.h"
#include "main.h"

#include <string.h>
#include <stdlib.h>
#ifdef __WICE_HTTP_SERVER__
//#pragma arm section code = "lwipcode", rodata = "lwipcode", rwdata = "lwipdata", zidata = "lwipdata"

/* we will use character "t" as tag for CGI */
char const* TAGCHAR="t";
char const** TAGS=&TAGCHAR;

/* CGI handler for LED control */
const char * LEDS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);

/* Html request for "/leds.cgi" will start LEDS_CGI_Handler */
const tCGI LEDS_CGI= {"/leds.cgi", LEDS_CGI_Handler};

/* Cgi call table, only one CGI used */
tCGI CGI_TAB[1];

uint get_ad_value(void);
static u16_t
SSIHandler(int iIndex, char *pcInsert, int iInsertLen)
{
    /* We have only one SSI handler iIndex = 0 */
    if (iIndex ==0)
    {
        //static uint32 ADCVal = 4000;
        uint32 ADCVal = 4000;
        uint8 Digit1 = 0;
        uint8 Digit2 = 0;
        uint8 Digit3 = 0;
        uint8 Digit4 = 0;
#ifdef __DRIVER_I6000_WIFI_C__
        ADCVal = get_ad_value();
        Digit1= ADCVal/1000;
        Digit2= (ADCVal-(Digit1*1000))/100 ;
        Digit3= (ADCVal-((Digit1*1000)+(Digit2*100)))/10;
        Digit4= ADCVal -((Digit1*1000)+(Digit2*100)+ (Digit3*10));
        ADCVal++;
#endif
        /* prepare data to be inserted in html */
        *pcInsert       = (char)(Digit1+0x30);
        *(pcInsert + 1) = (char)(Digit2+0x30);
        *(pcInsert + 2) = (char)(Digit3+0x30);
        *(pcInsert + 3) = (char)(Digit4+0x30);

        /* 4 characters need to be inserted in html*/
        return 4;
    }

    printf("SSIHandler id = %d\n",iIndex );
    return 0;
}

/**
  * @brief  CGI handler for LEDs control
  */
#ifdef __DRIVER_I6000_WIFI_C__
void led_set_onoff(uint id, uint on_off);
#else
#define led_set_onoff(x,y)
#endif
const char * LEDS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{


    uint32 i=0;

    /* We have only one SSI handler iIndex = 0 */
    if (iIndex==0)
    {
        /* All leds off */
        //STM_EVAL_LEDOff(LED1);

        /* Check cgi parameter : example GET /leds.cgi?led=2&led=4 */
        printf("led all off\n");
        led_set_onoff(1,0);
        //led_set_onoff(2,0);
        led_set_onoff(3,0);
        led_set_onoff(4,0);
        for (i=0; i<iNumParams; i++)
        {
            /* check parameter "led" */
            if (strcmp(pcParam[i] , "led")==0)
            {
                /* switch led1 ON if 1 */
                if(strcmp(pcValue[i], "1") ==0)
                {
                    //STM_EVAL_LEDOn(LED1);
                    printf("led 1 on\n");
                    led_set_onoff(1,1);
                }
                /* switch led2 ON if 2 */
                else if(strcmp(pcValue[i], "2") ==0)
                {
                    //STM_EVAL_LEDOn(LED1);
                    printf("led 2 on\n");
                    //led_set_onoff(2,1);
                }
                /* switch led3 ON if 3 */
                else if(strcmp(pcValue[i], "3") ==0)
                {
                    //STM_EVAL_LEDOn(LED1);
                    printf("led 3 on\n");
                    led_set_onoff(3,1);
                }
                /* switch led4 ON if 4 */
                else if(strcmp(pcValue[i], "4") ==0)
                {
                    //STM_EVAL_LEDOn(LED1);
                    printf("led 4 on\n");
                    led_set_onoff(4,1);
                }
            }
        }
    }
    /* uri to send after cgi call*/
    return "/RKLED.html";
}

/**
 * Initialize SSI handlers
 */
void httpd_ssi_init(void)
{
    /* configure SSI handlers (ADC page SSI) */
    http_set_ssi_handler(SSIHandler, (char const **)TAGS, 1);
}

/**
 * Initialize CGI handlers
 */
void httpd_cgi_init(void)
{
    /* configure CGI handlers (LEDs control CGI) */
    CGI_TAB[0] = LEDS_CGI;
    http_set_cgi_handlers(CGI_TAB, 1);
}
//#pragma arm section code
#endif
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
