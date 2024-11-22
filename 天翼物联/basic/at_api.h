/**
 ******************************************************************************
 * @file at_api.h
 * @author  zailushang
 * @brief
 * @version V1.0.0
 * @date    2024/4/22
 * 
 * @copyright Copyright (c) Flymodem (Shenzhen) Technology Co., Ltd. All Rights Reserved.
 ******************************************************************************
 */

#ifndef _AT_API_H_
#define _AT_API_H_

#include "fm_at_command_port.h"

extern int get_imei_init(void);
extern char *get_module_imei(void);
extern FM_STATUS_t set_operator(char *operator);

#endif