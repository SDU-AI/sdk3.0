#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "cm_sys.h"
#include "cm_virt_at.h"
#include "cm_modem_info.h"
#include "cm_modem.h"
#include "cm_modem_info.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "cm_os.h"

int32_t cm_modem_get_cpin(void)
{
    int32_t cm_cpin = -1;
    uint8_t rsp[16] = {0};
    int32_t *rsp_len = NULL;
    rsp_len = malloc(sizeof(int32_t));
    if(cm_virt_at_send_sync((const uint8_t *)"AT+CPIN?\r\n", rsp, rsp_len, 32) == 0)
    {
		if(0 == strncmp((char *)rsp, "+CPIN: READY", 11))
        {
			cm_cpin = 0;
        }
    }
    free(rsp_len);
    return cm_cpin;
}

int32_t cm_modem_get_cgmr(char *cgmr)
{
    int32_t get_cgmr = -1;
    uint8_t rsp[20] = {0};
    int32_t *rsp_len = NULL;
    rsp_len = malloc(sizeof(int32_t));
    if(cm_virt_at_send_sync((const uint8_t *)"AT+CGMR\r\n", rsp, rsp_len, 32) == 0)
    {
        strcpy(cgmr, (char *)rsp);
        get_cgmr = 0;
    }
    free(rsp_len);
    return get_cgmr;
}

int32_t cm_modem_get_cgmm(char *cgmm)
{
    int32_t get_cgmm = -1;
    uint8_t rsp[8] = {0};
    int32_t *rsp_len = NULL;
    rsp_len = malloc(sizeof(int32_t));
    if(cm_virt_at_send_sync((const uint8_t *)"AT+CGMM\r\n", rsp, rsp_len, 32) == 0)
    {
        strcpy(cgmm, (char *)rsp);
        get_cgmm = 0;
    }
    free(rsp_len);
    return get_cgmm;
}

int32_t cm_modem_get_cgmi(char *cgmi)
{
    int32_t get_cgmi = -1;
    uint8_t rsp[6] = {0};
    int32_t *rsp_len = NULL;
    rsp_len = malloc(sizeof(int32_t));
    if(cm_virt_at_send_sync((const uint8_t *)"AT+CGMI\r\n", rsp, rsp_len, 32) == 0)
    {
        strcpy(cgmi, (char *)rsp);
        get_cgmi = 0;
    }
    free(rsp_len);
    return get_cgmi;
}

int32_t cm_modem_get_cops(cm_cops_info_t *cops)
{
    int32_t get_cops = -1;
    uint8_t rsp[30] = {0};
    int32_t *rsp_len = NULL;
    char *result = NULL;  
    rsp_len = malloc(sizeof(int32_t));
    if(cm_virt_at_send_sync((const uint8_t *)"AT+COPS?\r\n", rsp, rsp_len, 32) == 0)
    {
        if(0 == strncmp((char *)rsp, "+COPS", 5))
        {
            result = strtok((char *)rsp, ":");
            if(NULL != result)
                result = strtok(NULL, ":");
            result = strtok(result, ",");
            cops->mode = (uint8_t)atoi(result);
            result = strtok(NULL, ",");
            cops->format = (uint8_t)atoi(result);
            result = strtok(NULL, ",");
            memcpy(cops->oper, result, sizeof(cops->oper));
            result = strtok(NULL, ",");
            cops->act = (uint8_t)atoi(result);
            get_cops = 0;
        }
    }
    free(rsp_len);
    return get_cops;
}

int32_t cm_modem_get_csq(char *rssi, char *ber)
{
    int32_t get_csq = -1;
    uint8_t rsp[16] = {0};
    int32_t *rsp_len = NULL;
    char *result = NULL;
    char oc_rssi[8] = {0};
    char oc_ber[8] = {0};
    rsp_len = malloc(sizeof(int32_t));
    if(cm_virt_at_send_sync((const uint8_t *)"AT+CSQ\r\n", rsp, rsp_len, 32) == 0)
    {
        if(0 == strncmp((char *)rsp, "+CSQ", 4))
        {
            result = strtok((char *)rsp, ":");
            if(NULL != result)
                result = strtok(NULL, ":");
            result = strtok(result, ",");
            memcpy(oc_rssi, result, strlen(result));
            result = strtok(NULL, ",");
            memcpy(oc_ber, result, strlen(result));
            strcpy(rssi, oc_rssi);
            strcpy(ber, oc_ber); 
            get_csq = 0;
        }
    }
    free(rsp_len);
    return get_csq;
}

int32_t cm_modem_get_radio_info(cm_radio_info_t *radio_info)
{
    int ret = -1; 
    cm_cops_info_t *cops = NULL;
    cops = malloc(sizeof(cm_cops_info_t));
    ret = cm_modem_get_cops(cops);
    if(cops != NULL)
    {
        radio_info->rat = cops->mode;
    }
    free(cops);
    cm_modem_info_radio(radio_info);
    return ret;
}

int32_t cm_modem_get_cell_info(cm_cell_info_t cell_info[], uint16_t cell_info_num)
{
    int ret = -1;
    ret = cm_modem_info_cell(cell_info,cell_info_num);
    return ret;
}

int32_t cm_modem_set_cfun(uint16_t fun)
{
    int32_t ret = -1;
    uint8_t rsp[16] = {0};
    int32_t *rsp_len = NULL;
    char cfun_cmd[12] = {0};
    rsp_len = malloc(sizeof(int32_t));
    snprintf(cfun_cmd, sizeof(cfun_cmd), "AT+CFUN=%hd\r\n", fun);
    if(cm_virt_at_send_sync((const uint8_t *)cfun_cmd, rsp, rsp_len, 32) == 0)
    {
        if(0 == strncmp((char *)rsp, "OK", 2))
        {
            ret = 0;
        }
    }
    free(rsp_len);
    return ret;
}

int32_t cm_modem_get_cfun(void)
{
    int32_t cm_cfun = -1;
    uint8_t rsp[10] = {0};
    int32_t *rsp_len = NULL;
    char *result = NULL;
    rsp_len = malloc(sizeof(int32_t));
    if(cm_virt_at_send_sync((const uint8_t *)"AT+CFUN?\r\n", rsp, rsp_len, 32) == 0)
    {
        if(0 == strncmp((char *)rsp, "+CFUN", 5))
        {
            result = strtok((char *)rsp, ":");
            if(result != NULL)
                result = strtok(NULL, ":");
            result = strtok(result, ",");
        }
        cm_cfun = (int32_t)atoi(result);
    }
    free(rsp_len);
    return cm_cfun;
}

int32_t cm_modem_set_edrx_cfg(const cm_edrx_cfg_set_t *cfg)
{
    int32_t cfg_r = -1;
    uint8_t rsp[16] = {0};
    int32_t *rsp_len = NULL;
    char cfg_cmd[30] = {0};
    char edrx_value_tmp[5] = {0};
    rsp_len = malloc(sizeof(int32_t));
    itoa((cfg->requested_edrx_value), edrx_value_tmp, 2);
    snprintf(cfg_cmd, sizeof(cfg_cmd), "AT+CEDRXS=%d, %d, %s\r\n", cfg->mode, cfg->act_type, edrx_value_tmp);
    if(cm_virt_at_send_sync((const uint8_t *)cfg_cmd, rsp, rsp_len, 32) == 0)
    {
        if(0 == strncmp((char *)rsp, "OK", 2))
        {
            cfg_r = 0;
        }
    }
    free(rsp_len);
    return cfg_r;
}

int32_t cm_modem_get_edrx_cfg(cm_edrx_cfg_get_t *cfg)
{
    int32_t get_edrx_cfg = -1;
    uint8_t rsp[16] = {0};
    int32_t *rsp_len = NULL;
    char *result = NULL;
    char act_type[10] = {0};
    char edrx_value[10] = {0};
    rsp_len = malloc(sizeof(int32_t));
    if(cm_virt_at_send_sync((const uint8_t *)"AT+CEDRXS?\r\n", rsp, rsp_len, 32) == 0)
    {
        if(0 == strncmp((char *)rsp, "+CEDRXS", 7))
        {
            result = strtok((char *)rsp, ":");
            if(NULL != result)
                result = strtok(NULL, ":");
            result = strtok(result, ",");
            if(NULL != result)
            {
                if(strlen(result) + 1 <= sizeof(act_type))
                    memcpy(act_type, result, strlen(result) + 1);
                else
                {
                    free(rsp_len);
                    return -1;
                } 
					
            }
            result = strtok(NULL, "\"");
            if(NULL != result)
            {
                if(strlen(result) + 1 <= sizeof(edrx_value))
                    memcpy(edrx_value, result, strlen(result) + 1);
                else
                {
                    free(rsp_len);
                	return -1;
                } 				
            }
            cfg->act_type = (uint8_t) atoi(act_type);
            cfg->requested_edrx_value = (uint8_t)atoi(edrx_value);
            get_edrx_cfg = 0;            
        }
    }
    free(rsp_len);
    return get_edrx_cfg;
}

int32_t cm_modem_set_psm_cfg(const cm_psm_cfg_t *cfg)
{
    uint8_t rsp[16] = {0};
    int32_t *rsp_len = NULL;
    int32_t set_cfg = -1;
    char cfg_cmd[50] = {0};
    char tau_tmp[9] = {0};
    char active_time_tmp[9] = {0};
    rsp_len = malloc(sizeof(int32_t));
    itoa((cfg->requested_periodic_tau), tau_tmp, 2);
    itoa((cfg->requested_active_time), active_time_tmp, 2);
    snprintf(cfg_cmd, sizeof(cfg_cmd), "AT+CPSMS=%d,,, %s, %s\r\n", cfg->mode, tau_tmp, active_time_tmp);
    if(cm_virt_at_send_sync((const uint8_t *)cfg_cmd, rsp, rsp_len, 32) == 0)
    {
        if(0 == strncmp((char *)rsp, "OK", 2))
        {
            set_cfg = 0;
        }
    }
    free(rsp_len);
    return set_cfg;
}

int32_t cm_modem_get_psm_cfg(cm_psm_cfg_t *cfg)
{
    int32_t get_cfg = -1;
    uint8_t tau_tmp = 0;
    uint8_t active_time_tmp = 0;
    int8_t periodic_tau[10] = {0};
    int8_t active_time[10] = {0};
    uint8_t rsp[40] = {0};
    int32_t *rsp_len = NULL;
    char *result = NULL;
    char mode[4] = {0};
    rsp_len = malloc(sizeof(int32_t));
    if(cm_virt_at_send_sync((const uint8_t *)"AT+CPSMS?\r\n", rsp, rsp_len, 32) == 0)
    {
        if(0 == strncmp((char *)rsp, "+CPSMS", 5))
        {
            result = strtok((char *)rsp, ":");
            if(NULL != result)
                result = strtok(NULL, ":");
            result = strtok(result, ",");
            memcpy(mode, result, strlen(result));
            result = strtok(NULL, ",");
            result = strtok(NULL, ",");
            result = strtok(NULL, ",");
            memcpy(periodic_tau, result, strlen(result) + 1);
            result = strtok(NULL, ",");
            memcpy(active_time, result, strlen(result) + 1);
            get_cfg = 0;
        }
        for(int32_t i = 8;i > 0;i--)
        {
            if('1' == periodic_tau[i])
            {
                tau_tmp = tau_tmp + pow(2, 8 - i);
            }
            if('1' == active_time[i])
            {
                active_time_tmp = active_time_tmp + pow(2,8 - i);
            }
        }
        cfg->mode = (uint8_t)atoi(mode);
        cfg->requested_active_time = active_time_tmp;
        cfg->requested_periodic_tau = tau_tmp; 
    }
    free(rsp_len);
    return get_cfg;
}

int32_t cm_modem_get_cereg_state(cm_cereg_state_t *cereg)
{
    int32_t get_cereg_state = -1;
    uint8_t rsp[40] = {0};
    uint32_t ci_num = 0;
    uint16_t lac_num = 0;
    char *result = NULL;
    char n[5]  = {0};
    char state[5]  = {0};
    char lac[7]  = {0};
    char ci[11]  = {0};
    char act[5]  = {0};
    int32_t *rsp_len = NULL;
    rsp_len = malloc(sizeof(int32_t));
    if(cm_virt_at_send_sync((const uint8_t *)"AT+CEREG=5;+CEREG?\r\n", rsp, rsp_len, 32) == 0)
    {
        if(0 == strncmp((char *)rsp, "+CEREG", 6))
        {
            result = strtok((char *)rsp, ":");
            if(NULL != result)
                result = strtok(NULL, ":");
            result = strtok(result, ",");
            memcpy(n, result, strlen(result));
            result = strtok(NULL, ",");
            memcpy(state, result, strlen(result));
            result = strtok(NULL, ",");
            memcpy(lac, result, strlen(result));
            result = strtok(NULL, ",");
            memcpy(ci, result, strlen(result));
            result = strtok(NULL, "O");
            memcpy(act, result, strlen(result));
            get_cereg_state = 0;
        }
        for(int32_t i = 8;i > 0;i--)
        {
            if(ci[i] >= 'A')
            {
                ci_num = ci_num + pow(16, (8 - i))*(ci[i] - 55);
            }
            else
            {
                ci_num = ci_num + pow(16, (8 - i))*(ci[i] - 48);
            }
        }
        for(int32_t i = 4; i > 0; i--)
        {
            lac_num = lac_num + pow(10, (4 - i))*(lac[i] - 48);
        }    
        cereg->n = (uint8_t)atoi(n);
        cereg->state = (uint8_t)atoi(state);
        cereg->lac = lac_num;
        cereg->ci = ci_num;
        cereg->act =  (uint8_t)atoi(act);
    }
    free(rsp_len);
    return get_cereg_state;
}

int32_t cm_modem_get_cscon(void)
{
    int32_t cm_cscon = -1;
    uint8_t rsp[16] = {0};
    int32_t *rsp_len = NULL;
    char *result = NULL;
    rsp_len = malloc(sizeof(int32_t));
    if(cm_virt_at_send_sync((const uint8_t *)"AT+CSCON?\r\n", rsp, rsp_len, 32) == 0)
    {
        if(0 == strncmp((char *)rsp, "+CSCON", 6))
        {
            result = strtok((char *)rsp, ":");
            if(NULL != result)
                result = strtok(NULL, ":");
            result = strtok(result, ",");
            result = strtok(NULL, ",");
            result = strtok(NULL, ",");
            cm_cscon = (int32_t)atoi(result);
        }
    }
    free(rsp_len);
    return cm_cscon;
}

int32_t cm_modem_activate_pdp(uint16_t cid)
{
    int32_t pdp_r = -1;
    uint8_t rsp[16] = {0};
    int32_t *rsp_len = NULL;
    char at_cmd[20] ={0};
    char cid_cmd[5] = {0};
    rsp_len = malloc(sizeof(int32_t));
    itoa((uint16_t)cid, cid_cmd, 10);
    snprintf(at_cmd, sizeof(at_cmd), "AT+CGACT=1, %s\r\n", cid_cmd);
    if(cm_virt_at_send_sync((const uint8_t *)at_cmd, rsp, rsp_len, 32) == 0)
    {
        if(0 == strncmp((char *)rsp, "OK", 2))
        {
            pdp_r = 0;
        }
    }
    free(rsp_len);
    return pdp_r;
}

int32_t cm_modem_deactivate_pdp(uint16_t cid)
{
    int32_t pdp_r = -1;
    uint8_t rsp[16] = {0};
    int32_t *rsp_len = NULL;
    char at_cmd[20] ={0};
    char cid_cmd[5] = {0};
    rsp_len = malloc(sizeof(int32_t));
    itoa((uint16_t)cid, cid_cmd, 10);
    snprintf(at_cmd, sizeof(at_cmd), "AT+CGACT=0, %s\r\n", cid_cmd);
    if(cm_virt_at_send_sync((const uint8_t *)at_cmd, rsp, rsp_len, 32) == 0)
    {
        if(0 == strncmp((char *)rsp, "OK", 2))
        {
            pdp_r = 0;
        }
    }
    free(rsp_len);
    return pdp_r;
}

int32_t cm_modem_get_pdp_state(uint16_t cid)
{
    int32_t pdp_state0 = -1;
    int32_t pdp_state1 = -1;
    int32_t pdp_state = -1;
    uint8_t rsp[16] = {0};
    int32_t *rsp_len = NULL;
    char *result = NULL;
    rsp_len = malloc(sizeof(int32_t));
    if(cm_virt_at_send_sync((const uint8_t *)"AT+CGACT?\r\n", rsp, rsp_len, 32) == 0)
    {
        if(0 == strncmp((char *)rsp, "+CGACT", 6))
        {
            result = strtok((char *)rsp, ".");
            while(1)
            {
                result = strtok(result, ":");
                if(NULL != result)
                    result = strtok(NULL, ".");
                result = strtok(result, ",");
                pdp_state0 = (unsigned short)atoi(result);
                result = strtok(NULL, "+");
                pdp_state1 = atoi(result);
                result = strtok(NULL, ".");
                if(cid == pdp_state0)
                {
                    pdp_state = pdp_state1;
                    break;
                }
                if(NULL == result)
                {
                    break;
                }
            }
        }
    }
    free(rsp_len);
    return pdp_state;
}
