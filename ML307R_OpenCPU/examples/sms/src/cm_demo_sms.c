#include "cm_os.h"
#include "cm_mem.h"
#include "cm_common.h"
#include "cm_sys.h"
#include "cm_demo_uart.h"
#include "cm_virt_at.h"
#include "cm_sms.h"

void oc_msg_cb(uint8_t *urc, int32_t urc_len)
{
    cm_demo_printf("oc_msg_cb urc:%s urc_len:%d\n", urc, urc_len);
}
void cm__virt_at_urc_cb_reg_test(unsigned char **cmd,int len)
{
    cm_virt_at_urc_cb_reg((const uint8_t *)"+CMTI:",(cm_virt_at_urc_cb)oc_msg_cb);
}

void cm__virt_at_urc_cb_dereg_test(unsigned char **cmd,int len)
{
    cm_virt_at_urc_cb_dereg((const uint8_t *)"+CMTI:",(cm_virt_at_urc_cb)oc_msg_cb);
}

void cm_sendmsg_test_txt(unsigned char **cmd,int len)
{
    cm_sms_send_txt("zhongyimozu", "1914xxxxxxx", CM_MSG_MODE_GSM_7, CM_SIM_ID_0);//SIM0卡发送txt格式GSM7编码短信
//    cm_sms_send_txt("4E2D79FB6A217EC4", "1914xxxxxxx",  CM_MSG_MODE_GSM_8, CM_SIM_ID_0);//SIM0卡发送txt格式GSM8编码短信
}

void cm_sendmsg_test_pdu(unsigned char **cmd,int len)
{
    cm_sms_send_pdu("0881683108200305F021000D91683126388788F80008084E2D79FB6A217EC4", "22", CM_MSG_MODE_GSM_7, CM_SIM_ID_0);//SIM0卡发送PDU格式GSM7编码短信
//    cm_sms_send_pdu("683108200305F021000D91683126388788F800000B7AF4DB7DCEA7DB6F7D1D", "24",  CM_MSG_MODE_GSM_8, CM_SIM_ID_0);//SIM0卡发送PDU格式GSM8编码短信
}