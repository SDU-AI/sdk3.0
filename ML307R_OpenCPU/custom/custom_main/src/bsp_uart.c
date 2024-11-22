
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

#include "bsp_uart.h"
/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
 
#define UART_BUF_LEN            1024

/****************************************************************************
 * Private Types
 ****************************************************************************/
 
typedef struct{
    int msg_type;
} uart_event_msg_t;

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/
 
void cm_demo_printf(char *str, ...);
static void cm_uart_recv_task(void *param);


/****************************************************************************
 * Private Data
 ****************************************************************************/

static int rx_rev_len = 0;
static char rx_rev_data[UART_BUF_LEN] = {0};
static osThreadId_t OC_Uart_TaskHandle = NULL; //串口数据接收、解析任务Handle
static void* g_uart_sem = NULL;
static osMessageQueueId_t uart_event_queue = NULL;
static osThreadId_t uart_event_thread = NULL;

cm_uart_cmd_recv_t gstUartCmdRecv = {0}; //串口命令缓冲区



/****************************************************************************
 * Private Functions
 ****************************************************************************/

/* 用于测试串口事件，用户可参考 */
static void uart_event_task(void *arg)
{
    uart_event_msg_t msg = {0};

    while (1)
    {
        if (osMessageQueueGet(uart_event_queue, &msg, NULL, osWaitForever) == osOK)
        {
            //cm_log_printf(0, "uart event msg type = %d\n", msg.msg_type);
            if (CM_UART_EVENT_TYPE_RX_OVERFLOW & msg.msg_type)
            {
                cm_log_printf(0, "CM_UART_EVENT_TYPE_RX_OVERFLOW... ...");
            }
        }
    }
}

/* 用于测试串口事件，用户可参考 */
static int uart_event_task_create(void)
{
    if (uart_event_queue == NULL)
    {
        uart_event_queue = osMessageQueueNew(10, sizeof(uart_event_msg_t), NULL);
    }

    if (uart_event_thread == NULL)
    {
        osThreadAttr_t attr1 = {
            .name = "uart_event",
            .priority = UART_TASK_PRIORITY,
            .stack_size = 1024,
        };
        uart_event_thread = osThreadNew(uart_event_task, NULL, (const osThreadAttr_t*)&attr1);
    }

    return 0;
}



/* 回调函数中不可输出LOG、串口打印、执行复杂任务或消耗过多资源，建议以信号量或消息队列形式控制其他线程执行任务 */
static void cm_serial_uart_callback(void *param, uint32_t type)
{
    uart_event_msg_t msg = {0};
    if (CM_UART_EVENT_TYPE_RX_ARRIVED & type)
    {
        /* 收到接收事件，触发其他线程执行读取数据 */
        osSemaphoreRelease(g_uart_sem);
    }
    
    if (CM_UART_EVENT_TYPE_RX_OVERFLOW & type)
    {
        /* 收到溢出事件，触发其他线程处理溢出事件 */
        msg.msg_type = type;
        
        if (uart_event_queue != NULL)//向队列发送数据
        {
            osMessageQueuePut(uart_event_queue, &msg, 0, 0);
        }
    }
}

/* 串口接收示例，平时使用信号量挂起，当收到接收事件后，释放信号量以触发读取任务 */
static void cm_uart_recv_task(void *param)
{
    int temp_len = 0;


    
    while (1)
    {
        if (g_uart_sem != NULL)
        {
            osSemaphoreAcquire(g_uart_sem, osWaitForever);//阻塞
        }
        
        if (rx_rev_len < UART_BUF_LEN)
        {
            temp_len = cm_uart_read(OPENCPU_MAIN_URAT, (void*)&rx_rev_data[rx_rev_len], UART_BUF_LEN - rx_rev_len, 1000);
            rx_rev_len += temp_len;
           cm_uart_write(OPENCPU_MAIN_URAT, rx_rev_data, rx_rev_len, 1000);
           rx_rev_len = 0;
            
        }


    }
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/* 从测试串口打印字符串 */
void cm_demo_printf(char *str, ...)
{
    static char s[600]; //This needs to be large enough to store the string TODO Change magic number
    va_list args;
    int len;
    
    if ((str == NULL) || (strlen(str) == 0))
    {
        return;
    }

    va_start(args, str);
    len = vsnprintf((char*)s, 600, str, args);
    va_end(args);
    cm_uart_write(OPENCPU_MAIN_URAT, s, len, 1000);
}

void my_printf(const char *str, ...)
{
    static char s[1000]; //This needs to be large enough to store the string TODO Change magic number
    va_list args;
    int len;
    
    if ((str == NULL) || (strlen(str) == 0))
    {
        return;
    }

    va_start(args, str);
    len = vsnprintf((char*)s, 1000, str, args);
    va_end(args);
    cm_uart_write(OPENCPU_MAIN_URAT, s, len, 1000);
    //uart_write_bytes(UART_NUM_1, (const char *) p_buf, len);
}

/* 若要修改测试串口可在cm_common.h中修改宏定义 */
void cm_demo_uart(void)
{
    int32_t ret = -1;

    /* 配置参数 */
    cm_uart_cfg_t config = 
    {
        CM_UART_BYTE_SIZE_8, 
        CM_UART_PARITY_NONE,
        CM_UART_STOP_BIT_ONE, 
        CM_UART_FLOW_CTRL_NONE, 
        CM_UART_BAUDRATE_115200,
        0   //配置为普通串口模式，若要配置为低功耗模式可改为1
    };

    /* 事件参数 */
    cm_uart_event_t event = 
    {
        CM_UART_EVENT_TYPE_RX_ARRIVED|CM_UART_EVENT_TYPE_RX_OVERFLOW,   //注册需要上报的事件类型
        "uart0",                                                        //用户参数
        cm_serial_uart_callback                                         //上报事件的回调函数
    };

    cm_log_printf(0, "uart NUM = %d demo start... ...", OPENCPU_MAIN_URAT);

    /* 配置引脚复用 */
    cm_iomux_set_pin_func(OPENCPU_TEST_UARTTX_IOMUX);
    cm_iomux_set_pin_func(OPENCPU_TEST_UARTRX_IOMUX);

    /* 注册事件和回调函数 */
    ret = cm_uart_register_event(OPENCPU_MAIN_URAT, &event);
    
    if (ret != RET_SUCCESS)
    {
        cm_log_printf(0, "uart register event err,ret=%d\n", ret);
        return;
    }

    /* 开启串口 */
    ret = cm_uart_open(OPENCPU_MAIN_URAT, &config);
    
    if (ret != RET_SUCCESS)
    {
        cm_log_printf(0, "uart init err,ret=%d\n", ret);
        return;
    }

    /* 配置串口唤醒 */
    /* 只有UART0具有串口唤醒功能 */
    if (OPENCPU_MAIN_URAT == CM_UART_DEV_0)
    {
        /* 配置uart唤醒功能，使能边沿检测才具备唤醒功能，仅主串口具有唤醒功能，用于唤醒的数据并不能被uart接收，请在唤醒后再进行uart数传 */
        cm_iomux_set_pin_cmd(OPENCPU_UART_WEKEUP_PIN, CM_IOMUX_PINCMD1_LPMEDEG, CM_IOMUX_PINCMD1_FUNC1_LPM_EDGE_RISE);
    }

    cm_log_printf(0, "cm_uart_register_event start... ...\n");

    /* 以下为串口接收示例，不影响串口配置，用户可酌情参考 */
    osThreadAttr_t uart_task_attr = {0};
    uart_task_attr.name = "uart_task";
    uart_task_attr.stack_size = 2048;
    uart_task_attr.priority= UART_TASK_PRIORITY;
    gstUartCmdRecv.cmd_execute = 0;
    
    if (g_uart_sem == NULL)
    {
        g_uart_sem = osSemaphoreNew(1, 0, NULL);
    }

    OC_Uart_TaskHandle = osThreadNew(cm_uart_recv_task, 0, &uart_task_attr);

    uart_event_task_create();
}

/* 关闭串口 */
void cm_test_uart_close(char **cmd, int len)
{
    cm_uart_dev_e dev = atoi(cmd[2]);
    
    if (0 == cm_uart_close(dev))
    {
        cm_demo_printf("uart%d close is ok\n", dev);
    }
    
    else
    {
        cm_demo_printf("uart%d close is error\n", dev);
    }
}
void bsp_uart_init(void)
{
    cm_demo_uart();
    cm_demo_printf("\r\nuart_init\r\n");
}

