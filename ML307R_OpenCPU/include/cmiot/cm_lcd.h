/**
 * @file        cm_lcd.h
 * @brief       内存管理
 * @copyright   Copyright © 2021 China Mobile IOT. All rights reserved.
 * @author      By surui
 * @date        2023/08/17
 *
 * @defgroup lcd
 * @ingroup lcd
 * @{
 */

#ifndef __CM_LCD_H__
#define __CM_LCD_H__

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include "stdint.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/**
 * @brief 配置时发送cmd
 *  
 * @param [in] cmd 需要发送的命令
 *
 * @return 
 *
 * @details More details
 */
void cm_lcd_write_cmd(unsigned char cmd);

/**
 * @brief 配置时发送data
 *  
 * @param [in] data 需要发送的数据
 *
 * @return 
 *
 * @details More details
 */
void cm_lcd_write_data(unsigned char data);

/**
 * @brief 发送刷屏数据
 *  
 * @param [in] data 需要发送的数据
 * @param [in] len  数据长度
 *
 * @return 
 *
 * @details More details
 */
void cm_lcd_write_buf(unsigned char *data, int len);

/**
 * @brief lcd 驱动初始化
 * 
 *
 * @return 
 *
 * @details More details
 */
void cm_lcd_init(void);

/**
 * @brief lcd 驱动去初始化
 * 
 *
 * @return 
 *
 * @details More details
 */
void cm_lcd_deinit(void);

/**
 * @brief lcd 获取ID
 * 
 *
 * @return 
 *   > 0 - lcd ID \n
 *   = 0 - 失败
 *
 * @details 只适配ST7735S
 */
uint32_t cm_lcd_read_id(void);

#endif
