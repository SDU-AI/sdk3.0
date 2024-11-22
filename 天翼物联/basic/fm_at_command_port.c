/************************************************************************************************
 * Copyright (C), Flymodem (Shenzhen) Technology Co., Ltd.
 ************************************************************************************************
 * File Name  : fm_at_command_port.c
 * Abstract   : at_command接口实现FM AT命令功能
 * Version    : 3.1.0
 * Author     : LTH
 * Date       : 2024-08-08
 * Modifies   : [2024-08-08]新增[Author:LTH](无)
************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "fm_at_command_port.h"
#include "send_at.h"

int CtAtCmdProcess(const char *cmd, unsigned int cmdLen, char *resp, unsigned int respBufLen)
{
    int ret = 0;
    printf("----------at cmd process----------\n");
    //printf("cmd:%s\n", cmd);
    ret = AtCmdProcess2(cmd, cmdLen, resp, respBufLen);
    if (ret > 0)
    {
        resp[ret] = '\0';
        ret = 0;
    }
else{
       ret = -1;
}
    printf("----------at cmd process end----------\n");
    return ret;
}
/*
int SendATCMDWaitResp(int sATPInd,char *in_str, int timeout, char *ok_fmt, int ok_flag, char *err_fmt, char *out_str, int resplen){
	
	return 0;
}*/
int load_at_utils()
{
	return 0;
} 
int unload_at_utils()
{
	return 0;
}


#if FM_SYSTEM == FM_SYSTEM_RTOS_1803S
int SendATCMDWaitResp(int sATPInd,char *in_str, int timeout, char *ok_fmt, int ok_flag, char *err_fmt, char *out_str, int resplen){
	return 0;
}
int load_at_utils()
{
	return 0;
} 
int unload_at_utils()
{
	return 0;
}
extern char ICCID[10];

#if 0
typedef enum RESP_STATUS 
{
	RESP_OK = 0,
	RESP_CME_ERROR,
	RESP_INVALID_PARAM,
	RESP_TIME_OUT,
	RESP_SYS_ERROR,
}E_RESP_STATUS;

/* Log module type. */
typedef enum 
{
    LOG_MODULE_BT           = 0x00,
    LOG_MODULE_LWIP         = 0x01,
    LOG_MODULE_LFS          = 0x02,
    LOG_MODULE_MAX          = 0xFF
}Log_Module_Type;

extern void UARTLogPrintf_Extend(Log_Module_Type cat, const char *fmt, ...);

#define CPUartLogPrintf(fmt,args...)                                                    \
{                                                                                       \
    UARTLogPrintf_Extend(LOG_MODULE_MAX, fmt, ##args);                              \
}
#endif

#elif FM_SYSTEM == FM_SYSTEM_LINUX_M24A
//#include "at_adaptor_zte/at_utils.h"

static int SendATCMDWaitResp(char *cmd_str, void *response) 
{
	return CtAtCmdProcess(cmd_str, strlen(cmd_str), response, 256);
}

#elif FM_SYSTEM == FM_SYSTEM_RTOS_ML307R
//typedef signed int __int32_t;
typedef __int32_t int32_t ;
//typedef unsigned char __uint8_t ;
typedef __uint8_t uint8_t ;
//typedef unsigned int __uint32_t;
typedef __uint32_t uint32_t ;

extern int32_t cm_virt_at_send_sync(const uint8_t *at_cmd, uint8_t *rsp, int32_t *rsp_len, uint32_t timeout);
#endif




//函数功能：格式化字符串，去除字符串中的 【空格、引号、回车、换行符】
void fm_format_string(char *str) 
{
	char *ptr1 = str, *ptr2 = str;
	while (*ptr2) 
	{
		if (*ptr2 != ' ' && *ptr2 != '\'' && *ptr2 != '\"' && *ptr2 != '\r' && *ptr2 != '\n') 
		{
			*ptr1 = *ptr2;
			ptr1++;
		}
		ptr2++;
	}
	*ptr1 = '\0';
}

#if FM_SYSTEM == FM_SYSTEM_LINUX_M24A
static void calculate_rssi_val(int *rssi) 
{
	if (*rssi == 99) 
	{
		*rssi = 0;
	} 
	else 
	{
		*rssi -= 111;
	}
}

static void calculate_rsrq_val(int *rsrq) 
{
	if (*rsrq == 255) 
	{
		*rsrq = 0;
	} 
	else 
	{
		*rsrq = (*rsrq-40)/2;
	}
}

static void calculate_rsrp_val(int *rsrp) 
{
	if (*rsrp == 255) 
	{
		*rsrp = 0;
	} 
	else 
	{
		*rsrp -= 141;
	}
}

static void calculate_sinr_val(char *r_str, int *sinr) 
{
    char *token;
    float numbers[4];  // 存放最终的小数结果
    int idx = 0;       // 数组索引
    int num1, num2;    // 用于存储每两个数字
    int pairs_count = 0;  // 实际使用的小数对计数

    // 使用 strtok 来分割字符串，确保输入的字符串是可修改的
    token = strtok(r_str, ",");
    while (token != NULL && pairs_count < 4) 
	{
        if (idx % 2 == 0) 
		{
            // 第一个数字
            num1 = atoi(token);
        } 
		else 
		{
            // 第二个数字，进行组合转换
            num2 = atoi(token);
            if (pairs_count < 4) 
			{ // 确保不会越界
                numbers[pairs_count] = num1 + num2 / 10.0;
                pairs_count++;
            }
        }
        token = strtok(NULL, ",");
        idx++;
    }

    if (pairs_count > 0) 
	{ // 确保至少有一个数字对被处理
        float sum = 0;
        for (int i = 0; i < pairs_count; i++) 
		{
            sum += numbers[i];
        }
        *sinr = (int)(sum / pairs_count);
    } 
	else 
	{
        *sinr = 0; // 没有有效数据时返回 0
    }
}
#endif


//函数功能：获取sim卡状态
//参数：sim_state - 指向sim卡状态数据指针
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_get_sim_state(char *sim_state) 
{
#if FM_SYSTEM == FM_SYSTEM_RTOS_1803S
	int ret = 0;
	char res_str[256] = {0};
	char at_str[64] = {0};

    if (sim_state == NULL) 
	{
        return FM_PARAM_NULL;
    }

	sprintf(at_str, "AT+CPIN?\r\n");
	ret = SendATCMDWaitResp(8, at_str, 150, NULL, 1, "+CME ERROR", res_str, sizeof(res_str));		//+CPIN: READY
	if (ret == 0) 
	{
		if (strstr(res_str, "+CME ERROR")) 
		{
			return FM_AT_RET_CME_ERROR;
		} 
		else if (strstr(res_str, "+CMS ERROR")) 
		{
			return FM_AT_RET_CMS_ERROR;
		} 
		else if (strstr(res_str, "ERROR")) 
		{
			return FM_AT_RET_ERROR;
		} 
		else if (strstr(res_str, "+CPIN: ")) 
		{
			if (sscanf(res_str, "+CPIN: %s", sim_state) != 1) 
			{
				return FM_STRING_PARSING_FAIL;
			} 
			else 
			{
				return FM_SUCCESS;
			}
		} 
		else 
		{
			return FM_AT_RET_OTHER_ERROR;
		}
	} 
	else 
	{
		return FM_AT_EXECUTE_FAIL;
	}
#elif FM_SYSTEM == FM_SYSTEM_LINUX_M24A
	int ret = 0;
	char res_str[256] = {0};
	char at_str[64] = {0};
	
	if (sim_state == NULL) 
	{
        return FM_PARAM_NULL;
    }

    if (load_at_utils() != 0) 
	{
		ret = FM_LOAD_AT_UTILS_ERROR;
    }

	sprintf(at_str, "AT+CPIN?\r\n");
	ret = SendATCMDWaitResp(at_str, res_str);		//READY
	if (ret == 0) 
	{
		if (res_str == NULL || res_str[0] == '\0' || strcmp(res_str, "") == 0 || strcmp(res_str, "0") == 0) 
		{
			ret = FM_AT_RET_OTHER_ERROR;
		} 
		else 
		{
			strncpy(sim_state, res_str, strlen(res_str));
			ret = FM_SUCCESS;
		}
	} 
	else 
	{
		ret = FM_AT_EXECUTE_FAIL;
	}

	unload_at_utils();

	return (FM_STATUS_t)ret;
#elif FM_SYSTEM == FM_SYSTEM_RTOS_ML307R
	int ret = 0;

	char at_str[64] = {0};
	uint8_t res_str[256] = {0};
	int32_t *res_len = NULL;

	if (sim_state == NULL) 
	{
        return FM_PARAM_NULL;
    }

    res_len = malloc(sizeof(int32_t));

	sprintf(at_str, "AT+CPIN?\r\n");
	ret = cm_virt_at_send_sync((const uint8_t *)at_str, res_str, res_len, 32);			//+CPIN: READY
    if(ret == 0) 
    {
		if (strstr((char *)res_str, "+CME ERROR")) 
		{
			ret = FM_AT_RET_CME_ERROR;
		} 
		else if (strstr((char *)res_str, "+CMS ERROR")) 
		{
			ret = FM_AT_RET_CMS_ERROR;
		} 
		else if (strstr((char *)res_str, "ERROR")) 
		{
			ret = FM_AT_RET_ERROR;
		} 
		else if (strstr((char *)res_str, "+CPIN: ")) 
		{
			if (sscanf((char *)res_str, "+CPIN: %s", sim_state) != 1) 
			{
				ret = FM_STRING_PARSING_FAIL;
			} 
			else 
			{
				ret = FM_SUCCESS;
			}
		} 
		else 
		{
			ret = FM_AT_RET_OTHER_ERROR;
		}
    } 
	else 
	{
		ret = FM_AT_EXECUTE_FAIL;
	}

    free(res_len);

	return (FM_STATUS_t)ret;
#endif
}

//函数功能：查询模组imei
//参数：imei - 指向模组imei数据指针
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_get_imei(char *imei) 
{
#if FM_SYSTEM == FM_SYSTEM_RTOS_1803S
	int ret = 0;
	char res_str[256] = {0};
	char at_str[64] = {0};
	
    if (imei == NULL) 
	{
        return FM_PARAM_NULL;
    }

	sprintf(at_str, "AT+CGSN\r\n");
	ret = SendATCMDWaitResp(8, at_str, 150, NULL, 1, "+CME ERROR", res_str, sizeof(res_str));		//+CGSN: 863579060466561
	if (ret == 0) 
	{
		if (strstr(res_str, "+CME ERROR")) 
		{
			return FM_AT_RET_CME_ERROR;
		} 
		else if (strstr(res_str, "+CMS ERROR")) 
		{
			return FM_AT_RET_CMS_ERROR;
		} 
		else if (strstr(res_str, "ERROR")) 
		{
			return FM_AT_RET_ERROR;
		} 
		else if (strstr(res_str, "+CGSN: ")) 
		{
			if (sscanf(res_str, "+CGSN: %s", imei) != 1) 
			{
				return FM_STRING_PARSING_FAIL;
			} 
			else 
			{
				return FM_SUCCESS;
			}
		} 
		else 
		{
			return FM_AT_RET_OTHER_ERROR;
		}
	} 
	else 
	{
		return FM_AT_EXECUTE_FAIL;
	}

#elif FM_SYSTEM == FM_SYSTEM_LINUX_M24A
	int ret = 0;
	char res_str[256] = {0};
	char at_str[64] = {0};
	char *ptr,*token = NULL;
    if (imei == NULL) 
	{
        return FM_PARAM_NULL;
    }

    if (load_at_utils() != 0) 
	{
		ret = FM_LOAD_AT_UTILS_ERROR;
    }

	sprintf(at_str, "AT+CGSN\r\n");
	ret = SendATCMDWaitResp(at_str, res_str);		//863579060466561
	printf("*** imei ret: %d\r\n", ret);	
	if (ret == 0) 
	{
		printf("imei resp: %s\r\n", res_str);
		if (res_str == NULL || res_str[0] == '\0' || strcmp(res_str, "") == 0 || strcmp(res_str, "0") == 0) 
		{
			ret = FM_AT_RET_OTHER_ERROR;
		} 
		else 
		{
			printf("imei 1resp1: %s\r\n", res_str);
			 /*ptr  = strstr(res_str, "+CGSN: ");
			 printf("ptr is %s ",ptr);
			if(ptr != NULL){
            token = strtok_r(ptr + 8, " \"", &ptr);
			imei[strlen(token)] = '\0';
			*/

			//strncpy(imei, res_str, strlen(res_str)-6);
			 char *imeiStart = strstr(res_str, "8"); // 假设IMEI以数字8开头，根据实际情况调整
			printf("imeistart is %s\r", imeiStart); 
			if(imeiStart!=NULL){
			
			strncpy(imei, imeiStart,15);
			printf("******imei:%s\r\n", imei);
			
			
			ret = FM_SUCCESS;
			}else{
				ret = FM_AT_RET_OTHER_ERROR;
			}
			
		}
	} 
	else 
	{
		ret = FM_AT_EXECUTE_FAIL;
	}

	unload_at_utils();

	return (FM_STATUS_t)ret;
#elif FM_SYSTEM == FM_SYSTEM_RTOS_ML307R
	int ret = 0;

	char at_str[64] = {0};
	uint8_t res_str[256] = {0};
	int32_t *res_len = NULL;

	if (imei == NULL) 
	{
        return FM_PARAM_NULL;
    }

    res_len = malloc(sizeof(int32_t));

	sprintf(at_str, "AT+CGSN=1\r\n");
	ret = cm_virt_at_send_sync((const uint8_t *)at_str, res_str, res_len, 32);			//+CGSN: 864979076389923
    if(ret == 0) 
    {
		if (strstr((char *)res_str, "+CME ERROR")) 
		{
			ret = FM_AT_RET_CME_ERROR;
		} 
		else if (strstr((char *)res_str, "+CMS ERROR")) 
		{
			ret = FM_AT_RET_CMS_ERROR;
		} 
		else if (strstr((char *)res_str, "ERROR")) 
		{
			ret = FM_AT_RET_ERROR;
		} 
		else if (strstr((char *)res_str, "+CGSN: ")) 
		{
			if (sscanf((char *)res_str, "+CGSN: %s", imei) != 1) 
			{
				ret = FM_STRING_PARSING_FAIL;
			} 
			else 
			{
				ret = FM_SUCCESS;
			}
		} 
		else 
		{
			ret = FM_AT_RET_OTHER_ERROR;
		}
    } 
	else 
	{
		ret = FM_AT_EXECUTE_FAIL;
	}

    free(res_len);

	return (FM_STATUS_t)ret;
#endif
}

//函数功能：查询sim卡iccid
//参数：iccid - 指向sim卡iccid数据指针
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_get_iccid(char *iccid) 
{
#if FM_SYSTEM == FM_SYSTEM_RTOS_1803S
	char tmp[21] = {0};
	char iccid_str[21] = {0};
	int iccid_ready = 0;
	int i = 0;

    if (iccid == NULL) 
	{
        return FM_PARAM_NULL;
    }

	snprintf(iccid_str, 21, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
	ICCID[0], ICCID[1], ICCID[2], ICCID[3],
	ICCID[4], ICCID[5], ICCID[6], ICCID[7],
	ICCID[8], ICCID[9]);

	for (i = 0; i < 20; i++) 
	{
		if (*(iccid_str + i) != '0') 
		{
			iccid_ready = 1;
			break;
		}
	}

	if (iccid_ready) 
	{
		for (i = 0; i < 10; i++) 
		{
			tmp[2 * i] = *(iccid_str + 2 * i + 1);
			tmp[2 * i + 1] = *(iccid_str + 2 * i);
		}
	}

	strcpy(iccid, tmp);

	return FM_SUCCESS;
#elif FM_SYSTEM == FM_SYSTEM_LINUX_M24A
	int ret = 0;
	char res_str[256] = {0};
	char at_str[64] = {0};
	
    if (iccid == NULL) 
	{
        return FM_PARAM_NULL;
    }

    if (load_at_utils() != 0) 
	{
		ret = FM_LOAD_AT_UTILS_ERROR;
    }

	sprintf(at_str, "AT^ICCID?\r\n");
	ret = SendATCMDWaitResp(at_str, res_str);		//89861123213040511545		or		00000000000000000000
printf("*** ret: %d\r\n", ret);	
if (ret == 0) 
	{
printf("resp: %s\r\n", res_str);
		if (res_str == NULL || res_str[0] == '\0' || strcmp(res_str, "") == 0 || strcmp(res_str, "0") == 0 || strcmp(res_str, "00000000000000000000") == 0) 
		{
			ret = FM_AT_RET_OTHER_ERROR;
		} 
		else 
		{
			char* p  = strstr(res_str, "^ICCID: ");
			if(p != NULL)
			{
				p += strlen("^ICCID: ");
				strncpy(iccid, p, 20);
           printf("******iccid:%s\r\n", iccid);
               printf("iccid lenth is %d\n",strlen(iccid));
			   iccid[20] = '\0';
				ret = FM_SUCCESS;
			}
			else
			{
				ret = FM_AT_RET_OTHER_ERROR;
			}
		}
	} 
	else 
	{
		ret = FM_AT_EXECUTE_FAIL;
	}

	unload_at_utils();

	return (FM_STATUS_t)ret;
#elif FM_SYSTEM == FM_SYSTEM_RTOS_ML307R
	int ret = 0;

	char at_str[64] = {0};
	uint8_t res_str[256] = {0};
	int32_t *res_len = NULL;

	if (iccid == NULL) 
	{
        return FM_PARAM_NULL;
    }

    res_len = malloc(sizeof(int32_t));

	sprintf(at_str, "AT*ICCID?\r\n");
	ret = cm_virt_at_send_sync((const uint8_t *)at_str, res_str, res_len, 32);			//*ICCID: "89860623590029100359"
    if(ret == 0) 
    {
		if (strstr((char *)res_str, "+CME ERROR")) 
		{
			ret = FM_AT_RET_CME_ERROR;
		} 
		else if (strstr((char *)res_str, "+CMS ERROR")) 
		{
			ret = FM_AT_RET_CMS_ERROR;
		} 
		else if (strstr((char *)res_str, "ERROR")) 
		{
			ret = FM_AT_RET_ERROR;
		} 
		else if (strstr((char *)res_str, "*ICCID: ")) 
		{
			if (sscanf((char *)res_str, "*ICCID: \"%[^\"]\"", iccid) != 1) 
			{
				ret = FM_STRING_PARSING_FAIL;
			} 
			else 
			{
				ret = FM_SUCCESS;
			}
		} 
		else 
		{
			ret = FM_AT_RET_OTHER_ERROR;
		}
    } 
	else 
	{
		ret = FM_AT_EXECUTE_FAIL;
	}

    free(res_len);

	return (FM_STATUS_t)ret;
#endif
}

//函数功能：查询模组信号强度
//参数：rsrp - 指向模组信号强度数据指针，单位:DBm
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_get_signal_strength(int *rsrp) 
{
#if FM_SYSTEM == FM_SYSTEM_RTOS_1803S
	int ret = 0;
	char res_str[256] = {0};
	char at_str[64] = {0};
	
	if (rsrp == NULL) 
	{
        return FM_PARAM_NULL;
    }

	sprintf(at_str, "AT+ZRSSI\r\n");
	ret = SendATCMDWaitResp(8, at_str, 150, NULL, 1, "+CME ERROR", res_str, sizeof(res_str));		//+ZRSSI: -102,-6.5,-76,19
	if (ret == 0) 
	{
		if (strstr(res_str, "+CME ERROR")) 
		{
			return FM_AT_RET_CME_ERROR;
		} 
		else if (strstr(res_str, "+CMS ERROR")) 
		{
			return FM_AT_RET_CMS_ERROR;
		} 
		else if (strstr(res_str, "ERROR")) 
		{
			return FM_AT_RET_ERROR;
		} 
		else if (strstr(res_str, "+ZRSSI: ")) 
		{
			if (sscanf(res_str, "+ZRSSI: %d", rsrp) != 1) 
			{
				return FM_STRING_PARSING_FAIL;
			} 
			else 
			{
				return FM_SUCCESS;
			}
		} 
		else 
		{
			return FM_AT_RET_OTHER_ERROR;
		}
	} 
	else 
	{
		return FM_AT_EXECUTE_FAIL;
	}
#elif FM_SYSTEM == FM_SYSTEM_LINUX_M24A
	int ret = 0;
	char res_str[256] = {0};
	char at_str[64] = {0};
	
    if (rsrp == NULL) 
	{
        return FM_PARAM_NULL;
    }

    if (load_at_utils() != 0) 
	{
		ret = FM_LOAD_AT_UTILS_ERROR;
    }

	sprintf(at_str, "AT+ZRSSI\r\n");
	ret = SendATCMDWaitResp(at_str, res_str);		//-52,6,-66,1
	if (ret == 0) 
	{
		if (res_str == NULL || res_str[0] == '\0' || strcmp(res_str, "") == 0 || strcmp(res_str, "0") == 0) 
		{
			ret = FM_AT_RET_OTHER_ERROR;
		} 
		else 
		{
			if (sscanf(res_str, "%d", rsrp) != 1) 
			{
				ret =  FM_STRING_PARSING_FAIL;
			} 
			else 
			{
				ret =  FM_SUCCESS;
			}
		}
	} 
	else 
	{
		ret = FM_AT_EXECUTE_FAIL;
	}

	unload_at_utils();

	return (FM_STATUS_t)ret;
#elif FM_SYSTEM == FM_SYSTEM_RTOS_ML307R
	int ret = 0;

	char at_str[64] = {0};
	uint8_t res_str[256] = {0};
	int32_t *res_len = NULL;

	if (rsrp == NULL) 
	{
        return FM_PARAM_NULL;
    }

    res_len = malloc(sizeof(int32_t));

	sprintf(at_str, "AT+CESQ\r\n");
	ret = cm_virt_at_send_sync((const uint8_t *)at_str, res_str, res_len, 32);			//+CESQ: 57,99,255,255,0,52
    if(ret == 0) 
    {
		if (strstr((char *)res_str, "+CME ERROR")) 
		{
			ret = FM_AT_RET_CME_ERROR;
		} 
		else if (strstr((char *)res_str, "+CMS ERROR")) 
		{
			ret = FM_AT_RET_CMS_ERROR;
		} 
		else if (strstr((char *)res_str, "ERROR")) 
		{
			ret = FM_AT_RET_ERROR;
		} 
		else if (strstr((char *)res_str, "+CESQ: ")) 
		{
			if (sscanf((char *)res_str, "+CESQ: %*d,%*d,%*d,%*d,%*d,%d", rsrp) != 1) 
			{
				ret = FM_STRING_PARSING_FAIL;
			} 
			else 
			{
				if (*rsrp == 255) 
				{
					*rsrp = 0;
				} 
				else 
				{
					*rsrp -= 141;
				}

				ret = FM_SUCCESS;
			}
		} 
		else 
		{
			ret = FM_AT_RET_OTHER_ERROR;
		}
    } 
	else 
	{
		ret = FM_AT_EXECUTE_FAIL;
	}

    free(res_len);

	return (FM_STATUS_t)ret;
#endif
}

//函数功能：查询模组网络注册状态
//参数：operator - 指向运营商数据指针["CHINA TELECOM", "CHINA UNICOM", "CHINA MOBILE", "CHINA BROADNET", "UNKNOWN"]
//参数：regstate - 指向注册状态数据指针
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_get_operator_regstate(char *operator, int *regstate) 
{
#if FM_SYSTEM == FM_SYSTEM_RTOS_1803S
	int ret = 0;
	char res_str[256] = {0};
	char at_str[64] = {0};
	
    if (operator == NULL || regstate == NULL) 
	{
        return FM_PARAM_NULL;
    }

	sprintf(at_str, "AT*ASRCOPS?\r\n");
	ret = SendATCMDWaitResp(8, at_str, 150, NULL, 1, "+CME ERROR", res_str, sizeof(res_str));		//*ASRCOPS: 0,2,"46001",1,"UNICOM",6
	if (ret == 0) 
	{
		if (strstr(res_str, "+CME ERROR")) 
		{
			return FM_AT_RET_CME_ERROR;
		} 
		else if (strstr(res_str, "+CMS ERROR")) 
		{
			return FM_AT_RET_CMS_ERROR;
		} 
		else if (strstr(res_str, "ERROR")) 
		{
			return FM_AT_RET_ERROR;
		} 
		else if (strstr(res_str, "*ASRCOPS: ")) 
		{
			if (sscanf(res_str, "*ASRCOPS: %*d,%*d,\"%[^\"]\",%*d,\"%*[^\"]\",%d", operator, regstate) != 2) 
			{
				return FM_STRING_PARSING_FAIL;
			} 
			else 
			{
				if (!strncmp(operator, "46003", strlen("46003")) ||
					!strncmp(operator, "46011", strlen("46011")) ||
					!strncmp(operator, "46012", strlen("46012")) ||
					!strncmp(operator, "46005", strlen("46005"))) 
				{
					memset(operator, 0, 6);
					strcpy(operator, "CHINA TELECOM");
				} 
				else if (!strncmp(operator, "46001", strlen("46001")) ||
					!strncmp(operator, "46010", strlen("46010")) ||
					!strncmp(operator, "46006", strlen("46006")) ||
					!strncmp(operator, "46009", strlen("46009"))) 
				{
					memset(operator, 0, 6);
					strcpy(operator, "CHINA UNICOM");
				} 
				else if (!strncmp(operator, "46000", strlen("46000")) ||
					!strncmp(operator, "46002", strlen("46002")) ||
					!strncmp(operator, "46004", strlen("46004")) ||
					!strncmp(operator, "46007", strlen("46007")) ||
					!strncmp(operator, "46008", strlen("46008"))) 
				{
					memset(operator, 0, 6);
					strcpy(operator, "CHINA MOBILE");
				} 
				else 
				{
					memset(operator, 0, 6);
					strcpy(operator, "Unknown");
				}

				return FM_SUCCESS;
			}
		} 
		else 
		{
			return FM_AT_RET_OTHER_ERROR;
		}
	} 
	else 
	{
		return FM_AT_EXECUTE_FAIL;
	}
#elif FM_SYSTEM == FM_SYSTEM_LINUX_M24A
	int ret = 0;
	char res_str[256] = {0};
	char at_str[64] = {0};
	
    if (operator == NULL || regstate == NULL) 
	{
        return FM_PARAM_NULL;
    }

    if (load_at_utils() != 0) 
	{
		ret = FM_LOAD_AT_UTILS_ERROR;
    }

	sprintf(at_str, "AT+COPS?\r\n");
	ret = SendATCMDWaitResp(at_str, res_str);			//+CME ERROR: 6003:[]		or		0,0,"CHN-CT",7,1	or		0,0,"CHINA MOBILE",7,0		or		0,2,"46000",7,1
	if (ret == 0) 
	{
		if (res_str == NULL || res_str[0] == '\0' || strcmp(res_str, "") == 0 || strcmp(res_str, "0") == 0) 
		{
			ret = FM_AT_RET_OTHER_ERROR;
		} 
		else 
		{
			//if (sscanf(res_str, "%*d,%*d,\"%[^\"]\",%*d,%d", operator, regstate) != 2) 
			if (sscanf(res_str, "%*d,%*d,\"%[^\"]\",%d", operator, regstate) != 2) 
			{
				ret = FM_STRING_PARSING_FAIL;
			} 
			else 
			{
				if (!strncmp(operator, "46003", strlen("46003")) ||
					!strncmp(operator, "46011", strlen("46011")) ||
					!strncmp(operator, "46012", strlen("46012")) ||
					!strncmp(operator, "46005", strlen("46005")) ||
					!strncmp(operator, "CHN-CT", strlen("CHN-CT"))) 
				{
					memset(operator, 0, 16);
					strcpy(operator, "CHINA TELECOM");
				} 
				else if (!strncmp(operator, "46001", strlen("46001")) ||
					!strncmp(operator, "46010", strlen("46010")) ||
					!strncmp(operator, "46006", strlen("46006")) ||
					!strncmp(operator, "46009", strlen("46009")) ||
					!strncmp(operator, "CHINA-UNICOM", strlen("CHINA-UNICOM"))) 
				{
					memset(operator, 0, 16);
					strcpy(operator, "CHINA UNICOM");
				} 
				else if (!strncmp(operator, "46000", strlen("46000")) ||
					!strncmp(operator, "46002", strlen("46002")) ||
					!strncmp(operator, "46004", strlen("46004")) ||
					!strncmp(operator, "46007", strlen("46007")) ||
					!strncmp(operator, "46008", strlen("46008")) ||
					!strncmp(operator, "CHINA MOBILE", strlen("CHINA MOBILE"))) 
				{
					memset(operator, 0, 16);
					strcpy(operator, "CHINA MOBILE");
				} 
				else 
				{
					memset(operator, 0, 16);
					strcpy(operator, "Unknown");
				}

				ret = FM_SUCCESS;
			}
		}
	} 
	else 
	{
		ret = FM_AT_EXECUTE_FAIL;
	}

	unload_at_utils();

	return (FM_STATUS_t)ret;
#elif FM_SYSTEM == FM_SYSTEM_RTOS_ML307R
	int ret = 0;

	char at_str[64] = {0};
	uint8_t res_str[256] = {0};
	int32_t *res_len = NULL;

	char iccid_data[21] = {0};

	if (operator == NULL || regstate == NULL) 
	{
        return FM_PARAM_NULL;
    }

    res_len = malloc(sizeof(int32_t));

	sprintf(at_str, "AT*ICCID?\r\n");
	ret = cm_virt_at_send_sync((const uint8_t *)at_str, res_str, res_len, 32);			//*ICCID: "89860623590029100359"
    if(ret == 0) 
    {
		if (strstr((char *)res_str, "+CME ERROR")) 
		{
			ret = FM_AT_RET_CME_ERROR;
		} 
		else if (strstr((char *)res_str, "+CMS ERROR")) 
		{
			ret = FM_AT_RET_CMS_ERROR;
		} 
		else if (strstr((char *)res_str, "ERROR")) 
		{
			ret = FM_AT_RET_ERROR;
		} 
		else if (strstr((char *)res_str, "*ICCID: ")) 
		{
			if (sscanf((char *)res_str, "*ICCID: \"%[^\"]\"", iccid_data) != 1) 
			{
				ret = FM_STRING_PARSING_FAIL;
			} 
			else 
			{
				if (strlen(iccid_data) == 20) 
				{
					char oper[3] = {iccid_data[4], iccid_data[5], '\0'};  // 创建一个新字符串包含第五和第六字符

					if ( !strncmp(oper, "03", strlen("03")) || 
						 !strncmp(oper, "11", strlen("11")) ) 
					{
						memset(operator, 0, 16);
						strcpy(operator, "CHINA TELECOM");					//中国电信
					} 
					else if ( !strncmp(oper, "01", strlen("01")) || 
							  !strncmp(oper, "06", strlen("06")) ||
							  !strncmp(oper, "09", strlen("09")) ) 
					{
						memset(operator, 0, 16);
						strcpy(operator, "CHINA UNICOM");					//中国联通
					} 
					else if ( !strncmp(oper, "00", strlen("00")) || 
							  !strncmp(oper, "02", strlen("02")) || 
							  !strncmp(oper, "04", strlen("04")) || 
							  !strncmp(oper, "07", strlen("07")) ||
							  !strncmp(oper, "08", strlen("08")) ) 
					{
						memset(operator, 0, 16);
						strcpy(operator, "CHINA MOBILE");					//中国移动
					} 
					else if ( !strncmp(oper, "15", strlen("15")) ) 
					{
						memset(operator, 0, 16);
						strcpy(operator, "CHINA BROADNET");					//中国广电
					} 
					else 
					{
						memset(operator, 0, 16);
						strcpy(operator, "UNKNOWN");
					}
					ret = FM_SUCCESS;
				} 
				else
				{
					ret = FM_GET_CONTENT_FAILED;
				}
			}
		} 
		else 
		{
			ret = FM_AT_RET_OTHER_ERROR;
		}
    } 
	else 
	{
		ret = FM_AT_EXECUTE_FAIL;
	}

 	memset(at_str, 0, sizeof(at_str));
	memset(res_str, 0, sizeof(res_str));

	sprintf(at_str, "AT*ASRCOPS?\r\n");
	ret = cm_virt_at_send_sync((const uint8_t *)at_str, res_str, res_len, 32);			//*ASRCOPS: 0,2,"46001",1,"UNICOM",7
	if (ret == 0) 
	{
		if (strstr((char *)res_str, "+CME ERROR")) 
		{
			ret = FM_AT_RET_CME_ERROR;
		} 
		else if (strstr((char *)res_str, "+CMS ERROR")) 
		{
			ret = FM_AT_RET_CMS_ERROR;
		} 
		else if (strstr((char *)res_str, "ERROR")) 
		{
			ret = FM_AT_RET_ERROR;
		} 
		else if (strstr((char *)res_str, "*ASRCOPS: ")) 
		{
			if (sscanf((char *)res_str, "*ASRCOPS: %*d,%*d,\"%*[^\"]\",%*d,\"%*[^\"]\",%d", regstate) != 1) 
			{
				ret = FM_STRING_PARSING_FAIL;
			} 
			else 
			{
				ret = FM_SUCCESS;
			}
		} 
		else 
		{
			ret = FM_AT_RET_OTHER_ERROR;
		}
	} 
	else 
	{
		ret = FM_AT_EXECUTE_FAIL;
	}
	
    free(res_len);

	return (FM_STATUS_t)ret;
#endif
}

//函数功能：查询模组网络注册状态
//参数：regstate - 指向注册状态数据指针
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_get_regstate(int *regstate) 
{
#if FM_SYSTEM == FM_SYSTEM_RTOS_1803S
	int ret = 0;
	char at_str[64] = {0};
	char res_str[256] = {0};
	
    if (regstate == NULL) 
	{
        return FM_PARAM_NULL;
    }

	sprintf(at_str, "AT*ASRCOPS?\r\n");
	ret = SendATCMDWaitResp(8, at_str, 150, NULL, 1, "+CME ERROR", res_str, sizeof(res_str));		//*ASRCOPS: 0,2,"46001",1,"UNICOM",6
	if (ret == 0) 
	{
		if (strstr(res_str, "+CME ERROR")) 
		{
			ret = FM_AT_RET_CME_ERROR;
		} 
		else if (strstr(res_str, "+CMS ERROR")) 
		{
			ret = FM_AT_RET_CMS_ERROR;
		} 
		else if (strstr(res_str, "ERROR")) 
		{
			ret = FM_AT_RET_ERROR;
		} 
		else if (strstr(res_str, "*ASRCOPS: ")) 
		{
			if (sscanf(res_str, "*ASRCOPS: %*d,%*d,\"%*[^\"]\",%*d,\"%*[^\"]\",%d", regstate) != 1) 
			{
				ret = FM_STRING_PARSING_FAIL;
			} 
			else 
			{
				ret = FM_SUCCESS;
			}
		} 
		else 
		{
			ret = FM_AT_RET_OTHER_ERROR;
		}
	} 
	else 
	{
		ret = FM_AT_EXECUTE_FAIL;
	}

	return (FM_STATUS_t)ret;
#elif FM_SYSTEM == FM_SYSTEM_LINUX_M24A
	int ret = 0;
	char at_str[64] = {0};
	char res_str[256] = {0};
	
    if (regstate == NULL) 
	{
        return FM_PARAM_NULL;
    }

    if (load_at_utils() != 0) 
	{
		ret = FM_LOAD_AT_UTILS_ERROR;
    }

	sprintf(at_str, "AT+COPS?\r\n");
	ret = SendATCMDWaitResp(at_str, res_str);	//+CME ERROR: 6003:[]	or	0,0,"CHN-CT",7,1	or	0,0,"CHINA MOBILE",7,0	or	0,2,"46000",7,1
	if (ret == 0) 
	{
		if (strstr(res_str, "+CME ERROR")) 
		{
			ret = FM_AT_RET_CME_ERROR;
		} 
		else if (strstr(res_str, "+CMS ERROR")) 
		{
			ret = FM_AT_RET_CMS_ERROR;
		} 
		else if (strstr(res_str, "ERROR")) 
		{
			ret = FM_AT_RET_ERROR;
		} 
		else if (res_str == NULL || res_str[0] == '\0' || strcmp(res_str, "") == 0 || strcmp(res_str, "0") == 0) 
		{
			ret = FM_AT_RET_OTHER_ERROR;
		} 
		else 
		{
			if (sscanf(res_str, "%*d,%*d,\"%*[^\"]\",%d", regstate) != 1) 
			{
				ret = FM_STRING_PARSING_FAIL;
			} 
			else 
			{
				ret = FM_SUCCESS;
			}
		}
	} 
	else 
	{
		ret = FM_AT_EXECUTE_FAIL;
	}

	unload_at_utils();

	return (FM_STATUS_t)ret;
#elif FM_SYSTEM == FM_SYSTEM_RTOS_ML307R
	int ret = 0;

	char at_str[64] = {0};
	uint8_t res_str[256] = {0};
	int32_t *res_len = NULL;

	if (regstate == NULL) 
	{
        return FM_PARAM_NULL;
    }

    res_len = malloc(sizeof(int32_t));

	sprintf(at_str, "AT*ASRCOPS?\r\n");
	ret = cm_virt_at_send_sync((const uint8_t *)at_str, res_str, res_len, 32);			//*ASRCOPS: 0,2,"46001",1,"UNICOM",7
	if (ret == 0) 
	{
		if (strstr((char *)res_str, "+CME ERROR")) 
		{
			ret = FM_AT_RET_CME_ERROR;
		} 
		else if (strstr((char *)res_str, "+CMS ERROR")) 
		{
			ret = FM_AT_RET_CMS_ERROR;
		} 
		else if (strstr((char *)res_str, "ERROR")) 
		{
			ret = FM_AT_RET_ERROR;
		} 
		else if (strstr((char *)res_str, "*ASRCOPS: ")) 
		{
			if (sscanf((char *)res_str, "*ASRCOPS: %*d,%*d,\"%*[^\"]\",%*d,\"%*[^\"]\",%d", regstate) != 1) 
			{
				ret = FM_STRING_PARSING_FAIL;
			} 
			else 
			{
				ret = FM_SUCCESS;
			}
		} 
		else 
		{
			ret = FM_AT_RET_OTHER_ERROR;
		}
	} 
	else 
	{
		ret = FM_AT_EXECUTE_FAIL;
	}
	
    free(res_len);

	return (FM_STATUS_t)ret;

#endif
}

//函数功能：查询模组网络注册状态
//参数：operator - 指向运营商数据指针["CHINA TELECOM", "CHINA UNICOM", "CHINA MOBILE", "CHINA BROADNET", "UNKNOWN"]
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_get_operator_index(char *operator,int *current_index) 
{
#if FM_SYSTEM == FM_SYSTEM_RTOS_1803S
	int ret = 0;
	char iccid_data[21] = {0};

    if (operator == NULL) 
	{
        return FM_PARAM_NULL;
    }

	ret = fm_get_iccid(iccid_data);
	if ( (FM_STATUS_t)ret == FM_SUCCESS ) 
	{
		if (strlen(iccid_data) == 20) 
		{
			char oper[3] = {iccid_data[4], iccid_data[5], '\0'};  // 创建一个新字符串包含第五和第六字符

			if ( !strncmp(oper, "03", strlen("03")) || 
				 !strncmp(oper, "11", strlen("11")) ) 
			{
				memset(operator, 0, 16);
				strcpy(operator, "CHINA TELECOM");					//中国电信
				
			} 
			else if ( !strncmp(oper, "01", strlen("01")) || 
					  !strncmp(oper, "06", strlen("06")) ||
					  !strncmp(oper, "09", strlen("09")) ) 
			{
				memset(operator, 0, 16);
				strcpy(operator, "CHINA UNICOM");					//中国联通
				
			} 
			else if ( !strncmp(oper, "00", strlen("00")) || 
					  !strncmp(oper, "02", strlen("02")) || 
					  !strncmp(oper, "04", strlen("04")) || 
					  !strncmp(oper, "07", strlen("07")) ||
					  !strncmp(oper, "08", strlen("08")) ) 
			{
				memset(operator, 0, 16);
				strcpy(operator, "CHINA MOBILE");					//中国移动
				
			} 
			else if ( !strncmp(oper, "15", strlen("15")) ) 
			{
				memset(operator, 0, 16);
				strcpy(operator, "CHINA BROADNET");					//中国广电
			} 
			else 
			{
				memset(operator, 0, 16);
				strcpy(operator, "UNKNOWN");
			}

			ret = FM_SUCCESS;
		} 
		else 
		{
			ret = FM_GET_CONTENT_FAILED;
		}
    }

	return (FM_STATUS_t)ret;
#elif FM_SYSTEM == FM_SYSTEM_LINUX_M24A
	int ret = 0;
	char iccid_data[21] = {0};
	
    if (operator == NULL) 
	{
        return FM_PARAM_NULL;
    }

	ret = fm_get_iccid(iccid_data);
	printf("index done 0.5\n");
	if ( (FM_STATUS_t)ret == FM_SUCCESS ) 
	{
	printf("index done 1\n");
		if (strlen(iccid_data) == 20) 
		{
			 //89860624640006946784
			char oper[3] = {iccid_data[4], iccid_data[5], '\0'};  // 创建一个新字符串包含第五和第六字符

			if ( !strncmp(oper, "03", strlen("03")) || 
				 !strncmp(oper, "11", strlen("11")) ) 
			{
				memset(operator, 0, 16);
				strcpy(operator, "CHINA TELECOM");					//中国电信
				printf("index done 2\n");
				*current_index=2;
				printf("index done 3\n");
			} 
			else if ( !strncmp(oper, "01", strlen("01")) || 
					  !strncmp(oper, "06", strlen("06")) ||
					  !strncmp(oper, "09", strlen("09")) ) 
			{
				memset(operator, 0, 16);
				strcpy(operator, "CHINA UNICOM");					//中国联通
				*current_index=0;
			} 
			else if ( !strncmp(oper, "00", strlen("00")) || 
					  !strncmp(oper, "02", strlen("02")) || 
					  !strncmp(oper, "04", strlen("04")) || 
					  !strncmp(oper, "07", strlen("07")) ||
					  !strncmp(oper, "08", strlen("08")) ) 
			{
				memset(operator, 0, 16);
				strcpy(operator, "CHINA MOBILE");					//中国移动
				*current_index=1;
			} 
			else if ( !strncmp(oper, "15", strlen("15")) ) 
			{
				memset(operator, 0, 16);
				strcpy(operator, "CHINA BROADNET");					//中国广电
			} 
			else 
			{
				memset(operator, 0, 16);
				strcpy(operator, "UNKNOWN");
			}

			ret = FM_SUCCESS;
		} 
		else 
		{
			ret = FM_GET_CONTENT_FAILED;
		}
    }

	return (FM_STATUS_t)ret;
#elif FM_SYSTEM == FM_SYSTEM_RTOS_ML307R
	int ret = 0;
	char iccid_data[21] = {0};
	
    if (operator == NULL) 
	{
        return FM_PARAM_NULL;
    }

	ret = fm_get_iccid(iccid_data);
	if ( (FM_STATUS_t)ret == FM_SUCCESS ) 
	{
		if (strlen(iccid_data) == 20) 
		{
			char oper[3] = {iccid_data[4], iccid_data[5], '\0'};  // 创建一个新字符串包含第五和第六字符

			if ( !strncmp(oper, "03", strlen("03")) || 
				 !strncmp(oper, "11", strlen("11")) ) 
			{
				memset(operator, 0, 16);
				strcpy(operator, "CHINA TELECOM");					//中国电信
			} 
			else if ( !strncmp(oper, "01", strlen("01")) || 
					  !strncmp(oper, "06", strlen("06")) ||
					  !strncmp(oper, "09", strlen("09")) ) 
			{
				memset(operator, 0, 16);
				strcpy(operator, "CHINA UNICOM");					//中国联通
			} 
			else if ( !strncmp(oper, "00", strlen("00")) || 
					  !strncmp(oper, "02", strlen("02")) || 
					  !strncmp(oper, "04", strlen("04")) || 
					  !strncmp(oper, "07", strlen("07")) ||
					  !strncmp(oper, "08", strlen("08")) ) 
			{
				memset(operator, 0, 16);
				strcpy(operator, "CHINA MOBILE");					//中国移动
			} 
			else if ( !strncmp(oper, "15", strlen("15")) ) 
			{
				memset(operator, 0, 16);
				strcpy(operator, "CHINA BROADNET");					//中国广电
			} 
			else 
			{
				memset(operator, 0, 16);
				strcpy(operator, "UNKNOWN");
			}

			ret = FM_SUCCESS;
		} 
		else 
		{
			ret = FM_GET_CONTENT_FAILED;
		}
    }

	return (FM_STATUS_t)ret;
#endif
}
FM_STATUS_t fm_get_operator(char *operator) 
{
#if FM_SYSTEM == FM_SYSTEM_RTOS_1803S
	int ret = 0;
	char iccid_data[21] = {0};

    if (operator == NULL) 
	{
        return FM_PARAM_NULL;
    }

	ret = fm_get_iccid(iccid_data);
	if ( (FM_STATUS_t)ret == FM_SUCCESS ) 
	{
		if (strlen(iccid_data) == 20) 
		{
			char oper[3] = {iccid_data[4], iccid_data[5], '\0'};  // 创建一个新字符串包含第五和第六字符

			if ( !strncmp(oper, "03", strlen("03")) || 
				 !strncmp(oper, "11", strlen("11")) ) 
			{
				memset(operator, 0, 16);
				strcpy(operator, "CHINA TELECOM");					//中国电信
				
			} 
			else if ( !strncmp(oper, "01", strlen("01")) || 
					  !strncmp(oper, "06", strlen("06")) ||
					  !strncmp(oper, "09", strlen("09")) ) 
			{
				memset(operator, 0, 16);
				strcpy(operator, "CHINA UNICOM");					//中国联通
				
			} 
			else if ( !strncmp(oper, "00", strlen("00")) || 
					  !strncmp(oper, "02", strlen("02")) || 
					  !strncmp(oper, "04", strlen("04")) || 
					  !strncmp(oper, "07", strlen("07")) ||
					  !strncmp(oper, "08", strlen("08")) ) 
			{
				memset(operator, 0, 16);
				strcpy(operator, "CHINA MOBILE");					//中国移动
				
			} 
			else if ( !strncmp(oper, "15", strlen("15")) ) 
			{
				memset(operator, 0, 16);
				strcpy(operator, "CHINA BROADNET");					//中国广电
			} 
			else 
			{
				memset(operator, 0, 16);
				strcpy(operator, "UNKNOWN");
			}

			ret = FM_SUCCESS;
		} 
		else 
		{
			ret = FM_GET_CONTENT_FAILED;
		}
    }

	return (FM_STATUS_t)ret;
#elif FM_SYSTEM == FM_SYSTEM_LINUX_M24A
	int ret = 0;
	char iccid_data[21] = {0};
	
    if (operator == NULL) 
	{
        return FM_PARAM_NULL;
    }

	ret = fm_get_iccid(iccid_data);
	if ( (FM_STATUS_t)ret == FM_SUCCESS ) 
	{
		if (strlen(iccid_data) == 20) 
		{
			 //89860624640006946784
			char oper[3] = {iccid_data[4], iccid_data[5], '\0'};  // 创建一个新字符串包含第五和第六字符

			if ( !strncmp(oper, "03", strlen("03")) || 
				 !strncmp(oper, "11", strlen("11")) ) 
			{
				memset(operator, 0, 16);
				strcpy(operator, "CHINA TELECOM");					//中国电信
			} 
			else if ( !strncmp(oper, "01", strlen("01")) || 
					  !strncmp(oper, "06", strlen("06")) ||
					  !strncmp(oper, "09", strlen("09")) ) 
			{
				memset(operator, 0, 16);
				strcpy(operator, "CHINA UNICOM");					//中国联通
			} 
			else if ( !strncmp(oper, "00", strlen("00")) || 
					  !strncmp(oper, "02", strlen("02")) || 
					  !strncmp(oper, "04", strlen("04")) || 
					  !strncmp(oper, "07", strlen("07")) ||
					  !strncmp(oper, "08", strlen("08")) ) 
			{
				memset(operator, 0, 16);
				strcpy(operator, "CHINA MOBILE");					//中国移动
			} 
			else if ( !strncmp(oper, "15", strlen("15")) ) 
			{
				memset(operator, 0, 16);
				strcpy(operator, "CHINA BROADNET");					//中国广电
			} 
			else 
			{
				memset(operator, 0, 16);
				strcpy(operator, "UNKNOWN");
			}

			ret = FM_SUCCESS;
		} 
		else 
		{
			ret = FM_GET_CONTENT_FAILED;
		}
    }

	return (FM_STATUS_t)ret;
#elif FM_SYSTEM == FM_SYSTEM_RTOS_ML307R
	int ret = 0;
	char iccid_data[21] = {0};
	
    if (operator == NULL) 
	{
        return FM_PARAM_NULL;
    }

	ret = fm_get_iccid(iccid_data);
	if ( (FM_STATUS_t)ret == FM_SUCCESS ) 
	{
		if (strlen(iccid_data) == 20) 
		{
			char oper[3] = {iccid_data[4], iccid_data[5], '\0'};  // 创建一个新字符串包含第五和第六字符

			if ( !strncmp(oper, "03", strlen("03")) || 
				 !strncmp(oper, "11", strlen("11")) ) 
			{
				memset(operator, 0, 16);
				strcpy(operator, "CHINA TELECOM");					//中国电信
			} 
			else if ( !strncmp(oper, "01", strlen("01")) || 
					  !strncmp(oper, "06", strlen("06")) ||
					  !strncmp(oper, "09", strlen("09")) ) 
			{
				memset(operator, 0, 16);
				strcpy(operator, "CHINA UNICOM");					//中国联通
			} 
			else if ( !strncmp(oper, "00", strlen("00")) || 
					  !strncmp(oper, "02", strlen("02")) || 
					  !strncmp(oper, "04", strlen("04")) || 
					  !strncmp(oper, "07", strlen("07")) ||
					  !strncmp(oper, "08", strlen("08")) ) 
			{
				memset(operator, 0, 16);
				strcpy(operator, "CHINA MOBILE");					//中国移动
			} 
			else if ( !strncmp(oper, "15", strlen("15")) ) 
			{
				memset(operator, 0, 16);
				strcpy(operator, "CHINA BROADNET");					//中国广电
			} 
			else 
			{
				memset(operator, 0, 16);
				strcpy(operator, "UNKNOWN");
			}

			ret = FM_SUCCESS;
		} 
		else 
		{
			ret = FM_GET_CONTENT_FAILED;
		}
    }

	return (FM_STATUS_t)ret;
#endif
}

//函数功能：设置模组运营商
//参数：operator - 运营商
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_set_operator(char *operator) 
{
#if FM_SYSTEM == FM_SYSTEM_RTOS_1803S
	int ret = 0;
	char res_str[256] = {0};
	char at_str[64] = {0};

	if (operator == NULL) 
	{
        return FM_PARAM_NULL;
    }

	if (!strncmp(operator, "CHINA UNICOM", strlen("CHINA UNICOM"))) 
	{
		//sprintf(at_str, "AT+CLCK=\"SC\",1,\"0100\"\r");
		sprintf(at_str, "AT+CSIM=10,80F2EE0100\r");		//AT+CSIM=10,80F2EE0100
	} 
	else if (!strncmp(operator, "CHINA MOBILE", strlen("CHINA MOBILE"))) 
	{
		//sprintf(at_str, "AT+CLCK=\"SC\",1,\"0200\"\r");
		sprintf(at_str, "AT+CSIM=10,80F2EE0200\r");
	} 
	else if (!strncmp(operator, "CHINA TELECOM", strlen("CHINA TELECOM"))) 
	{
		//sprintf(at_str, "AT+CLCK=\"SC\",1,\"0300\"\r");
		sprintf(at_str, "AT+CSIM=10,80F2EE0300\r");
	} 
	else 
	{
		return FM_PARAM_ERROR;
	}

	//ret = SendATCMDWaitResp(8, at_str, 150, "+CLCK", 1, "+CME ERROR", res_str, sizeof(res_str));	//+CLCK: 1
	ret = SendATCMDWaitResp(8, at_str, 150, "+CSIM", 1, "+CME ERROR", res_str, sizeof(res_str));	//+CSIM: 1
	if (ret != 0) 
	{
		return FM_AT_EXECUTE_FAIL;
	}

	return FM_SUCCESS;
#elif FM_SYSTEM == FM_SYSTEM_LINUX_M24A
	int ret = 0;
	char res_str[1024] = {0};
	//char *res_str = calloc(1, 1024);
	char at_str[64] = {0};
	
	if (operator == NULL) 
	{
        return FM_PARAM_NULL;
    }

	if (!strncmp(operator, "CHINA UNICOM", strlen("CHINA UNICOM"))) 
	{
		sprintf(at_str, "AT+CLCK=\"SC\",1,\"0100\"\r\n");
		//sprintf(at_str, "AT+CSIM=10,80F2EE0100\r\n");		//AT+CSIM=10,80F2EE0100
	} 
	else if (!strncmp(operator, "CHINA MOBILE", strlen("CHINA MOBILE"))) 
	{
		sprintf(at_str, "AT+CLCK=\"SC\",1,\"0200\"\r\n");
		//sprintf(at_str, "AT+CSIM=10,80F2EE0200\r\n");
	} 
	else if (!strncmp(operator, "CHINA TELECOM", strlen("CHINA TELECOM"))) 
	{
		sprintf(at_str, "AT+CLCK=\"SC\",1,\"0300\"\r\n");
		//sprintf(at_str, "AT+CSIM=10,80F2EE0300\r\n");
	} 
	else 
	{
		ret = FM_PARAM_ERROR;
	}

    if (load_at_utils() != 0) 
	{
		ret = FM_LOAD_AT_UTILS_ERROR;
    }

	//ret = SendATCMDWaitResp(at_str, res_str);			//+CLCK: 1
	ret = SendATCMDWaitResp(at_str, res_str);			//+CSIM: 1
	if (ret != 0) 
	{
		ret = FM_AT_EXECUTE_FAIL;
	} 
	else 
	{
		ret = FM_SUCCESS;
	}

	unload_at_utils();

	return (FM_STATUS_t)ret;
#elif FM_SYSTEM == FM_SYSTEM_RTOS_ML307R
	int ret = 0;

	char at_str[64] = {0};
	uint8_t res_str[256] = {0};
	int32_t *res_len = NULL;

	if (operator == NULL) 
	{
        return FM_PARAM_NULL;
    }

	if (!strncmp(operator, "CHINA UNICOM", strlen("CHINA UNICOM"))) 
	{
		sprintf(at_str, "AT+CLCK=\"SC\",1,\"0100\"\r\n");
		//sprintf(at_str, "AT+CSIM=10,80F2EE0100\r\n");		//AT+CSIM=10,80F2EE0100
	} 
	else if (!strncmp(operator, "CHINA MOBILE", strlen("CHINA MOBILE"))) 
	{
		sprintf(at_str, "AT+CLCK=\"SC\",1,\"0200\"\r\n");
		//sprintf(at_str, "AT+CSIM=10,80F2EE0200\r\n");
	} 
	else if (!strncmp(operator, "CHINA TELECOM", strlen("CHINA TELECOM"))) 
	{
		sprintf(at_str, "AT+CLCK=\"SC\",1,\"0300\"\r\n");
		//sprintf(at_str, "AT+CSIM=10,80F2EE0300\r\n");
	} 
	else 
	{
		return FM_PARAM_ERROR;
	}

    res_len = malloc(sizeof(int32_t));

	ret = cm_virt_at_send_sync((const uint8_t *)at_str, res_str, res_len, 32);			//*ICCID: "89860623590029100359"
	if (ret != 0) 
	{
		ret = FM_AT_EXECUTE_FAIL;
	} 
	else 
	{
		ret = FM_SUCCESS;
	}

	free(res_len);

	return (FM_STATUS_t)ret;
#endif
}

#if FM_SYSTEM == FM_SYSTEM_RTOS_1803S
//函数功能：循环设置模组运营商
//参数：无
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_set_operator_next(void) 
{
	static int ope_index = 0;
	static char *oper_list[] = {"CHINA TELECOM", "CHINA UNICOM", "CHINA MOBILE"};

	ope_index += 1;
	if (ope_index >= 3) 
	{
		ope_index = 0;
	}

	return fm_set_operator(oper_list[ope_index]);
}
#endif


#if FM_SYSTEM != FM_SYSTEM_RTOS_ML307R
//函数功能：获取plmn值
//参数：plmn - 指向plmn值的数据指针
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_get_plmn_value(int *plmn) 
{
#if FM_SYSTEM == FM_SYSTEM_RTOS_1803S
	int ret = 0;
	char res_str[256] = {0};
	char at_str[64] = {0};

	if (plmn == NULL) 
	{
        return FM_PARAM_NULL;
    }
	
	sprintf(at_str, "AT*ASRCOPS?\r\n");
	ret = SendATCMDWaitResp(8, at_str, 150, NULL, 1, "+CME ERROR", res_str, sizeof(res_str));		//*ASRCOPS: 0,2,"46001",1,"UNICOM",6
	if (ret == 0) 
	{
		if (strstr(res_str, "+CME ERROR")) 
		{
			return FM_AT_RET_CME_ERROR;
		} 
		else if (strstr(res_str, "+CMS ERROR")) 
		{
			return FM_AT_RET_CMS_ERROR;
		} 
		else if (strstr(res_str, "ERROR")) 
		{
			return FM_AT_RET_ERROR;
		} 
		else if (strstr(res_str, "*ASRCOPS: ")) 
		{
			if (sscanf(res_str, "*ASRCOPS: %*d,%*d,\"%d", plmn) != 1) 
			{
				return FM_STRING_PARSING_FAIL;
			} 
			else 
			{
				return FM_SUCCESS;
			}
		} 
		else 
		{
			return FM_AT_RET_OTHER_ERROR;
		}
	} 
	else 
	{
		return FM_AT_EXECUTE_FAIL;
	}
#elif FM_SYSTEM == FM_SYSTEM_LINUX_M24A
	int ret = 0;
	char res_str[256] = {0};
	char at_str[64] = {0};
	
	if (plmn == NULL) 
	{
        return FM_PARAM_NULL;
    }

    if (load_at_utils() != 0) 
	{
		ret = FM_LOAD_AT_UTILS_ERROR;
    }

	//AT+COPS=3,0:char AT+COPS=3,2:num
	sprintf(at_str, "AT+COPS=3,2;+COPS?\r\n");
	ret = SendATCMDWaitResp(at_str, res_str);			//+CME ERROR: 6003:[]		or		0,2,"46000",7,0
	if (ret == 0) 
	{
		if (res_str == NULL || res_str[0] == '\0' || strcmp(res_str, "") == 0 || strcmp(res_str, "0") == 0) 
		{
			ret = FM_AT_RET_OTHER_ERROR;
		} 
		else 
		{
			if (sscanf(res_str, "%*d,%*d,\"%d", plmn) != 1) 
			{
				ret = FM_STRING_PARSING_FAIL;
			} 
			else 
			{
				ret = FM_SUCCESS;
			}
		}
	} 
	else 
	{
		ret = FM_AT_EXECUTE_FAIL;
	}

	unload_at_utils();

	return (FM_STATUS_t)ret;
#endif
}

//函数功能：获取信号值
//参数：rsrp - rsrp值
//参数：rsrq - rsrq值
//参数：rssi - rssi值
//参数：sinr - sinr值
//返回值：FM_SUCCESS为成功，其他为对应的错误码


FM_STATUS_t fm_get_signal_value(int *rsrp, int *rsrq, int *rssi, int *sinr) 
{
#if FM_SYSTEM == FM_SYSTEM_RTOS_1803S
	int ret = 0;
	char res_str[256] = {0};
	char at_str[64] = {0};

	if (rsrp == NULL || rsrq == NULL || rssi == NULL || sinr == NULL) 
	{
        return FM_PARAM_NULL;
    }

	sprintf(at_str, "AT+ZRSSI\r");
	ret = SendATCMDWaitResp(8, at_str, 150, NULL, 1, "+CME ERROR", res_str, sizeof(res_str));		//+ZRSSI: -102,-6.5,-76,19
	if (ret == 0) 
	{
		if (strstr(res_str, "+CME ERROR")) 
		{
			return FM_AT_RET_CME_ERROR;
		} 
		else if (strstr(res_str, "+CMS ERROR")) 
		{
			return FM_AT_RET_CMS_ERROR;
		} 
		else if (strstr(res_str, "ERROR")) 
		{
			return FM_AT_RET_ERROR;
		} 
		else if (strstr(res_str, "+ZRSSI: ")) 
		{
			if (sscanf(res_str, "+ZRSSI: %d,%d%*[^,],%d,%d", rsrp, rsrq, rssi, sinr) != 4) 
			{
				return FM_STRING_PARSING_FAIL;
			} 
			else 
			{
				return FM_SUCCESS;
			}
		} 
		else 
		{
			return FM_AT_RET_OTHER_ERROR;
		}
	} 
	else 
	{
		return FM_AT_EXECUTE_FAIL;
	}
#elif FM_SYSTEM == FM_SYSTEM_LINUX_M24A
	int ret = 0;
	char res_str[256] = {0};
	char at_str[64] = {0};

	if (rsrp == NULL || rsrq == NULL || rssi == NULL || sinr == NULL) 
	{
        return FM_PARAM_NULL;
    }

	if (load_at_utils() != 0) 
	{
		ret = FM_LOAD_AT_UTILS_ERROR;
    }

	sprintf(at_str, "AT+CESQ\r\n");
	ret = SendATCMDWaitResp(at_str, res_str);			//99,99,255,255,24,47
	if (ret == 0) 
	{
		if (res_str == NULL || res_str[0] == '\0' || strcmp(res_str, "") == 0 || strcmp(res_str, "0") == 0) 
		{
			ret = FM_AT_RET_OTHER_ERROR;
		} 
		else 
		{
			if (sscanf(res_str, "%d,%*d,%*d,%*d,%d,%d", rssi, rsrq, rsrp) != 3) 
			{
				ret = FM_STRING_PARSING_FAIL;
			} 
			else 
			{
				calculate_rssi_val(rssi);
				calculate_rsrq_val(rsrq);
				calculate_rsrp_val(rsrp);
			}
		}
	} 
	else 
	{
		ret = FM_AT_EXECUTE_FAIL;
	}

	memset(res_str, 0, sizeof(res_str));
	memset(at_str, 0, sizeof(at_str));

	sprintf(at_str, "AT+ZSINR\r\n");
	ret = SendATCMDWaitResp(at_str, res_str);			//13,2,10,3,13,1,8,6
	if (ret == 0) 
	{
		if (res_str == NULL || res_str[0] == '\0' || strcmp(res_str, "") == 0 || strcmp(res_str, "0") == 0) 
		{
			ret = FM_AT_RET_OTHER_ERROR;
		} 
		else 
		{
			if (sscanf(res_str, "%*d,%*d,%*d,%*d,%*d,%*d,%*d,%d", sinr) != 1) 
			{
				ret = FM_STRING_PARSING_FAIL;
			} 
			else 
			{
				calculate_sinr_val(res_str, sinr);
				ret = FM_SUCCESS;
			}
		}
	} 
	else 
	{
		ret = FM_AT_EXECUTE_FAIL;
	}

	unload_at_utils();

	return (FM_STATUS_t)ret;
#endif
}
#endif

//函数功能：获取是否是分身卡
//参数：fly_card - 1：是飞猫分身卡；2：不是飞猫分身卡
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_get_fly_card(int *fly_card) 
{
#if FM_SYSTEM == FM_SYSTEM_RTOS_1803S
	int ret = 0;
	char data[16] = {0};
	char res_str[256] = {0};
	char at_str[64] = {0};

	if (fly_card == NULL) 
	{
        return FM_PARAM_NULL;
    }
	
	sprintf(at_str, "AT+CSIM=10,0B5AFA0005\r\n");
	ret = SendATCMDWaitResp(8, at_str, 150, NULL, 1, "+CME ERROR", res_str, sizeof(res_str));		//+CSIM:14,"464D3130319000"
	if (ret == 0) 
	{
		if (strstr(res_str, "+CME ERROR")) 
		{
			return FM_AT_RET_CME_ERROR;
		}
		else if (strstr(res_str, "+CMS ERROR")) 
		{
			return FM_AT_RET_CMS_ERROR;
		}
		else if (strstr(res_str, "ERROR")) 
		{
			return FM_AT_RET_ERROR;
		}
		else if (strstr(res_str, "+CSIM:"))
		{
			if (sscanf(res_str, "+CSIM:%*d,\"%[^\"]", data) != 1) 
			{
				return FM_STRING_PARSING_FAIL;
			} 
			else 
			{
				if ( !strncmp(data, "464D", strlen("464D")) || !strncmp(data, "5A59", strlen("5A59")) ) 
				{
					*fly_card = 1;
				} 
				else 
				{
					*fly_card = 2;
				}
				return FM_SUCCESS;
			}
		}
		else
		{
			return FM_AT_RET_OTHER_ERROR;
		}
	} 
	else 
	{
		return FM_AT_EXECUTE_FAIL;
	}
#elif FM_SYSTEM == FM_SYSTEM_LINUX_M24A
	int ret = 0;
	char data[16] = {0};
	char res_str[256] = {0};
	char at_str[64] = {0};
	
	if (fly_card == NULL) 
	{
        return FM_PARAM_NULL;
    }

	if (load_at_utils() != 0) 
	{
		ret = FM_LOAD_AT_UTILS_ERROR;
    }

	sprintf(at_str, "AT+CSIM=10,0B5AFA0005\r\n");
	ret = SendATCMDWaitResp(at_str, res_str);			//14,464D3130319000
	if (ret == 0) 
	{
		if (res_str == NULL || res_str[0] == '\0' || strcmp(res_str, "") == 0 || strcmp(res_str, "0") == 0) 
		{
			ret = FM_AT_RET_OTHER_ERROR;
		} 
		else 
		{
			if (sscanf(res_str, "%*d,%s", data) != 1) 
			{
				ret = FM_STRING_PARSING_FAIL;
			} 
			else 
			{
				if ( !strncmp(data, "464D", strlen("464D")) || !strncmp(data, "5A59", strlen("5A59")) ) 
				{
					*fly_card = 1;
				} 
				else 
				{
					*fly_card = 2;
				}
				ret = FM_SUCCESS;
			}
		}
	} 
	else 
	{
		ret = FM_AT_EXECUTE_FAIL;
	}

	unload_at_utils();

	return (FM_STATUS_t)ret;
#elif FM_SYSTEM == FM_SYSTEM_RTOS_ML307R
	int ret = 0;

	char at_str[64] = {0};
	uint8_t res_str[256] = {0};
	int32_t *res_len = NULL;

	char data[16] = {0};

	if (fly_card == NULL) 
	{
        return FM_PARAM_NULL;
    }

    res_len = malloc(sizeof(int32_t));

	sprintf(at_str, "AT+CSIM=10,0B5AFA0005\r\n");
	ret = cm_virt_at_send_sync((const uint8_t *)at_str, res_str, res_len, 32);			//+CSIM: 14,"5A594D49469000"
	if (ret == 0) 
	{
		if (strstr((char *)res_str, "+CME ERROR")) 
		{
			ret = FM_AT_RET_CME_ERROR;
		} 
		else if (strstr((char *)res_str, "+CMS ERROR")) 
		{
			ret = FM_AT_RET_CMS_ERROR;
		} 
		else if (strstr((char *)res_str, "ERROR")) 
		{
			ret = FM_AT_RET_ERROR;
		} 
		else if (strstr((char *)res_str, "+CSIM: ")) 
		{
			if (sscanf((char *)res_str, "+CSIM: %*d,\"%[^\"]", data) != 1) 
			{
				ret = FM_STRING_PARSING_FAIL;
			} 
			else 
			{
				if ( !strncmp(data, "464D", strlen("464D")) || !strncmp(data, "5A59", strlen("5A59")) ) 
				{
					*fly_card = 1;
				} 
				else 
				{
					*fly_card = 2;
				}

				ret = FM_SUCCESS;
			}
		} 
		else 
		{
			ret = FM_AT_RET_OTHER_ERROR;
		}
	} 
	else 
	{
		ret = FM_AT_EXECUTE_FAIL;
	}

    free(res_len);

	return (FM_STATUS_t)ret;
#endif
}



#if FM_SYSTEM == FM_SYSTEM_RTOS_ML307R 
//函数功能：获取设备厂商
//参数：无
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_get_vendor_name(char *vendor_name) 
{
	int ret = 0;

	char at_str[64] = {0};
	uint8_t res_str[256] = {0};
	int32_t *res_len = NULL;

	if (vendor_name == NULL) 
	{
        return FM_PARAM_NULL;
    }

    res_len = malloc(sizeof(int32_t));

	sprintf(at_str, "AT+CGMI\r\n");
	ret = cm_virt_at_send_sync((const uint8_t *)at_str, res_str, res_len, 32);			//CMCC
	if (ret == 0) 
	{
		if (strstr((char *)res_str, "+CME ERROR")) 
		{
			ret = FM_AT_RET_CME_ERROR;
		} 
		else if (strstr((char *)res_str, "+CMS ERROR")) 
		{
			ret = FM_AT_RET_CMS_ERROR;
		} 
		else if (strstr((char *)res_str, "ERROR")) 
		{
			ret = FM_AT_RET_ERROR;
		} 
		else 
		{
			if (sscanf((char *)res_str, "%s", vendor_name) != 1) 
			{
				ret = FM_STRING_PARSING_FAIL;
			} 
			else 
			{
				ret = FM_SUCCESS;
			}
		}
	} 
	else 
	{
		ret = FM_AT_EXECUTE_FAIL;
	}

	free(res_len);

	return (FM_STATUS_t)ret;
}
#endif

#if  FM_SYSTEM == FM_SYSTEM_RTOS_1803S || FM_SYSTEM == FM_SYSTEM_RTOS_ML307R 
//函数功能：获取设备型号
//参数：无
//返回值：FM_SUCCESS为成功，其他为对应的错误码
FM_STATUS_t fm_get_model_name(char *model_name) 
{
#if  FM_SYSTEM == FM_SYSTEM_RTOS_1803S 
	int ret = 0;
	char res_str[512] = {0};
	char at_str[64] = {0};
	
    if (model_name == NULL) 
	{
        return FM_PARAM_NULL;
    }

	sprintf(at_str, "AT*READVER\r\n");
	ret = SendATCMDWaitResp(3, at_str, 150, NULL, 1, "+CME ERROR", res_str, sizeof(res_str));		//*READVER: \r\nRelease_Version: FM_UA601_U8_RTE3.0.2_20240318 (SDK 3.0.2_20240318)\r\nRelease_Time: Thu Jul 13 14:30:00 2023\r\n(DSP: FALCON_LW_M13_A0_AUOFF_MSA_1.057.058 13.7.2023 15:52)\r\n(RF: Falcon_FLYMODEM_1803MPW_PM802_DKB_V1R0_RFAPI_v5.53d_FEM_v5.45_RC9_RFVersion NULL)\r\n(HW: ASR1802_EVB_V1.0 by ASR)\r\nModel_Name: U 8\r\n\r\n\r\nOK
	if (ret == 0) 
	{
		if (strstr(res_str, "+CME ERROR")) 
		{
			return FM_AT_RET_CME_ERROR;
		}
		else if (strstr(res_str, "+CMS ERROR")) 
		{
			return FM_AT_RET_CMS_ERROR;
		}
		else if (strstr(res_str, "ERROR")) 
		{
			return FM_AT_RET_ERROR;
		}
		else if (strstr(res_str, "*READVER: ")) 
		{
			char *result = strstr(res_str, "Model_Name");
			if (result == NULL) 
			{
				return FM_STRING_PARSING_FAIL;
			}
			else
			{
				result += strlen("Model_Name: ");

				if (sscanf(result, "%s", model_name) != 1) 
				{
					return FM_STRING_PARSING_FAIL;
				} 
				else 
				{
					return FM_SUCCESS;
				}
			}
		}else 
		{
			return FM_AT_RET_OTHER_ERROR;
		}
	} 
	else 
	{
		return FM_AT_EXECUTE_FAIL;
	}
#elif FM_SYSTEM == FM_SYSTEM_RTOS_ML307R 
	int ret = 0;

	char at_str[64] = {0};
	uint8_t res_str[256] = {0};
	int32_t *res_len = NULL;

	if (model_name == NULL) 
	{
        return FM_PARAM_NULL;
    }

    res_len = malloc(sizeof(int32_t));

	sprintf(at_str, "AT+CGMM\r\n");
	ret = cm_virt_at_send_sync((const uint8_t *)at_str, res_str, res_len, 32);			//ML307R
	if (ret == 0) 
	{
		if (strstr((char *)res_str, "+CME ERROR")) 
		{
			ret = FM_AT_RET_CME_ERROR;
		} 
		else if (strstr((char *)res_str, "+CMS ERROR")) 
		{
			ret = FM_AT_RET_CMS_ERROR;
		} 
		else if (strstr((char *)res_str, "ERROR")) 
		{
			ret = FM_AT_RET_ERROR;
		} 
		else 
		{
			if (sscanf((char *)res_str, "%s", model_name) != 1) 
			{
				ret = FM_STRING_PARSING_FAIL;
			} 
			else 
			{
				ret = FM_SUCCESS;
			}
		}
	} 
	else 
	{
		ret = FM_AT_EXECUTE_FAIL;
	}

	free(res_len);

	return (FM_STATUS_t)ret;
#endif
}
#endif


#if FM_SYSTEM == FM_SYSTEM_LINUX_M24A
FM_STATUS_t fm_get_software_version(char *software_version) 
{
	int ret = 0;
	char res_str[256] = {0};
	char at_str[64] = {0};
	
	if (software_version == NULL) 
	{
        return FM_PARAM_NULL;
    }

	if (load_at_utils() != 0) 
	{
		ret = FM_LOAD_AT_UTILS_ERROR;
    }

	sprintf(at_str, "AT+ZVERSION\r\n");
	ret = SendATCMDWaitResp(at_str, res_str);			//14,464D3130319000
	if (ret == 0) 
	{
		if (res_str == NULL || res_str[0] == '\0' || strcmp(res_str, "") == 0 || strcmp(res_str, "0") == 0) 
		{
			ret = FM_AT_RET_OTHER_ERROR;
		} 
		else 
		{
			strncpy(software_version, res_str, strlen(res_str));
			ret = FM_SUCCESS;
		}
	} 
	else 
	{
		ret = FM_AT_EXECUTE_FAIL;
	}

	unload_at_utils();

	return (FM_STATUS_t)ret;
}
#endif

