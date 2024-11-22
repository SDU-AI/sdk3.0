/*********************************************************
 *  @file    cm_demo_ftp.c
 *  @brief   OpenCPU ftp示例
 *  Copyright (c) 2023 China Mobile IOT.
 *  All rights reserved.
 *  created by ShiMingRui 2023/6/29
 ********************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdio.h> 
#include "string.h"
#include "cm_ftp.h"
#include "cm_demo_ftp.h"
#include "cm_demo_uart.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/


/****************************************************************************
 * Private Types
 ****************************************************************************/


/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/
 
 
/****************************************************************************
 * Private Data
 ****************************************************************************/

/* 测试前请先补充如下参数 */
static int32_t port = 21;   //测试端口
static char hostname[16] = "";    //测试地址
static char username[16] = "";    //测试用户名
static char password[16] = "";    //测试密码
static char dir[32] = "/ml307a_test_dir";   //测试文件夹名，请根据实际平台情况修改
static char home_dir[32] = "/home/uftp";    //测试文件夹名，请根据实际平台情况修改
static char file[32] = "ml307a_ftp_test7.txt";  //测试文件名，请根据实际平台情况修改
static char new_file[32] = "ml307a_ftp_test8.txt";  //测试文件名，请根据实际平台情况修改
static char get_dir[256] = {0};
static char data[32] = "1234567890";    //测试数据
static char out_data[32] = {0};
static cm_ftp_file_data_t file_data_test = {0};

/****************************************************************************
 * Private Functions
 ****************************************************************************/


/****************************************************************************
 * Public Functions
 ****************************************************************************/

/**
 *  FTP功能调试使用示例，注意使用前需先设置port，hostname，username，password
 *
 */
void cm_test_ftp(unsigned char **cmd,int len)
{
    int32_t handle = -1;
    int32_t ret = -1;
    int32_t find_fd1 = -1;

    cm_ftp_state_e state = 0;

    /* 配置连接参数 */
    cm_ftp_config_t config = {0};
    config.url = (uint8_t *)hostname;       //需补充服务器地址
    config.port = port;                     //需补充服务器端口号
    config.username = (uint8_t *)username;  //需补充用户名
    config.passwd = (uint8_t *)password;    //需补充密码
    config.rsptimeout = 10*1000;            //超时时间10秒
    config.cid = 1;

    /* 连接平台 */
    handle = cm_ftp_open(&config);
    
    if (handle < 0)
    {
        cm_demo_printf("[%s] ftp open fail\n", __func__);
    }

    /* 创建一个新文件夹 */
    ret = cm_ftp_create_dir(handle, dir);
    
    if (ret < 0)
    {
        cm_demo_printf("[%s] ftp create dir fail\n", __func__);
    }
    
    /* 设置之前的新建文件夹为当前工作目录 */
    ret = cm_ftp_set_current_dir(handle, dir);
    
    if (ret < 0)
    {
        cm_demo_printf("[%s] ftp change dir fail\n", __func__);
    }

    /* 获取当前工作目录 */
    ret = cm_ftp_get_current_dir(handle, get_dir);
    
    if (ret < 0)
    {
        cm_demo_printf("[%s] ftp change dir fail\n", __func__);
    }
    
    cm_demo_printf("[%s] current_dir=[%s]\n", __func__, get_dir);

    /* 设置其他文件夹作为当前工作目录 */
    ret = cm_ftp_set_current_dir(handle, home_dir);
    
    if (ret < 0)
    {
        cm_demo_printf("[%s] ftp change dir fail\n", __func__);
    }

    /* 删除之前新建的文件夹 */
    ret = cm_ftp_remove_dir(handle, dir);
    
    if (ret < 0)
    {
        cm_demo_printf("[%s] ftp remove dir fail\n", __func__);
    }

    /* 上传文件，模式0：新建或覆盖 */
    ret = cm_ftp_put_file(handle, 0, file, (uint8_t*)data, strlen(data));
    
    if (ret < 0)
    {
        cm_demo_printf("[%s] ftp put 0 fail\n", __func__);
    }

    /* 上传文件，模式1：新建或追加 */
    ret = cm_ftp_put_file(handle, 1, file, (uint8_t*)data, strlen(data));
    
    if (ret < 0)
    {
        cm_demo_printf("[%s] ftp put 1 fail\n", __func__);
    }

    /* 获取文件大小 */
    ret = cm_ftp_get_file_size(handle, file);
    
    if (ret < 0)
    {
        cm_demo_printf("[%s] ftp get file size fail\n", __func__);
    }

    cm_demo_printf("[%s] file size=%d\n", __func__, ret);

    /* 下载文件内容 */
    /* 每次拉取需要建立连接，建立连接需要耗时，所以下载大文件时，单次拉取的长度越大效率越高 */
    ret = cm_ftp_get_file(handle, 0, file, 0, (uint8_t*)out_data, 20);
    
    if (ret < 0)
    {
        cm_demo_printf("[%s] ftp get file fail\n",__func__);
    }
    
    out_data[ret] = '\0';
    
    cm_demo_printf("[%s] len=%d data=[%s]\n", __func__, ret, out_data);

    /* 修改文件名 */
    ret = cm_ftp_rename_file(handle, file, new_file);
    
    if (ret < 0)
    {
        cm_demo_printf("[%s] ftp rename fail\n", __func__);
    }
    
    /* 删除文件 */
    ret = cm_ftp_delete_file(handle, new_file);
    
    if (ret < 0)
    {
        cm_demo_printf("[%s] ftp del file fail\n", __func__);
    }  

    /* 获取文件列表信息 */
    /* 打开查找并进行首次查找 */
    find_fd1 = cm_ftp_find_first(handle, ".", &file_data_test);
    
    if (find_fd1 < 0)
    {
        cm_demo_printf("[%s] ftp find first fail\n", __func__);
    }
    
    cm_demo_printf("[%s,%d] name=%s,size=%d,time=%d,attr=%d,permision=%d\n", __func__, __LINE__, 
         file_data_test.file_name, file_data_test.file_size, file_data_test.file_modify_time,
         file_data_test.file_attr, file_data_test.file_permision);

    /* 循环查找 */
    do{
        find_fd1 = cm_ftp_find_next(handle, find_fd1, &file_data_test);
        if (find_fd1 >= 0)
        {
            cm_demo_printf("[%s,%d] name=%s,size=%d,time=%d,attr=%d,permision=%d\n", __func__, __LINE__, 
                 file_data_test.file_name, file_data_test.file_size, file_data_test.file_modify_time,
                 file_data_test.file_attr, file_data_test.file_permision);
        }
        
    }while (find_fd1 >= 0);

    /* 关闭查找 */
    ret = cm_ftp_find_close(handle, find_fd1);
    
    if (ret < 0)
    {
        cm_demo_printf("[%s] ftp find close fail\n", __func__);
    }
    
    /* 获取连接状态 */
    state = cm_ftp_get_state(handle);
    
    if (ret < 0)
    {
        cm_demo_printf("[%s] ftp get state fail\n", __func__);
    }
    
    cm_demo_printf("[%s] ftp state=%d\n", __func__, state);

    /* 关闭连接 */
    ret = cm_ftp_close(handle);
    
    if (ret < 0)
    {
        cm_demo_printf("[%s] ftp close fail\n", __func__);
    }
    
    return;
}
