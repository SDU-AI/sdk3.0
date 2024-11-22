
#ifndef __BSP_UART__H_
#define __BSP_UART__H_




/**
 *  \brief 串口接收数据结构体
 */
typedef struct 
{
    unsigned char buf[1024];
    int len;
}cm_uart_recv_data_buf_t;
/**命令结构体*/
typedef struct cm_cmd {
    char * cmdstr;
    void (* cmdfunc)(unsigned char **,int);
} cm_cmd_t;

/**
 *  \brief 串口接收命令接口
 */
typedef struct 
{
    int cmd_execute;  //命令执行标志，1未被执行，0执行完成
    unsigned char * buf[20]; //最多接受20个参数
    int len;
}cm_uart_cmd_recv_t;

extern cm_uart_cmd_recv_t gstUartCmdRecv;

void cm_demo_uart(void);
void cm_demo_printf (char *str, ...);
void my_printf(const char *str, ...);

void cm_test_uart_close(char **cmd, int len);

void bsp_uart_init(void);
#endif
















