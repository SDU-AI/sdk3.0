/************************************************************************************************
 * Copyright (C), Flymodem (Shenzhen) Technology Co., Ltd.
 ************************************************************************************************
 * File Name  : fm_config_port.c
 * Abstract   : config接口实现FM 公共配置参数声明
 * Version    : 3.1.0
 * Author     : LTH
 * Date       : 2024-07-24
 * Modifies   : [2024-07-24]新增[Author:LTH](无)
************************************************************************************************/

#ifndef _FM_CONFIG_PORT_H_
#define _FM_CONFIG_PORT_H_
typedef enum FM_STATUS 
{
	FM_SUCCESS = 0,                 /**< 返回成功. */
	FM_PARAM_NULL,                  /**< 至少有一个参数为空. */
    FM_PARAM_ERROR,                 /**< 传入的参数错误. */
    FM_AT_EXECUTE_TIME_OUT,         /**< AT命令执行超时. */
    FM_AT_EXECUTE_FAIL,             /**< AT命令执行失败. */
    FM_AT_RET_CME_ERROR,            /**< AT返回 +CME ERROR 错误. */
    FM_AT_RET_CMS_ERROR,            /**< AT返回 +CMS ERROR 错误. */
    FM_AT_RET_ERROR,                /**< AT返回 ERROR 错误. */
    FM_AT_RET_OTHER_ERROR,          /**< AT返回其他错误. */
    FM_STRING_PARSING_FAIL,         /**< 字符串解析失败. */
    FM_SYS_ERROR,                   /**< 系统错误. */
    FM_GET_CONTENT_FAILED,          /**< 获取的内容错误. */
    FM_RET_ERROR,                   /**< 返回值错误. */
    FM_LOAD_AT_UTILS_ERROR,         /**< 加载AT常用工具错误. */
    FM_ERROR,                       /**< 返回失败状态. */
    FM_UNKNOWN                      /**< 无. */
} FM_STATUS_t;


#define FM_P2X_SDK_VERSION "3.1"
#define FM_P2X_LIB_VERSION "3.0"

#define FM_SYSTEM_RTOS_1803S 0
#define FM_SYSTEM_LINUX_M24A 1
#define FM_SYSTEM_RTOS_ML307R 2

#define FM_SYSTEM FM_SYSTEM_RTOS_ML307R



#endif



