
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#define FM_FS_RB            (0)      /*!< rb，打开一个二进制文件，文件必须存在，只允许读 */
#define FM_FS_WB            (1)      /*!< wb，新建一个二进制文件，已存在的文件内容清空，只允许写 */
#define FM_FS_AB            (2)      /*!< ab，打开或新建一个二进制文件，只允许在文件末尾追写 */
#define FM_FS_WBPLUS        (3)      /*!< wb+，新建一个二进制文件，已存在的文件内容清空，允许读写 */
#define FM_FS_ABPLUS        (4)      /*!< ab+，打开或新建一个二进制文件，可读，只允许在文件末尾追写 */
#define FM_FS_RBPLUS        (5)      /*!< rb+，打开一个二进制文件，文件必须存在，允许读写 */

#define FM_FS_SEEK_SET      (0)      /*!< SEEK_SET，文件开头 */
#define FM_FS_SEEK_CUR      (1)      /*!< SEEK_CUR，当前位置 */
#define FM_FS_SEEK_END      (2)      /*!< SEEK_END，文件结尾 */

#include "fm_p2x_cfg_port.h"

FM_STATUS_t fm_get_lost_network_policy_status(FM_POLICY_STATUS_t *policy_status) 
{
    int fd;
    int ret=0;
    uint32_t len = 0;
    char read_data[8] = {0};
    const char *file_name = "/data/tywl/fm_lost_network_policy_status.cfg";

    fd = open(file_name, O_RDONLY | O_NOCTTY, 0);
    printf("fd: %d", fd);
    if( fd >= 0 )
    {
		//读取文件
        read(fd, read_data, sizeof(read_data));
		//关闭文件
        close(fd);

        printf("read_data: %s", read_data);
		if ( strncmp(read_data, "1", 1) )
		{
            *policy_status = FM_DISABLE_POLICY;
		}
        else
        {
            *policy_status = FM_ENABLE_POLICY;
		}
        ret=FM_SUCCESS;
	}
    else
    {
		*policy_status = FM_ENABLE_POLICY;
        
	}

    
	return (FM_STATUS_t)ret;
}


FM_STATUS_t fm_set_lost_network_policy_status(FM_POLICY_STATUS_t policy_status) 
{
    int fd;
    int ret=0;
    const char *file_name = "/data/tywl/fm_lost_network_policy_status.cfg";
	uint32_t wirte_len = 0;

    fd = open(file_name, O_RDWR | O_NOCTTY | O_CREAT | O_TRUNC, 0);
	if( fd >= 0 )
	{
		if ( policy_status == FM_ENABLE_POLICY )
		{
            wirte_len = write(fd, "1", strlen("1"));
		}
        else if( policy_status == FM_DISABLE_POLICY )
		{
            wirte_len = write(fd, "2", strlen("2"));
		}
        else
		{
            wirte_len = write(fd, "0", strlen("0"));
		}
		close(fd);
		
		ret = FM_SUCCESS;
	}
    else
    {
		ret = FM_ERROR;
	}

	return (FM_STATUS_t)ret;
}
/*
FM_STATUS_t fm_set_weak_signal_threshold(int signal_threshold)
{
    int fd;
    int ret = 0;
    const char *file_name = "/data/tywl/weak_signal_threshold.cfg";
    uint32_t wirte_len = 0;
    
    fd = open(file_name, O_RDWR | O_NOCTTY | O_CREAT | O_TRUNC, 0);
    if( fd >= 0 )
    {
        char signal_threshold_str[8] = {0};
        sprintf(signal_threshold_str, "%d", signal_threshold);
        wirte_len = write(fd, signal_threshold_str, strlen(signal_threshold_str));
        close(fd);

        ret = FM_SUCCESS;
    }
    else
    {
        ret = FM_ERROR;
    }

    return (FM_STATUS_t)ret;
}

int fm_get_weak_signal_threshold(void)
{
    int fd;
    int signal_threshold = 0;
    const char *file_name = "/data/tywl/weak_signal_threshold.cfg";
    char read_data[8] = {0};
    uint32_t len = 0;

    fd = open(file_name, O_RDONLY | O_NOCTTY, 0);
    if( fd >= 0 )
    {
        //读取文件
        read(fd, read_data, sizeof(read_data));
        //关闭文件
        close(fd);

        signal_threshold = atoi(read_data);
    }
    else
    {
        signal_threshold = -140;//默认弱信号阈值
    }

    return signal_threshold;
}
*/
FM_STATUS_t fm_set_prefer_operator_policy(FM_NETWORK_OPERATOR_t operator) 

{

	int ret = 0;
	int32_t fd;
    const char *file_name = "/data/tywl/fm_prefer_operator_policy.cfg";
    int flags = FM_FS_WB;
	
	fd =  open(file_name, O_RDWR | O_NOCTTY | O_CREAT | O_TRUNC, 0);
	if( fd >= 0 ) 
	{
		if ( operator == CHINA_UNICOM_OPERATOR ) 
		{
			write(fd, "1", strlen("1"));
		} 
		else if( operator == CHINA_MOBILE_OPERATOR ) 
		{
			write(fd, "2", strlen("2"));
		} 
		else if( operator == CHINA_TELECOM_OPERATOR ) 
		{
			write(fd, "3", strlen("3"));
		} 
		else if( operator == CHINA_BROADNET_OPERATOR ) 
		{
			write(fd, "4", strlen("4"));
		} 
		else if( operator == LAST_TIME_OPERATOR ) 
		{
			write(fd, "5", strlen("5"));
		} 
		else 
		{
			write(fd, "0", strlen("0"));
		}
		close(fd);
		
		ret = FM_SUCCESS;
	} 
	else 
	{
		printf("open file failed\n");
		ret = FM_ERROR;
	}

	return (FM_STATUS_t)ret;

}

FM_STATUS_t fm_get_prefer_operator_policy(char *operator,int *index) 
{

	int ret = 0;
	int32_t fd;
	char read_data[32] = {0};
    const char *file_name = "/data/tywl/fm_prefer_operator_policy.cfg";
    int flags = FM_FS_RB;

	fd = open(file_name, O_RDONLY | O_NOCTTY, 0);
	if( fd >= 0 ) 
    {
		read(fd, read_data, 32);
        close(fd);

		if ( strncmp(read_data, "1", 1) == 0  ) 
		{
			memset(operator, 0, 16);
			strcpy(operator, "CHINA UNICOM");
			*index=0;
		} 
		else if( strncmp(read_data, "2", 1) == 0  ) 
		{
			memset(operator, 0, 16);
			strcpy(operator, "CHINA MOBILE");	
			*index=1;
		} 
		else if( strncmp(read_data, "3", 1) == 0  ) 
		{
			memset(operator, 0, 16);
			strcpy(operator, "CHINA TELECOM");
			*index=2;
		} 
		
		else if( strncmp(read_data, "5", 1) == 0  ) 
		{
			
			memset(operator, 0, 16);
			strcpy(operator, "LAST TIME");
			*index=3;
		} 
		else 
		{
			memset(operator, 0, 16);
			strcpy(operator, "UNKNOWN");
			*index=-1;
		}
		

		ret = FM_SUCCESS;
	} 
	else 
	{
		ret = FM_ERROR;
		*index=-1;
	}

	return (FM_STATUS_t)ret;

}
