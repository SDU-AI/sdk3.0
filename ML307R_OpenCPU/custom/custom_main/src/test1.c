#include "cm_os.h"
#include "cm_mem.h"
#include "cm_common.h"
#include "cm_sys.h"
#include "cm_demo_uart.h"
#include "cm_virt_at.h"

typedef enum QX_Network_Operator
{
    UNKNOWN_OPERATOR = 0,		/**< 无. */
    CHINA_UNICOM_OPERATOR,		/**< 中国联通运营商. */
    CHINA_MOBILE_OPERATOR,		/**< 中国移动运营商. */
    CHINA_TELECOM_OPERATOR,		/**< 中国电信运营商. */
} QX_Network_Operator_t;

typedef enum QX_STATUS
{
	QX_UNKNOWN = 0,				/**< 无. */
	QX_SUCCESS,					/**< 返回成功状态. */
    QX_ERROR,					/**< 返回失败状态. */
} QX_STATUS_t;

// 回调函数，用于处理AT指令的响应
void at_response_cb(cm_virt_at_param_t *param) {
    if (param->event == CM_VIRT_AT_OK) {
        // 打印接收到的响应
        cm_log_printf(0,"AT switch network response: rsp=%s rsp_len=%d\n", param->rsp, param->rsp_len);
    } else {
        // 打印错误信息
        cm_log_printf(0,"Error occurred while sending AT+CIMI\n");
    }
}

// 函数用于发送AT+CIMI指令并异步接收响应

 QX_STATUS_t qx_switching_operator(FM_Network_Operator_t net_operator) {
    char *operation = "AT+CLCK=\"SC\",1,\"0301\"\r\n"; // AT指令字符串
    if (cm_virt_at_send_async((const uint8_t *)operation, at_response_cb, "AT+CIMI") != 0) {
        // 如果发送失败，打印错误信息
        cm_demo_printf("Failed to send AT+CIMI command asynchronously\n");
        return QX_ERROR;
    }else{
        return QX_SUCCESS;
    }
}

