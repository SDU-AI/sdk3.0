/************************************************************************************************
 * Copyright (C), Flymodem (Shenzhen) Technology Co., Ltd.
 ************************************************************************************************
 * File Name  : fm_mqtt_port.h
 * Abstract   : MQTT接口实现FM MQTT功能
 * Version    : 3.1.0
 * Author     : LTH
 * Date       : 2024-08-08
 * Modifies   : [2024-08-08]新增[Author:LTH](无)
************************************************************************************************/
#ifndef	FM_MQTT_PORT_H
#define FM_MQTT_PORT_H
#include "fm_config_port.h"

#if 0
    #include "fm_mqtt_port_public.h"
#endif

typedef enum MQTTStatus MQTTStatus_t;

typedef struct MQTTContext MQTTContext_t;

typedef struct NetworkContext NetworkContext_t;
typedef uint32_t (* MQTTGetCurrentTimeFunc_t )( void );
typedef void (* MQTTEventCallback_t )( struct MQTTContext * pContext,
                                       struct MQTTPacketInfo * pPacketInfo,
                                       struct MQTTDeserializedInfo * pDeserializedInfo );
typedef struct MQTTFixedBuffer MQTTFixedBuffer_t;

typedef struct MQTTConnectInfo MQTTConnectInfo_t;

typedef struct MQTTSubscribeInfo MQTTSubscribeInfo_t;

typedef struct MQTTPublishInfo MQTTPublishInfo_t;



#if FM_SYSTEM == FM_SYSTEM_RTOS_ML307R
typedef enum FM_MQTT_SERVER
{
    FM_MQTT_SERVER_UNKNOWN = 0,		/**< 无. */
    FM_MQTT_SERVER_DEFAULT,			/**< 使用飞猫MQTT服务器. */
    FM_MQTT_SERVER_CUSTOM,			/**< 使用自己定制的MQTT服务器. */
} FM_MQTT_SERVER_t;



//函数功能：获取MQTT服务器配置文件信息
//参数：mqtt_server - 指向MQTT配置文件数据指针，FM_DEFAULT：默认使用飞猫MQTT服务器，FM_CUSTOM：使用自己定制的MQTT服务器
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_get_mqtt_server(FM_MQTT_SERVER_t *mqtt_server);

//函数功能：设置MQTT服务器配置文件信息
//参数：mqtt_server - 指向MQTT配置文件数据指针，FM_DEFAULT：默认使用飞猫MQTT服务器，FM_CUSTOM：使用自己定制的MQTT服务器
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_set_mqtt_server(FM_MQTT_SERVER_t mqtt_server);
#endif

MQTTStatus_t fm_mqtt_receiveloop(MQTTContext_t *mqtt_context);
MQTTStatus_t fm_mqtt_init(MQTTContext_t *mqtt_context, NetworkContext_t *network_context, MQTTGetCurrentTimeFunc_t getTimeStampMs, MQTTEventCallback_t eventCallback, MQTTFixedBuffer_t *fixed_buffer);
MQTTStatus_t fm_mqtt_connect(MQTTContext_t *mqtt_context, NetworkContext_t *network_context, MQTTConnectInfo_t *connect_info);
MQTTStatus_t fm_mqtt_get_state(MQTTContext_t *mqtt_context);
MQTTStatus_t fm_mqtt_subscribe(MQTTContext_t *mqtt_context, MQTTSubscribeInfo_t *subscriptionInfo);
MQTTStatus_t fm_mqtt_publish(MQTTContext_t *mqtt_context, MQTTPublishInfo_t *publishInfo);
MQTTStatus_t fm_mqtt_unsubscribe(MQTTContext_t *mqtt_context, MQTTSubscribeInfo_t *unsubscribeInfo);
MQTTStatus_t fm_mqtt_disconnect(MQTTContext_t *mqtt_context);

#endif



