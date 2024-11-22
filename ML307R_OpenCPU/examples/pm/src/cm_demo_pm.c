/*********************************************************
 *  @file    cm_demo_pm.c
 *  @brief   OpenCPU PM示例
 *  Copyright (c) 2021 China Mobile IOT.
 *  All rights reserved.
 *  created by zyf 2021/08/30
 ********************************************************/
#include "stdlib.h"
#include "cm_sys.h"
#include "cm_pm.h"
#include "cm_demo_pm.h"
#include "cm_os.h"
#include "cm_demo_uart.h"

void cm_test_pm_reboot(unsigned char **cmd,int len)
{
    if( (cmd == NULL) || (len !=2 ))
    {
        cm_demo_printf(" cmd = NULL or cmd len = %d error!\n", len);
        return;
    }
    
    cm_demo_printf("[REBOOT]rebooting...\n");
    cm_pm_reboot();
}

void cm_test_pm_poweroff(unsigned char **cmd,int len)
{
    if( (cmd == NULL) || (len !=2 ))
    {
        cm_demo_printf(" cmd = NULL or cmd len = %d error!\n", len);
        return;
    }
    
    cm_demo_printf("[POWEROFF]poweroff...\n");
    cm_pm_poweroff();
}

void cm_test_pm_power_on_reason(unsigned char **cmd,int len)
{
    
    if( (cmd == NULL) || (len !=2 ))
    {
        cm_demo_printf(" cmd = NULL or cmd len = %d error!\n", len);
        return;
    }
    cm_demo_printf("[PM]power on reason:%d\n", cm_pm_get_power_on_reason());
}

void cm_pm_enty_callback(void)
{
    ;//不可以执行打印等长时间占用任务
}

void cm_pm_exit_callback(void)
{
    ;//不可以执行打印等长时间占用任务
}
void cm_test_sleep_or_wakeup_enable(unsigned char **cmd,int len)
{
    uint8_t mode = 0;

    if( (cmd == NULL) || (len !=3 ))
    {
        cm_demo_printf(" cmd = NULL or cmd len = %d error!\n", len);
        return;
    }
    
    mode = atoi((char *)cmd[2]);
    
    if(mode == 1)
    {
        cm_pm_work_unlock();
    }
    else if(mode == 2)
    {
        cm_pm_work_lock();
    }
    else if(mode == 0)
    {
        cm_pm_cfg_t pm_cfg = {cm_pm_enty_callback,cm_pm_exit_callback};
        cm_pm_init(pm_cfg);
    }
    else
    {
        cm_demo_printf("[PM]dormant mode(%d) is error...\n",mode);
    }
}

void cm_pm_powerkey_callback(cm_powerkey_event_e event)
{
    if(CM_POWERKEY_EVENT_RELEASE == event)
    {
        cm_demo_printf("[PM]powerkey CM_POWERKEY_EVENT_RELEASE\n");
    }
    else
    {
        cm_demo_printf("[PM]powerkey CM_POWERKEY_EVENT_PRESS\n");
    }
}

void cm_test_pm_powerkey(unsigned char **cmd,int len)
{
    uint8_t mode = 0;
    
    if( (cmd == NULL) || (len !=3 ))
    {
        cm_demo_printf(" cmd = NULL or cmd len = %d error!\n", len);
        return;
    }
    
    mode = atoi((char *)cmd[2]);
    
    if(mode == 0)
    {
        cm_pm_powerkey_regist_callback(NULL);
    }
    else
    {
        cm_pm_powerkey_regist_callback(cm_pm_powerkey_callback);
    }
}



