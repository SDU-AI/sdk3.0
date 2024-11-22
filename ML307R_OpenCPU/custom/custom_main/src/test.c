
#include "custom_main.h"

#include "cm_iomux.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "cm_os.h"
#include "cm_mem.h"
#include "cm_common.h"
#include "cm_sys.h"
#include "cm_demo_uart.h"
#include "cm_demo_virt_at.h"
#include "cm_virt_at.h"
#include "cm_fs.h"
#include "cm_mqtt.h"

#include "custom_main.h"
#include <stdint.h>
#include "cm_adc.h"
#include "cm_os.h"
#include "cm_uart.h"
#include "cm_iomux.h"
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

#define FILE_NAME "example.txt"

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
static char *cmmqtt_glob_subtopic = "$oc/devices/6617524971d845632a04c3d3_stm32/sys/messages/down";                         //订阅的主题
static char *cmmqtt_glob_pubtopic = "$oc/devices/6617524971d845632a04c3d3_stm32/sys/properties/report";                         //发布的主题
static char *cmmqtt_glob_pubmessage_str = "helloword";                   //发布的消息
static char cmqtt_glob_cfgserver[129] = "broker.emqx.io";                     //MQTT测试服务器地址
static int cmqtt_glob_cfgport = 1883;                           //MQTT测试端口号 1883
//static int cmqtt_glob_cfgport_ssl = 8883;                       //MQTTS测试端口号
static char cmqtt_glob_cfgclient_id[129] = "mqttx_099b6eda";                  //MQTT测试client_id
static int cmqtt_glob_cfgkeepAlive = 120;                       //保活时间
static char cmqtt_glob_cfguser[129] = "6617524971d845632a04c3d3_stm32";                       //MQTT测试用户名
static char cmqtt_glob_cfgpasswd[129] = "e3420b38df53c138d43d9752bdb42a53f2ef3e8b7bd8167a3d6671eeae08ff3d";                     //MQTT测试密码
static int cmqtt_glob_cfgclean = 1;                             //clean模式，是否清除会话，0为不清除，1为清除，若测试onenet平台则必须为1

/* 客户端示例，满足内存空间大小的条件下，最多支持5路连接 */
static cm_mqtt_client_t *_mqtt_client[CM_MQTT_CLIENT_MAX] = {0};


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
    
                     //订阅的主题
    if(memcmp(topic, cmmqtt_glob_subtopic,sizeof(cmmqtt_glob_subtopic)) == 0)
    {
        cm_mqtt_client_publish(_mqtt_client[0], cmmqtt_glob_pubtopic, payload, strlen(payload), CM_MQTT_PUBLISH_QOS_1);
    }
    
    return 0;
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

/*
#include "flymodem.h"

#include <pthread.h> // 包含线程库头文件
*/
// 线程入口函数
// 回调函数，用于处理AT指令的响应

void at_response_cb(cm_virt_at_param_t *param) {
    if (param->event == CM_VIRT_AT_OK) {
        // 打印接收到的响应
        cm_log_printf(0,"AT+CIMI response: rsp=%s rsp_len=%d\n", param->rsp, param->rsp_len);
    } else {
        // 打印错误信息
        cm_log_printf(0,"Error occurred while sending AT+CIMI\n");
    }
}

// 函数用于发送AT+CIMI指令并异步接收响应
void send_at_cimi(const char *operation) {
  
    if (cm_virt_at_send_async((const uint8_t *)operation, at_response_cb, "AT+CIMI") != 0) {
        // 如果发送失败，打印错误信息
        cm_log_printf(0,"Failed to send AT+CIMI command asynchronously\n");
    }
}
#if 0
void creat_file(){

    int32_t fd; // 文件描述符
    int32_t ret; // 函数返回值
    const char *data = "HelloWorld!"; // 要写入的字符串数据
    size_t data_size = strlen(data); // 字符串的实际长度
    cm_log_printf(0, "log length is %zu\n", data_size);
    // 首先检查文件是否存在
    fd = cm_fs_open(FILE_NAME, CM_FS_RB); // 尝试以只读模式打开文件
    if (fd >= 0) {
        // 文件存在，关闭文件并进行下一步
        
        cm_log_printf(0, "File already exists. Proceeding to the next step.\n");
    } else {
        // 文件不存在，创建文件
        fd = cm_fs_open(FILE_NAME, CM_FS_WB); // 以写入模式打开文件，这将创建文件
        if (fd < 0) {
            cm_log_printf(0, "Failed to create file.\n");
            return -1;
        }

        // 写入数据
        ret = cm_fs_write(fd, data, data_size);
        if (ret < 0) {
            cm_log_printf(0, "Failed to write data to file.\n");
            cm_fs_close(fd); // 写入失败，关闭文件
            return -1;
        }else{
          cm_log_printf(0, "File created and data written successfully.\n");
        }
    }
        // 关闭文件
        ret = cm_fs_close(fd);
        if (ret != 0) {
            cm_log_printf(0, "Failed to close file.\n");
            return -1;
        }
        cm_log_printf(0, "file closed successfully\n");
        
    
    return 0;
}


void read_filedata(){
    
    int32_t fd; // 文件描述符
    int32_t ret; // 函数返回值
    char buffer[4096]; // 用于存储读取的数据
    uint32_t bytes_read; // 读取的字节数

    // 打开文件，使用CM_FS_RB模式，只允许读取
    fd = cm_fs_open(FILE_NAME, CM_FS_RB);
    if (fd < 0) {
        cm_log_printf(0, "Failed to open file for reading.\n");
        return;
    }

    // 读取数据
    bytes_read = cm_fs_read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read < 0) {
        cm_log_printf(0, "Failed to read data from file.\n");
        cm_fs_close(fd); // 读取失败，关闭文件
        return;
    }

    // 确保字符串以空字符结尾
    buffer[bytes_read] = '\0';

    // 打印读取的数据
    cm_log_printf(0, "Data read from file: %s\n", buffer);

    // 关闭文件
    ret = cm_fs_close(fd);
    if (ret != 0) {
        cm_log_printf(0, "Failed to close file after reading.\n");
        return;
    }

}
#endif
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
    int dns_priority = 0;//MQTT dns解析ipv6优先

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

static void my_sdk_enter()
{

 //const char *at_command = "AT+CIMI\r\n";
 //send_at_cimi(at_command);

 //creat_file();
 //#read_filedata();
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
    
    int ret = cm_mqtt_client_connect(_mqtt_client[0], &conn_options);//连接   
    
    if (0 == ret)
    {
        cm_log_printf(0,"\r\ncm_mqtt_client_connect SUCCESS!!!\r\n");
    }else{

        cm_log_printf(0,"\r\ncm_mqtt_client_connect wrong!!!\r\n");
    }
     osDelay(150);
    char *topic_tmp[1] = {0};
    topic_tmp[0] = cmmqtt_glob_subtopic;
    char qos_tmp[1] = {0};
    qos_tmp[0] = 0;

    ret = cm_mqtt_client_subscribe(_mqtt_client[0], (const char**)topic_tmp, qos_tmp, 1);
    
    if (0 > ret)
    {
        cm_log_printf(0,"\r\n[MQTT]CM MQTT subscribe ERROR!!!, ret = %d\r\n", ret);
    }else{
        cm_log_printf(0,"\r\n[MQTT]CM MQTT subscribe SUCCESS!!!, ret = %d\r\n", ret);
    }


    uint8_t publish_flags = 0;
    publish_flags |= CM_MQTT_PUBLISH_QOS_0;
    char *pubmessage = cmmqtt_glob_pubmessage_str;
    ret = cm_mqtt_client_publish(_mqtt_client[0], cmmqtt_glob_pubtopic, pubmessage, strlen(pubmessage), publish_flags);
    if (0 > ret)
    {
        cm_log_printf(0,"\r\n[MQTT]CM MQTT publish ERROR!!!, ret = %d\r\n", ret);
    }else{
        cm_log_printf(0,"\r\n[MQTT]CM MQTT publish SUCCESS!!!, ret = %d\r\n", ret);
    }
}
void test_printf(void) {
    
    //static osThreadId_t OC_APP_TaskHandle = NULL;
   
    
    // 打开文件，使用CM_FS_WB模式，新建文件并清空已有内容，只允许写入
    cm_demo_uart();
	cm_log_printf(0,"hello..1. 1.2..");
    osDelay(1000);
    osDelay(1000);
    // 发送AT+CIMI指令
    
	/*
    osThreadAttr_t app_task_attr = {0};
    app_task_attr.name  = "main_task";
    app_task_attr.stack_size = 4096 * 2;
    app_task_attr.priority = osPriorityNormal;
*/
    //OC_APP_TaskHandle = osThreadNew((osThreadFunc_t)my_sdk_enter, 0, &app_task_attr);
    my_sdk_enter();
    cm_log_printf(0,"hello... 1.2..");
    cm_log_printf(0, "hello... .2.3.");

    // 清理线程属性
    //pthread_attr_destroy(&attr);
}










