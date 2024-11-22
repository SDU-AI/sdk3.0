#include <stdio.h>
#include <sys/types.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/un.h>
#include <time.h>
#include "cJSON.h"
#include <netdb.h>
#include <stdint.h>
#include "fm_mqtt_port_new.h"
#include "fm_config_port.h"
#include "fm_mqtt_port.h"
#include "fm_mqtt_port_public.h"
#include "fm_at_command_port.h"
#include "fm_p2x_cfg_port.h"

#define MQTT_SERVER_PORT "1883"
#define MQTT_SERVER_ADDR "39.106.145.82"
#define MQTT_CLIENT_FIXED_BUF_LEN 2148

#define MQTT_TOPIC_LEN 256
#define MQTT_CONNECT_CLIENTID "mqttx_47842d0e"
#define MQTT_CONNECT_USERNAME "ctwing_sdk_mqtt"
#define MQTT_CONNECT_PASSWORD "EJd2xwy21Q28hN3"
#define MQTT_CONNECT_KEEP_ALIVE_SECONDS 60

#define HEARTBEAT_TOPIC "JSON/Heartbeat/CTIOT"
#define MODEL_TOPIC "JSON/CTIOT"

#define MODEM "5g-redcap"

#define MQTT_CLIENT_SUB_QOS MQTTQoS1
// #define MQTT_CLIENT_SUB_TOPIC "JSON/FM/CPE/868998061757114"
// #define MQTT_CLIENT_SUB_TOPIC_NUM 1

#define MQTT_CLIENT_PUB_QOS MQTTQoS1
// #define MQTT_CLIENT_PUB_TOPIC "JSON/Heartbeat/FM/CPE"
// #define MQTT_CLIENT_PUB_TOPIC_NUM 1

#define FALSE 0
#define TRUE !FALSE

FM_STATUS_t fm_status = FM_UNKNOWN;
FM_POLICY_STATUS_t policy_status = FM_UNKNOWN_POLICY;
pthread_t hThread;
pthread_t hThread1;

FM_NETWORK_OPERATOR_t prefer_operator = UNKNOWN_OPERATOR;
FM_MQTT_CONTEXT_t fm_mqtt_context = {0};
MQTTContext_t mqtt_ctx = {0x0};
NetworkContext_t net_ctx = {0x0};
MQTTPublishInfo_t pub_info = {0x0};

MQTTStatus_t status;
MQTTFixedBuffer_t msg_fixed_buffer = {0x0};
uint8_t msg_fixed_buffer_buf[MQTT_CLIENT_FIXED_BUF_LEN] = {0};
MQTTConnectInfo_t connect_info = {0x0};
MQTTSubscribeInfo_t sub_info = {0x0};
MQTTSubscribeInfo_t unsub_info = {0x0};

static int is_mqtt_connected = 0;
static int ope_index = 0;
static char *oper_list[] = {"CHINA UNICOM", "CHINA MOBILE", "CHINA TELECOM", "LAST TIME"};

int fm_mqtt_p2x_OSATaskCreate_ReceiveLoop(void);

void fm_resp_replace(char **resp)
{
    int i = 0, j = 0;
    char *tmp = NULL;

    if (resp == NULL || *resp == NULL)
        return;

    tmp = malloc(256);
    if (tmp != NULL)
    {
        memset(tmp, 0, 256);
        for (i = 0, j = 0; i < strlen(*resp);)
        {
            if ((*resp)[i] == ' ' || (*resp)[i] == '"' || (*resp)[i] == '\r' || (*resp)[i] == '\n')
            {
                i += 1;
                continue;
            }
            else
            {
                tmp[j] = (*resp)[i];
            }
            i++;
            j++;
        }
        free(*resp);
        *resp = tmp;
    }
}
/*
int check_network_link(void) {
    int sockfd;
    struct sockaddr_in server_addr;
    struct hostent *host;

    // 解析域名
    host = gethostbyname("www.baidu.com");
    if (host == NULL) {
        printf("Failed to resolve hostname!\n");
        return FALSE;
    }

    // 创建套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // 第三个参数是协议，通常设置为0，让系统选择默认协议
    if (sockfd < 0) {
        printf("Failed to create socket!\n");
        return FALSE;
    }

    printf("TCP socket file descriptor: [%d]\n", sockfd);

    // 初始化目标地址信息
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(80); // HTTP默认端口是80
    memcpy(&server_addr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);

    // 尝试连接到目标地址
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Failed to connect to server!\n");
        close(sockfd);
        return FALSE;
    }

    // 连接成功后，可以在这里执行其他操作，例如发送或接收数据

    // 关闭套接字
    close(sockfd);

    return TRUE;
}
*/

extern int CtAtCmdProcess(const char *cmd, unsigned int cmdLen, char *resp, unsigned int respBufLen);

static int CtGetCGATTStatus(void)
{
    char *cmd = "AT+CGATT?\r\n";
    char resp[64] = {0};
    int ret = 0;

    ret = CtAtCmdProcess(cmd, strlen(cmd), resp, 64);
    if (ret == 0)
    {
        if (strstr(resp, "+CGATT: 1"))
        {
            return 0;
        }
    }
    return -1;
}

int check_network_link()
{
    int sockfd;
    struct sockaddr_in server_addr;

    // struct hostent *host;
    struct in_addr ip_addr;

    if (CtGetCGATTStatus() != 0)
    {
        printf("get CGATT != 1 !!!\n");
        return FALSE;
    }

    inet_pton(AF_INET, "39.106.145.82", &ip_addr);

    /* // 解析域名
     host = gethostbyname("www.baidu.com");
     if (host == NULL) {
         printf("Failed to resolve hostname!\n");
         return FALSE;
     }
    */

    // 创建套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // 第三个参数是协议，通常设置为0，让系统选择默认协议
    if (sockfd < 0)
    {
        printf("Failed to create socket!\n");
        return FALSE;
    }

    printf("TCP socket file descriptor: [%d]\n", sockfd);

    // 初始化目标地址信息
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(1883); // HTTP默认端口是80
    memcpy(&server_addr.sin_addr.s_addr, &ip_addr, sizeof(ip_addr));
    // memcpy(&server_addr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);

    // 尝试连接到目标地址
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Failed to connect to server!\n");
        close(sockfd);
        return FALSE;
    }

    printf("socket ping sucess!\n");
    // 连接成功后，可以在这里执行其他操作，例如发送或接收数据

    // 关闭套接字
    close(sockfd);
    printf("cfd closed!\n");
    return TRUE;
}

int at_command_set_operator_next(int current_index)
{
    static int ope_index = 0;
    FM_STATUS_t state = FM_SUCCESS;

    ope_index = current_index;
    ope_index += 1;
    if (ope_index >= 3)
        ope_index = 0;

    state = fm_set_operator(oper_list[ope_index]);

    return state;
}
static void create_heart_beat_msg(char **heartbeat_msg)
{
    FM_STATUS_t ret = FM_SUCCESS;
    int i = 0;

    char iccid[32] = {0};
    char current_operator[32] = {0};
    int current_index = 0;
    char imei[32] = {0};

    cJSON *cjson_test = NULL;

    // 创建JSON的根对象，并将其命名为cjson_test
    cjson_test = cJSON_CreateObject();
    memset(imei, 0, 32);
    memset(iccid, 0, 32);

    fm_get_imei(imei);
    fm_get_iccid(iccid);

    fm_get_operator(current_operator);
    printf("current_operator is %s", current_operator);
    cJSON_AddItemToObject(cjson_test, "payload_name", cJSON_CreateString("Heartbeat"));
    cJSON_AddItemToObject(cjson_test, "msg_version", cJSON_CreateString("1"));
    cJSON_AddItemToObject(cjson_test, "vendor_name", cJSON_CreateString("CTIOT"));
    cJSON_AddItemToObject(cjson_test, "model_name", cJSON_CreateString("5g-redcap"));

    cJSON_AddItemToObject(cjson_test, "imei_data", cJSON_CreateString(imei));
    cJSON_AddItemToObject(cjson_test, "iccid_data", cJSON_CreateString(iccid));
    cJSON_AddItemToObject(cjson_test, "current_operator", cJSON_CreateString(current_operator));

    cJSON_AddItemToObject(cjson_test, "heart_beat_interval", cJSON_CreateNumber(3600));
    cJSON_AddItemToObject(cjson_test, "client_version_name", cJSON_CreateString("SDK 3.0"));

    *heartbeat_msg = cJSON_Print(cjson_test);
    cJSON_Delete(cjson_test);
    printf("[%s], json_string: [%s]", __func__, heartbeat_msg);
}
int mqtt_client_publish(char *topic, int payloadlen, const void *payload)
{
    MQTTStatus_t status;

    if (is_mqtt_connected != 1)
    {
        printf("mqtt client not connected");
        return FALSE;
    }

    pub_info.qos = MQTT_CLIENT_PUB_QOS;
    pub_info.pTopicName = topic;
    pub_info.topicNameLength = strlen(pub_info.pTopicName);
    pub_info.pPayload = payload;
    pub_info.payloadLength = strlen(pub_info.pPayload);

    printf("pub_info.pTopicName: [%s]", pub_info.pTopicName);
    printf("pub_info.topicNameLength: [%d]", pub_info.topicNameLength);
    printf("pub_info.pPayload: [%s]", pub_info.pPayload);
    printf("pub_info.payloadLength: [%d]", pub_info.payloadLength);

    status = fm_mqtt_publish(&mqtt_ctx, &pub_info);
    if (status != MQTTSuccess)
    {
        printf("mqtt publish error! erno:%d", status);
        return FALSE;
    }

    return TRUE;
}
void restart_receive_loop_thread(void)
{
    int status;

    // 取消当前线程
    status = pthread_cancel(hThread);
    if (status != 0)
    {
        printf("pthread_cancel failed");
        return;
    }

    // 等待线程退出
    status = pthread_join(hThread, NULL);
    if (status != 0)
    {
        printf("pthread_join failed");
        return;
    }

    // 重新创建线程
    status = fm_mqtt_p2x_OSATaskCreate_ReceiveLoop();
    if (status != 0)
    {
        printf("Failed to restart ReceiveLoop thread");
        return;
    }

    printf("ReceiveLoop thread restarted successfully");
}

int32_t fm_mqtt_reconnect()
{
    MQTTStatus_t status;
    int ret = 0;
    int network_lost_times = 0;
    int current_index = 0;
    // char sim_state[CONFIG_HEARTBEAT_LEN] = {0};

    char current_operator[32] = {0};
    MQTTConnectInfo_t reconnect_info = {0x0};
    MQTTSubscribeInfo_t resub_info = {
        .qos = MQTT_CLIENT_SUB_QOS,
        .pTopicFilter = NULL,
        .topicFilterLength = 0,
    };
    char *repub_msg_data = NULL;

/** 重连时，对SIM卡做自检 **/
#if 0
	while ( 1 )
	{
		//检查模组是否插卡
		fm_get_sim_state(sim_state);
		if(strncmp(sim_state, "READY", strlen(sim_state)))
		{
			printf("[%s] Uninserted Card!", __func__);
            sleep(5);
			continue;
		}
		printf("[%s] SIM Card OK", __func__);
		break;
	}
#endif

    /** 断网切断策略 **/
    // fm_socket_check_net 检查网络是否连接
    while (1)
    {
        ret = check_network_link();

        if (ret != TRUE)
        {
            printf("[%s],  UN NETWORK connect! erno:%d", __func__, ret);
            sleep(5);
            network_lost_times += 1;
            printf("[%s],  UN NETWORK connect! network_lost_times: %d", __func__, network_lost_times);
            if (network_lost_times > 30)
            {   
                // 重置连接网络次数
                network_lost_times = 0;
                ret = fm_get_lost_network_policy_status(&policy_status);
                if (ret == FM_SUCCESS)
                {
                    printf("777 GET SUCCESS lost_network status:[%d]", policy_status);
                }
                else
                {
                    printf("777 GET SUCCESS lost_network status:[%d], ret:[%d]", policy_status, ret);
                }
                if (policy_status == FM_ENABLE_POLICY)

                {
                    fm_get_operator_index(current_operator, &current_index);

                    if (at_command_set_operator_next(current_index) == 0)
                    {
                        // operation_log_push_log("local", "SimSwitchTo", "Success", "Network lost policy");
                    }
                    else
                    {
                        // operation_log_push_log("local", "SimSwitchTo", "Error", "Network lost policy");
                        printf("[%s], switching operator ERROR", __func__);
                    }
                }
                else
                {
                    printf("[%s],  policy DISABLE", __func__);
                }
            }
            continue;
        }
        sleep(5);

        network_lost_times = 0;
        break;
    }
    printf("[%s], NETWORK done!", __func__);

    reconnect_info.cleanSession = true;
    // printf("[%s] fm_mqtt_context.connect_clientid:[%s]", __func__, fm_mqtt_context.connect_clientid);
    reconnect_info.pClientIdentifier = fm_mqtt_context.connect_clientid; // fm_mqtt_context.connect_clientid
    reconnect_info.clientIdentifierLength = strlen(reconnect_info.pClientIdentifier);
    reconnect_info.keepAliveSeconds = MQTT_CONNECT_KEEP_ALIVE_SECONDS;
    reconnect_info.pUserName = MQTT_CONNECT_USERNAME;
    reconnect_info.userNameLength = strlen(reconnect_info.pUserName);
    reconnect_info.pPassword = MQTT_CONNECT_PASSWORD;
    reconnect_info.passwordLength = strlen(reconnect_info.pPassword);

    /* -------------------------- MQTT 重连 -------------------------- */
    // fm_mqtt_connect 尝试重新连接MQTT
    status = fm_mqtt_connect(&mqtt_ctx, &net_ctx, &reconnect_info);
    if (status != MQTTSuccess)
    {
        printf("ReConnect WRONG! erno:[%d]", status);
        return -2;
    }
    else
    {
        printf("ReConnect done!");
    }

    resub_info.qos = MQTT_CLIENT_SUB_QOS;
    // printf("[%s] fm_mqtt_context.sub_topic111:[%s]", __func__, fm_mqtt_context.sub_topic);
    resub_info.pTopicFilter = fm_mqtt_context.sub_topic;
    resub_info.topicFilterLength = strlen(resub_info.pTopicFilter);

    // fm_mqtt_subscribe 重新订阅MQTT消息主题
    status = fm_mqtt_subscribe(&mqtt_ctx, &resub_info);
    if (status != MQTTSuccess)
    {
        printf("ReSub WRONG! erno:[%d]", status);
        return -1;
    }
    else
    {
        printf("ReSub done!");
    }

    create_heart_beat_msg(&repub_msg_data);

    // QoS of publish.
    pub_info.qos = MQTT_CLIENT_PUB_QOS;
    // printf("[%s] fm_mqtt_context.pub_topic111:[%s]", __func__, fm_mqtt_context.pub_topic);
    pub_info.pTopicName = fm_mqtt_context.pub_topic;
    pub_info.topicNameLength = strlen(pub_info.pTopicName);
    pub_info.pPayload = repub_msg_data;
    pub_info.payloadLength = strlen(repub_msg_data);

    // fm_mqtt_publish MQTT发布消息
    status = fm_mqtt_publish(&mqtt_ctx, &pub_info);
    if (status != MQTTSuccess)
    {
        printf("RePub WRONG! erno:[%d]", status);
        free(repub_msg_data);
        return 0;
    }
    else
    {
        printf("RePub done!");
    }

    // 重新启动接受循环线程
    restart_receive_loop_thread();

    free(repub_msg_data);

    return 1;
}
void fm_mqtt_context_init(void)
{
    int ret = 0;
    char imei[32];
    // MQTT客户端ID
    memset(imei, 0, 32);
    ret = fm_get_imei(imei);
    if (ret == FM_SUCCESS)
    {
        printf("IMEI: %s\n", imei); // 打印IMEI号码
    }
    else
    {
        printf("get IMEI FAIL \n");
    }
    // printf("imei_length: %d\n", imei_length); // 打印IMEI长度，这里缺少分号

    fm_mqtt_context.connect_clientid = (char *)malloc(MQTT_TOPIC_LEN);
    if (fm_mqtt_context.connect_clientid == NULL)
    {
        printf("malloc fm_mqtt_context.connect_clientid failed");
        return;
    }

    memset(fm_mqtt_context.connect_clientid, 0, MQTT_TOPIC_LEN);
    sprintf(fm_mqtt_context.connect_clientid, "%s", imei);
    fm_resp_replace(&fm_mqtt_context.connect_clientid);
    printf("[%s], fm connect_clientid: [%s]", __func__, fm_mqtt_context.connect_clientid);

    // MQTT上报主题 ( "JSON/Heartbeat/设备厂商/设备型号" )
    fm_mqtt_context.pub_topic = (char *)malloc(MQTT_TOPIC_LEN);
    if (fm_mqtt_context.pub_topic == NULL)
    {
        printf("malloc fm_mqtt_context.pub_topic failed");
        return;
    }
    memset(fm_mqtt_context.pub_topic, 0, MQTT_TOPIC_LEN);

    sprintf(fm_mqtt_context.pub_topic, "%s/%s", HEARTBEAT_TOPIC, MODEM);
    fm_resp_replace(&fm_mqtt_context.pub_topic);
    printf("[%s], fm pub_topic: [%s]", __func__, fm_mqtt_context.pub_topic);

    // MQTT订阅主题 ( "JSON/设备厂商/设备型号/IMEI" )
    fm_mqtt_context.sub_topic = (char *)malloc(MQTT_TOPIC_LEN);
    if (fm_mqtt_context.sub_topic == NULL)
    {
        printf("malloc fm_mqtt_context.sub_topic failed");
        return;
    }
    memset(fm_mqtt_context.sub_topic, 0, MQTT_TOPIC_LEN);
    sprintf(fm_mqtt_context.sub_topic, "%s/%s/%s", MODEL_TOPIC, MODEM, imei);
    fm_resp_replace(&fm_mqtt_context.sub_topic);
    printf("[%s], fm sub_topic: [%s]", __func__, fm_mqtt_context.sub_topic);
}
void *ReceivedLoop(void *argument)
{
    MQTTStatus_t status;

    sleep(30);

    while (1)
    {
        // 调用回调函数 eventCallback 来处理，返回的（SUBACK）数据包。
        status = fm_mqtt_receiveloop(&mqtt_ctx);
        if (status != MQTTSuccess)
        {
            printf("[%s] ReceiveLoop WRONG! erno:[%d]", __func__, status);
        }
    }
}
/*
void* Weaksignal()
{
    int signal_strength = 0;
    int default_signal_strength = 0;
    int signal_weak_times = 0;
    char current_operator[32] = {0};
    int current_index=0;
    fm_get_signal_strength(&signal_strength);
    default_signal_strength = fm_get_weak_signal_threshold();
    if (signal_strength < default_signal_strength)
    {
        signal_weak_times += 1;
        printf("signal_weak_times: %d", signal_weak_times);
        if (signal_weak_times > 10)
        {
            printf("at_command_set_operator_next");
            signal_weak_times = 0;
            fm_get_operator(current_operator,&current_index);


            if (at_command_set_operator_next(current_index) == 0)
            {
            //operation_log_push_log("local", "SimSwitchTo", "Success", "Signal weak policy");
            }
            else
            {
            //operation_log_push_log("local", "SimSwitchTo", "Error", "Signal weak policy");
            }

            }
    }

}
*/
int fm_mqtt_p2x_OSATaskCreate_ReceiveLoop()
{

    int result = pthread_create(&hThread, NULL, ReceivedLoop, NULL);

    if (result)
    {
        printf("Error: pthread_create() return code: %d\n", result);
        return 1;
    }
    else
    {
        return 0;
    }

    // 等待线程结束
    return status;
}
/*
int fm_mqtt_p2x_OSATaskCreate_Weaksignal()
{

        int result = pthread_create(&hThread1, NULL, Weaksignal, NULL);

        if (result) {
        printf("Error: pthread_create() return code: %d\n", result);
        return 1;
        }else{
            return 0;
        }

        // 等待线程结束
        return status;
}
*/
uint32_t get_Time_Stamp(void)
{
    uint32_t timestamp = 0;

    // 使用 time 函数获取当前时间
    timestamp = time(NULL);

    return timestamp;
}
static int recvsub_id = 1;
void mqtt_at_message_handle(MQTTContext_t *pContext, MQTTPacketInfo_t *pPacketInfo, MQTTDeserializedInfo_t *pDeserializedInfo)
{
    char *msg_payload = NULL;
    if ((pPacketInfo->type & 0xF0) == MQTT_PACKET_TYPE_PUBLISH)
    {
        ("[%s], recvSub_id:[%d], packetIdentifier: [%d]", __func__, recvsub_id++, pDeserializedInfo->packetIdentifier);

        if (pDeserializedInfo && pDeserializedInfo->pPublishInfo->pTopicName)
        {
            printf("[%s], topicNameLength=[%d]", __func__, pDeserializedInfo->pPublishInfo->topicNameLength);
            if (pDeserializedInfo->pPublishInfo->topicNameLength > 0)
            {
                printf("[%s], pTopicName=[%s]", __func__, (char *)pDeserializedInfo->pPublishInfo->pTopicName);
            }
        }
        if (pDeserializedInfo && pDeserializedInfo->pPublishInfo->pPayload)
        {
            printf("[%s], payloadLength=[%zu]", __func__, pDeserializedInfo->pPublishInfo->payloadLength);
            if (pDeserializedInfo->pPublishInfo->payloadLength > 0)
            {
                printf("[%s], pPayload=[%s]", __func__, (char *)pDeserializedInfo->pPublishInfo->pPayload);

                msg_payload = (char *)malloc(pDeserializedInfo->pPublishInfo->payloadLength + 1);
                if (msg_payload == NULL)
                {
                    printf("malloc msg_payload failed");
                    return;
                }
                memset(msg_payload, 0, pDeserializedInfo->pPublishInfo->payloadLength + 1);
                memcpy(msg_payload, pDeserializedInfo->pPublishInfo->pPayload, pDeserializedInfo->pPublishInfo->payloadLength);
                msg_payload[pDeserializedInfo->pPublishInfo->payloadLength] = '\0';

                int ret = 0;

                cJSON *root = NULL;
                /*
                cJSON *simSwitchToValue= NULL;
                cJSON *cjson_policy_operator_first = NULL;
                cJSON *cjson_policy_operator_first_prefer_operator = NULL;

                cJSON *cjson_policy_network_lost = NULL;
                cJSON *cjson_policy_network_lost_network_lost = NULL;

                cJSON *cjson_policy_signal_weak =NULL;
                cJSON *cjson_policy_signal_weak_weak_signal_threshold=NULL;

                cJSON *requestDeviceInfoValue = NULL;
                */

                root = cJSON_Parse(msg_payload);
                if (root == NULL)
                {
                    printf("parse fail.\n");
                }
                else
                {
                    // 手动切卡
                    /*
                    simSwitchToValue = cJSON_GetObjectItem(cjson_test, "sim_switch_to");
                    //优先卡
                    cjson_policy_operator_first = cJSON_GetObjectItem(cjson_test, "policy_operator_first");
                    cjson_policy_operator_first_prefer_operator = cJSON_GetObjectItem(cjson_policy_operator_first, "prefer_operator");
                    //自动切卡使能
                    cjson_policy_network_lost = cJSON_GetObjectItem(cjson_test, "policy_network_lost");
                    cjson_policy_network_lost_network_lost = cJSON_GetObjectItem(cjson_policy_network_lost, "network_lost");


                    //一键上报心跳
                    requestDeviceInfoValue =  cJSON_GetObjectItem(cjson_test, "request_device_info");
                    //弱信号
                    cjson_policy_signal_weak = cJSON_GetObjectItem(cjson_test,"policy_signal_weak");
                    cjson_policy_signal_weak_weak_signal_threshold = cJSON_GetObjectItem(cjson_policy_signal_weak,"weak_signal_threshold");
                    */
                    //------------------------------//切网----------

                    cJSON *simSwitchToValue = cJSON_GetObjectItem(root, "sim_switch_to");
                    if (simSwitchToValue == NULL)
                    {
                        printf("[%s], NOT switch NET", __func__);
                    }
                    else
                    {
                        const char *operator_value_net_str = simSwitchToValue->valuestring; // 假设您可以直接获取 cJSON 对象的整数值

                        printf("[%s], valuestring=[%s]", __func__, simSwitchToValue->valuestring);

                        char current_operator[32] = {0};

                        memset(current_operator, 0, 32);

                        fm_status = fm_get_operator(current_operator);
                        if (ret == FM_SUCCESS)
                        {
                            printf("GET SUCCESS current_operator:[%s]", current_operator);
                        }
                        else
                        {
                            printf("GET ERROR current_operator:[%s], ret:[%d]", current_operator, fm_status);
                        }

                        if (strcmp(operator_value_net_str, current_operator) != 0)
                        {
                            if (strcmp(operator_value_net_str, "CHINA UNICOM") == 0)
                            {
                                printf("send switch-unicom success!");
                                ope_index = 0;

                                fm_status = fm_set_operator(oper_list[ope_index]);
                                if (fm_status == FM_SUCCESS)
                                {
                                    printf("[%s], switch CHINA UNICOM SUCCESS", __func__);
                                    sleep(10);
                                }
                                else
                                {
                                    printf("[%s], switch CHINA UNICOM ERROR", __func__);
                                }
                            }
                            else if (strcmp(operator_value_net_str, "CHINA MOBILE") == 0)
                            {
                                printf("send switch-mobile success!");
                                ope_index = 1;
                                fm_status = fm_set_operator(oper_list[ope_index]);
                                if (fm_status == FM_SUCCESS)
                                {
                                    printf("[%s], switch CHINA_MOBILE SUCCESS", __func__);
                                    sleep(10);
                                }
                                else
                                {
                                    printf("[%s], switch CHINA_MOBILE ERROR", __func__);
                                }
                            }
                            else if (strcmp(operator_value_net_str, "CHINA TELECOM") == 0)
                            {
                                printf("send switch-telecom success!");
                                ope_index = 2;
                                fm_status = fm_set_operator(oper_list[ope_index]);
                                if (fm_status == FM_SUCCESS)
                                {
                                    printf("[%s], switch CHINA_TELECOM SUCCESS", __func__);
                                    sleep(10);
                                }
                                else
                                {
                                    printf("[%s], switch CHINA_TELECOM ERROR", __func__);
                                }
                            }
                            else
                            {
                                // 处理未知的运营商字符串
                                printf("send - UNKNOWN_OPERATOR- success!");
                            }
                        }
                        else
                        {
                            printf("Operator is same - no need switch - ");
                        }
                    }

#if 1
                    //----------------------------------------//优先卡----

                    cJSON *preferOperatorObj = cJSON_GetObjectItem(root, "policy_operator_first");
                    if (preferOperatorObj == NULL)
                    {
                        printf("[%s], NOT operator NET", __func__);
                    }
                    else
                    {
                        cJSON *preferOperatorValue = cJSON_GetObjectItem(preferOperatorObj, "prefer_opetrator");

                        if (preferOperatorValue == NULL)
                        {
                            printf("[%s], Error accessing prefer_opetrator value.", __func__);
                        }
                        else
                        {
                            const char *operator_value_prefer_net_str = preferOperatorValue->valuestring; // 假设您可以直接获取 cJSON 对象的整数值
                            printf("[%s], pre_valuestring=[%s]", __func__, preferOperatorValue->valuestring);

                            if (strcmp(operator_value_prefer_net_str, "CHINA UNICOM") == 0)
                            {

                                printf("send unicom-first success!");

                                prefer_operator = CHINA_UNICOM_OPERATOR;
                            }
                            else if (strcmp(operator_value_prefer_net_str, "CHINA MOBILE") == 0)
                            {
                                printf("send mobile-first success!");

                                prefer_operator = CHINA_MOBILE_OPERATOR;
                            }
                            else if (strcmp(operator_value_prefer_net_str, "CHINA TELECOM") == 0)
                            {
                                printf("send telecom-first success!");
                                prefer_operator = CHINA_TELECOM_OPERATOR;
                            }
                            else if (strcmp(operator_value_prefer_net_str, "LAST TIME") == 0)
                            {
                                printf("send last-time success!");
                                prefer_operator = LAST_TIME_OPERATOR;
                            }
                            else
                            {
                                // 处理未知的运营商字符串

                                printf("[%s], prefer_operator is UNkNOW", __func__);
                            }
                            fm_status = fm_set_prefer_operator_policy(prefer_operator);
                            if (fm_status == FM_SUCCESS)
                            {
                                printf("[%s], save prefer opetrator SUCCESS , fm_status:[%d]", __func__, fm_status);
                            }
                            else
                            {
                                printf("[%s], save prefer opetrator ERROR , fm_status:[%d]", __func__, fm_status);
                            }
                        }
                    }
#endif

                    //-------------------------------------//自动切卡
                    // 原始代码段的一部分

                    // 修改后的代码段
                    // 首先检查 network_lost 是否存在于 JSON 对象中
                    cJSON *networkLostObj = cJSON_GetObjectItem(root, "policy_network_lost");
                    if (networkLostObj == NULL)
                    {
                        printf("[%s], NOT policy net lost.", __func__);
                    }
                    else
                    {

                        cJSON *cjson_policy_network_lost_network_lost = cJSON_GetObjectItem(networkLostObj, "network_lost");
                        if (cjson_policy_network_lost_network_lost == NULL)
                        {

                            printf("[%s], NOT lost value.", __func__);
                        }
                        else
                        {
                            int network_lost_value = cjson_policy_network_lost_network_lost->valueint;
                            printf("[%s], network_lost_value is [%d]", __func__, network_lost_value);
                            // 根据network_lost的值设置fm_policy_status

                            if (cjson_policy_network_lost_network_lost->valueint)
                            {
                                policy_status = FM_ENABLE_POLICY;
                                printf("[%s], going to enable network_lost", __func__);
                            }
                            else
                            {
                                policy_status = FM_DISABLE_POLICY;
                                printf("[%s], going to disable network_lost", __func__);
                            }
#if 1
                            // 根据 fm_policy_status 设置丢失网络策略状态
                            fm_status = fm_set_lost_network_policy_status(policy_status);
                            if (fm_status == FM_SUCCESS)
                            {
                                printf("[%s], set lost_network_polic success", __func__);
                            }
                            else
                            {
                                printf("[%s], set lost_network_polic ERROR", __func__);
                            }
                        }
                    }
// 每次使用后给pPayload值制0，防止数据遗留问题
#endif

                    //---------------弱信号-----------
                    cJSON *weakSignalObj = cJSON_GetObjectItem(root, "policy_signal_weak");

                    if (weakSignalObj == NULL)
                    {
                        printf("[%s], NOT policy weak signal.", __func__);
                    }
                    else
                    {
                        cJSON *weakSignalValue = cJSON_GetObjectItem(weakSignalObj, "weak_signal_threshold");
                        if (weakSignalValue == NULL)
                        {
                            printf("[%s], Error accessing weak signal value.", __func__);
                        }
                        else
                        {
                            int weak_signal_threshold = weakSignalValue->valueint;
                            printf("[%s], weak signal value: [%d]", __func__, weak_signal_threshold);

// 设置弱信号切换运营商策略状态
#if 0
                    fm_status = fm_set_weak_signal_threshold(weakSignalValue->valueint);
                    if ( fm_status == FM_SUCCESS )
                    {
                    printf("[%s], save weak signal threshold SUCCESS , fm_status:[%d]", __func__, fm_status);
                    }else{
                    printf("[%s], save weak signal threshold ERROR , fm_status:[%d]", __func__, fm_status);
                    }

                                            
                    int weak_signal_threshold = fm_get_weak_signal_threshold();
                    printf("[%s], get weak signal threshold, weak_signal_threshold:[%d]", __func__, weak_signal_threshold);
                    char signal_weak_threshhold_str[16] = {0};
                    sprintf(signal_weak_threshhold_str, "%d", weak_signal_threshold);
                    operation_log_push_log("remote", "PolicySignalWeak_WeakSignalThreshold", "Success", signal_weak_threshhold_str);
#endif
                        }
                    }

                    //----------一键上报心跳

                    cJSON *requestDeviceInfoValue = cJSON_GetObjectItem(root, "request_device_info");
                    if (requestDeviceInfoValue == NULL)
                    {
                        printf("[%s], NOT request device info.", __func__);
                    }
                    else
                    {
                        int request_device_info_value = requestDeviceInfoValue->valueint;
                        if (request_device_info_value != 1)
                        {
                            printf("[%s], NOT request device info.", __func__);
                        }
                        else
                        {

                            if (requestDeviceInfoValue->valueint)
                            {
                                char *pub_msg_data = NULL;
                                create_heart_beat_msg(&pub_msg_data);
                                if (pub_msg_data != NULL)
                                {
                                    // 发布消息
                                    if (mqtt_client_publish(fm_mqtt_context.pub_topic, (int)strlen(pub_msg_data), pub_msg_data) == TRUE)
                                    {
                                        printf("publish heartbeat success");
                                    }
                                    else
                                    {
                                        printf("publish heartbeat failed");
                                    }
                                }
                                if (pub_msg_data != NULL)
                                {
                                    free(pub_msg_data);
                                }
                            }
                        }
                    }
                    cJSON_Delete(root);
                }
                if (msg_payload != NULL)
                {
                    free(msg_payload);
                    msg_payload = NULL;
                }

                memset((void *)pDeserializedInfo->pPublishInfo->pPayload, 0, pDeserializedInfo->pPublishInfo->payloadLength);
            }
        }
    }
}
int main(int argc, char *argv[])
{

    int ret;
    int network_lost_times = 0;
    int network_lost_times1 = 0;
    char iccid[32] = {0};
    char current_operator[32] = {0};
    int current_index = 0;

    memset(iccid, 0, 32);

    fm_get_iccid(iccid);

    printf("ICCID is %s\n", iccid);

    fm_status = fm_get_lost_network_policy_status(&policy_status);
    if (fm_status == FM_SUCCESS)
    {
        printf("[%s], get lost_network_polic success", __func__);
    }
    else
    {
        printf("[%s], get lost_network_polic ERROR", __func__);
    }
    while (1)
    {
        printf("machine start check NETWORK start ! \n");
        ret = check_network_link();
        printf("ret is %d \n", ret);
        if (ret != 1)
        {
            printf("[%s],  UN NETWORK connect! erno:%d", __func__, ret);
            sleep(5);
            network_lost_times1 += 1;
            printf("[%s],  UN NETWORK connect! network_lost_times: %d", __func__, network_lost_times1);
            if (network_lost_times1 > 30)
            {
                // 重置连接网络次数
                network_lost_times1 = 0;
                fm_get_operator_index(current_operator, &current_index);
                printf("in lost network circle of machine start  : current operator is %s,index is %d\n", current_operator, current_index);

                if (policy_status == FM_ENABLE_POLICY)
                {
                    at_command_set_operator_next(current_index);

                    sleep(15);
                    printf("done 1\n");
                }
                else
                {
                    printf("[%s],  policy DISABLE", __func__);
                }
            }
            continue;
        }

        network_lost_times1 = 0;
        printf("[%s],  check NETWORK connect! erno:%d", __func__, ret);
        break;
    }

    net_ctx.socket_fd = -1;
    net_ctx.server_addr = MQTT_SERVER_ADDR;
    net_ctx.server_port = MQTT_SERVER_PORT;
    msg_fixed_buffer.pBuffer = msg_fixed_buffer_buf;
    msg_fixed_buffer.size = MQTT_CLIENT_FIXED_BUF_LEN;
    /*
    printf("server port:%s\n",net_ctx.server_port);
    printf("server addr:%s\n",net_ctx.server_addr);
    */
    char *pub_msg_data = NULL;
    int pub_sleep = 0;
    uint32_t pub_start_time = 0;
    uint32_t pub_end_time = 0;

    status = fm_mqtt_init(&mqtt_ctx, &net_ctx, get_Time_Stamp, mqtt_at_message_handle, &msg_fixed_buffer);
    if (status != MQTTSuccess)
    {
        printf("Init WRONG! erno:%d\n", status);
        goto EXIT;
    }
    printf("Init done!\n");

    fm_mqtt_context_init();

    connect_info.cleanSession = true;
    connect_info.pClientIdentifier = fm_mqtt_context.connect_clientid;
    connect_info.clientIdentifierLength = strlen(connect_info.pClientIdentifier);
    connect_info.keepAliveSeconds = MQTT_CONNECT_KEEP_ALIVE_SECONDS;
    connect_info.pUserName = MQTT_CONNECT_USERNAME;
    connect_info.userNameLength = strlen(connect_info.pUserName);
    connect_info.pPassword = MQTT_CONNECT_PASSWORD;
    connect_info.passwordLength = strlen(connect_info.pPassword);

    /*
     printf("member1: %s\n", connect_info.pClientIdentifier);
     printf("member2: %s\n", connect_info.pUserName);
     printf("member3: %s\n", connect_info.pPassword);
   */

    printf("[%s], START", __func__);
    // 获取mqtt订阅、发布主题

    while (1)
    {
        status = fm_mqtt_connect(&mqtt_ctx, &net_ctx, &connect_info);
        if (status != MQTTSuccess)
        {
            printf("Connect WRONG! erno:%d\n", status);
            sleep(3);
            continue;
        }
        else
        {
            printf("Connect done!\n");
            is_mqtt_connected = 1;
        }

        status = fm_mqtt_get_state(&mqtt_ctx);
        if (status != MQTTSuccess)
        {
            printf("get_state WRONG! erno:%d\n", status);
            sleep(5);
            ;
            continue;
        }
        else
        {
            printf("MQTT connected done!\n");
        }
        break;
    }
    sub_info.qos = MQTT_CLIENT_SUB_QOS;
    sub_info.pTopicFilter = fm_mqtt_context.sub_topic;
    sub_info.topicFilterLength = strlen(sub_info.pTopicFilter);

    status = fm_mqtt_subscribe(&mqtt_ctx, &sub_info);
    if (status != MQTTSuccess)
    {
        printf("Sub WRONG! erno:%d\n", status);
        goto DISCONN;
    }
    printf("Sub done!\n");

    status = fm_mqtt_p2x_OSATaskCreate_ReceiveLoop();
    if (status != FM_SUCCESS)
    {
        printf("ReceiveLoop TaskCreate WRONG! erno:%d", status);
        goto UNSUB;
    }
    printf("ReceiveLoop TaskCreate done!");
    // MQTT重连检测机制
    /*
        status = fm_mqtt_p2x_OSATaskCreate_Weaksignal();
        if (status != FM_SUCCESS)
        {
            printf("Weaksignal TaskCreate WRONG! erno:%d", status);
            goto UNSUB;
        }
        printf("Weaksignal TaskCreate done!");
    */
    while (1)
    {
        printf("MQTT check !\n");
        pub_start_time = get_Time_Stamp();
        create_heart_beat_msg(&pub_msg_data);

        pub_info.qos = MQTT_CLIENT_PUB_QOS;
        pub_info.pTopicName = fm_mqtt_context.pub_topic;
        pub_info.topicNameLength = strlen(pub_info.pTopicName);
        pub_info.pPayload = pub_msg_data;
        pub_info.payloadLength = strlen(pub_info.pPayload);

        status = fm_mqtt_publish(&mqtt_ctx, &pub_info);
        if (status != MQTTSuccess)
        {
            printf("Pub WRONG! erno:%d\n", status);
        }
        else
        {
            printf("Pub done!\n");
        }
        free(pub_msg_data);

        while (1)
        {

            // fm_mqtt_get_state 获取客户端与服务器的连接状态
            status = fm_mqtt_get_state(&mqtt_ctx);
            ret = check_network_link();
            printf("get_state status done:%d!\n", ret);
            printf("get_MQTT_state status done:%d!\n", status);
            if (!ret || status)
            {
                is_mqtt_connected = 0;
                // 客户端与服务器的连接状态异常处理
                printf("get_state WRONG! erno:%d", ret);
                ret = fm_mqtt_reconnect();
                if (ret != 1) // 重连失败
                {
                    printf("RECONNECT ERROR! erno:%d", ret);
                }
                else
                {
                    is_mqtt_connected = 1;
                    printf("RECONNECT OK!");
                }
            }
            else
            {
                if (is_mqtt_connected == 0) // 上次重连失败，本次在进行一次重连
                {
                    ret = fm_mqtt_reconnect();
                    if (ret != 1)
                    {
                        printf("RECONNECT ERROR! erno:%d", ret);
                    }
                    else
                    {
                        is_mqtt_connected = 1;
                        printf("RECONNECT OK!");
                    }
                }
                printf("get_state done!");
            }
            pub_end_time = get_Time_Stamp();
            pub_sleep = (pub_end_time - pub_start_time);
            printf("pub_sleep:[%d] s", pub_sleep);
            if (pub_sleep > 1800) // 1h 3600s
            {
                break;
            }
            else
            {
                sleep(1);
            }
        }
    }
UNSUB:
    unsub_info.pTopicFilter = fm_mqtt_context.sub_topic;
    unsub_info.topicFilterLength = strlen(unsub_info.pTopicFilter);
    status = fm_mqtt_unsubscribe(&mqtt_ctx, &unsub_info);
    if (status != MQTTSuccess)
    {
        printf("unsub WRONG! erno:%d\n", status);
        goto DISCONN;
    }
    printf("unsub done!\n");

DISCONN:
    status = fm_mqtt_disconnect(&mqtt_ctx);
    if (status != MQTTSuccess)
    {
        printf("disconn WRONG! erno:%d\n", status);
        goto EXIT;
    }
    printf("disconnect done!\n");

EXIT:
    printf("exit done!\n");

    printf("END!!!!!!!!!!!!!!!!!!!!!\n");
}
