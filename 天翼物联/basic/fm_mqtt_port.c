/************************************************************************************************
 * Copyright (C), Flymodem (Shenzhen) Technology Co., Ltd.
 ************************************************************************************************
 * File Name  : fm_mqtt_port.c
 * Abstract   : MQTT接口实现FM MQTT功能
 * Version    : 3.1.0
 * Author     : LTH
 * Date       : 2024-08-08
 * Modifies   : [2024-08-08]新增[Author:LTH](无)
************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "core_mqtt.h"
#include "transport_interface.h"

#include "fm_config_port.h"
#include "fm_mqtt_port.h"


static TransportInterface_t transport_interface;


#if 1
//MQTT client Sub
//#define MQTT_CLIENT_SUB_QOS		MQTTQoS1					//MQTT订阅QOS级别 1
//#define MQTT_CLIENT_SUB_TOPIC	"JSON/FM/CPE/868998061757114"	//MQTT订阅主题 "JSON/设备厂商/设备型号/IMEI"
#define MQTT_CLIENT_SUB_TOPIC_NUM	1							//MQTT订阅主题数量 1

//MQTT client Pub
//#define MQTT_CLIENT_PUB_QOS		MQTTQoS1					//MQTT上报QOS级别 1
//#define MQTT_CLIENT_PUB_TOPIC	"JSON/Heartbeat/FM/CPE"			//MQTT上报主题 "JSON/Heartbeat/设备厂商/设备型号"
//#define MQTT_CLIENT_PUB_TOPIC_NUM	1							//MQTT上报主题数据量 1
#endif


#if FM_SYSTEM == FM_SYSTEM_RTOS_ML307R
//MQTT Init
#define FM_MQTT_SERVER_PORT	"1883"								//MQTT连接端口
#define FM_MQTT_SERVER_ADDR	"broker.emqx.io"					//MQTT连接地址

//MQTT connect
#define FM_MQTT_CONNECT_USERNAME	"admin"						//MQTT代理账号
#define FM_MQTT_CONNECT_PASSWORD	"123456"					//MQTT代理密码

/* 模组使用LittleFS文件系统，请按照下文描述的文件打开方式使用。错误使用时文件系统无法保证文件功能正常 */
#define FM_FS_RB            (0)      /*!< rb，打开一个二进制文件，文件必须存在，只允许读 */
#define FM_FS_WB            (1)      /*!< wb，新建一个二进制文件，已存在的文件内容清空，只允许写 */
#define FM_FS_AB            (2)      /*!< ab，打开或新建一个二进制文件，只允许在文件末尾追写 */
#define FM_FS_WBPLUS        (3)      /*!< wb+，新建一个二进制文件，已存在的文件内容清空，允许读写 */
#define FM_FS_ABPLUS        (4)      /*!< ab+，打开或新建一个二进制文件，可读，只允许在文件末尾追写 */
#define FM_FS_RBPLUS        (5)      /*!< rb+，打开一个二进制文件，文件必须存在，允许读写 */

#define FM_FS_SEEK_SET      (0)      /*!< SEEK_SET，文件开头 */
#define FM_FS_SEEK_CUR      (1)      /*!< SEEK_CUR，当前位置 */
#define FM_FS_SEEK_END      (2)      /*!< SEEK_END，文件结尾 */

extern int fm_open(const char* pathname, int flags, mode_t mode);
extern ssize_t fm_read(int fd, void* buf, size_t count);
extern ssize_t fm_write(int fd, const void* buf, size_t count);
extern int fm_close(int fd);
extern off_t fm_lseek(int fd, off_t offset, int whence);




//函数功能：获取MQTT配置文件信息
//参数：mqtt_server - 指向MQTT配置文件数据指针，1：默认使用飞猫MQTT服务器，2：使用自己定制的MQTT服务器
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_get_mqtt_server(FM_MQTT_SERVER_t *mqtt_server) 
{
	FM_STATUS_t ret = 0;
	int32_t fd;
	char read_data[32] = {0};
    const char *file_name = "fm_mqtt_server.cfg";
    int flags = FM_FS_RB;

	fd = fm_open(file_name, flags, 0);
	if( fd >= 0 ) 
    {
		fm_read(fd, read_data, 32);
        fm_close(fd);

		if ( strncmp(read_data, "2", 1) == 0  ) 
		{
			*mqtt_server = FM_MQTT_SERVER_CUSTOM;
		} 
		else 
		{
			*mqtt_server = FM_MQTT_SERVER_DEFAULT;
		}

		ret = FM_SUCCESS;
	} 
	else 
	{
		// 默认开启
		*mqtt_server = FM_MQTT_SERVER_DEFAULT;
		ret = FM_ERROR;
	}

	return ret;
}

//函数功能：设置MQTT配置文件信息
//参数：mqtt_server - 指向MQTT配置文件数据指针，1：默认使用飞猫MQTT服务器，2：使用自己定制的MQTT服务器
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_set_mqtt_server(FM_MQTT_SERVER_t mqtt_server) 
{
	FM_STATUS_t ret = 0;
	int32_t fd;
    const char *file_name = "fm_mqtt_server.cfg";
    int flags = FM_FS_WB;

	fd =  fm_open(file_name, flags, 0);
	if( fd >= 0 ) 
	{
		if ( mqtt_server == FM_MQTT_SERVER_DEFAULT ) 
		{
			fm_write(fd, "1", strlen("1"));
		} 
		else if( mqtt_server == FM_MQTT_SERVER_CUSTOM ) 
		{
			fm_write(fd, "2", strlen("2"));
		} 
		else 
		{
			fm_write(fd, "0", strlen("0"));
		}
		fm_close(fd);

		ret = FM_SUCCESS;
	} 
	else 
	{
		ret = FM_ERROR;
	}

	return ret;
}
#endif



MQTTStatus_t fm_mqtt_receiveloop(MQTTContext_t *mqtt_context) 
{
	MQTTStatus_t ret;
    uint32_t timeoutMs = 0;

	ret = MQTT_ReceiveLoop(mqtt_context, timeoutMs);
	
	return ret;
}


MQTTStatus_t fm_mqtt_init(MQTTContext_t *mqtt_context, NetworkContext_t *network_context, MQTTGetCurrentTimeFunc_t getTimeStampMs, MQTTEventCallback_t eventCallback, MQTTFixedBuffer_t *fixed_buffer) 
{
	MQTTStatus_t ret;

	memset( ( void * )  mqtt_context, 0x00, sizeof( MQTTContext_t ) );

#if FM_SYSTEM == FM_SYSTEM_RTOS_ML307R
	FM_MQTT_SERVER_t mqtt_server = FM_MQTT_SERVER_DEFAULT;

	fm_get_mqtt_server(&mqtt_server);

	if ( mqtt_server == FM_MQTT_SERVER_DEFAULT ) 
	{
		network_context->server_addr = FM_MQTT_SERVER_ADDR;
		network_context->server_port = FM_MQTT_SERVER_PORT;
	}
#endif

	transport_interface.open = transport_open_func;
	transport_interface.close = transport_close_func;
	transport_interface.send = transport_send_func;
	transport_interface.recv = transport_recv_func;
	transport_interface.pNetworkContext = network_context;
	
	ret = MQTT_Init(mqtt_context, &transport_interface, getTimeStampMs, eventCallback, fixed_buffer);

	return ret;
}


MQTTStatus_t fm_mqtt_connect(MQTTContext_t *mqtt_context, NetworkContext_t *network_context, MQTTConnectInfo_t *connect_info) 
{
	MQTTStatus_t ret = MQTTSuccess;
	bool session_present;

#if FM_SYSTEM == FM_SYSTEM_RTOS_ML307R
	MQTTConnectInfo_t connectInfo = {0x0};
	FM_MQTT_SERVER_t mqtt_server = FM_MQTT_SERVER_DEFAULT;

	connectInfo.cleanSession = connect_info->cleanSession;
    connectInfo.pClientIdentifier = connect_info->pClientIdentifier;
    connectInfo.clientIdentifierLength = connect_info->clientIdentifierLength;
    connectInfo.keepAliveSeconds = connect_info->keepAliveSeconds;
    
	fm_get_mqtt_server(&mqtt_server);
	if ( mqtt_server == FM_MQTT_SERVER_DEFAULT ) 
	{
		connectInfo.pUserName = FM_MQTT_CONNECT_USERNAME;
		connectInfo.userNameLength = strlen(connectInfo.pUserName);
		connectInfo.pPassword = FM_MQTT_CONNECT_PASSWORD;
		connectInfo.passwordLength = strlen(connectInfo.pPassword);
	} 
	else 
	{
		connectInfo.pUserName = connect_info->pUserName;
		connectInfo.userNameLength = connect_info->userNameLength;
		connectInfo.pPassword = connect_info->pPassword;
		connectInfo.passwordLength = connect_info->passwordLength;
	}
#endif
      printf(" done 8.\n");
    if (network_context->socket_fd > 0) 
	{  printf(" done 9.\n");
        transport_interface.close(network_context);
    }
     printf(" done 10.\n");
    if (transport_interface.open(network_context) == 0) 
	{   printf(" done 11.\n");
        ret = MQTT_Connect(mqtt_context, connect_info, NULL, 10000, &session_present);
         printf(" done 12.\n");
        if (ret == MQTTSuccess) 
		{printf(" done 13.\n");
			// Since we requested a clean session, this must be false
			//ASSERT( session_present == false );
			assert( session_present == false );
        } 
		else 
		{  printf(" done 14.\n");
			transport_interface.close(network_context);
		}
    }printf(" done 15.\n");

	return ret;
}


MQTTStatus_t fm_mqtt_get_state(MQTTContext_t *mqtt_context) 
{
	MQTTStatus_t ret;

    ret = MQTT_Ping(mqtt_context);

	return ret;
}


MQTTStatus_t fm_mqtt_subscribe(MQTTContext_t *mqtt_context, MQTTSubscribeInfo_t *subscriptionInfo) 
{
	MQTTStatus_t ret;
	uint16_t packetId;

	packetId = MQTT_GetPacketId( mqtt_context );

	ret = MQTT_Subscribe(mqtt_context, subscriptionInfo, MQTT_CLIENT_SUB_TOPIC_NUM, packetId);

	return ret;
}


MQTTStatus_t fm_mqtt_publish(MQTTContext_t *mqtt_context, MQTTPublishInfo_t *publishInfo) 
{
	MQTTStatus_t ret;
	uint16_t packetId;

	packetId = MQTT_GetPacketId( mqtt_context );

	ret = MQTT_Publish( mqtt_context, publishInfo, packetId );

	return ret;
}


MQTTStatus_t fm_mqtt_unsubscribe(MQTTContext_t *mqtt_context, MQTTSubscribeInfo_t *unsubscribeInfo) 
{
	MQTTStatus_t ret;
	uint16_t packetId;

	packetId = MQTT_GetPacketId( mqtt_context );

	ret = MQTT_Unsubscribe( mqtt_context, unsubscribeInfo, MQTT_CLIENT_SUB_TOPIC_NUM, packetId );

	return ret;
}


MQTTStatus_t fm_mqtt_disconnect(MQTTContext_t *mqtt_context) 
{
	MQTTStatus_t ret;

	ret = MQTT_Disconnect( mqtt_context );

	return ret;
}


