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
#define MQTT_CONNECT_CLIENTID	"mqttx_47842d0e"				//MQTT客户端ID
#define MQTT_CONNECT_USERNAME	"root"							//MQTT代理账号
#define MQTT_CONNECT_PASSWORD	"qx@123456"					//MQTT代理密码
#define MQTT_CONNECT_KEEP_ALIVE_SECONDS		60					//MQTT连接保活的时间

//MQTT client Sub
#define MQTT_CLIENT_SUB_QOS		MQTTQoS1						//MQTT订阅QOS级别 1
//#define MQTT_CLIENT_SUB_TOPIC	"JSON/FM/CPE/868998061757114"	//MQTT订阅主题 "JSON/设备厂商/设备型号/IMEI"
//#define MQTT_CLIENT_SUB_TOPIC_NUM	1						//MQTT订阅主题

//MQTT client Pub
#define MQTT_CLIENT_PUB_QOS		MQTTQoS1						//MQTT上报QOS级别 1
//#define MQTT_CLIENT_PUB_TOPIC	"JSON/Heartbeat/FM/CPE"			//MQTT上报主题 "JSON/Heartbeat/设备厂商/设备型号"
//#define MQTT_CLIENT_PUB_TOPIC_NUM	1

MQTTContext_t mqtt_ctx = {0x0};
NetworkContext_t net_ctx = {0x0};
MQTTPublishInfo_t pub_info = {0x0};


FM_NETWORK_OPERATOR_t prefer_operator = UNKNOWN_OPERATOR;//优先卡
FM_NETWORK_OPERATOR_t last_operator = UNKNOWN_OPERATOR;//上次驻网的运营商
FM_NETWORK_OPERATOR_t operator = UNKNOWN_OPERATOR;
FM_NETWORK_OPERATOR_t current_workingcard = UNKNOWN_OPERATOR;

static int ope_index = 0;
static char *oper_list[] = {"CHINA UNICOM", "CHINA MOBILE", "CHINA TELECOM"};

FM_STATUS_t fm_status = FM_UNKNOWN;
FM_POLICY_STATUS_t policy_status = FM_UNKNOWN_POLICY;

MQTTFixedBuffer_t msg_fixed_buffer = {0x0};
uint8_t msg_fixed_buffer_buf[MQTT_CLIENT_FIXED_BUF_LEN] = {0};
MQTTConnectInfo_t connect_info = {0x0};
MQTTSubscribeInfo_t sub_info = {0x0};
MQTTSubscribeInfo_t unsub_info = {0x0};



FM_NETWORK_OPERATOR_t string_to_operator(const char* operator_str) {
    if (strcmp(operator_str, "CHINA UNICOM") == 0) {
        return CHINA_UNICOM_OPERATOR;
    } else if (strcmp(operator_str, "CHINA MOBILE") == 0) {
        return CHINA_MOBILE_OPERATOR;
    } else if (strcmp(operator_str, "CHINA TELECOM") == 0) {
        return CHINA_TELECOM_OPERATOR;
    } 
      
}


int32_t fm_socket_check_net(void) 
{
	int sockfd;
    struct sockaddr_in server_addr;
    struct hostent *host;

	// 解析域名
    host = gethostbyname("www.baidu.com1");
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


char *get_publish_str(void)
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
    

	fm_get_sim_state(sim_state);
    fm_get_imei(imei);
	fm_get_iccid(iccid);
	fm_get_signal_strength(&signal);
	fm_get_operator(current_operator);
	fm_get_fly_card(&fly_card);
	fm_get_vendor_name(vendor_name);
    fm_get_model_name(model_name);

      


    cJSON_AddItemToObject(cjson_test, "payload_name", cJSON_CreateString("Heartbeat"));
    cJSON_AddItemToObject(cjson_test, "msg_version", cJSON_CreateString("1"));
    cJSON_AddItemToObject(cjson_test, "vendor_name", cJSON_CreateString(&vendor_name));
    cJSON_AddItemToObject(cjson_test, "model_name", cJSON_CreateString(&model_name));
  
   
    cJSON_AddItemToObject(cjson_test, "imei_data", cJSON_CreateString(imei));
    cJSON_AddItemToObject(cjson_test, "iccid_data", cJSON_CreateString(iccid));
    cJSON_AddItemToObject(cjson_test, "current_operator", cJSON_CreateString(current_operator));

    cJSON_AddItemToObject(cjson_test, "simcard_state", cJSON_CreateString(sim_state));
    cJSON_AddItemToObject(cjson_test, "heart_beat_interval", cJSON_CreateNumber(3600));
    cJSON_AddItemToObject(cjson_test, "client_version_name", cJSON_CreateString("SDK 3.0"));
    cJSON_AddItemToObject(cjson_test, "signal_rsrp", cJSON_CreateNumber(signal));


    // 将cjson_test对象转换为字符串
    char *json_string = cJSON_Print(cjson_test);

    // 输出JSON字符串
    //fm_p2x_print("%s\n", json_string);
    FM_LOG_INFO("[%s], json_string: [%s]", __func__, json_string);
    // 释放JSON字符串和对象
    
    
	
    return json_string;
   
	
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
static void fm_mqtt_p2x_mqttcheck_main(void *argument){
    MQTTStatus_t status;
	
	while(1){
	 FM_SLEEP(5);
	 status = fm_mqtt_get_state(&mqtt_ctx);
	if(status != MQTTSuccess)
	{

		FM_LOG_INFO("get_state WRONG! erno:%d", status);
		status = fm_mqtt_connect(&mqtt_ctx, &net_ctx, &connect_info);
		if(status != MQTTSuccess)
			{
				FM_LOG_INFO("MQTT ReConnect WRONG! erno:%d", status);
				
			}else{
				FM_LOG_INFO("MQTT ReConnect done!");
			}
		status = fm_mqtt_subscribe(&mqtt_ctx, &sub_info);
		if(status != MQTTSuccess)
		{
			FM_LOG_INFO("Sub WRONG! erno:%d", status);
		
		}
		FM_LOG_INFO("Sub done!");
	}else{

		FM_LOG_INFO("MQTT check connected done!");

	}
	}
}

static void fm_mqtt_p2x_policyprefer_main(void *argument) 
{       
	    FM_SLEEP(15);
		char current_operator[32] = {0};
		memset(current_operator, 0, 32);
	    
        //获取优先网络策略使用的运营商
	if (prefer_operator!=UNKNOWN_OPERATOR){
        fm_status = fm_get_prefer_operator_policy(&prefer_operator);
        if ( fm_status == FM_SUCCESS ) 
        {
            FM_LOG_INFO("111 GET SUCCESS prefer operator:[%d]", operator);
        } 
        else 
        {
            FM_LOG_INFO("111 GET ERROR prefer operator, ret:[%d]",  fm_status);
        }
        
		fm_status = fm_get_operator(current_operator);
		if ( fm_status == FM_SUCCESS ) 
			{
				FM_LOG_INFO("GET SUCCESS current_operator:[%s]", current_operator);
			} 
		else 
			{
				FM_LOG_INFO("GET ERROR current_operator:[%s], fm_status:[%d]", current_operator, fm_status);
			}

		FM_NETWORK_OPERATOR_t current_operator_enum = string_to_operator(current_operator);
         
        if (prefer_operator == LAST_TIME_OPERATOR){
			 
			fm_status = fm_get_last_operator_policy(&last_operator);
				if ( fm_status == FM_SUCCESS ) 
				{
					FM_LOG_INFO("444 GET SUCCESS last operator:[%d]", last_operator);
				} 
				else 
				{
					FM_LOG_INFO("444 GET ERROR last operator, ret:[%d]",fm_status);
				}

				if(current_operator_enum!=last_operator){
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
        else if(prefer_operator !=current_operator_enum ){
	
      	if (prefer_operator == CHINA_UNICOM_OPERATOR)
		{   fm_status= fm_set_operator(oper_list[0]);
		    if ( fm_status == FM_SUCCESS )
				{
					FM_LOG_INFO("[%s], switch CHINA_UNICOM SUCCESS", __func__);
					FM_SLEEP(10);
				}else{
					FM_LOG_INFO("[%s], switch CHINA_UNICOM ERROR", __func__);
					 }
		}
		else if (prefer_operator == CHINA_MOBILE_OPERATOR)
		{   fm_status = fm_set_operator(oper_list[1]);
		    if ( fm_status == FM_SUCCESS ) {
			FM_LOG_INFO("[%s],SET NET prefer Operator is :[CHINA_MOBILE_OPERATOR]", __func__);
			}
		}else if (prefer_operator == CHINA_TELECOM_OPERATOR)
		{   fm_status = fm_set_operator(oper_list[2]);
		    if ( fm_status == FM_SUCCESS ) {
			FM_LOG_INFO("[%s], SET NET prefer Operator is :[CHINA_TELECOM_OPERATOR]", __func__);
			}
		} 
		}else {
			FM_LOG_INFO("[%s],prefer and current operator is same no need switch", __func__);
		}

		FM_SLEEP(20);
		fm_status = fm_set_last_operator_policy();
		if ( fm_status == FM_SUCCESS ) 
			{
			FM_LOG_INFO("555 set SUCCESS last operator, fm_status:[%d]",fm_status);
			} 
			else 
			{
			FM_LOG_INFO("555 set ERROR last operator, fm_status:[%d]",fm_status);
			}


}else {
	    FM_LOG_INFO("Policy prefer operator is unknow");
}
}
static void fm_mqtt_p2x_netcheck_main(void *argument) 
{
   int ret =0;



   
   
	while(1)

{       FM_SLEEP(2);
		ret=fm_get_lost_network_policy_status(&policy_status);
		if ( ret == FM_SUCCESS ) 
		{
			FM_LOG_INFO("777 GET SUCCESS lost_network status:[%d]", policy_status);
		} 
		else 
		{
			FM_LOG_INFO("777 GET SUCCESS lost_network status:[%d], ret:[%d]", policy_status, ret);
		}
	if (policy_status==FM_ENABLE_POLICY){  	
		FM_LOG_INFO("777 GET SUCCESS lost_network is enabled:[%d]", policy_status);

	for(int32_t i=0;i<3;i++){
	 FM_SLEEP(10);
	 ret = fm_socket_check_net();
        if ( ret == 1 ) 
        {
            FM_LOG_INFO("NETWORK OK!!!");
			i=0;
        } 
        else 
        {
            FM_LOG_INFO("NETWORK ERROR!!!");
			
        }
   
	}  
    
	char current_operator[32] = {0};

	memset(current_operator, 0, 32);

	
	fm_status = fm_get_operator(current_operator);
	if ( ret == FM_SUCCESS ) 
	{
		FM_LOG_INFO("net check GET SUCCESS current_card:[%s]", current_operator);
	} 
	else 
		{
		FM_LOG_INFO("net check GET ERROR current_card:[%s], ret:[%d]", current_operator, fm_status);
		}       
	
	if (strcmp(current_operator, "CHINA UNICOM") == 0) {
			FM_LOG_INFO("net check send switch-Mobile success!");			
			fm_status = fm_set_operator(oper_list[1]);
		if ( fm_status == FM_SUCCESS )
		{
			FM_LOG_INFO("[%s], net check switch CHINA Mobile SUCCESS", __func__);
			FM_SLEEP(15);
		}else{
			FM_LOG_INFO("[%s],net check  switch CHINA Mobile ERROR", __func__);
			 }
		} else if (strcmp(current_operator, "CHINA MOBILE") == 0) {
			FM_LOG_INFO("send switch-CHINA_TELECOM success!");
			fm_status = fm_set_operator(oper_list[2]);
			if ( fm_status == FM_SUCCESS )
			{
			FM_LOG_INFO("[%s],net check  switch CHINA_TELECOM SUCCESS", __func__);
			FM_SLEEP(15);
			}else{
			    FM_LOG_INFO("[%s],net check  switch CHINA_TELECOM ERROR", __func__);
			}
			} else if (strcmp(current_operator, "CHINA TELECOM") == 0) {
				FM_LOG_INFO("send switch-CHINA_UNICOM success!");
				
			fm_status = fm_set_operator(oper_list[0]);
			if ( fm_status == FM_SUCCESS )
			{
			FM_LOG_INFO("[%s], net check switch CHINA_UNICOM SUCCESS", __func__);
			FM_SLEEP(15);
			}else{
			FM_LOG_INFO("[%s], net check switch CHINA_UNICOM ERROR", __func__);
			}
			}
	}

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
static int recvsub_id = 1;		//接受订阅报文ID

void mqtt_at_message_handle(MQTTContext_t *pContext, MQTTPacketInfo_t *pPacketInfo, MQTTDeserializedInfo_t *pDeserializedInfo) 
{
	//FM_LOG_INFO( "eventCallback: [%s], packetIdentifier: [%d]", (char *) MQTT_Status_strerror(pDeserializedInfo->deserializationResult), pDeserializedInfo->packetIdentifier );

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
                
				int ret =0;
				cJSON *cjson_test = NULL;
                cJSON *cjson_sim_switch_to= NULL;
                cJSON *cjson_policy_operator_first = NULL;
				cJSON *cjson_policy_operator_first_prefer_operator = NULL;

				cJSON *cjson_policy_network_lost = NULL;
				cJSON *cjson_policy_network_lost_network_lost = NULL;
				char *message =	NULL;
                
				message=pDeserializedInfo->pPublishInfo->pPayload;
                FM_LOG_INFO("JSON string starting from first bracket: %s\n", message);
			    cjson_test=cJSON_Parse(message);
                if(cjson_test == NULL)
				{
			    FM_LOG_INFO("parse fail.\n");
			    return -1;
				}
				//手动切卡
    			cjson_sim_switch_to = cJSON_GetObjectItem(cjson_test, "sim_switch_to");
                //优先卡
				cjson_policy_operator_first = cJSON_GetObjectItem(cjson_test, "policy_operator_first");
				cjson_policy_operator_first_prefer_operator = cJSON_GetObjectItem(cjson_policy_operator_first, "prefer_operator");
                //自动切卡使能
                cjson_policy_network_lost = cJSON_GetObjectItem(cjson_test, "policy_network_lost");
				cjson_policy_network_lost_network_lost = cJSON_GetObjectItem(cjson_policy_network_lost, "network_lost");

				//------------------------------//切网----------	
                const char *operator_value_net_str = cjson_sim_switch_to->valuestring; // 假设您可以直接获取 cJSON 对象的整数值
                
				FM_LOG_INFO("[%s], valuestring=[%s]", __func__, cjson_sim_switch_to->valuestring);
     			
				char current_operator[32] = {0};

				memset(current_operator, 0, 32);

				
				fm_status = fm_get_operator(current_operator);
				if ( ret == FM_SUCCESS ) 
				{
					FM_LOG_INFO("GET SUCCESS current_operator:[%s]", current_operator);
				} 
				else 
				{
				FM_LOG_INFO("GET ERROR current_operator:[%s], ret:[%d]", current_operator, fm_status);
				}
                  

                
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
					} else {
						// 处理未知的运营商字符串
						FM_LOG_INFO("send - UNKNOWN_OPERATOR- success!");
					}

				} else {
                        FM_LOG_INFO("Operator is same - no need switch - ");   
				}

								

               
    			#if 1
				//----------------------------------------//优先卡----
				
                
				const char *operator_value_prefer_net_str = cjson_policy_operator_first_prefer_operator->valuestring;  // 假设您可以直接获取 cJSON 对象的整数值
                FM_LOG_INFO("[%s], pre_valuestring=[%s]", __func__,  cjson_policy_operator_first_prefer_operator->valuestring);

				
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

              
              
            
	
			
               //-------------------------------------//自动切卡
			 // 原始代码段的一部分
				

				// 修改后的代码段
				// 首先检查 network_lost 是否存在于 JSON 对象中
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

				FM_SLEEP(20);
				fm_status = fm_set_last_operator_policy();
				if (fm_status == FM_SUCCESS ) 
				{
					FM_LOG_INFO("555 set SUCCESS last operator, ret:[%d]", fm_status);
				} 
				else 
				{
					FM_LOG_INFO("555 set ERROR last operator, ret:[%d]",fm_status);
				}
				#endif
			memset((void *)pDeserializedInfo->pPublishInfo->pPayload, 0, pDeserializedInfo->pPublishInfo->payloadLength);
	    }
		}	
		}
}


uint32_t get_Time_Stamp() {
    UINT32 ticks = 0;
    UINT32 mseconds = 0;

    ticks = osKernelGetTickCount ();
    mseconds = ticks * 5;
    return mseconds;
}



void test_printf(void) 
{   
	FM_STATUS_t ret = FM_SUCCESS;
    int i = 0;

    char sim_state[32] = {0};
    char imei[32] = {0};
    char iccid[32] = {0};
    int signal = -1;
    
    int fly_card = -1;
    char vendor_name[32] = {0};
    char model_name[32] = {0};
    
	FM_STATUS_t ms_ret;
	FM_MQTT_SERVER_t mqtt_server = FM_MQTT_SERVER_UNKNOWN;
	FM_MQTT_SERVER_t set_mqtt_server = FM_MQTT_SERVER_UNKNOWN;

    //获取默认的MQTT服务器
    FM_LOG_INFO("000 p2x mqtt_server:[%d]", mqtt_server);
	fm_get_mqtt_server(&mqtt_server);
    FM_LOG_INFO("111 p2x mqtt_server:[%d]", mqtt_server);

 	set_mqtt_server = FM_MQTT_SERVER_CUSTOM;
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


    connect_info.cleanSession = true;
    //FM_LOG_INFO("[%s] fm_mqtt_context.connect_clientid:[%s]", __func__, fm_mqtt_context.connect_clientid);
    connect_info.pClientIdentifier = MQTT_CONNECT_CLIENTID;		//fm_mqtt_context.connect_clientid
    connect_info.clientIdentifierLength = strlen(connect_info.pClientIdentifier);
    connect_info.keepAliveSeconds = MQTT_CONNECT_KEEP_ALIVE_SECONDS;
    if ( mqtt_server == FM_MQTT_SERVER_CUSTOM ) 
	{
        connect_info.pUserName = MQTT_CONNECT_USERNAME;
        connect_info.userNameLength = strlen(connect_info.pUserName);
        connect_info.pPassword = MQTT_CONNECT_PASSWORD;
        connect_info.passwordLength = strlen(connect_info.pPassword);
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
		{
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
	//FM_LOG_INFO("[%s] fm_mqtt_context.sub_topic111:[%s]", __func__, fm_mqtt_context.sub_topic);    
	sub_info.pTopicFilter = "ML307R/001";
	sub_info.topicFilterLength = strlen( sub_info.pTopicFilter );
		
	//fm_mqtt_subscribe MQTT消息订阅主题
	status = fm_mqtt_subscribe(&mqtt_ctx, &sub_info);
	if(status != MQTTSuccess) 
    {
		FM_LOG_INFO("Sub WRONG! erno:%d", status);
		goto DISCONN;
    }
	FM_LOG_INFO("Sub done!");



	//fm_mqtt_p2x_ReceiveLoop 创建新的任务,执行ReceiveLoop处理服务器返回的数据
	fm_status = fm_mqtt_p2x_ReceiveLoop();
	if (fm_status != 0) 
	{
		FM_LOG_INFO("fm_mqtt_p2x_ReceiveLoop WRONG! erno:[%d]", fm_status);
		goto UNSUB;
	}
	FM_LOG_INFO("fm_mqtt_p2x_ReceiveLoop done!");
    //MQTT保活
    fm_status = fm_mqtt_p2x_mqttcheck();
	if (fm_status != 0) 
	{
		FM_LOG_INFO("fm_mqtt_p2x_mqttcheck WRONG! erno:[%d]", fm_status);
		goto UNSUB;
	}
	FM_LOG_INFO("fm_mqtt_p2x_mqttcheck done!");

     
    fm_status = fm_mqtt_p2x_policyprefer();
	if (fm_status != 0) 
	{
		FM_LOG_INFO("fm_mqtt_p2x_policyprefer WRONG! erno:[%d]", fm_status);
		goto UNSUB;
	}
	FM_LOG_INFO("fm_mqtt_p2x_policyprefer done!");

    fm_status = fm_mqtt_p2x_netcheck();
	if (fm_status != 0) 
	{
		FM_LOG_INFO("fm_mqtt_p2x_netcheck WRONG! erno:[%d]", fm_status);
		goto UNSUB;
	}
	FM_LOG_INFO("fm_mqtt_p2x_netcheck done!");
/*
	while ( 1 ) 
	{
		// QoS of publish.
		char *json_string = get_publish_str();
		pub_info.qos = MQTT_CLIENT_PUB_QOS;
		//FM_LOG_INFO("[%s] fm_mqtt_context.pub_topic111:[%s]", __func__, fm_mqtt_context.pub_topic);
		pub_info.pTopicName = "ML307R/002";
		pub_info.topicNameLength = strlen( pub_info.pTopicName );
		pub_info.pPayload = json_string;
		pub_info.payloadLength = strlen( pub_info.pPayload );
		
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
         
		FM_SLEEP(5);
	}
*/

UNSUB:
	unsub_info.pTopicFilter = "ML307R/001";
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

EXIT:
    FM_LOG_INFO("exit done!");






    FM_LOG_INFO("END!!!!!!!!!!!!!!!!!!!!!");
}
