#include <string.h>
#include "custom_main.h"
#include "cm_iomux.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "cm_os.h"
#include "cm_mem.h"
#include "cm_sys.h"

#include "cm_virt_at.h"

#include "fm_config_port.h"
#include "fm_at_command_port.h"
#include "fm_mqtt_port.h"
#include "fm_fs_port.h"
#include "fm_p2x_cfg_port.h"
#include "fm_pthread_port.h"
#include "cJSON.h"

#include "lwip/lwip/netdb.h"
#include "lwip/lwip/sockets.h"



#define MQTT_SERVER_PORT	"1883"								//MQTT连接端口
#define MQTT_SERVER_ADDR	"123.57.71.54"					//MQTT连接地址
#define MQTT_CLIENT_FIXED_BUF_LEN   2148						//MQTT缓冲区大小

//MQTT connect

#define MQTT_TOPIC_LEN 256
#define MQTT_CONNECT_CLIENTID	"mqttx_47842d0e"				//MQTT客户端ID
#define MQTT_CONNECT_USERNAME	"chengdu"							//MQTT代理账号
#define MQTT_CONNECT_PASSWORD	"365de778fa058988c154085468d0a978"					//MQTT代理密码
#define MQTT_CONNECT_KEEP_ALIVE_SECONDS		60					//MQTT连接保活的时间

#define HEARTBEAT_TOPIC "JSON/Heartbeat/CMCC"
#define MODEL_TOPIC "JSON/CMCC"

#define MODEM  "ML307R-DC"
//MQTT client Sub
#define MQTT_CLIENT_SUB_QOS		MQTTQoS1						//MQTT订阅QOS级别 1
//#define MQTT_CLIENT_SUB_TOPIC	"JSON/FM/CPE/868998061757114"	//MQTT订阅主题 "JSON/设备厂商/设备型号/IMEI"
//#define MQTT_CLIENT_SUB_TOPIC_NUM	1						//MQTT订阅主题

//MQTT client Pub
#define MQTT_CLIENT_PUB_QOS		MQTTQoS1						//MQTT上报QOS级别 1
//#define MQTT_CLIENT_PUB_TOPIC	"JSON/Heartbeat/FM/CPE"			//MQTT上报主题 "JSON/Heartbeat/设备厂商/设备型号"
//#define MQTT_CLIENT_PUB_TOPIC_NUM	1

#define CALL_INTERVAL_MS 60000
uint32_t last_call_time = 0;

static int is_mqtt_connected=0;
MQTTContext_t mqtt_ctx = {0x0};
NetworkContext_t net_ctx = {0x0};
MQTTPublishInfo_t pub_info = {0x0};
FM_MQTT_CONTEXT_t fm_mqtt_context = {0};

FM_NETWORK_OPERATOR_t prefer_operator = UNKNOWN_OPERATOR;//优先卡
FM_NETWORK_OPERATOR_t last_operator = UNKNOWN_OPERATOR;//上次驻网的运营商
FM_NETWORK_OPERATOR_t operator = UNKNOWN_OPERATOR;
FM_NETWORK_OPERATOR_t current_workingcard = UNKNOWN_OPERATOR;

static int ope_index = 0;
static char *oper_list[] = {"CHINA UNICOM", "CHINA MOBILE", "CHINA TELECOM","LAST TIME"};
int array[3] = {0};
int default_signal_threshold = 0 ;
FM_STATUS_t fm_status = FM_UNKNOWN;
FM_POLICY_STATUS_t policy_status = FM_UNKNOWN_POLICY;

MQTTFixedBuffer_t msg_fixed_buffer = {0x0};
uint8_t msg_fixed_buffer_buf[MQTT_CLIENT_FIXED_BUF_LEN] = {0};
MQTTConnectInfo_t connect_info = {0x0};
MQTTSubscribeInfo_t sub_info = {0x0};
MQTTSubscribeInfo_t unsub_info = {0x0};



FM_NETWORK_OPERATOR_t string_to_operator(const char* operator_str) 
{
    if (strcmp(operator_str, "CHINA UNICOM") == 0) {
        return CHINA_UNICOM_OPERATOR;
    } else if (strcmp(operator_str, "CHINA MOBILE") == 0) {
        return CHINA_MOBILE_OPERATOR;
    } else if (strcmp(operator_str, "CHINA TELECOM") == 0) {
        return CHINA_TELECOM_OPERATOR;
    } 
      
}
int count_array_size(int array[3])
{
    int count = 0;
    for (int i = 0; i < 3; ++i) {
        if (array[i] != 0) {
            count++;
        }
    }
    return count;
}
uint32_t get_Time_Stamp() 
{
    UINT32 ticks = 0;
    UINT32 mseconds = 0;

    ticks = osKernelGetTickCount ();
    mseconds = ticks * 5;
    return mseconds;
}
int get_operator_from_sim(char *operator_buffer, size_t buffer_size)
{
    int time = 5;
    char current_operator[32] = {0}; // 确保数组足够大以存储运营商名称

    while (time--) 
	{
        int fm_status = fm_get_operator(current_operator);
        if (fm_status == FM_SUCCESS) { // 检查fm_get_operator是否成功
            for (int i = 0; i < sizeof(oper_list) / sizeof(oper_list[0]); i++) {
                if (strcmp(current_operator, oper_list[i]) == 0) {
                    // 使用strcmp比较字符串
                    strncpy(operator_buffer, current_operator, buffer_size); // 将找到的运营商复制到输出参数
                    operator_buffer[buffer_size - 1] = '\0'; // 确保字符串以NULL结尾
                    FM_LOG_INFO("GET SUCCESS current_operator in SIM SWITCH circle:[%s]", current_operator);
                    FM_LOG_INFO("GET operator time is :[%d]\n", time);
                    return 1; // 成功找到匹配的运营商
                }
            }
            FM_LOG_INFO("GET SUCCESS current_operator failed\n");
        } else {
            FM_LOG_INFO("fm_get_operator failed with status: %d\n", fm_status);
            
        }
	    FM_LOG_INFO("GET operator time is :[%d]\n", time);
		FM_SLEEP(1);	
    }

    FM_LOG_INFO("GET operator time is :[%d]\n", time);
    return 0; // 超时或未找到匹配的运营商
}
int fm_sett_prefer_policy(const char *new_operator)
{   
	uint32_t current_time =get_Time_Stamp();

    // 检查是否超过了60秒的调用间隔
    if (current_time - last_call_time < CALL_INTERVAL_MS ) {
        FM_LOG_INFO("In prefer Function is called too frequently. Please wait for %ld ms before calling again.\n",
                    CALL_INTERVAL_MS - (current_time - last_call_time));
        return 0; // 返回0表示失败
    }
    
	last_call_time=get_Time_Stamp();

	char temp_operator[100];
    strcpy(temp_operator, new_operator);
	fm_status=fm_set_prefer_policy(temp_operator);
	if ( fm_status == FM_SUCCESS ) 
	{
		FM_LOG_INFO("SET fm_set_prefer_policy SUCCESS\n");
		return 1; // 返回1表示成功
	} 
	else 
	{
		FM_LOG_INFO("switched failed\n");
		return 0; // 返回0表示失败
	}
}
int fm_sett_operator(const char *new_operator)
{   
	uint32_t current_time =get_Time_Stamp();

    // 检查是否超过了60秒的调用间隔
    if (current_time - last_call_time < CALL_INTERVAL_MS ) {
        FM_LOG_INFO("In Switch Function is called too frequently. Please wait for %ld ms before calling again.\n",
                    CALL_INTERVAL_MS - (current_time - last_call_time));
        return 0; // 返回0表示失败
    }
    
	last_call_time=get_Time_Stamp();

    char operator_buffer[32] = {0}; // 用于存储运营商信息的缓冲区
    int result = get_operator_from_sim(operator_buffer, sizeof(operator_buffer));
    if (result == 1) 
	{
        FM_LOG_INFO("Operator found: %s\n", operator_buffer);
    } else {
        FM_LOG_INFO("Failed to get operator or timeout.\n");
    }
     
    if (strcmp(operator_buffer, new_operator) == 0) 
	{   
        
        FM_LOG_INFO("Operators are the same, no need to switch.\n");
        return 0; // 返回0表示失败
    } 
	else 
	{   
		char temp_operator[100];
        strcpy(temp_operator, new_operator);
		
        fm_status = fm_set_operator(temp_operator);
		if(fm_status==FM_SUCCESS)
		{
            FM_LOG_INFO("switch success\n", new_operator);
		    return 1; // 返回1表示成功
        }
		else
		{
			FM_LOG_INFO("Switch failed.\n");
			return 0; // 返回1表示成功
		}
		
    }
	
}
int at_command_set_operator_next(int current_index)
{
	static int ope_index = 0;
    FM_STATUS_t state = FM_SUCCESS;

    ope_index=current_index;
	ope_index += 1;
	if (ope_index >= 3)
		ope_index = 0;

    state = fm_sett_operator(oper_list[ope_index]);

	return state;
}
void fm_resp_replace(char **resp) 
{
    int i = 0, j = 0;
    char *tmp = NULL;

    if (resp == NULL || *resp == NULL)
        return;

    tmp = malloc(256);
    if (tmp != NULL) {
        memset(tmp, 0, 256);
        for (i = 0, j = 0; i < strlen(*resp);) {
            if ((*resp)[i] == ' ' || (*resp)[i] == '"' || (*resp)[i] == '\r' || (*resp)[i] == '\n') {
                i += 1;
                continue;
            } else {
                tmp[j] = (*resp)[i];
            }
            i++;
            j++;
        }
        free(*resp);
        *resp = tmp;
    }
}

void fm_mqtt_context_init(void)
{   
    int imei_length;
    char imei[100];
	//MQTT客户端ID
    
    imei_length = fm_get_imei(imei);

    FM_LOG_INFO("IMEI: %s\n", imei); // 打印IMEI号码
    FM_LOG_INFO("imei_length: %d\n", imei_length); // 打印IMEI长度，这里缺少分号
    
    fm_mqtt_context.connect_clientid = (char *)malloc(MQTT_TOPIC_LEN);
    if (fm_mqtt_context.connect_clientid == NULL)
    {
        FM_LOG_INFO("malloc fm_mqtt_context.connect_clientid failed");
        return;
    }
    
    memset(fm_mqtt_context.connect_clientid, 0, MQTT_TOPIC_LEN);
    sprintf(fm_mqtt_context.connect_clientid, "%s", imei);
	fm_resp_replace(&fm_mqtt_context.connect_clientid);
	FM_LOG_INFO("[%s], fm connect_clientid: [%s]", __func__, fm_mqtt_context.connect_clientid);

	//MQTT上报主题 ( "JSON/Heartbeat/设备厂商/设备型号" )
    fm_mqtt_context.pub_topic = (char *)malloc(MQTT_TOPIC_LEN);
    if (fm_mqtt_context.pub_topic == NULL)
    {
        FM_LOG_INFO("malloc fm_mqtt_context.pub_topic failed");
        return;
    }
    memset(fm_mqtt_context.pub_topic, 0, MQTT_TOPIC_LEN);

   sprintf(fm_mqtt_context.pub_topic, "%s/%s", HEARTBEAT_TOPIC, MODEM);
	fm_resp_replace(&fm_mqtt_context.pub_topic);
	FM_LOG_INFO("[%s], fm pub_topic: [%s]", __func__, fm_mqtt_context.pub_topic);

	//MQTT订阅主题 ( "JSON/设备厂商/设备型号/IMEI" )
    fm_mqtt_context.sub_topic = (char *)malloc(MQTT_TOPIC_LEN);
    if (fm_mqtt_context.sub_topic == NULL)
    {
        FM_LOG_INFO("malloc fm_mqtt_context.sub_topic failed");
        return;
    }
    memset(fm_mqtt_context.sub_topic, 0, MQTT_TOPIC_LEN);
    sprintf(fm_mqtt_context.sub_topic, "%s/%s/%s", MODEL_TOPIC, MODEM, imei);
	fm_resp_replace(&fm_mqtt_context.sub_topic);
	FM_LOG_INFO("[%s], fm sub_topic: [%s]", __func__, fm_mqtt_context.sub_topic);
}

int fm_get_weak_signal_threshold(void)
{
    int fd;
    
	int flags = FM_FS_RB;
    const char *file_name = "fm_weak_signal_threshold.cfg";
    char read_data[8] = {0};
    uint32_t len = 0;
    fd = fm_open(file_name, flags, 0);
    if( fd >= 0 )
    {
        fm_read(fd, read_data, sizeof(read_data));
        //关闭文件
        fm_close(fd);

        default_signal_threshold = atoi(read_data);
    }
    else
    {
        default_signal_threshold = -140;//默认弱信号阈值
    }

     return 0;
}

FM_STATUS_t fm_set_weak_signal_threshold(int signal_threshold)
{
    int fd;
    int ret = 0;
	int flags = FM_FS_WB;
    const char *file_name = "fm_weak_signal_threshold.cfg";
    uint32_t wirte_len = 0;
    
    fd = fm_open(file_name, flags, 0);
    if( fd >= 0 )
    {  

        
        char signal_threshold_str[8] = {0};
        sprintf(signal_threshold_str, "%d", signal_threshold);
        wirte_len = fm_write(fd, signal_threshold_str, strlen(signal_threshold_str));
        fm_close(fd);

        ret = FM_SUCCESS;
    }
    else
    {
        ret = FM_ERROR;
    }

    return (FM_STATUS_t)ret;
}

int32_t fm_socket_check_net(void) 
{
	int sockfd;
    struct sockaddr_in server_addr;
    struct hostent *host;

	// 解析域名
    host = gethostbyname("www.baidu.com");
    if ( host == NULL ) 
	{
        FM_LOG_INFO("Failed to resolve hostname!");
        return 0;
    }

    // 创建套接字
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //if ( sockfd < 0 ) 
    if ( sockfd == -1 ) 
	{
        FM_LOG_INFO("Failed to create socket!");
        return -1;
    }
	
    FM_LOG_INFO("\ntcp sockfd:[%d]\n", sockfd);

    // 初始化目标地址信息
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_len = sizeof(server_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(80); // HTTP默认端口是80
    memcpy(&server_addr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);

	// 尝试连接到目标地址
	if ( connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0 ) 
	{
		FM_LOG_INFO("Failed to connect to server!!!");		
		close(sockfd);
		return -2;
	}
	
	// 这里关闭套接字
    close(sockfd);
	
    return 1;
}


static void create_heart_beat_msg(char **heartbeat_msg)
{
	
	FM_STATUS_t ret = FM_SUCCESS;
    int i = 0;

	char sim_state[32] = {0};
    char imei[32] = {0};
    char iccid[32] = {0};
    int signal = -1;
    char current_operator[32] = {0};
    int fly_card = -1;
    char vendor_name[32] = {0};
    char model_name[32] = {0};
    char model[32] = {0};
    cJSON *cjson_test=NULL;

    // 创建JSON的根对象，并将其命名为cjson_test
    cjson_test = cJSON_CreateObject();

    // 添加各个字段到cjson_test对象中
    
	memset(sim_state, 0, 32);
    memset(imei, 0, 32);
    memset(iccid, 0, 32);
	memset(current_operator, 0, 32);
    signal = 0;
    memset(vendor_name, 0, 32);
    memset(model_name, 0, 32);
    
    strcpy(vendor_name,"CMCC");
	fm_get_sim_state(sim_state);
    fm_get_imei(imei);
	fm_get_iccid(iccid);
	fm_get_signal_strength(&signal);
	fm_get_operator(current_operator);
	//fm_get_fly_card(&fly_card);
	//fm_get_vendor_name(vendor_name);
    fm_get_model_name(model_name);

      
    strcpy(model, MODEM);

    cJSON_AddItemToObject(cjson_test, "payload_name", cJSON_CreateString("Heartbeat"));
    cJSON_AddItemToObject(cjson_test, "msg_version", cJSON_CreateString("3.0"));
	FM_SLEEP(1);
    cJSON_AddItemToObject(cjson_test, "vendor_name", cJSON_CreateString(vendor_name));
	FM_SLEEP(1);
    cJSON_AddItemToObject(cjson_test, "model_name", cJSON_CreateString(model));
    
   
    cJSON_AddItemToObject(cjson_test, "imei_data", cJSON_CreateString(imei));
	FM_SLEEP(1);
    cJSON_AddItemToObject(cjson_test, "iccid_data", cJSON_CreateString(iccid));
	FM_SLEEP(1);
    cJSON_AddItemToObject(cjson_test, "current_operator", cJSON_CreateString(current_operator));
	FM_SLEEP(1);

    cJSON_AddItemToObject(cjson_test, "simcard_state", cJSON_CreateString(sim_state));
	FM_SLEEP(1);
    cJSON_AddItemToObject(cjson_test, "heart_beat_interval", cJSON_CreateNumber(3600));
    //cJSON_AddItemToObject(cjson_test, "client_version_name", cJSON_CreateString("SDK 3.0"));
    cJSON_AddItemToObject(cjson_test, "signal_rsrp", cJSON_CreateNumber(signal));


    // 将cjson_test对象转换为字符串
    *heartbeat_msg=cJSON_Print(cjson_test);
    cJSON_Delete(cjson_test);
    // 输出JSON字符串
    //fm_p2x_print("%s\n", json_string);
    FM_LOG_INFO("[%s], json_string: [%s]", __func__, heartbeat_msg);
    // 释放JSON字符串和对象
    
    
	
    
   
	
}

static void fm_mqtt_p2x_ReceiveLoop_main(void *argument) 
{
	MQTTStatus_t status;

	FM_SLEEP(30);

	while( 1 ) 
	{
		//调用回调函数 eventCallback 来处理，返回的（SUBACK）数据包。
		status = fm_mqtt_receiveloop(&mqtt_ctx);
		if( status != MQTTSuccess ) 
		{
			FM_LOG_INFO("[%s] ReceiveLoop WRONG! erno:[%d]", __func__, status);
		}
	}
}
int32_t fm_mqtt_reconnect()
{
	MQTTStatus_t status;
	char *repub_msg_data = NULL;
	int ret =0;
   char current_operator[32] = {0};
   char sim_state[32] = {0};
   int network_lost_times=0;
   int current_index=0;
   memset(current_operator, 0, 32);
   while ( 1 )
	{
		//检查模组是否插卡
		fm_get_sim_state(sim_state);
		

		if(strncmp(sim_state, "READY", strlen(sim_state)))
		{
			FM_LOG_INFO("[%s] Uninserted Card!", __func__);
            FM_SLEEP(5);
			continue;
		}
		FM_LOG_INFO("[%s] SIM Card OK", __func__);
		break;
	}

	while(1)

{  
	ret = fm_socket_check_net();
	
    if(ret != 1)
    {
        FM_LOG_INFO("[%s],  UN NETWORK connect! erno:%d", __func__, ret);
        FM_SLEEP(5);
	#if 1
        network_lost_times += 1;
        FM_LOG_INFO("[%s],  UN NETWORK connect! network_lost_times: %d", __func__, network_lost_times);
        if ( network_lost_times > 20 )
        {

			network_lost_times=0;

			if(policy_status == FM_ENABLE_POLICY)
			{
				FM_LOG_INFO("net work policy is opened\n"); 
				int result1 = get_operator_from_sim(current_operator, sizeof(current_operator));
				if (result1 == 1) 
				{ // 检查fm_get_operator是否成功
					FM_LOG_INFO("Current operator is: %s", current_operator);
					current_index=find_operator_index(current_operator);
					if (current_index!=-1)
					{
						int result=at_command_set_operator_next(current_index);
						if (result == 0) 
						{
							FM_LOG_INFO("switch next Command execution failed in NET LOST Circle .\n");
							FM_LOG_INFO("waite for next.\n");
						} 
						else 
						{
							FM_LOG_INFO("switch next Command execution Success in NET LOST Circle\n");
						}
					}
					else 
					{
							FM_LOG_INFO("GET Operator index failed\n");
					}
				}
				else
				{
					FM_LOG_INFO("Operator not found.\n");
				}
	        }
			else
			{
                  FM_LOG_INFO("net lost policy is closed\n");
			}
	  #endif
                
	   }
	   continue;
	}
        network_lost_times = 0;
		FM_LOG_INFO("[%s],  check NETWORK connect! erno:%d", __func__, ret);
        break;
}   
    fm_status = fm_set_last_operator_policy();
	if ( fm_status == FM_SUCCESS ) 
	{
		FM_LOG_INFO("555 set SUCCESS last Register operator, fm_status:[%d]",fm_status);
	} 
	else 
	{
		FM_LOG_INFO("555 set ERROR last Register operator, fm_status:[%d]",fm_status);
	}
	status = fm_mqtt_connect(&mqtt_ctx, &net_ctx, &connect_info);
    if(status != MQTTSuccess)
    {
        FM_LOG_INFO("ReConnect WRONG! erno:[%d]", status);
        return -2;
    }else
    {
        FM_LOG_INFO("ReConnect done!");
    }
    sub_info.qos = MQTT_CLIENT_SUB_QOS;  
	sub_info.pTopicFilter = fm_mqtt_context.sub_topic;
	sub_info.topicFilterLength = strlen( sub_info.pTopicFilter );
	status = fm_mqtt_subscribe(&mqtt_ctx, &sub_info);
    if(status != MQTTSuccess)
    {
        FM_LOG_INFO("ReSub WRONG! erno:[%d]", status);
        return -1;
    }else{
        FM_LOG_INFO("ReSub done!");
    }
    
    create_heart_beat_msg(&repub_msg_data);
    
	pub_info.qos = MQTT_CLIENT_PUB_QOS;
	FM_LOG_INFO("[%s] fm_mqtt_context.pub_topic111:[%s]", __func__, fm_mqtt_context.pub_topic);
	pub_info.pTopicName = fm_mqtt_context.pub_topic;
	pub_info.topicNameLength = strlen( pub_info.pTopicName );
	pub_info.pPayload = repub_msg_data;
	pub_info.payloadLength = strlen( pub_info.pPayload );
    status = fm_mqtt_publish(&mqtt_ctx, &pub_info);

    if(status != MQTTSuccess)
    {
        FM_LOG_INFO("RePub WRONG! erno:[%d]", status);
        free(repub_msg_data);
        return 0;
    }
    else
    {
        FM_LOG_INFO("RePub done!");
    }

    free(repub_msg_data);
    return 1;
}




 void fm_mqtt_p2x_policyprefer(void) 
{       
	    FM_SLEEP(15);
		char current_operator[32] = {0};
		memset(current_operator, 0, 32);

		fm_status = fm_get_prefer_operator_policy(&prefer_operator);
        if ( fm_status == FM_SUCCESS ) 
        {
            FM_LOG_INFO("111 GET SUCCESS prefer operator:[%d]", operator);
        } 
        else 
        {
            FM_LOG_INFO("111 GET ERROR prefer operator, ret:[%d]",  fm_status);
        }
	    
        //获取优先网络策略使用的运营商
	if (prefer_operator!=UNKNOWN_OPERATOR)
	{

        //获取当前运营商
		fm_status = fm_get_operator(current_operator);
		if ( fm_status == FM_SUCCESS ) 
			{
				FM_LOG_INFO("GET SUCCESS current_operator:[%s]", current_operator);
			} 
		else 
			{
				FM_LOG_INFO("GET ERROR current_operator:[%s], fm_status:[%d]", current_operator, fm_status);
			}
         
		 //获取当前运营商转变为 fm_network_operator模式
		 FM_NETWORK_OPERATOR_t current_operator_enum = string_to_operator(current_operator);
         
        if (prefer_operator == LAST_TIME_OPERATOR)
		{
			 //判断优先策略是否为上次驻网优先策略
			    fm_status = fm_get_last_operator_policy(&last_operator);
				if ( fm_status == FM_SUCCESS ) 
				{
					FM_LOG_INFO("444 GET SUCCESS last operator:[%d]", last_operator);
				} 
				else 
				{
					FM_LOG_INFO("444 GET ERROR last operator, ret:[%d]",fm_status);
				}
             //判断当前运营商是否和上次驻网优先运营商相等
				if(current_operator_enum!=last_operator)
				{
			 	if (last_operator == CHINA_UNICOM_OPERATOR)
					{    fm_status = fm_set_operator(oper_list[0]);
						FM_LOG_INFO("[%s],Switch last_operator [CHINA_UNICOM_OPERATOR] success", __func__);
					}
					else if (last_operator == CHINA_MOBILE_OPERATOR)
					{   fm_status = fm_set_operator(oper_list[1]);
						FM_LOG_INFO("[%s],Switch last_operator [CHINA_MOBILE_OPERATOR] success", __func__);
					}else if (last_operator == CHINA_TELECOM_OPERATOR)
					{   fm_status = fm_set_operator(oper_list[2]);
						FM_LOG_INFO("[%s], Switch last_operator [CHINA_TELECOM_OPERATOR] success", __func__);
					}
				}else{
					    FM_LOG_INFO("[%s], last and current operator is same no need switch", __func__);
				}
		}       
        else if(prefer_operator !=current_operator_enum )
		{
			//判断优先策略为优先卡策略
				if (prefer_operator == CHINA_UNICOM_OPERATOR)
				{   fm_status= fm_set_operator(oper_list[0]);
					if ( fm_status == FM_SUCCESS )
						{
							FM_LOG_INFO("[%s], switch CHINA_UNICOM SUCCESS", __func__);
							FM_SLEEP(10);
						}
						else
						{
							FM_LOG_INFO("[%s], switch CHINA_UNICOM ERROR", __func__);
						}
				}
				else if (prefer_operator == CHINA_MOBILE_OPERATOR)
				{   
					fm_status = fm_set_operator(oper_list[1]);
					if ( fm_status == FM_SUCCESS ) 
					{
					FM_LOG_INFO("[%s],SET NET prefer Operator is :[CHINA_MOBILE_OPERATOR]", __func__);
					}
				}
				else if (prefer_operator == CHINA_TELECOM_OPERATOR)
				{   fm_status = fm_set_operator(oper_list[2]);
					if ( fm_status == FM_SUCCESS ) 
					{
					FM_LOG_INFO("[%s], SET NET prefer Operator is :[CHINA_TELECOM_OPERATOR]", __func__);
					}
				} 
		}
		else 
		{
			FM_LOG_INFO("[%s],prefer and current operator is same no need switch", __func__);
		}

	}
	else 
	{
			FM_LOG_INFO("Policy prefer operator is unknow");
	}
}


  
int find_operator_index(const char *current_operator) {
    for (int i = 0; i < 3; i++) {
        if (strcmp(current_operator, oper_list[i]) == 0) {
            return i; // 返回找到的索引
        }
    }
    return -1; // 如果没有找到，返回-1
}
int findMaxIndex(int array[],  int index1, int index2, int index3) 
{
    int max = array[index1]; // 假设第一个元素是最大的
    int maxIndex = index1;

    if (array[index2] > max) {
        max = array[index2];
        maxIndex = index2;
    }

    if (array[index3] > max) {
        max = array[index3];
        maxIndex = index3;
    }

    return maxIndex;
}
//----------------------弱信号--------------------------//
static void fm_mqtt_p2x_weaksignal_main(void *argument) 
{   int ret =0 ;
	int signal_strength = 0;
    char current_operator[32] = {0};
    int current_index=-1;
	int found =0 ;
    int maxIndex=0;
    int flag = 1;
    uint32_t weak_start_time = 0;
    uint32_t weak_end_time = 0;
    uint32_t weak_sleep = 0;

	while (1)
	{  
	
		weak_end_time= get_Time_Stamp();
		
		weak_sleep = (weak_end_time - weak_start_time)/1000;
		if(weak_sleep>1800)
		{ FM_LOG_INFO("weak signal is weak up\n");
			flag=1;
		}
		while(flag)
		{
			memset(current_operator, 0, 32);
			
			
			
			if(default_signal_threshold>-140)
			{ 
				fm_status=fm_get_signal_strength(&signal_strength);
				if(signal_strength==0)
				{
					signal_strength=-141;
				}
				if(fm_status==FM_SUCCESS)
				{
					FM_LOG_INFO("current signal_strength is: %d", signal_strength);
					FM_LOG_INFO("default signal_strength is: %d", default_signal_threshold);
					int signal_qujian=signal_strength-default_signal_threshold;
					
					if (signal_qujian < 0 )
					{  
						
						FM_LOG_INFO("[%s], current_signal_strength  is smaller\n", __func__);

						int result = get_operator_from_sim(current_operator, sizeof(current_operator));
						if (result == 1) 
						{ // 检查fm_get_operator是否成功
							FM_LOG_INFO("Current operator is: %s", current_operator);
							current_index=find_operator_index(current_operator);;
							array[current_index]=signal_qujian;
						}
						else
						{
							FM_LOG_INFO("Operator not found.\n");
						}
						
					
						int SIZE = count_array_size(array);
						if(SIZE<3)
						{
							int result=at_command_set_operator_next(current_index);
							if (result == 0) 
							{
								FM_LOG_INFO("switch next Command execution failed in signal check .\n");
								FM_LOG_INFO("waite for next.\n");
							} 
							else 
							{
								FM_LOG_INFO("switch next Command execution Success\n");
							}

						}
						

						if(SIZE==3)
						{
							
							FM_LOG_INFO("[%s],signal_oprator_change_times is 3\n",__func__);
							//default_signal_threshold=-140;
							
							maxIndex=findMaxIndex(array, 0, 1, 2);
							FM_LOG_INFO("[%s],in sleep 300,maxIndex is %d\n",__func__,maxIndex);
							for (int i = 0; i < 3; i++) 
							{
							FM_LOG_INFO("%d ", array[i]);
							}
							FM_LOG_INFO("\n");
							if(maxIndex!=current_index)
							{
								int result=fm_sett_operator(oper_list[maxIndex]);
								if (result == 0) 
								{
									FM_LOG_INFO("switch Command to MAX Signal execution failed.\n");
									FM_LOG_INFO("waite for next.\n");
								} 
								else 
								{
									FM_LOG_INFO("switch Command to MAX Signal execution Success\n");
									FM_LOG_INFO("set max operator success\n");
								
								
									FM_SLEEP(10);
									
									
									memset(array, 0, sizeof(array));
									FM_LOG_INFO("signal check finished\n");
									weak_start_time= get_Time_Stamp();
									flag = 0 ;
								}
								
							
							}
							else
							{
								
								weak_start_time= get_Time_Stamp();
								memset(array, 0, sizeof(array));
								FM_LOG_INFO("signal check finished\n");
								flag = 0;
							}
						}
						
					}
					else
					{
							FM_LOG_INFO("[%s], current_signal_strength  is bigger\n", __func__);
					}  


				}
				else
				{      
						FM_LOG_INFO("get signal failed!\n");
				}
			}
			else
			{
			FM_LOG_INFO("weak signal change is closed\n");
			}
			FM_SLEEP(20);
		}
	 FM_SLEEP(30);
	 FM_LOG_INFO("weak signal is in sleepmod\n");
	}
}
int fm_mqtt_p2x_ReceiveLoop(void) 
{
	int status;

    FM_Pthread_Parameters fm_mqtt_p2x_ReceiveLoop_pm = {0};
    fm_mqtt_p2x_ReceiveLoop_pm.thread_attr = malloc(sizeof(osThreadAttr_t));

    fm_mqtt_p2x_ReceiveLoop_pm.func = fm_mqtt_p2x_ReceiveLoop_main;
    fm_mqtt_p2x_ReceiveLoop_pm.arg = NULL;
    fm_mqtt_p2x_ReceiveLoop_pm.thread_attr->name = "func_fm_mqtt_p2x_ReceiveLoop_main";
    fm_mqtt_p2x_ReceiveLoop_pm.thread_attr->stack_size = 10240;
    fm_mqtt_p2x_ReceiveLoop_pm.thread_attr->priority = osPriorityNormal;

    status = fm_pthread_create(&fm_mqtt_p2x_ReceiveLoop_pm);
    if (status == 0)
    {
        FM_LOG_INFO("fm_pthread_create fm_mqtt_p2x_ReceiveLoop_pm SUCCESS");
    } 
    else 
    {
        FM_LOG_INFO("fm_pthread_create fm_mqtt_p2x_ReceiveLoop_pm ERROR");
    }

    free(fm_mqtt_p2x_ReceiveLoop_pm.thread_attr);

	return status;
}
/*
int fm_mqtt_p2x_mqttcheck(void) 
{
	int status;

    FM_Pthread_Parameters fm_mqtt_p2x_mqttcheck_pm = {0};
    fm_mqtt_p2x_mqttcheck_pm.thread_attr = malloc(sizeof(osThreadAttr_t));

    fm_mqtt_p2x_mqttcheck_pm.func = fm_mqtt_p2x_mqttcheck_main;
    fm_mqtt_p2x_mqttcheck_pm.arg = NULL;
    fm_mqtt_p2x_mqttcheck_pm.thread_attr->name = "func_fm_mqtt_p2x_mqttcheck_main";
    fm_mqtt_p2x_mqttcheck_pm.thread_attr->stack_size = 10240;
    fm_mqtt_p2x_mqttcheck_pm.thread_attr->priority = osPriorityNormal;

    status = fm_pthread_create(&fm_mqtt_p2x_mqttcheck_pm);
    if (status == 0)
    {
        FM_LOG_INFO("fm_pthread_create fm_mqtt_p2x_mqttcheck_pm SUCCESS");
    } 
    else 
    {
        FM_LOG_INFO("fm_pthread_create fm_mqtt_p2x_mqttcheck_pm ERROR");
    }

    free(fm_mqtt_p2x_mqttcheck_pm.thread_attr);

	return status;
}

int fm_mqtt_p2x_policyprefer(void) 
{
	int status;

    FM_Pthread_Parameters fm_mqtt_p2x_policyprefer_pm = {0};
    fm_mqtt_p2x_policyprefer_pm.thread_attr = malloc(sizeof(osThreadAttr_t));

    fm_mqtt_p2x_policyprefer_pm.func = fm_mqtt_p2x_policyprefer_main;
    fm_mqtt_p2x_policyprefer_pm.arg = NULL;
    fm_mqtt_p2x_policyprefer_pm.thread_attr->name = "func_fm_mqtt_p2x_policyprefer_main";
    fm_mqtt_p2x_policyprefer_pm.thread_attr->stack_size = 10240;
    fm_mqtt_p2x_policyprefer_pm.thread_attr->priority = osPriorityNormal;

    status = fm_pthread_create(&fm_mqtt_p2x_policyprefer_pm);
    if (status == 0)
    {
        FM_LOG_INFO("fm_pthread_create fm_mqtt_p2x_policyprefer_pm SUCCESS");
    } 
    else 
    {
        FM_LOG_INFO("fm_pthread_create fm_mqtt_p2x_policyprefer_pm ERROR");
    }

    free(fm_mqtt_p2x_policyprefer_pm.thread_attr);

	return status;
}

int fm_mqtt_p2x_netcheck(void) 
{
	int status;

    FM_Pthread_Parameters fm_mqtt_p2x_netcheck_pm = {0};
    fm_mqtt_p2x_netcheck_pm.thread_attr = malloc(sizeof(osThreadAttr_t));

    fm_mqtt_p2x_netcheck_pm.func = fm_mqtt_p2x_netcheck_main;
    fm_mqtt_p2x_netcheck_pm.arg = NULL;
    fm_mqtt_p2x_netcheck_pm.thread_attr->name = "func_fm_mqtt_p2x_netcheck_main";
    fm_mqtt_p2x_netcheck_pm.thread_attr->stack_size = 10240;
    fm_mqtt_p2x_netcheck_pm.thread_attr->priority = osPriorityNormal;

    status = fm_pthread_create(&fm_mqtt_p2x_netcheck_pm);
    if (status == 0)
    {
        FM_LOG_INFO("fm_pthread_create fm_mqtt_p2x_netcheck_pm SUCCESS");
    } 
    else 
    {
        FM_LOG_INFO("fm_pthread_create fm_mqtt_p2x_netcheck_pm ERROR");
    }

    free(fm_mqtt_p2x_netcheck_pm.thread_attr);

	return status;
}
*/
int fm_mqtt_p2x_weaksignal(void)
{
   	int status;

    FM_Pthread_Parameters fm_mqtt_p2x_weaksignal_pm = {0};
    fm_mqtt_p2x_weaksignal_pm.thread_attr = malloc(sizeof(osThreadAttr_t));

    fm_mqtt_p2x_weaksignal_pm.func = fm_mqtt_p2x_weaksignal_main;
    fm_mqtt_p2x_weaksignal_pm.arg = NULL;
    fm_mqtt_p2x_weaksignal_pm.thread_attr->name = "func_fm_mqtt_p2x_weaksignal_main";
    fm_mqtt_p2x_weaksignal_pm.thread_attr->stack_size = 10240;
    fm_mqtt_p2x_weaksignal_pm.thread_attr->priority = osPriorityNormal;

    status = fm_pthread_create(&fm_mqtt_p2x_weaksignal_pm);
    if (status == 0)
    {
        FM_LOG_INFO("fm_pthread_create fm_mqtt_p2x_weaksignal_pm SUCCESS");
    } 
    else 
    {
        FM_LOG_INFO("fm_pthread_create fm_mqtt_p2x_weaksignal_pm ERROR");
    }

    free(fm_mqtt_p2x_weaksignal_pm.thread_attr);

	return status;
}
static int recvsub_id = 1;		//接受订阅报文ID

void mqtt_at_message_handle(MQTTContext_t *pContext, MQTTPacketInfo_t *pPacketInfo, MQTTDeserializedInfo_t *pDeserializedInfo) 
{
	//FM_LOG_INFO( "eventCallback: [%s], packetIdentifier: [%d]", (char *) MQTT_Status_strerror(pDeserializedInfo->deserializationResult), pDeserializedInfo->packetIdentifier );
    char *msg_payload = NULL;
	if( ( pPacketInfo->type & 0xF0 ) == MQTT_PACKET_TYPE_PUBLISH ) 
	{
		FM_LOG_INFO( "[%s], recvSub_id:[%d], packetIdentifier: [%d]", __func__, recvsub_id++, pDeserializedInfo->packetIdentifier );

	    if (pDeserializedInfo && pDeserializedInfo->pPublishInfo->pTopicName) 
	    {
	        FM_LOG_INFO("[%s], topicNameLength=[%d]", __func__, pDeserializedInfo->pPublishInfo->topicNameLength);
	        if (pDeserializedInfo->pPublishInfo->topicNameLength > 0) 
	        {
	            FM_LOG_INFO("[%s], pTopicName=[%s]", __func__, (char *) pDeserializedInfo->pPublishInfo->pTopicName);
	        }
	    }
	    if (pDeserializedInfo && pDeserializedInfo->pPublishInfo->pPayload) 
	    {    FM_LOG_INFO("[%s], payloadLength=[%d]", __func__, pDeserializedInfo->pPublishInfo->payloadLength);
	        if (pDeserializedInfo->pPublishInfo->payloadLength > 0)
	        {
	            FM_LOG_INFO("[%s], pPayload=[%s]", __func__, (char *) pDeserializedInfo->pPublishInfo->pPayload);
                
				msg_payload = (char *)malloc(pDeserializedInfo->pPublishInfo->payloadLength + 1);
                if (msg_payload == NULL)
                {
                    FM_LOG_INFO("malloc msg_payload failed");
                    return;
                }
                memset(msg_payload, 0, pDeserializedInfo->pPublishInfo->payloadLength + 1);
                memcpy(msg_payload, pDeserializedInfo->pPublishInfo->pPayload, pDeserializedInfo->pPublishInfo->payloadLength);
                msg_payload[pDeserializedInfo->pPublishInfo->payloadLength] = '\0';

				int ret =0;
				int found=0;
				char sim_state[32] = {0};
				cJSON *root= NULL;
				
				root=cJSON_Parse(msg_payload);
                if(root == NULL)
				{
			    FM_LOG_INFO("parse fail.\n");
				}
            else
           {
				//手动切卡
    			while(1){ 
				
                fm_get_sim_state(sim_state);
				if(strncmp(sim_state, "READY", strlen(sim_state)))
				{
					FM_LOG_INFO("[%s] Uninserted Card!", __func__);
					FM_SLEEP(1);
					continue;
				}
				FM_LOG_INFO("[%s] SIM Card OK", __func__);
				break;
				}
                

				//------------------------------//切网----------	
                cJSON *simSwitchToValue = cJSON_GetObjectItem(root, "sim_switch_to");
                if (simSwitchToValue == NULL) 
                {
                     FM_LOG_INFO("[%s], NOT switch NET", __func__);
                }
                else
				{
                const char *operator_value_net_str = simSwitchToValue->valuestring; // 假设您可以直接获取 cJSON 对象的整数值
                
				FM_LOG_INFO("[%s], valuestring=[%s]", __func__, simSwitchToValue->valuestring);
     			
				char temp_operator[100];
                strcpy(temp_operator, operator_value_net_str);

				FM_LOG_INFO("[%s], valuestring=[%s]", __func__, simSwitchToValue->valuestring);

				int result=fm_sett_operator(temp_operator);
				if (result == 0) 
				{
                    FM_LOG_INFO("switch Command execution failed.\n");
                } 
				else 
				{
                    FM_LOG_INFO("switch Command execution Success\n");
                }
				
				#if 0
				char current_operator[32] = {0};
                memset(current_operator, 0, 32);
                FM_SLEEP(2);
				 while(found==0)
			   {
				fm_status = fm_get_operator(current_operator);
				if (fm_status == FM_SUCCESS) { // 检查fm_get_operator是否成功
						for (int i = 0; i < 4; i++) {
							if (strcmp(current_operator, oper_list[i]) == 0) { // 使用strcmp比较字符串
								found = 1; // 找到匹配的运营商
								FM_LOG_INFO("GET SUCCESS current_operator in SIM SWITCH circle:[%s]", current_operator);
								
							}
						}
				}
						if (found==0) {
							FM_LOG_INFO("Operator not in list is unknow in SIM SWITCH circle: current_operator:[%s]", current_operator);
						}
		        }
					
					
					
				if (found){
				if (strcmp(operator_value_net_str, current_operator)!=0){
					if (strcmp(operator_value_net_str, "CHINA UNICOM") == 0) {
						FM_LOG_INFO("send switch-unicom success!");
						ope_index=0;
						
						fm_status = fm_set_operator(oper_list[ope_index]);
						if ( fm_status == FM_SUCCESS )
						{
								FM_LOG_INFO("[%s], switch CHINA UNICOM SUCCESS", __func__);
								FM_SLEEP(10);
						}else{
								FM_LOG_INFO("[%s], switch CHINA UNICOM ERROR", __func__);
						}
					} else if (strcmp(operator_value_net_str, "CHINA MOBILE") == 0) {
						FM_LOG_INFO("send switch-mobile success!");
						ope_index=1;
						fm_status = fm_set_operator(oper_list[ope_index]);
						if ( fm_status == FM_SUCCESS )
						{
								FM_LOG_INFO("[%s], switch CHINA_MOBILE SUCCESS", __func__);
								FM_SLEEP(10);
						}else{
								FM_LOG_INFO("[%s], switch CHINA_MOBILE ERROR", __func__);
						}
					} else if (strcmp(operator_value_net_str, "CHINA TELECOM") == 0) {
						FM_LOG_INFO("send switch-telecom success!");
						ope_index=2;
						fm_status = fm_set_operator(oper_list[ope_index]);
						if ( fm_status == FM_SUCCESS )
						{
								FM_LOG_INFO("[%s], switch CHINA_TELECOM SUCCESS", __func__);
								FM_SLEEP(10);
						}else{
								FM_LOG_INFO("[%s], switch CHINA_TELECOM ERROR", __func__);
						}
					}

				} else {
                        FM_LOG_INFO("Operator is same - no need switch - ");   
				}

				} else {
						// 处理未知的运营商字符串
						FM_LOG_INFO("send - UNKNOWN_OPERATOR- success!");
					}	
				#endif		
				}
               
    			#if 1
				//----------------------------------------//优先卡----
				
                cJSON *preferOperatorObj = cJSON_GetObjectItem(root, "policy_operator_first");
				
                 if (preferOperatorObj == NULL) 
                 {
                     FM_LOG_INFO("[%s], NOT operator NET", __func__);
                 }
                  else
                {
					cJSON *preferOperatorValue =cJSON_GetObjectItem(preferOperatorObj, "prefer_operator");    
			

					if (preferOperatorValue == NULL) 
					{
					FM_LOG_INFO("[%s], Error accessing prefer_operator value.", __func__);
					}
					else
					{  
					const char *operator_value_prefer_net_str = preferOperatorValue->valuestring;  // 假设您可以直接获取 cJSON 对象的整数值
					FM_LOG_INFO("[%s], pre_valuestring=[%s]", __func__, preferOperatorValue->valuestring);
//--------------------------------------卡指令版本---------------------				
					#if 0
					char temp_operator[100];
					strcpy(temp_operator, operator_value_prefer_net_str);

					FM_LOG_INFO("[%s], valuestring=[%s]", __func__, preferOperatorValue->valuestring);

					int result=fm_sett_prefer_policy(temp_operator);
					if (result == 0) 
					{
						FM_LOG_INFO("switch prefer Command execution failed.\n");
					} 
					else 
					{
						FM_LOG_INFO("switch prefer Command execution Success\n");
					}
					#endif
//-----------------------------------------CFG版本----------------------
					#if 1
					if (strcmp(operator_value_prefer_net_str, "CHINA UNICOM") == 0) {
						prefer_operator = CHINA_UNICOM_OPERATOR;
						
						FM_LOG_INFO("send unicom-first success!");
						

					} else if (strcmp(operator_value_prefer_net_str, "CHINA MOBILE") == 0) {
						prefer_operator = CHINA_MOBILE_OPERATOR;

						FM_LOG_INFO("send mobile-first success!");
						
					} else if (strcmp(operator_value_prefer_net_str, "CHINA TELECOM") == 0) {
						prefer_operator = CHINA_TELECOM_OPERATOR;

						FM_LOG_INFO("send telecom-first success!");
						

					} else if (strcmp(operator_value_prefer_net_str, "LAST TIME") == 0) {
						prefer_operator = LAST_TIME_OPERATOR;

						FM_LOG_INFO("send last-time success!");
						
					}
					else {
						// 处理未知的运营商字符串
						prefer_operator = UNKNOWN_OPERATOR;
						FM_LOG_INFO("[%s], prefer_operator is UNkNOW", __func__);
					}

					}
					
					fm_status = fm_set_prefer_operator_policy(prefer_operator);
					if ( fm_status == FM_SUCCESS )
					{
						FM_LOG_INFO("[%s], save prefer opetrator SUCCESS , fm_status:[%d]", __func__, fm_status);
					}
					else
					{
						FM_LOG_INFO("[%s], save prefer opetrator ERROR , fm_status:[%d]", __func__, fm_status);
					}
					
					#endif
					}
				
				
	
			
               //-------------------------------------//自动切卡
			 // 原始代码段的一部分
				

				// 修改后的代码段
				// 首先检查 network_lost 是否存在于 JSON 对象中

				cJSON *networkLostObj = cJSON_GetObjectItem(root, "policy_network_lost");
                 if (networkLostObj == NULL) 
                {
                    FM_LOG_INFO("[%s], NOT policy net lost.", __func__);
                }else{

                cJSON *cjson_policy_network_lost_network_lost=cJSON_GetObjectItem(networkLostObj, "network_lost");
				if (cjson_policy_network_lost_network_lost == NULL) 
                {

                    FM_LOG_INFO("[%s], NOT lost value.", __func__);
                }
                else
				{
				int network_lost_value = cjson_policy_network_lost_network_lost->valueint;
				FM_LOG_INFO("[%s], network_lost_value is [%d]", __func__,network_lost_value);
				// 根据network_lost的值设置fm_policy_status
				
				if (cjson_policy_network_lost_network_lost->valueint) {
					policy_status = FM_ENABLE_POLICY;
					FM_LOG_INFO("[%s], going to enable network_lost", __func__);
				} else {
					policy_status = FM_DISABLE_POLICY;
					FM_LOG_INFO("[%s], going to disable network_lost", __func__);
				}

				// 根据 fm_policy_status 设置丢失网络策略状态
				fm_status = fm_set_lost_network_policy_status(policy_status);
				if (fm_status == FM_SUCCESS) {
					FM_LOG_INFO("[%s], set lost_network_polic success", __func__);
				} else {
					FM_LOG_INFO("[%s], set lost_network_polic ERROR", __func__);
				}
							
			//每次使用后给pPayload值制0，防止数据遗留问题

			
				#endif
				}
				}
			
			    
            cJSON *weakSignalObj = cJSON_GetObjectItem(root, "policy_signal_weak");
           
            if (weakSignalObj == NULL) 
              {
                FM_LOG_INFO("[%s], NOT policy weak signal.", __func__);
              }
              else
           {
               cJSON *weakSignalValue = cJSON_GetObjectItem(weakSignalObj, "weak_signal_threshold");
               if (weakSignalValue == NULL) 
                {
                  FM_LOG_INFO("[%s], Error accessing weak signal value.", __func__);
                }
               else
                {  
                    int weak_signal_threshold = weakSignalValue->valueint;
					default_signal_threshold=weakSignalValue->valueint;
                    FM_LOG_INFO("[%s], weak signal value: [%d]", __func__, weak_signal_threshold);

                    //设置弱信号切换运营商策略状态
                    #if 1
                    fm_status = fm_set_weak_signal_threshold(weakSignalValue->valueint);
                    if ( fm_status == FM_SUCCESS )
                    {
                    FM_LOG_INFO("[%s], save weak signal threshold SUCCESS , fm_status:[%d]", __func__, fm_status);
                    }else{
                    FM_LOG_INFO("[%s], save weak signal threshold ERROR , fm_status:[%d]", __func__, fm_status);
                    }
                    #endif
                    #if 0                        
                    int weak_signal_threshold = fm_get_weak_signal_threshold();
                    FM_LOG_INFO("[%s], get weak signal threshold, weak_signal_threshold:[%d]", __func__, weak_signal_threshold);
                    char signal_weak_threshhold_str[16] = {0};
                    sprintf(signal_weak_threshhold_str, "%d", weak_signal_threshold);
                    FM_LOG_INFO("remote", "PolicySignalWeak_WeakSignalThreshold", "Success", signal_weak_threshhold_str);
                    #endif
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




void test_printf(void) 
{   
	FM_STATUS_t ret = FM_SUCCESS;
    int i = 0;
	int ret1 = -1;
    char *pub_msg_dataa=NULL;
    char sim_state[32] = {0};
    char imei[32] = {0};
    char iccid[32] = {0};
    int signal = -1;
    int pub_sleep = 0;
    uint32_t pub_start_time = 0;
	uint32_t pub_end_time = 0;
    char current_operator[32] = {0};
    int fly_card = -1;
    char vendor_name[32] = {0};
    char model_name[32] = {0};
    int current_index=-1;
	FM_STATUS_t ms_ret;
	FM_MQTT_SERVER_t mqtt_server = FM_MQTT_SERVER_UNKNOWN;
	FM_MQTT_SERVER_t set_mqtt_server = FM_MQTT_SERVER_UNKNOWN;
    int network_lost_times = 0;
    //获取默认的MQTT服务器
    FM_LOG_INFO("000 p2x mqtt_server:[%d]", mqtt_server);
	fm_get_mqtt_server(&mqtt_server);
    FM_LOG_INFO("111 p2x mqtt_server:[%d]", mqtt_server);

 	set_mqtt_server =FM_MQTT_SERVER_CUSTOM;
    ms_ret = fm_set_mqtt_server(set_mqtt_server);
    if ( ms_ret == FM_SUCCESS ) 
    {
        FM_LOG_INFO("333 p2x fm_set_mqtt_server SUCCESS");
    } 
    else 
    {
        FM_LOG_INFO("333 p2x fm_set_mqtt_server ERROR");
    }
    fm_get_mqtt_server(&mqtt_server);
    FM_LOG_INFO("333 p2x mqtt_server:[%d]", mqtt_server);

    MQTTStatus_t status;
    int fm_status;



    net_ctx.socket_fd = -1;
    if ( mqtt_server == FM_MQTT_SERVER_CUSTOM ) 
	{
        net_ctx.server_addr = MQTT_SERVER_ADDR;
        net_ctx.server_port = MQTT_SERVER_PORT;
    }

	
	// Set msg_fixed_buffer_buf members.
	msg_fixed_buffer.pBuffer = msg_fixed_buffer_buf;
	msg_fixed_buffer.size = MQTT_CLIENT_FIXED_BUF_LEN;

    //fm_mqtt_init 初始化MQTT
	status = fm_mqtt_init(&mqtt_ctx, &net_ctx, get_Time_Stamp, mqtt_at_message_handle, &msg_fixed_buffer);
	if(status != MQTTSuccess) 
    {
        FM_LOG_INFO("Init WRONG! erno:%d", status);
		goto EXIT;
    }
	FM_LOG_INFO("Init done!");

    fm_mqtt_context_init();

    connect_info.cleanSession = true;
    FM_LOG_INFO("[%s] fm_mqtt_context.connect_clientid:[%s]", __func__, fm_mqtt_context.connect_clientid);
    connect_info.pClientIdentifier = fm_mqtt_context.connect_clientid;		//fm_mqtt_context.connect_clientid
    connect_info.clientIdentifierLength = strlen(connect_info.pClientIdentifier);
    connect_info.keepAliveSeconds = 60;
    if ( mqtt_server == FM_MQTT_SERVER_CUSTOM ) 
	{
        connect_info.pUserName = MQTT_CONNECT_USERNAME;
        connect_info.userNameLength = strlen(connect_info.pUserName);
        connect_info.pPassword = MQTT_CONNECT_PASSWORD;
        connect_info.passwordLength = strlen(connect_info.pPassword);
    }
	while ( 1 )
	{
			//检查模组是否插卡
		fm_get_sim_state(sim_state);
			

		if(strncmp(sim_state, "READY", strlen(sim_state)))
		{
			FM_LOG_INFO("[%s] Uninserted Card!", __func__);
			FM_SLEEP(5);
			continue;
		}
		FM_LOG_INFO("[%s] SIM Card OK", __func__);
		break;
	}
    //时间初始化
    last_call_time=get_Time_Stamp();

    
	fm_mqtt_p2x_policyprefer();
    

		while(1)

	{  
		ret1 = fm_socket_check_net();
		
		if(ret1 != 1)
		{
			FM_LOG_INFO("[%s],  UN NETWORK connect! erno:%d", __func__, ret);
			FM_SLEEP(5);
		#if 1
			network_lost_times += 1;
			FM_LOG_INFO("[%s],  UN NETWORK connect! network_lost_times: %d", __func__, network_lost_times);
			if ( network_lost_times > 12 )
			{

				network_lost_times=0;

				int result1 = get_operator_from_sim(current_operator, sizeof(current_operator));
				if (result1 == 1) 
				{ // 检查fm_get_operator是否成功
					FM_LOG_INFO("Current operator is: %s", current_operator);
					current_index=find_operator_index(current_operator);
					if (current_index!=-1)
					{
						int result=at_command_set_operator_next(current_index);
						if (result == 0) 
						{
							FM_LOG_INFO("switch next Command execution failed in machine start NET LOST Circle .\n");
							FM_LOG_INFO("waite for next.\n");
						} 
						else 
						{
							FM_LOG_INFO("switch next Command execution Success in NET LOST Circle\n");
						}
					}
						else 
						{
								FM_LOG_INFO("GET Operator index failed\n");
						}
					}
					else
					{
						FM_LOG_INFO("Operator not found.\n");
					}
			
		#endif
					
		}
		continue;
		}
			network_lost_times = 0;
			FM_LOG_INFO("[%s],  check NETWORK connect! erno:%d", __func__, ret);
			break;
	}
	fm_status = fm_set_last_operator_policy();
	if ( fm_status == FM_SUCCESS ) 
	{
	FM_LOG_INFO("555 set SUCCESS last Register operator, fm_status:[%d]",fm_status);
	} 
	else 
	{
	FM_LOG_INFO("555 set ERROR last Register operator, fm_status:[%d]",fm_status);
	}
	while ( 1 ) 
	{
		//fm_mqtt_connect 连接MQTT平台
		status = fm_mqtt_connect(&mqtt_ctx, &net_ctx, &connect_info);
		if(status != MQTTSuccess) 
	    {
	        FM_LOG_INFO("Connect WRONG! erno:%d", status);
			FM_SLEEP(5);
			continue;
	    } 
		else 
		{    is_mqtt_connected = 1;
			FM_LOG_INFO("Connect done!");
		}
		
		
		//fm_mqtt_get_state 获取客户端与服务器的连接状态
		status = fm_mqtt_get_state(&mqtt_ctx);
		if(status != MQTTSuccess) 
	    {
	        FM_LOG_INFO("get_state WRONG! erno:%d", status);
			FM_SLEEP(5);
			continue;
	    } 
		else 
		{
			FM_LOG_INFO("MQTT connected done!");
		}
		break;
	}


	sub_info.qos = MQTT_CLIENT_SUB_QOS;
	    
	sub_info.pTopicFilter = fm_mqtt_context.sub_topic;
	sub_info.topicFilterLength = strlen( sub_info.pTopicFilter );
		
	//fm_mqtt_subscribe MQTT消息订阅主题
	status = fm_mqtt_subscribe(&mqtt_ctx, &sub_info);
	if(status != MQTTSuccess) 
    {
		FM_LOG_INFO("Sub WRONG! erno:%d", status);
		goto DISCONN;
    }
	FM_LOG_INFO("Sub done!");
    FM_LOG_INFO("[%s] fm_mqtt_context.sub_topic111:[%s]", __func__, fm_mqtt_context.sub_topic);


	//fm_mqtt_p2x_ReceiveLoop 创建新的任务,执行ReceiveLoop处理服务器返回的数据
	fm_status = fm_mqtt_p2x_ReceiveLoop();
	if (fm_status != 0) 
	{
		FM_LOG_INFO("fm_mqtt_p2x_ReceiveLoop WRONG! erno:[%d]", fm_status);
		goto UNSUB;
	}
	FM_LOG_INFO("fm_mqtt_p2x_ReceiveLoop done!");
   
	
     
    
    
   
   //------------------------断网切网策略开启----------------
   ret=fm_get_lost_network_policy_status(&policy_status);
   if ( ret == FM_SUCCESS ) 
    {
        FM_LOG_INFO("777 GET SUCCESS lost_network status:[%d]", policy_status);
    } 
    else 
    {
        FM_LOG_INFO("777 GET SUCCESS lost_network status:[%d], ret:[%d]", policy_status, ret);
    }
   #if 1
   //-------------------------弱信号策略开启------------
    ret=fm_get_weak_signal_threshold();
	if (ret==FM_SUCCESS)
	{
       FM_LOG_INFO("get_weak_signal_success\n");
	}else {
       FM_LOG_INFO("get_weak_signal_faiil\n"); 
	}
    ret=fm_mqtt_p2x_weaksignal();
	if (ret==FM_SUCCESS)
	{
       FM_LOG_INFO("pthread_weak_signal_start_success\n");
	}else {
       FM_LOG_INFO("pthread_weak_weak_signal_start_faiil\n"); 
	}
   #endif
while ( 1 )

{       FM_LOG_INFO("Enter main circle\n");
        create_heart_beat_msg(&pub_msg_dataa);

		// QoS of publish.
		pub_info.qos = MQTT_CLIENT_PUB_QOS;
		pub_info.pTopicName = fm_mqtt_context.pub_topic;
		pub_info.topicNameLength = strlen( pub_info.pTopicName );
		pub_info.pPayload = pub_msg_dataa;
		pub_info.payloadLength = strlen( pub_msg_dataa);
		
		//fm_mqtt_publish MQTT发布消息
		status = fm_mqtt_publish(&mqtt_ctx, &pub_info);
		if(status != MQTTSuccess)
		{
			FM_LOG_INFO("Pub WRONG! erno:%d", status);
		}
        else
        {
			FM_LOG_INFO("Pub done!");
		}
        free(pub_msg_dataa);

        pub_start_time = get_Time_Stamp();
        while( 1 )
		{   
			FM_LOG_INFO("Enter second circle\n");
			//fm_mqtt_get_state 获取客户端与服务器的连接状态
			status = fm_mqtt_get_state(&mqtt_ctx);
            //ret=fm_socket_check_net(); 

			if(status != FM_SUCCESS)
		    {
                is_mqtt_connected = 0;
		    	//客户端与服务器的连接状态异常处理
		        FM_LOG_INFO("get_state WRONG! erno:%d", status);
				ret = fm_mqtt_reconnect();
				if(ret != 1)//重连失败
				{
					FM_LOG_INFO("RECONNECT ERROR! erno:%d", ret);
				}
                else
                {
                    is_mqtt_connected = 1;
					FM_LOG_INFO("RECONNECT OK!");
				}
		    }
            else
            {
                if(is_mqtt_connected == 0) //上次重连失败，本次在进行一次重连
                {
                    ret = fm_mqtt_reconnect();
                    if(ret != 1)
                    {
                       FM_LOG_INFO("RECONNECT ERROR! erno:%d", ret);
                    }
                    else
                    {
                        is_mqtt_connected = 1;
                        FM_LOG_INFO("RECONNECT OK!");
                    }
                }
				FM_LOG_INFO("get_state done!");
			}
				
			pub_end_time = get_Time_Stamp();
            pub_sleep = (pub_end_time - pub_start_time)/ 1000;
			FM_LOG_INFO("pub_sleep:[%d] s", pub_sleep);
			//心跳间隔1h 3600s 上报一次
			if ( pub_sleep > 1800 )	//1h 3600s
			{
				break;
			}
            else
            {
                FM_SLEEP(2);
			}
		}
}
	


UNSUB:
	unsub_info.pTopicFilter = fm_mqtt_context.sub_topic;
	unsub_info.topicFilterLength = strlen( unsub_info.pTopicFilter );
	//fm_mqtt_unsubscribe MQTT取消订阅
	status = fm_mqtt_unsubscribe(&mqtt_ctx, &unsub_info);
	if(status != MQTTSuccess) 
	{
		FM_LOG_INFO(" unsub WRONG! erno:%d", status);
		goto DISCONN;
	}
	FM_LOG_INFO("unsub done!");

DISCONN:
    //fm_mqtt_disconnect 断开MQTT连接
	status = fm_mqtt_disconnect(&mqtt_ctx);
    if(status != MQTTSuccess) 
    {
        FM_LOG_INFO("disconn WRONG! erno:%d", status);
		goto EXIT;
    }
    FM_LOG_INFO("disconnect done!");
    is_mqtt_connected = 0;
EXIT:
    FM_LOG_INFO("exit done!");






    FM_LOG_INFO("END!!!!!!!!!!!!!!!!!!!!!");
}
