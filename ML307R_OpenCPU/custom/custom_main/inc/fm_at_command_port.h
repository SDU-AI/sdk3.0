/************************************************************************************************
 * Copyright (C), Flymodem (Shenzhen) Technology Co., Ltd.
 ************************************************************************************************
 * File Name  : fm_at_command_port.c
 * Abstract   : at_command接口实现FM AT命令功能
 * Version    : 3.1.0
 * Author     : LTH
 * Date       : 2024-07-24
 * Modifies   : [2024-07-24]新增[Author:LTH](无)
************************************************************************************************/

#ifndef _FM_AT_COMMAND_PORT_H_
#define _FM_AT_COMMAND_PORT_H_
#include "fm_config_port.h"


//函数功能：获取sim卡状态
//参数：sim_state - 指向sim卡状态数据指针
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_get_sim_state(char *sim_state);

//函数功能：查询模组imei
//参数：imei - 指向模组imei数据指针
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_get_imei(char *imei);

//函数功能：查询sim卡iccid
//参数：iccid - 指向sim卡iccid数据指针
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_get_iccid(char *iccid);

//函数功能：查询模组信号强度
//参数：rsrp - 指向模组信号强度数据指针，单位:DBm
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_get_signal_strength(int *rsrp);

//函数功能：查询模组网络注册状态
//参数：operator - 指向运营商数据指针["CHINA TELECOM", "CHINA UNICOM", "CHINA MOBILE", "CHINA BROADNET", "UNKNOWN"]
//参数：regstate - 指向注册状态数据指针
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_get_operator_regstate(char *operator, int *regstate);

//函数功能：查询模组网络注册状态
//参数：regstate - 指向注册状态数据指针
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_get_regstate(int *regstate);

//函数功能：查询模组网络注册状态
//参数：operator - 指向运营商数据指针["CHINA TELECOM", "CHINA UNICOM", "CHINA MOBILE", "CHINA BROADNET", "UNKNOWN"]
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_get_operator(char *operator);

//函数功能：设置模组运营商
//参数：operator - 运营商->["CHINA TELECOM", "CHINA UNICOM", "CHINA MOBILE"]
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_set_operator(char *operator);


#if FM_SYSTEM == FM_SYSTEM_RTOS_ML307R
//函数功能：设置运营商优先策略
//参数：operator - 指向优先策略的运营商["CHINA TELECOM", "CHINA UNICOM", "CHINA MOBILE", "LAST TIME"]
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_set_prefer_policy(char *operator);
#endif


//函数功能：获取是否是分身卡
//参数：fly_card - 1：是飞猫分身卡；2：不是飞猫分身卡
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_get_fly_card(int *fly_card);

#if FM_SYSTEM == FM_SYSTEM_RTOS_ML307R 
//函数功能：获取设备厂商
//参数：vendor_name - 指向设备厂商数据指针
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_get_vendor_name(char *vendor_name);
#endif

#if  FM_SYSTEM == FM_SYSTEM_RTOS_1803S || FM_SYSTEM == FM_SYSTEM_RTOS_ML307R 
//函数功能：获取设备型号
//参数：model_name - 指向设备型号数据指针
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_get_model_name(char *model_name);
#endif


#endif

