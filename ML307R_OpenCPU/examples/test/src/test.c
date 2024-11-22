
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

/*
#include "flymodem.h"

#include <pthread.h> // 包含线程库头文件
*/
// 线程入口函数
// 回调函数，用于处理AT指令的响应

void at_response_cb(cm_virt_at_param_t *param) {
    if (param->event == CM_VIRT_AT_OK) {
        // 打印接收到的响应
        cm_demo_printf("AT+CIMI response: rsp=%s rsp_len=%d\n", param->rsp, param->rsp_len);
    } else {
        // 打印错误信息
        cm_demo_printf("Error occurred while sending AT+CIMI\n");
    }
}

// 函数用于发送AT+CIMI指令并异步接收响应
void send_at_cimi() {
    char *operation = "AT+CIMI\r\n"; // AT指令字符串
    if (cm_virt_at_send_async((const uint8_t *)operation, at_response_cb, "AT+CIMI") != 0) {
        // 如果发送失败，打印错误信息
        cm_demo_printf("Failed to send AT+CIMI command asynchronously\n");
    }
}
void test_printf(void) {
    cm_demo_uart();

    // 发送AT+CIMI指令
    send_at_cimi();
	
    cm_log_printf(0, "hello... 1.2..");
    cm_log_printf(0, "hello... .2.3.");

    // 清理线程属性
    //pthread_attr_destroy(&attr);
}










