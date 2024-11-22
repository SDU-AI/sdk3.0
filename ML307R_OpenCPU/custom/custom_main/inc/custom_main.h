/*********************************************************
*  @file    cm_main.h
*  @brief   ML302 OpenCPU main header file
*  Copyright (c) 2019 China Mobile IOT.
*  All rights reserved.
*  created by XieGangLiang 2019/10/08
********************************************************/
#ifndef __CM_CUSTOM_MAIN_H__
#define __CM_CUSTOM_MAIN_H__
#include "cm_sys.h"

#define FM_LOG_INFO(fmt, ...) \
    do { \
        cm_log_printf(0, fmt, ##__VA_ARGS__); \
    } while (0)

#define FM_SLEEP(x) osDelay((200*x))

int cm_opencpu_entry(void *param);

#endif
