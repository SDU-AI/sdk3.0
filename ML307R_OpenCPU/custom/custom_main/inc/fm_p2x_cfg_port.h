/************************************************************************************************
 * Copyright (C), Flymodem (Shenzhen) Technology Co., Ltd.
 ************************************************************************************************
 * File Name  : fm_p2x.h
 * Abstract   : p2x接口实现FM p2x配置文件功能
 * Version    : 3.1.0
 * Author     : LTH
 * Date       : 2024-07-24
 * Modifies   : [2024-07-24]新增[Author:LTH](无)
************************************************************************************************/
#ifndef	FM_P2X_PORT_H
#define FM_P2X_PORT_H
#include "fm_config_port.h"

typedef enum FM_NETWORK_OPERATOR 
{
    UNKNOWN_OPERATOR = 0,		/**< 无. */
    CHINA_UNICOM_OPERATOR,		/**< 中国联通运营商. */
    CHINA_MOBILE_OPERATOR,		/**< 中国移动运营商. */
    CHINA_TELECOM_OPERATOR,		/**< 中国电信运营商. */
    CHINA_BROADNET_OPERATOR,    /**< 中国广电运营商. */
    LAST_TIME_OPERATOR,         /**< 上次注网运营商. */
} FM_NETWORK_OPERATOR_t;

typedef enum FM_POLICY_STATUS 
{
    FM_UNKNOWN_POLICY = 0,		/**< 无. */
    FM_ENABLE_POLICY,			/**< 开启策略. */
    FM_DISABLE_POLICY,			/**< 关闭策略. */
} FM_POLICY_STATUS_t;

typedef struct FM_MQTT_CONTEXT{
    char *connect_clientid;		//mqtt连接时的客户端ID
    char *pub_topic;			//mqtt上报主题
    char *sub_topic;			//mqtt订阅主题
} FM_MQTT_CONTEXT_t;

//函数功能：获取优先网络策略使用的运营商
//参数：operator - 运营商
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_get_prefer_operator_policy(FM_NETWORK_OPERATOR_t *operator);

//函数功能：设置优先网络策略使用的运营商
//参数：operator - 运营商
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_set_prefer_operator_policy(FM_NETWORK_OPERATOR_t operator);



//函数功能：获取上次注网使用的运营商
//参数：operator - 运营商
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_get_last_operator_policy(FM_NETWORK_OPERATOR_t *operator);

//函数功能：保存当前注网成功的运营商
//参数：NULL
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_set_last_operator_policy(void);



//函数功能：获取断网切换网络策略状态
//参数：policy_status - 策略状态
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_get_lost_network_policy_status(FM_POLICY_STATUS_t *policy_status);

//函数功能：设置断网切换网络策略状态
//参数：policy_status - 策略状态
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_set_lost_network_policy_status(FM_POLICY_STATUS_t policy_status);


#endif



