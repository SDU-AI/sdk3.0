#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "custom_main.h"
#include <stdint.h>
#include "cm_adc.h"
#include "cm_os.h"
#include "cm_uart.h"
#include "cm_iomux.h"
#include "cm_os.h"
#include "cm_mem.h"
#include "cm_common.h"
#include "cm_sys.h"
#include "cm_fs.h"
#include "cm_rtc.h"
#include "cm_sim.h"
#include "cm_modem.h"
#include "cJSON.h"
#include "lwip/sockets.h"
#include "lwip/arch.h"
#include "cm_asocket.h"
#include "cm_eloop.h"
#include "cm_http.h"
#include "cm_ssl.h"
#include "cm_gpio.h"
#include "cm_mqtt.h"
#include "cm_ssl.h"
#include "cm_virt_at.h"
/*
#include "bsp_uart.h"
#include "bsp_network.h"
#include "time.h"
#include "bsp_sockettcp.h"
#include "bsp_socketudp.h"
#include "def.h"
#include "bsp_mqtt.h"
*/

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define CM_MQTT_CLIENT_MAX          5

/****************************************************************************
 * Private Types
 ****************************************************************************/

/** An enumeration of the PUBLISH flags. */
typedef enum
{
    CM_MQTT_PUBLISH_DUP = 8u,
    CM_MQTT_PUBLISH_QOS_0 = ((0u << 1) & 0x06),
    CM_MQTT_PUBLISH_QOS_1 = ((1u << 1) & 0x06),
    CM_MQTT_PUBLISH_QOS_2 = ((2u << 1) & 0x06),
    CM_MQTT_PUBLISH_QOS_MASK = ((3u << 1) & 0x06),
    CM_MQTT_PUBLISH_RETAIN = 0x01
} cm_mqtt_publish_flags_e;

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* 测试前请先补充如下参数 */
static char *cmmqtt_glob_subtopic = "$$oc/devices/6617524971d845632a04c3d3_stm32/sys/messages/down";                         //订阅的主题
static char *cmmqtt_glob_pubtopic = "$oc/devices/6617524971d845632a04c3d3_stm32/sys/properties/report";                         //发布的主题
static char *cmmqtt_glob_pubmessage_str = "hello word";                   //发布的消息
static char cmqtt_glob_cfgserver[1024] = "broker.emqx.io";                     //MQTT测试服务器地址
static int cmqtt_glob_cfgport = 1883;                           //MQTT测试端口号 1883
//static int cmqtt_glob_cfgport_ssl = 8883;                       //MQTTS测试端口号
static char cmqtt_glob_cfgclient_id[1024] = "mqttx_77d629af";                  //MQTT测试client_id
static int cmqtt_glob_cfgkeepAlive = 120;                       //保活时间
static char cmqtt_glob_cfguser[1024] = "6617524971d845632a04c3d3_stm32";                       //MQTT测试用户名
static char cmqtt_glob_cfgpasswd[1024] = "e3420b38df53c138d43d9752bdb42a53f2ef3e8b7bd8167a3d6671eeae08ff3d";                     //MQTT测试密码
static int cmqtt_glob_cfgclean = 1;                             //clean模式，是否清除会话，0为不清除，1为清除，若测试onenet平台则必须为1

/* 客户端示例，满足内存空间大小的条件下，最多支持5路连接 */
static cm_mqtt_client_t *_mqtt_client[CM_MQTT_CLIENT_MAX] = {0};

 cJSON *cjson_test = NULL;
 cJSON *cjson_sim_switch_to= NULL;
 cJSON *cjson_policy_operator_first = NULL;
 cJSON *cjson_policy_operator_first_prefer_operator = NULL;

 cJSON *cjson_policy_network_lost = NULL;
 cJSON *cjson_policy_network_lost_network_lost = NULL;
 char *message =	NULL;

 FM_Network_Operator_t Current_Operator = UNKNOWN_OPERATOR;
 FM_Network_Operator_t Net_Operator = UNKNOWN_OPERATOR;
/****************************************************************************
 * AT Functions
 ****************************************************************************/
FM_Network_Operator_t parse_network_operator(const char* name) {
    if (strcmp(name, "CHINA_MOBILE_OPERATOR") == 0) {
        cm_log_printf(0,"GET NET Operator is :[CHINA_MOBILE_OPERATOR]");
        return CHINA_MOBILE_OPERATOR;
    }
    if (strcmp(name, "CHINA_UNICOM_OPERATOR") == 0) {
        cm_log_printf(0,"GET NET Operator is :[CHINA_UNICOM_OPERATOR]");
        return CHINA_UNICOM_OPERATOR;
    }
    if (strcmp(name, "CHINA_TELECOM_OPERATOR") == 0) {
        cm_log_printf(0,"GET NET Operator is :[CHINA_TELECOM_OPERATOR]");
        return CHINA_TELECOM_OPERATOR;
    }

    return     UNKNOWN_OPERATOR;		/**< 无. */
 // 默认值
}

void at_response_cb(cm_virt_at_param_t *param) {
    if (param->event == CM_VIRT_AT_OK) {
        // 打印接收到的响应
        cm_log_printf(0,"AT response: rsp=%s rsp_len=%d\n", param->rsp, param->rsp_len);
    } else {
        // 打印错误信息
        cm_log_printf(0,"Error occurred while sending AT+CIMI\n");
    }
}

// 函数用于发送AT+CIMI指令并异步接收响应
void send_at(const char *operation) {
  
    if (cm_virt_at_send_async((const uint8_t *)operation, at_response_cb, "AT+CIMI") != 0) {
        // 如果发送失败，打印错误信息
        cm_log_printf(0,"Failed to send AT+CIMI command asynchronously\n");
    }
}
void att_response_cb(cm_virt_at_param_t *param) {
    if (param->event == CM_VIRT_AT_OK) {
        // 打印接收到的响应
        cm_log_printf(0,"AT response: rsp=%s rsp_len=%d\n", param->rsp, param->rsp_len);
        if (strncmp(param->rsp, "46008", 5) == 0) {
                Current_Operator = CHINA_MOBILE_OPERATOR;
                cm_log_printf(0,"Current_Operator = CHINA_MOBILE_OPERATOR");
            } else if (strncmp(param->rsp, "46006", 5) == 0) {
                Current_Operator = CHINA_UNICOM_OPERATOR;
                cm_log_printf(0,"Current_Operator = CHINA_UNICOM_OPERATOR");
            } else if (strncmp(param->rsp, "46011", 5) == 0) {
                Current_Operator = CHINA_TELECOM_OPERATOR;
                cm_log_printf(0,"Current_Operator = CHINA_TELECOM_OPERATOR");
            }
    } else {
        // 打印错误信息
        cm_log_printf(0,"Error occurred while sending AT+CIMI\n");
    }
    
}
void get_current_operator(const char *operation){
     
     
     if (cm_virt_at_send_async((const uint8_t *)operation, att_response_cb, "AT+CIMI") != 0) {
        // 如果发送失败，打印错误信息
        cm_log_printf(0,"Failed to send AT+CIMI command asynchronously\n");
    }
}

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/**
 *  \brief 连接状态回调
 *  
 *  \param [in] client mqtt客户端
 *  \param [in] session session标志
 *  \param [in] conn_res 连接状态
 *  \return 成功返回0，失败返回-1
 *  
 *  \details More details
 */
static int __mqtt_manager_default_connack_cb(cm_mqtt_client_t* client, int session, cm_mqtt_conn_state_e conn_res)
{

    int i = 0;
    
    for (; i < CM_MQTT_CLIENT_MAX; i++)
    {
        if (_mqtt_client[i] == client)
        {
            break;
        }
    }
    
    cm_log_printf(0,"\r\n[MQTT]CM MQTT index[%d] , CONNECT: %d\r\n", i, conn_res);
           
    return 0;
}
char *get_publish_operator_str()
{
    cJSON *cjson_test=NULL;
    cjson_test = cJSON_CreateObject();
    get_current_operator(at_command);

    const char *operator_to_string(FM_Network_Operator_t operator){
    switch (operator) {
        case CHINA_UNICOM_OPERATOR: return "CHINA_UNICOM_OPERATOR";
        case CHINA_MOBILE_OPERATOR: return "CHINA_MOBILE_OPERATOR";
        case CHINA_TELECOM_OPERATOR: return "CHINA_TELECOM_OPERATOR";
        default: return "UNKNOWN_OPERATOR";
    }
}

    cJSON_AddItemToObject(cjson_test, "current_operator", cJSON_CreateString(operator_to_string(Current_Operator)));

   return json_string;
}

/**
 *  \brief server->client发布消息回调
 *  
 *  \param [in] client mqtt客户端
 *  \param [in] msgid 消息ID
 *  \param [in] topic 主题
 *  \param [in] payload 负载
 *  \param [in] payload_len 负载长度
 *  \param [in] total_len 负载总长度
 *  \return 成功返回0，失败返回-1
 *  
 *  \details 
 */
static int __mqtt_manager_default_publish_cb(cm_mqtt_client_t* client, unsigned short msgid, char* topic, int total_len, int payload_len, char* payload)
{
    int i = 0;
    
    for (; i < CM_MQTT_CLIENT_MAX; i++)
    {
        if (_mqtt_client[i] == client)
        {
            break;
        }
    }

      cm_log_printf(0,"\r\n[MQTT]CM MQTT index[%d] , recv: %d,%s,%d,%d\r\n", i, msgid, topic, total_len, payload_len);

    /* 由于测试示例限制打印长度 */
    int printf_size = payload_len > 600 ? 600 : payload_len;
    cm_log_printf(0,"\r\n[MQTT]CM MQTT index[%d] , recv: %.*s\r\n", i, printf_size, payload);
    
   

    message=payload;

     cjson_test=cJSON_Parse(message);
     if(cjson_test == NULL)
	{
		printf("parse fail.\n");
		return -1;
	}
    cjson_sim_switch_to = cJSON_GetObjectItem(cjson_test, "sim_switch_to");
    cjson_policy_operator_first = cJSON_GetObjectItem(cjson_test, "policy_operator_first");
	cjson_policy_operator_first_prefer_operator = cJSON_GetObjectItem(cjson_policy_operator_first, "prefer_operator");
   
#if 1               //订阅的主题
    if(memcmp(topic, cmmqtt_glob_subtopic,sizeof(cmmqtt_glob_subtopic)) == 0)
    {
        cm_mqtt_client_publish(_mqtt_client[0], cmmqtt_glob_pubtopic, payload, strlen(payload), CM_MQTT_PUBLISH_QOS_1);
    }

#endif
    
    
    //------------------------------//切网----------
 
    cm_log_printf(0,"parse SIM Switch success!\n");
    const char *operator_value_net_str = cjson_sim_switch_to->valuestring;
    char *at_command="at+cimi\r\n";
    get_current_operator(at_command);
    
    Net_Operator = parse_network_operator(cjson_sim_switch_to->valuestring);
    
    osDelay(1200);
   
    if(Current_Operator!=Net_Operator){
        
        cm_log_printf(0,"Operator is different\n");
        if (strcmp(operator_value_net_str, "CHINA_UNICOM_OPERATOR") == 0) {
                const char *at_command = "AT+CLCK=\"SC\",1,\"0101\"\r\n";
                char *json_string = get_publish_operator_str();
                send_at(at_command);
                osDelay(600);
                cm_log_printf(0,"send switch-unicom command success\n");
                osDelay(4000);
                cm_mqtt_client_publish(_mqtt_client[0], cmmqtt_glob_pubtopic, payload, strlen(payload), CM_MQTT_PUBLISH_QOS_1);

            } else if (strcmp(operator_value_net_str, "CHINA_MOBILE_OPERATOR") == 0) {
                const char *at_command = "AT+CLCK=\"SC\",1,\"0201\"\r\n";
                send_at(at_command);
                osDelay(600);
                cm_log_printf(0,"send switch-mobile success\n");
                osDelay(4000);
                cm_mqtt_client_publish(_mqtt_client[0], cmmqtt_glob_pubtopic, payload, strlen(payload), CM_MQTT_PUBLISH_QOS_1);
            } else if (strcmp(operator_value_net_str, "CHINA_TELECOM_OPERATOR") == 0) {
                const char *at_command = "AT+CLCK=\"SC\",1,\"0301\"\r\n";
                send_at(at_command);
                osDelay(600);
                cm_log_printf(0,"send switch-telecom success\n");
            } else {
                            // 处理未知的运营商字符串
                
            }
    }else{
                cm_log_printf(0,"operator is same no need switch\n");
    }        

    //------------------------------//三网优先----------
    const char *operator_value_prefer_net_str = cjson_policy_operator_first_prefer_operator->valuestring;  // 假设您可以直接获取 cJSON 对象的整数值
    cm_log_printf(0,"parse operator-first success!\n");

    if (strcmp(operator_value_prefer_net_str, "CHINA_UNICOM_OPERATOR") == 0) {
		const char *at_command = "AT+CSIM=10,\"80F2EE9100\"\r\n";
        send_at(at_command);
        osDelay(600);
        cm_log_printf(0,"send UNICOM FIRST success\n");
	} else if (strcmp(operator_value_prefer_net_str, "CHINA_MOBILE_OPERATOR") == 0) {
		const char *at_command = "AT+CSIM=10,\"80F2EE9200\"\r\n";
        send_at(at_command);
        osDelay(600);
        cm_log_printf(0,"send MOBILE FIRST success\n");
	} else if (strcmp(operator_value_prefer_net_str, "CHINA_TELECOM_OPERATOR") == 0) {
		const char *at_command = "AT+CSIM=10,\"80F2EE9300\"\r\n";
        send_at(at_command);
        osDelay(600);
        cm_log_printf(0,"send TELECOM FIRST success\n");
	} else if (strcmp(operator_value_prefer_net_str, "LAST_TIME") == 0){
		const char *at_command = "AT+CSIM=10,\"80F2EE9400\"\r\n";
        send_at(at_command);
        osDelay(600);
        cm_log_printf(0,"send LAST_TIME success\n");	
	}
    

}
/**
 *  \brief client->server发布消息ack回调
 *  
 *  \param [in] client mqtt客户端
 *  \param [in] msgid 消息ID
 *  \param [in] dup dup标志
 *  \return 成功返回0，失败返回-1
 *  
 *  \details More details
 */
static int __mqtt_manager_default_puback_cb(cm_mqtt_client_t* client, unsigned short msgid, char dup)
{
    int i = 0;
    
    for (; i < CM_MQTT_CLIENT_MAX; i++)
    {
        if (_mqtt_client[i] == client)
        {
            break;
        }
    }
    
      cm_log_printf(0,"\r\n[MQTT]CM MQTT index[%d] , pub_ack: %d,%d\r\n", i, msgid, dup);
    return 0;
}

/**
 *  \brief client->server发布消息recv回调
 *  
 *  \param [in] client mqtt客户端
 *  \param [in] msgid 消息ID
 *  \param [in] dup dup标志
 *  \return 成功返回0，失败返回-1
 *  
 *  \details More details
 */
static int __mqtt_manager_default_pubrec_cb(cm_mqtt_client_t* client, unsigned short msgid, char dup)
{
    int i = 0;
    
    for (; i < CM_MQTT_CLIENT_MAX; i++)
    {
        if (_mqtt_client[i] == client)
        {
            break;
        }
    }
    
      cm_log_printf(0,"\r\n[MQTT]CM MQTT index[%d] , pub_rec: %d,%d\r\n", i, msgid, dup);
    return 0;
}

/**
 *  \brief client->server发布消息comp回调
 *  
 *  \param [in] client mqtt客户端
 *  \param [in] msgid 消息ID
 *  \param [in] dup dup标志
 *  \return 成功返回0，失败返回-1
 *  
 *  \details More details
 */
static int __mqtt_manager_default_pubcomp_cb(cm_mqtt_client_t* client, unsigned short msgid, char dup)
{
    int i = 0;
    
    for (; i < CM_MQTT_CLIENT_MAX; i++)
    {
        if (_mqtt_client[i] == client)
        {
            break;
        }
    }
    
      cm_log_printf(0,"\r\n[MQTT]CM MQTT index[%d] , pub_comp: %d,%d\r\n", i, msgid, dup);
    return 0;
}

/**
 *  \brief 订阅ack回调
 *  
 *  \param [in] client mqtt客户端
 *  \param [in] msgid 消息ID
 *  \return 成功返回0，失败返回-1
 *  
 *  \details More details
 */
static int __mqtt_manager_default_suback_cb(cm_mqtt_client_t* client, unsigned short msgid, int count, int qos[])
{
    int i = 0;
    
    for (; i < CM_MQTT_CLIENT_MAX; i++)
    {
        if (_mqtt_client[i] == client)
        {
            break;
        }
    }
    
      cm_log_printf(0,"\r\n[MQTT]CM MQTT index[%d] , sub_ack: %d\r\n", i, msgid);
    return 0;
}

/**
 *  \brief 取消订阅ack回调
 *  
 *  \param [in] client mqtt客户端
 *  \param [in] msgid 消息ID
 *  \return 成功返回0，失败返回-1
 *  
 *  \details More details
 */
static int __mqtt_manager_default_unsuback_cb(cm_mqtt_client_t* client, unsigned short msgid)
{
    int i = 0;
    
    for (; i < CM_MQTT_CLIENT_MAX; i++)
    {
        if (_mqtt_client[i] == client)
        {
            break;
        }
    }
    
      cm_log_printf(0,"\r\n[MQTT]CM MQTT index[%d] , unsub_ack: %d\r\n", i, msgid);
    return 0;
}

/**
 *  \brief ping回调
 *  
 *  \param [in] client mqtt客户端
 *  \param [in] ret 消息状态，0：ping成功，1：ping超时
 *  \return 成功返回0，失败返回-1
 *  
 *  \details More details
 */
static int __mqtt_manager_default_pingresp_cb(cm_mqtt_client_t* client, int ret)
{
    int i = 0;
    
    for (; i < CM_MQTT_CLIENT_MAX; i++)
    {
        if (_mqtt_client[i] == client)
        {
            break;
        }
    }
    
      cm_log_printf(0,"\r\n[MQTT]CM MQTT index[%d] , pingrsp: %d\r\n", i, ret);
    return 0;
}

/**
 *  \brief 消息超时回调，包括publish/subscribe/unsubscribe等
 *  
 *  \param [in] client mqtt客户端
 *  \param [in] msgid 消息ID
 *  \return 成功返回0，失败返回-1
 *  
 *  \details More details
 */
static int __mqtt_manager_default_timeout_cb(cm_mqtt_client_t* client, unsigned short msgid)
{
    int i = 0;
    
    for (; i < CM_MQTT_CLIENT_MAX; i++)
    {
        if (_mqtt_client[i] == client)
        {
            break;
        }
    }
    
      cm_log_printf(0,"\r\n[MQTT]CM MQTT index[%d] , timeout: %d\r\n", i, msgid);
    return 0;
}



static int __mqtt_client_init(void)
{
    if(_mqtt_client[0])
    {
          cm_log_printf(0,"\r\n[MQTT]CM MQTT CLIENT IS RUN!!!\r\n");
        return -1;
    }
    
    _mqtt_client[0] = cm_mqtt_client_create();  //client初始化，最大支持五个实例
    
    if (NULL == _mqtt_client[0])
    {
          cm_log_printf(0,"\r\n[MQTT]CM MQTT CREATE CLIENT ERROR!!!\r\n");
        return -1;
    }
    
    /* 设置回调函数，连接、订阅、发布等接口均为异步接口，结果请根据回调函数返回进行判断 */
    cm_mqtt_client_cb_t callback = {0};
    callback.connack_cb = __mqtt_manager_default_connack_cb;
    callback.publish_cb = __mqtt_manager_default_publish_cb;
    callback.puback_cb = __mqtt_manager_default_puback_cb;
    callback.pubrec_cb = __mqtt_manager_default_pubrec_cb;
    callback.pubcomp_cb = __mqtt_manager_default_pubcomp_cb;
    callback.suback_cb = __mqtt_manager_default_suback_cb;
    callback.unsuback_cb = __mqtt_manager_default_unsuback_cb;
    callback.pingresp_cb = __mqtt_manager_default_pingresp_cb;
    callback.timeout_cb = __mqtt_manager_default_timeout_cb;

    /* 设置client参数 */
    int version = 4;//版本3.1.1
    int pkt_timeout = 10;//发送超时10秒
    int reconn_times = 3;//重连三次
    int reconn_cycle = 20;//重连间隔20秒
    int reconn_mode = 0;//以固定间隔尝试重连
    int retry_times = 3;//重传三次
    int ping_cycle = 60;//ping周期60秒
    int dns_priority = 2;//MQTT dns解析ipv6优先

    cm_mqtt_client_set_opt(_mqtt_client[0], CM_MQTT_OPT_EVENT, (void*)&callback);
    cm_mqtt_client_set_opt(_mqtt_client[0], CM_MQTT_OPT_VERSION, (void*)&version);
    cm_mqtt_client_set_opt(_mqtt_client[0], CM_MQTT_OPT_PKT_TIMEOUT, (void*)&pkt_timeout);
    cm_mqtt_client_set_opt(_mqtt_client[0], CM_MQTT_OPT_RETRY_TIMES, (void*)&retry_times);
    cm_mqtt_client_set_opt(_mqtt_client[0], CM_MQTT_OPT_RECONN_MODE, (void*)&reconn_mode);
    cm_mqtt_client_set_opt(_mqtt_client[0], CM_MQTT_OPT_RECONN_TIMES, (void*)&reconn_times);
    cm_mqtt_client_set_opt(_mqtt_client[0], CM_MQTT_OPT_RECONN_CYCLE, (void*)&reconn_cycle);
    cm_mqtt_client_set_opt(_mqtt_client[0], CM_MQTT_OPT_PING_CYCLE, (void*)&ping_cycle);
    cm_mqtt_client_set_opt(_mqtt_client[0], CM_MQTT_OPT_DNS_PRIORITY, (void*)&dns_priority);
    return 0;
}

void test_printf(void) 
{
    osDelay(2200);
    __mqtt_client_init();
    /* 配置连接参数，对于字符串参数，内部仅保留指针，不分配空间 */
    cm_mqtt_connect_options_t conn_options = {
        .hostport = (unsigned short)cmqtt_glob_cfgport,
        .hostname = (const char*)cmqtt_glob_cfgserver,
        .clientid = (const char*)cmqtt_glob_cfgclient_id,
        .username = (const char*)cmqtt_glob_cfguser,
        .password = (const char*)cmqtt_glob_cfgpasswd,
        .keepalive = (unsigned short)cmqtt_glob_cfgkeepAlive,
        .will_topic = NULL,
        .will_message = NULL,
        .will_message_len = 0,
        .will_flag = 0, //若要使用遗嘱机制请置1，并补充相关遗嘱信息
        .clean_session = (char)cmqtt_glob_cfgclean,
        };
     osDelay(2200);
    
    for(int i=1;i<3;i++)
    {
    int ret = cm_mqtt_client_connect(_mqtt_client[0], &conn_options);//连接   
      cm_log_printf(0,"\r\ncm_mqtt_client_connect !!!, ret = %d\r\n", ret);
    if (0 == ret)
    {
          cm_log_printf(0,"\r\ncm_mqtt_client_connect SUCCESS!!!\r\n");
    }
    osDelay(300);
    }
    osDelay(1200);
    char *topic_tmp[1] = {0};
    topic_tmp[0] = cmmqtt_glob_subtopic;
    char qos_tmp[1] = {0};
    qos_tmp[0] = 0;
    int ret = cm_mqtt_client_subscribe(_mqtt_client[0], (const char**)topic_tmp, qos_tmp, 1);
    
    if (0 > ret)
    {
          cm_log_printf(0,"\r\n[MQTT]CM MQTT subscribe ERROR!!!, ret = %d\r\n", ret);
    }else {
          cm_log_printf(0,"\r\n[MQTT]CM MQTT subscribe success!!!, ret = %d\r\n", ret);
    }

    osDelay(1200);
    uint8_t publish_flags = 0;
    publish_flags |= CM_MQTT_PUBLISH_QOS_1;
    char *pubmessage = cmmqtt_glob_pubmessage_str;
    ret = cm_mqtt_client_publish(_mqtt_client[0], cmmqtt_glob_pubtopic, pubmessage, strlen(pubmessage), publish_flags);
    if (0 >= ret)
    {
          cm_log_printf(0,"\r\n[MQTT]CM MQTT publish ERROR!!!, ret = %d\r\n", ret);
    }else {
          cm_log_printf(0,"\r\n[MQTT]CM MQTT publish success!!!, ret = %d\r\n", ret);
    }
}








