/************************************************************************************************
 * Copyright (C), Flymodem (Shenzhen) Technology Co., Ltd.
 ************************************************************************************************
 * File Name  : transport_interface_asr.c
 * Abstract   : transport_interface_asr接口实现FM MQTT tcp连接功能
 * Version    : 3.1.0
 * Author     : LTH
 * Date       : 2024-08-08
 * Modifies   : [2024-08-08]新增[Author:LTH](无)
************************************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "transport_interface.h"
#include "fm_config_port.h"
/**********************************************************
 * 适配ASR RTOS, START
 *********************************************************/
#define AF_UNSPEC           0
#define SOCK_STREAM         1
#define F_GETFL             3
#define F_SETFL             4
#define O_NONBLOCK          1

#define  EAGAIN             11
#define  EWOULDBLOCK        EAGAIN

typedef uint8_t u8_t;
typedef uint32_t u32_t;
typedef u32_t socklen_t;
typedef u8_t sa_family_t;
typedef int _ssize_t;
typedef unsigned char UINT8;
typedef unsigned int UINT32;

#define SOL_SOCKET      0xfff
#define SO_SNDTIMEO     0x1005  /* Unimplemented: send timeout */
#define SO_RCVTIMEO     0x1006  /* receive timeout */


struct sockaddr 
{
    u8_t sa_len;
    sa_family_t sa_family;
    char sa_data[14];
};

struct timeval 
{
    int tv_sec;        /* seconds */
    int tv_usec;       /* and microseconds */
};

struct addrinfo 
{
    int ai_flags;      /* Input flags. */
    int ai_family;     /* Address family of socket. */
    int ai_socktype;   /* Socket type. */
    int ai_protocol;   /* Protocol of socket. */
    socklen_t ai_addrlen;    /* Length of socket address. */
    struct sockaddr *ai_addr;       /* Socket address of socket. */
    char *ai_canonname;  /* Canonical name of service location. */
    struct addrinfo *ai_next;       /* Pointer to next in list. */
};


#if FM_SYSTEM == FM_SYSTEM_RTOS_1803S
// 在 1803S RTOS  上运行
extern void OSATaskSleep(UINT32 ticks);

extern int lwip_socket(int domain, int type, int protocol);

extern int lwip_connect(int s, const struct sockaddr *name, socklen_t namelen);

extern int lwip_close(int s);

extern int lwip_fcntl(int s, int cmd, int val);

extern int lwip_getaddrinfo(const char *nodename, const char *servname,
                            const struct addrinfo *hints, struct addrinfo **res);

extern void lwip_freeaddrinfo(struct addrinfo *ai);

extern _ssize_t lwip_recv(int s, void *mem, size_t len, int flags);

extern _ssize_t lwip_send(int s, const void *data, size_t size, int flags);

extern _ssize_t lwip_write(int s, const void *data, size_t size);

extern _ssize_t lwip_read(int s, void *mem, size_t len);

extern int lwip_setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen);


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
    UARTLogPrintf_Extend(LOG_MODULE_MAX, fmt, ##args);                                  \
}
#elif FM_SYSTEM == FM_SYSTEM_LINUX_M24A
// 在 Linux  上运行
extern unsigned int sleep(unsigned int seconds);

extern int socket(int domain, int type, int protocol);

extern int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

extern int close(int fd);

extern int fcntl(int fd, int cmd, ... /* arg */ );

extern int getaddrinfo(const char *node, const char *service,
                const struct addrinfo *hints, struct addrinfo **res);

extern void freeaddrinfo(struct addrinfo *res);

extern _ssize_t recv(int sockfd, void *buf, size_t len, int flags);

extern _ssize_t send(int sockfd, const void *buf, size_t len, int flags);

extern _ssize_t write(int fd, const void *buf, size_t count);

extern _ssize_t read(int fd, void *buf, size_t count);

extern int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);

/* Log module type. */
enum LogLevel 
{
    LogLevel_TRACE,
    LogLevel_DEBUG,
    LogLevel_INFO,
    LogLevel_WARN,
    LogLevel_ERROR,
    LogLevel_FATAL,
};

#if defined(_WIN32) || defined(_WIN64)
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif /* defined(_WIN32) || defined(_WIN64) */

extern void logger_log(enum LogLevel level, const char *file, int line, const char *fmt, ...);
#define LOG_INFO(fmt, ...)  logger_log(LogLevel_INFO , __FILENAME__, __LINE__, fmt, ##__VA_ARGS__)
#elif FM_SYSTEM == FM_SYSTEM_RTOS_ML307R
/** Status code values returned by CMSIS-RTOS functions. */
typedef enum 
{
  osOK                      =  0,         /**< Operation completed successfully. */
  osError                   = -1,         /**< Unspecified RTOS error: run-time error but no other error message fits. */
  osErrorTimeout            = -2,         /**< Operation not completed within the timeout period. */
  osErrorResource           = -3,         /**< Resource not available. */
  osErrorParameter          = -4,         /**< Parameter error. */
  osErrorNoMemory           = -5,         /**< System is out of memory: it was impossible to allocate or reserve memory for the operation. */
  osErrorISR                = -6,         /**< Not allowed in ISR context: the function cannot be called from interrupt service routines. */
  osStatusReserved          = 0x7FFFFFFF  /**< Prevents enum down-size compiler optimization. */
} osStatus_t;

extern osStatus_t osDelay(uint32_t ticks);

extern int lwip_socket(int domain, int type, int protocol);

extern int lwip_connect(int s, const struct sockaddr *name, socklen_t namelen);

extern int lwip_close(int s);

extern int lwip_fcntl(int s, int cmd, int val);

extern int lwip_getaddrinfo(const char *nodename, const char *servname,
                    const struct addrinfo *hints, struct addrinfo **res);

extern void lwip_freeaddrinfo(struct addrinfo *ai);

extern int lwip_recv(int s, void *mem, size_t len, int flags);

extern int lwip_send(int s, const void *dataptr, size_t size, int flags);

extern int lwip_write(int s, const void *dataptr, size_t size);

extern int lwip_read(int s, void *mem, size_t len);

extern int lwip_setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen);

extern void cm_log_printf(uint8_t log_level, const char *fmt, ...);
#endif


/**********************************************************
 * 适配ASR RTOS, END
 *********************************************************/
int32_t transport_open_func(NetworkContext_t *pNetworkContext) 
{
    struct addrinfo hints = {0};
    int sockfd = -1, ret = -1;
    struct addrinfo *p, *servinfo;

    if (pNetworkContext == NULL)
        return -1;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

#if FM_SYSTEM == FM_SYSTEM_RTOS_1803S || FM_SYSTEM == FM_SYSTEM_RTOS_ML307R 
    ret = lwip_getaddrinfo(pNetworkContext->server_addr, pNetworkContext->server_port, &hints, &servinfo);
#elif FM_SYSTEM == FM_SYSTEM_LINUX_M24A
    ret = getaddrinfo(pNetworkContext->server_addr, pNetworkContext->server_port, &hints, &servinfo);
#endif
    
    if (ret != 0) 
    {
        return -1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) 
    {
#if FM_SYSTEM == FM_SYSTEM_RTOS_1803S || FM_SYSTEM == FM_SYSTEM_RTOS_ML307R 
        sockfd = lwip_socket(p->ai_family, p->ai_socktype, p->ai_protocol);
#elif FM_SYSTEM == FM_SYSTEM_LINUX_M24A
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
#endif
        if (sockfd == -1)
            continue;
#if FM_SYSTEM == FM_SYSTEM_RTOS_1803S || FM_SYSTEM == FM_SYSTEM_RTOS_ML307R 
        ret = lwip_connect(sockfd, p->ai_addr, p->ai_addrlen);
#elif FM_SYSTEM == FM_SYSTEM_LINUX_M24A
        ret = connect(sockfd, p->ai_addr, p->ai_addrlen);
#endif
        if (ret == -1) 
        {
#if FM_SYSTEM == FM_SYSTEM_RTOS_1803S || FM_SYSTEM == FM_SYSTEM_RTOS_ML307R 
            lwip_close(sockfd);
#elif FM_SYSTEM == FM_SYSTEM_LINUX_M24A
            close(sockfd);
#endif
            sockfd = -1;
            continue;
        }
        break;
    }
#if FM_SYSTEM == FM_SYSTEM_RTOS_1803S || FM_SYSTEM == FM_SYSTEM_RTOS_ML307R 
    lwip_freeaddrinfo(servinfo);
#elif FM_SYSTEM == FM_SYSTEM_LINUX_M24A
    freeaddrinfo(servinfo);
#endif
    if (sockfd != -1) 
    {
        //lwip_fcntl(sockfd, F_SETFL, O_NONBLOCK);
        //fcntl(sockfd, F_SETFL, O_NONBLOCK);
        pNetworkContext->socket_fd = sockfd;
        ret = 0;
    }
    return ret;
}

int32_t transport_close_func(NetworkContext_t *pNetworkContext) 
{
    if (pNetworkContext != NULL) 
    {
        if (pNetworkContext->socket_fd > 0) 
        {
#if FM_SYSTEM == FM_SYSTEM_RTOS_1803S || FM_SYSTEM == FM_SYSTEM_RTOS_ML307R 
            lwip_close(pNetworkContext->socket_fd);
#elif FM_SYSTEM == FM_SYSTEM_LINUX_M24A
            close(pNetworkContext->socket_fd);
#endif
            pNetworkContext->socket_fd = -1;
        }
    }
    return 0;
}

int32_t transport_send_func(NetworkContext_t *pNetworkContext, const void *pBuffer, size_t bytesToSend) 
{
    _ssize_t ret;
    int32_t sent = 0;
    struct timeval tv;

    //printf("Send func bytesToSend : %d", bytesToSend);
    tv.tv_sec = 10;
    tv.tv_usec = 0;

#if FM_SYSTEM == FM_SYSTEM_RTOS_1803S || FM_SYSTEM == FM_SYSTEM_RTOS_ML307R 
    lwip_setsockopt(pNetworkContext->socket_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval));
#elif FM_SYSTEM == FM_SYSTEM_LINUX_M24A
    setsockopt(pNetworkContext->socket_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval));
#endif

    while (sent < bytesToSend) 
    {
#if FM_SYSTEM == FM_SYSTEM_RTOS_1803S || FM_SYSTEM == FM_SYSTEM_RTOS_ML307R 
        ret = lwip_write(pNetworkContext->socket_fd, (const char *) pBuffer + sent, bytesToSend - sent);
#elif FM_SYSTEM == FM_SYSTEM_LINUX_M24A
        ret = write(pNetworkContext->socket_fd, (const char *) pBuffer + sent, bytesToSend - sent);
#endif
        //printf("Send func rv : %d", ret);
        if (ret < 0) 
        {
#if FM_SYSTEM == FM_SYSTEM_RTOS_1803S
            CPUartLogPrintf("Send func Error reading from socket! ++++++++++++++++++++++++++++++++++++++++++++++++\n");
#elif FM_SYSTEM == FM_SYSTEM_LINUX_M24A
			printf("Send func Error reading from socket! ++++++++++++++++++++++++++++++++++++++++++++++++\n");
            //LOG_INFO("Send func Error reading from socket! ++++++++++++++++++++++++++++++++++++++++++++++++\n");
#elif FM_SYSTEM == FM_SYSTEM_RTOS_ML307R
            cm_log_printf(0, "Send func Error reading from socket! ++++++++++++++++++++++++++++++++++++++++++++++++\n");
#endif
			//transport_close_func(pNetworkContext);
			//transport_open_func(pNetworkContext);
            break;
        }

        if (ret == 0) 
        {
            break;
        }

        sent += ret;
    }

    //printf("Send func sent : %d", sent);
    return sent;
}

int32_t transport_recv_func(NetworkContext_t *pNetworkContext, void *pBuffer, size_t bytesToRecv) 
{
    _ssize_t ret;
    const void *const start = pBuffer;
    struct timeval tv;
	//int32_t ret_rec;

    //printf("Recv func bytesToRecv : %d", bytesToRecv);
    tv.tv_sec = 10;
    tv.tv_usec = 0;

#if FM_SYSTEM == FM_SYSTEM_RTOS_1803S || FM_SYSTEM == FM_SYSTEM_RTOS_ML307R 
    lwip_setsockopt(pNetworkContext->socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
#elif FM_SYSTEM == FM_SYSTEM_LINUX_M24A
    setsockopt(pNetworkContext->socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
#endif

    do {
#if FM_SYSTEM == FM_SYSTEM_RTOS_1803S || FM_SYSTEM == FM_SYSTEM_RTOS_ML307R 
        ret = lwip_read(pNetworkContext->socket_fd, pBuffer, bytesToRecv);
#elif FM_SYSTEM == FM_SYSTEM_LINUX_M24A
        ret = read(pNetworkContext->socket_fd, pBuffer, bytesToRecv);
#endif
        //printf("Recv func rv : %d", ret);
        if (ret == 0) 
        {
            break;
        }

        if (ret < 0) 
        {
#if FM_SYSTEM == FM_SYSTEM_RTOS_1803S
            CPUartLogPrintf("Recv func Error reading from socket! ************************************************\n");
            OSATaskSleep(5 * 200);
#elif FM_SYSTEM == FM_SYSTEM_LINUX_M24A
			printf("Recv func Error reading from socket! ************************************************\n");
            //LOG_INFO("Recv func Error reading from socket! ************************************************\n");
            sleep(5);
#elif FM_SYSTEM == FM_SYSTEM_RTOS_ML307R
            cm_log_printf(0, "Recv func Error reading from socket! ************************************************\n");
            osDelay(5 * 200);
#endif
			//transport_close_func(pNetworkContext);
			//transport_open_func(pNetworkContext);
            break;
        }
        pBuffer = (char *) pBuffer + ret;
        bytesToRecv -= ret;
    } while (bytesToRecv > 0);

    //printf("Recv func : %d", (char *) pBuffer - (const char *) start);
    return (char *) pBuffer - (const char *) start;
}





