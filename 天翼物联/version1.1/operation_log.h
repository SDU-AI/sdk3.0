/**
 ******************************************************************************
 * @file operation_log.h
 * @author  zailushang
 * @brief
 * @version V1.0.0
 * @date    2024/4/22
 * 
 * @copyright Copyright (c) Flymodem (Shenzhen) Technology Co., Ltd. All Rights Reserved.
 ******************************************************************************
 */

#ifndef _OPERATION_LOG_H_
#define _OPERATION_LOG_H_

#if 1
#define MAX_LOG_TMP		200

extern int operation_log_initialize(void);
extern void operation_log_push_log(char *operator, char *operate_type, char *operate_result, char *operate_detail);

#define OPERATION_LOG_FLAG 1

#define OPERATOR_LEN 64

typedef struct
{
  char operator_name[OPERATOR_LEN];
  char operate[OPERATOR_LEN];
  char operate_type[OPERATOR_LEN];
  char operate_result[OPERATOR_LEN];
  char operate_detail[OPERATOR_LEN];
  char operate_time[OPERATOR_LEN];
  char operate_imei[OPERATOR_LEN];
  char operate_iccid[OPERATOR_LEN];
}OperationLog;
#endif

#endif
