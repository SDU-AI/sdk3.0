/************************************************************************************************
 * Copyright (C), Flymodem (Shenzhen) Technology Co., Ltd.
 ************************************************************************************************
 * File Name  : fm_fs_port.c
 * Abstract   : fs接口实现FM fs读写文件命令功能
 * Version    : 3.1.0
 * Author     : LTH
 * Date       : 2024-07-24
 * Modifies   : [2024-07-24]新增[Author:LTH](无)
************************************************************************************************/

#ifndef _FM_FS_PORT_H_
#define _FM_FS_PORT_H_
#include "fm_config_port.h"

#if FM_SYSTEM == FM_SYSTEM_RTOS_ML307R
//typedef unsigned int __uint32_t;
typedef __uint32_t __mode_t;
typedef	__mode_t mode_t;		/* permissions */

//#define __SIZE_TYPE__ long unsigned int
//typedef __SIZE_TYPE__ _ssize_t;
typedef _ssize_t ssize_t;

//#define __SIZE_TYPE__ long unsigned int
typedef __SIZE_TYPE__ size_t;

//typedef long _off_t;
typedef _off_t __off_t;
typedef	__off_t off_t;		/* file offset */

/* 模组使用LittleFS文件系统，请按照下文描述的文件打开方式使用。错误使用时文件系统无法保证文件功能正常 */
#define FM_FS_RB            (0)      /*!< rb，打开一个二进制文件，文件必须存在，只允许读 */
#define FM_FS_WB            (1)      /*!< wb，新建一个二进制文件，已存在的文件内容清空，只允许写 */
#define FM_FS_AB            (2)      /*!< ab，打开或新建一个二进制文件，只允许在文件末尾追写 */
#define FM_FS_WBPLUS        (3)      /*!< wb+，新建一个二进制文件，已存在的文件内容清空，允许读写 */
#define FM_FS_ABPLUS        (4)      /*!< ab+，打开或新建一个二进制文件，可读，只允许在文件末尾追写 */
#define FM_FS_RBPLUS        (5)      /*!< rb+，打开一个二进制文件，文件必须存在，允许读写 */

#define FM_FS_SEEK_SET      (0)      /*!< SEEK_SET，文件开头 */
#define FM_FS_SEEK_CUR      (1)      /*!< SEEK_CUR，当前位置 */
#define FM_FS_SEEK_END      (2)      /*!< SEEK_END，文件结尾 */
#endif



// 函数功能：打开文件并返回文件描述符
// 参数：filename - 文件名
// 参数：flags - 打开文件的标志
// 参数：mode - 文件的权限掩码（仅在创建文件时使用）
// 返回值：文件描述符，如果打开失败则返回-1
int fm_open(const char* pathname, int flags, mode_t mode);

// 函数功能：读取文件内容
// 参数：fd - 文件描述符
// 参数：buf - 读取数据的缓冲区
// 参数：count - 读取数据的长度
// 返回值：实际读取的字节数，如果读取失败则返回-1
ssize_t fm_read(int fd, void* buf, size_t count);

// 函数功能：写入数据到文件
// 参数：fd - 文件描述符
// 参数：buf - 写入数据的缓冲区
// 参数：count - 写入数据的长度
// 返回值：实际写入的字节数，如果写入失败则返回-1
ssize_t fm_write(int fd, const void* buf, size_t count);

// 函数功能：关闭文件
// 参数：fd - 文件描述符
// 返回值：0，如果关闭失败则返回-1
int fm_close(int fd);

//函数功能：移动文件读写指针
//参数：fd - 文件描述符
//参数：offset - 偏移量
//参数：whence - 偏移起始位置
//返回值：返回新的文件读写指针位置，如果移动失败则返回-1
off_t fm_lseek(int fd, off_t offset, int whence);

#endif




