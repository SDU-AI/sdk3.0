/* Copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * RDA assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. RDA reserves the right to make changes in the
 * software without notification.  RDA also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */

#include "stdio.h"
#include "stdlib.h"
#include "cm_demo_main.h"
#include "cm_demo_uart.h"
#include "cm_demo_fs.h"
#include "cm_demo_fota.h"
#include "cm_demo_asocket.h"
#include "cm_demo_ntp.h"
#include "cm_demo_http.h"
#include "cm_demo_gpio.h"
#include "cm_demo_keypad.h"
#include "cm_demo_pwm.h"
#include "cm_demo_pm.h"
#include "cm_demo_lbs.h"
#include "cm_demo_lcd.h"
#include "cm_demo_cJSON.h"
#include "cm_demo_clk.h"
#include "cm_demo_ssl.h"
#include "cm_demo_mqtt.h"
#include "cm_demo_ftp.h"
#include "cm_demo_adc.h"
#include "cm_demo_i2c.h"
#include "cm_demo_spi.h"
#include "cm_demo_audio.h"
#include "cm_demo_wifiscan.h"
#include "cm_demo_lwip.h"

#include "cm_fs.h"
#include "cm_mem.h"
#include "cm_sys.h"
#include "cm_sim.h"
#include "cm_rtc.h"

#include "cm_virt_at.h"
#include "cm_demo_virt_at.h"
#include "cm_demo_sms.h"
#include "cm_modem.h"
#include "cm_demo_modem.h"

#define SECOND_OF_DAY (24*60*60)

typedef struct cm_tm {
    int tm_sec; /* 秒 – 取值区间为[0,59] */   
    int tm_min; /* 分 - 取值区间为[0,59] */
    int tm_hour; /* 时 - 取值区间为[0,23] */
    int tm_mday; /* 一个月中的日期 - 取值区间为[1,31] */
    int tm_mon; /* 月份 */
    int tm_year; /* 年份 */
}cm_tm_t;

static osThreadId_t OC_APP_TaskHandle = NULL;
osEventFlagsId_t cmd_task_flag = NULL;

static const char * weekday[] = {"Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","Sunday"};
static const char DayOfMon[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

static void cm_sec_to_date(long lSec, cm_tm_t *tTime)
{
    unsigned short i,j,iDay;
    unsigned long lDay;

    lDay = lSec / SECOND_OF_DAY;    
    lSec = lSec % SECOND_OF_DAY;

    i = 1970;
    while(lDay > 365)
    {
        if(((i%4==0)&&(i%100!=0)) || (i%400==0))
        {
            lDay -= 366;
        }
        else
        {
            lDay -= 365;
        }
        i++;
    }
    if((lDay == 365) && !(((i%4==0)&&(i%100!=0)) || (i%400==0)))
    {
       lDay -= 365;
       i++;
    }
    tTime->tm_year = i;   
    for(j=0;j<12;j++)   
    {
        if((j==1) && (((i%4==0)&&(i%100!=0)) || (i%400==0)))
        {
            iDay = 29;
        }
        else
        {
            iDay = DayOfMon[j];
        }
        if(lDay >= iDay) lDay -= iDay;
        else break;
  }
    tTime->tm_mon = j+1;
    tTime->tm_mday = lDay+1;
    tTime->tm_hour = ((lSec / 3600))%24;//这里注意，世界时间已经加上北京时间差8，
    tTime->tm_min = (lSec % 3600) / 60;
    tTime->tm_sec = (lSec % 3600) % 60;
}

static uint8_t cm_time_to_weekday(cm_tm_t *t)
{
    uint32_t u32WeekDay = 0;
    uint32_t u32Year = t->tm_year;
    uint8_t u8Month = t->tm_mon;
    uint8_t u8Day = t->tm_mday;
    if(u8Month < 3U)
    {
        /*D = { [(23 x month) / 9] + day + 4 + year + [(year - 1) / 4] - [(year - 1) / 100] + [(year - 1) / 400] } mod 7*/
        u32WeekDay = (((23U * u8Month) / 9U) + u8Day + 4U + u32Year + ((u32Year - 1U) / 4U) - ((u32Year - 1U) / 100U) + ((u32Year - 1U) / 400U)) % 7U;
    }
    else
    {
        /*D = { [(23 x month) / 9] + day + 4 + year + [year / 4] - [year / 100] + [year / 400] - 2 } mod 7*/
        u32WeekDay = (((23U * u8Month) / 9U) + u8Day + 4U + u32Year + (u32Year / 4U) - (u32Year / 100U) + (u32Year / 400U) - 2U ) % 7U;
    }

    if (0U == u32WeekDay)
    {
        u32WeekDay = 7U;
    }

    return (uint8_t)u32WeekDay;
}

static cm_cmd_t cmd_vector[] = {
    {"FWRITE", cm_test_write_file},     //CM:FWRITE
    {"FREAD", cm_test_read_file},       //CM:FREAD
    {"FMOVE", cm_test_move_file},       //CM:FMOVE
    {"FDELETE", cm_test_delete_file},   //CM:FDELETE
    {"FOTA", cm_test_fota},             //CM:FOTA:operation
    {"OTA", cm_test_ota},               //CM:OTA:operation
    {"ASOCKET", cm_test_asocket},       //CM:ASOCKET:operation
    {"NTP", cm_test_ntp},               //CM:NTP:operation
    {"SSL", cm_test_ssl},               //CM:SSL:operation
    {"MQTT", cm_test_mqtt},              //CM:MQTT:operation
    {"FTP", cm_test_ftp},               //CM:FTP
    {"HTTP", cm_test_http},             //CM:HTTP:operation
    {"I2C", cm_test_i2c},               //CM:I2C
    {"ADC", cm_test_adc},               //CM:ADC
    {"SPI", cm_test_spi},               //CM:SPI    
    {"GPIO_READ", cm_test_gpio_read},//CM:GPIO_READ:NUM:0xiomuxpin(16进制):0xiomuxfun(16进制)
    {"GPIO_WRITE", cm_test_gpio_write},//CM:GPIO_WRITE:NUM:0xiomuxpin(16进制):0xiomuxfun(16进制):value
    {"GPIO_IRQ", cm_test_gpio_irq},//CM:GPIO_IRQ:NUM:0xiomuxpin(16进制):0xiomuxfun(16进制)
    {"GPIO_IRQCOUNT", cm_test_acqure_irq_count},//CM:GPIO_IRQCOUNT
    {"GPIO_DEINIT", cm_test_gpio_deinit},//CM:GPIO_DEINIT:NUM
    {"PIN_CMD", cm_test_pin_cmd},//CM:PIN_CMD:pin:pincmd:pimcmdEUM  例如56脚GPIO 双边沿唤醒功能设置,GPIO唤醒之后如果不清除（失能边沿唤醒设置），将一直保持唤醒状态：CM:PIN_CMD:56:1:3          
    {"PM_REBOOT", cm_test_pm_reboot},//CM:PM_REBOOT
    {"PM_POWEROFF", cm_test_pm_poweroff},//CM:PM_POWEROFF
    {"PM_REASON", cm_test_pm_power_on_reason},//CM:PM_REASON
    {"PM_SLEEP", cm_test_sleep_or_wakeup_enable},//CM:PM_SLEEP
    {"PM_POWERKEY", cm_test_pm_powerkey},//CM:PM_POWERKEY:mode   mode=0:注册为空；mode非0:注册回调函数；
    {"KEYPAD", cm_test_keypad},//CM:KEYPAD:行数(5):行号0,行号1:行号2:行号3:行号4:列数(5):列号0:列号1:列号2:列号3:列号4;例如CM:KEYPAD:5:0:1:2:3:4:5:0:1:2:3:4      备注:5x5的键盘
    {"PWM_OPEN", cm_test_pwm_start},//CM:PWM_OPEN:dev:period:period_h
    {"PWM_CLOSE", cm_test_pwm_close},//CM:PWM_CLOSE:dev   
    {"LBS", cm_test_lbs_start},//CM:LBS
    {"LCD", cm_test_lcd},               //CM:LCD
    {"cJSON", cm_test_cJSON},           //CM:cJSON
    {"CLK_OPEN", cm_test_clk_open},           //CM:CLK_OPEN
    {"CLK_CLOSE", cm_test_clk_close},           //CM:CLK_CLOSE
    {"AUDIO_PLAY", cm_test_audio_play},     //CM:AUDIO_PLAY:operation:param
    {"WIFISCAN", cm_test_wifiscan},     //CM:WIFISCAN:operation
    {"LWIP", cm_test_lwip},             //CM:LWIP:operation
    {"VIRT_AT_SYNC", cm_virt_at_test_sync}, //CM:VIRT_AT_SYNC:AT+xxxx
    {"VIRT_AT_ASYNC", cm_virt_at_test_async},//CM:VIRT_AT_ASYNC:AT+xxxx
    {"MSG_TXT", cm_sendmsg_test_txt},   //CM:MSG_TXT
    {"MSG_PDU", cm_sendmsg_test_pdu},   //CM:MSG_PDU
    {"MODEM", cm_test_modem},           //CM:MODEM
    {"URC_REG", cm__virt_at_urc_cb_reg_test},//CM:URC_REG
    {"URC_DEREG", cm__virt_at_urc_cb_dereg_test},//CM:URC_DEREG
};

static void my_appimg_enter(char *param)
{
    cm_demo_uart();

    cm_uart_cmd_recv_t *pUartCmdRecv_t = &gstUartCmdRecv;
    char buf[CM_VER_LEN] = {0};
    cm_tm_t t;
    int i = 0;
    int ret;

    cm_fs_system_info_t info = {0, 0};
    cm_heap_stats_t stats = {0};
    cm_demo_printf("\n\n\n\n\n\n\n\n\n\n");
    cm_demo_printf("CM OpenCPU Starts\n");
    cm_sys_get_cm_ver(buf, CM_VER_LEN);
    cm_demo_printf("SDK VERSION:%s\n", buf);
    cm_fs_getinfo(&info);
    cm_mem_get_heap_stats(&stats);
    cm_demo_printf("fs total:%d,remain:%d\n", info.total_size, info.free_size);
    cm_demo_printf("heap total:%d,remain:%d\n",stats.total_size,stats.free);
    int pdp_time_out=0;
    cm_demo_printf("waiting for network...\n");
    while(1)
    {
        if(pdp_time_out>10)
        {
            cm_demo_printf("network timeout\n");
            break;
        }
        if(cm_modem_get_pdp_state(1) == 1)
        {
            cm_demo_printf("network ready\n");        
            break;
        }
        osDelay(200);   
        pdp_time_out++;
    }

    cm_sec_to_date((long)(cm_rtc_get_current_time() + cm_rtc_get_timezone() * 60 * 60), &t);
    cm_demo_printf("Now:%d-%d-%d:%d:%d:%d,%s\n", t.tm_year, t.tm_mon , t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec,weekday[cm_time_to_weekday(&t)-1]);

    memset(buf, 0, CM_VER_LEN);
    ret = cm_sys_get_sn(buf);
    if (ret == 0)
    {
        cm_demo_printf("SN:%s\n", buf);
    }
    else
    {
        cm_demo_printf("SN ERROR\n");
    }

    memset(buf, 0, CM_VER_LEN);
    ret = cm_sys_get_imei(buf);
    if (ret == 0)
    {
        cm_demo_printf("IMEI:%s\n", buf);
    }
    else
    {
        cm_demo_printf("IMEI ERROR\n");
    }

    memset(buf, 0, CM_VER_LEN);
    ret = cm_sim_get_imsi(buf);
    if (ret == 0)
    {
        cm_demo_printf("IMSI:%s\n", buf);
    }
    else
    {
        cm_demo_printf("IMSI ERROR\n");
    }

    while (1)
    {
        cm_demo_printf("\nplease input cmds:\n");
        
        pUartCmdRecv_t->cmd_execute = 0;
        osEventFlagsWait(cmd_task_flag, 0x00000001U, osFlagsWaitAny, osWaitForever);

        if ((pUartCmdRecv_t->len < 2))
        {
            cm_demo_printf("CMD NOT DEFINE\n");
        }
        
        else
        {
            for (i = 0; i < (sizeof(cmd_vector) / sizeof(cm_cmd_t)); i++)
            {
                if (NULL == pUartCmdRecv_t->buf[1])
                {
                    break;
                }
                
                if (strcmp((const char *)pUartCmdRecv_t->buf[1], cmd_vector[i].cmdstr) == 0)
                {
                    (*(cmd_vector[i].cmdfunc))(pUartCmdRecv_t->buf, pUartCmdRecv_t->len);
                    cm_demo_printf("OK\n");
                    break;
                }
            }
            
            if (i >= (sizeof(cmd_vector) / sizeof(cm_cmd_t)))
            {
                cm_demo_printf("CMD NOT DEFINE\n");
            }
        }
        
        for (i = 0; i < pUartCmdRecv_t->len; i++)
        {
            if (NULL != pUartCmdRecv_t->buf[i])
            {
                cm_free(pUartCmdRecv_t->buf[i]);
                pUartCmdRecv_t->buf[i] = NULL;
            }
        }
    }
}

int cm_opencpu_entry(char *param)
{
    cmd_task_flag = osEventFlagsNew(0);
    
    if (cmd_task_flag == 0)
    {
        cm_log_printf(0, "event flag init failed");
        return -1;
    }

    osThreadAttr_t app_task_attr = {0};
    app_task_attr.name  = "main_task";
    app_task_attr.stack_size = 4096 * 2;
    app_task_attr.priority = osPriorityNormal;

    OC_APP_TaskHandle = osThreadNew((osThreadFunc_t)my_appimg_enter, 0, &app_task_attr);

    return 0;
}

