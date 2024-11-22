/**
 ******************************************************************************
 * @file p2x.h
 * @author  zailushang
 * @brief
 * @version V1.0.0
 * @date    2024/6/24
 * 
 * @copyright Copyright (c) Flymodem (Shenzhen) Technology Co., Ltd. All Rights Reserved.
 ******************************************************************************
 */

#ifndef _MAIN_H_
#define _MAIN_H_

#include <stdio.h>
#include <unistd.h>

/*软件版本*/
#define P2X_SOFT_VERSION    "P2X_20240723_V1.0.0"

#define DEFAULT_LOG_PATH "/etc_rw/fly_p2xclient.log"

//typedef int OSA_STATUS

#define VERSION_LEN 64

#define MODEM "M888A"

#define OS_SUCCESS 0

#define FM_SLEEP(x) sleep((x))
#define FM_USLEEP(x) usleep((x))

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE !TRUE
#endif

#endif /* _MAIN_H_ */
