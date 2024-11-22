/************************************************************************************************
 * Copyright (C), Flymodem (Shenzhen) Technology Co., Ltd.
 ************************************************************************************************
 * File Name  : fm_pthread_port.h
 * Abstract   : pthread接口实现FM pthread功能
 * Version    : 3.1.0
 * Author     : LTH
 * Date       : 2024-07-24
 * Modifies   : [2024-07-24]新增[Author:LTH](无)
************************************************************************************************/
#ifndef	FM_PTHREAD_PORT_H
#define FM_PTHREAD_PORT_H
#include "fm_config_port.h"

#if FM_SYSTEM == FM_SYSTEM_RTOS_ML307R
//typedef unsigned int __uint32_t;

typedef __uint32_t uint32_t;

/** Thread ID identifies the thread. */
typedef void *osThreadId_t;

/** Entry point of a thread. */
typedef void (*osThreadFunc_t) (void *argument);

/** Data type that identifies secure software modules called by a process. */
typedef uint32_t TZ_ModuleId_t;

/** Priority values. */
/**
 typedef enum 
 {
  osPriorityIdle          = 0,         ///< Reserved for Idle thread.
  osPriorityLow           = 2,         ///< Priority: low
  osPriorityBelowNormal   = 3,         ///< Priority: below normal
  osPriorityNormal        = 4,         ///< Priority: normal
  osPriorityAboveNormal   = 5,         ///< Priority: above normal
  osPriorityHigh          = 6,         ///< Priority: high
  osPrioritySoftRealtime  = 7,         ///< Priority: realtime
  osPriorityHardRealtime  = 8,         ///< Priority: realtime
  osPriorityISR           = 32,        ///< Reserved for ISR deferred thread,Do not use!
  osPriorityError         = -1,         ///< System cannot determine priority or illegal priority.
  osPriorityReserved      = 0x7FFFFFFF  ///< Prevents enum down-size compiler optimization.
} osPriority_t;
*/

/** Attributes structure for thread. */
/**
typedef struct 
{
  const char                   *name;   ///< name of the thread. 
  uint32_t                 attr_bits;   ///< attribute bits. 
  void                      *cb_mem;    ///< memory for control block. 
  uint32_t                   cb_size;   ///< size of provided memory for control block. 
  void                   *stack_mem;    ///< memory for stack. 
  uint32_t                stack_size;   ///< size of stack. 
  osPriority_t              priority;   ///< initial thread priority (default: osPriorityNormal). 
  TZ_ModuleId_t            tz_module;   ///< TrustZone module identifier. 
  uint32_t                  reserved;   ///< reserved (must be 0). 
} osThreadAttr_t;
*/

/** Mutex ID identifies the mutex. */
typedef void *osMutexId_t;

/** Attributes structure for mutex. */
/**
typedef struct 
{
  const char                   *name;   ///< name of the mutex. 
  uint32_t                 attr_bits;   ///< attribute bits. 
  void                      *cb_mem;    ///< memory for control block. 
  uint32_t                   cb_size;   ///< size of provided memory for control block. 
} osMutexAttr_t;
*/

/** Event Flags ID identifies the event flags. */
typedef void *osEventFlagsId_t;

/** Attributes structure for event flags. */
/**
typedef struct 
{
  const char                   *name;   ///< name of the event flags. 
  uint32_t                 attr_bits;   ///< attribute bits. 
  void                      *cb_mem;    ///< memory for control block. 
  uint32_t                   cb_size;   ///< size of provided memory for control block. 
} osEventFlagsAttr_t;
*/
#endif


typedef struct 
{
    void *arg;

    // RTOS ML307R threads parameters
    osThreadId_t thread_id;
    osThreadFunc_t func;
    //const osThreadAttr_t *thread_attr;
    osThreadAttr_t *thread_attr;

    // RTOS ML307R mutex parameters
    osMutexId_t mutex_id;
    //const osMutexAttr_t *mutex_attr;
    osMutexAttr_t *mutex_attr;
    uint32_t timeout;

    //RTOS ML307R EventFlags parameters
    osEventFlagsId_t ef_id;
    //const osEventFlagsAttr_t *ef_attr;
    osEventFlagsAttr_t *ef_attr;
    uint32_t flags;
    uint32_t options;
    //uint32_t timeout;
} FM_Pthread_Parameters;



//函数功能：创建一个新的线程
int fm_pthread_create (FM_Pthread_Parameters *params);

//函数功能：创建一个互斥锁
int fm_pthread_mutex_create (FM_Pthread_Parameters *params);
//函数功能：加锁一个互斥锁
int fm_pthread_mutex_lock (FM_Pthread_Parameters *params);
//函数功能：解锁一个互斥锁
int fm_pthread_mutex_unlock (FM_Pthread_Parameters *params);
//函数功能：销毁一个互斥锁
int fm_pthread_mutex_delete (FM_Pthread_Parameters *params);

//初始化条件变量。
int fm_pthread_cond_create (FM_Pthread_Parameters *params);
// 销毁条件变量。
int fm_pthread_cond_delete (FM_Pthread_Parameters *params);
//等待条件变量的信号。
int fm_pthread_cond_timedwait (FM_Pthread_Parameters *params);
//发送信号给一个等待条件变量的线程。
int fm_pthread_cond_signal (FM_Pthread_Parameters *params);

#endif


