/**
 * @file        cm_audio_player.h
 * @brief       Audio player接口
 * @copyright   Copyright © 2021 China Mobile IOT. All rights reserved.
 * @author      By WangPeng
 * @date        2023/07/27
 *
 * @defgroup audio_player
 * @ingroup audio_player
 * @{
 */

#ifndef __OPENCPU_AUDIO_PLAYER_H__
#define __OPENCPU_AUDIO_PLAYER_H__


/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "cm_audio_common.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/


/****************************************************************************
 * Public Types
 ****************************************************************************/

/** 音频播放通道支持 */
typedef enum
{
    CM_AUDIO_PLAY_CHANNEL_RECEIVER = 1,         /*!< 听筒通道，不支持设置 */
    CM_AUDIO_PLAY_CHANNEL_HEADSET,              /*!< 耳机通道，不支持设置 */
    CM_AUDIO_PLAY_CHANNEL_SPEAKER,              /*!< 扬声器通道，不支持设置 */
#if 0 
    CM_AUDIO_PLAY_CHANNEL_REMOTE,               /*!< 远端播放（需建立通话），暂不支持 */
#endif
} cm_audio_play_channel_e;

/** 音频播放回调的事件类型 */
typedef enum
{
    CM_AUDIO_PLAY_EVENT_FINISHED = 1,           /*!< 播放结束 */
#if 0
    CM_AUDIO_PLAY_EVENT_INTERRUPT,              /*!< 播放中断 */
#endif
} cm_audio_play_event_e;

/** 音频播放设置类型 */
typedef enum
{
    CM_AUDIO_PLAY_CFG_CHANNEL = 1,              /*!< 播放通道，支持范围参见cm_audio_play_channel_e枚举量，不支持设置 */
    CM_AUDIO_PLAY_CFG_VOLUME,                   /*!< 播放音量，支持范围参见0~100 */
} cm_audio_play_cfg_type_e;


/****************************************************************************
 * Public Data
 ****************************************************************************/


/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/**
 * @brief 播放回调函数
 *
 * @param [in] event 事件类型
 * @param [in] param  事件参数
 *
 * @details  须在播放API中传入 \n
 *           通知事件为中断触发，不可阻塞，不可做耗时较长的操作，例如不可使用UART打印log
 */
typedef void (*cm_audio_play_callback_t)(cm_audio_play_event_e event, void *param);


/**
 * @brief 设置播放参数
 *
 * @param [in] type  设置参数类型
 * @param [in] value 设置参数数值
 *
 * @return
 *   =  0 - 成功 \n
 *   = -1 - 失败
 *
 * @details 不支持设置播放通道（模组仅一个播放通道，默认使用该通道，该通道外接扬声器/耳机/听筒由用户决定）
 */
int32_t cm_audio_play_set_cfg(cm_audio_play_cfg_type_e type, void *value);

/**
 * @brief 读取播放参数
 *
 * @param [in]  type  读取参数类型
 * @param [out] value 读取参数数值
 *
 * @return
 *   =  0 - 成功 \n
 *   = -1 - 失败
 *
 * @details 不支持获取播放通道（模组仅一个播放通道，默认使用该通道，该通道外接扬声器/耳机/听筒由用户决定）
 */
int32_t cm_audio_play_get_cfg(cm_audio_play_cfg_type_e type, void *value);

/**
 *  @brief 从文件系统播放音频文件
 *  
 *  @param [in] path         文件路径/名称
 *  @param [in] format       播放格式
 *  @param [in] sample_param 播放音频PCM采样参数（format参数为CM_AUDIO_PLAY_FORMAT_PCM使用，其余情况传入NULL）
 *  @param [in] cb           音频播放回调函数（回调函数在音频处理线程中被执行）
 *  @param [in] cb_param     用户参数（与cm_audio_play_callback回调函数中param参数相对应）
 *
 *  @return
 *   =  0 - 成功 \n
 *   = -1 - 失败
 *  
 *  @details 支持mp3、amr格式 \n
 *           通知事件为中断触发，不可阻塞，不可做耗时较长的操作 \n
 *           模组无内置codec芯片，播放声音的方式为pwm输出音频数据波形驱动扬声器进行播放 \n
 *           因模组无内置codec芯片，不支持音频参数校准 \n
 *           pwm驱动能力有限，无法驱动大功率的扬声器，若用户需要使用大功率扬声器，需自行选型PA芯片并进行软硬件开发 \n
 *           若用户引入PA芯片和PA电路，开启或关闭音频设备时电路可能会出现电压跳变导致爆破音 \n
 *           若出现爆破音且用户的硬件或者使用的PA芯片无法处理爆破音，请参考【ML307R音频开发指导】，从软件上进行规避
 */
int32_t cm_audio_play_file(const char *path, cm_audio_play_format_e format, cm_audio_sample_param_t *sample_param, cm_audio_play_callback_t cb, void *cb_param);

/**
 *  @brief 暂停播放
 *  
 *  @return
 *   =  0 - 成功 \n
 *   = -1 - 失败
 *  
 *  @details 支持MP3、AMR格式;调用该接口到暂停播放存在一定延时
 */
int32_t cm_audio_player_pause(void);

/**
 *  @brief 继续播放
 *  
 *  @return
 *   =  0 - 成功 \n
 *   = -1 - 失败
 */
int32_t cm_audio_player_resume(void);

/**
 *  @brief 停止播放
 *  
 *  @return
 *   =  0 - 成功 \n
 *   = -1 - 失败
 *  
 *  @details 支持MP3、AMR格式;调用该接口到停止播放存在一定延时
 */
int32_t cm_audio_player_stop(void);

/**
 *  @brief 从管道/消息队列中播放音频（开启）
 *  
 *  @param [in] format       播放格式（暂只支持PCM）
 *  @param [in] sample_param 播放音频PCM采样参数（format参数为CM_AUDIO_PLAY_FORMAT_PCM使用，其余情况传入NULL）
 *
 *  @return
 *   =  0 - 成功 \n
 *   = -1 - 失败
 *  
 *  @details 支持PCM、MP3、AMR格式 \n
 *           模组无内置codec芯片，播放声音的方式为pwm输出音频数据波形驱动扬声器进行播放 \n
 *           因模组无内置codec芯片，不支持音频参数校准 \n
 *           pwm驱动能力有限，无法驱动大功率的扬声器，若用户需要使用大功率扬声器，需自行选型PA芯片并进行软硬件开发 \n
 *           若用户引入PA芯片和PA电路，开启或关闭音频设备时电路可能会出现电压跳变导致爆破音 \n
 *           若出现爆破音且用户的硬件或者使用的PA芯片无法处理爆破音，请参考【ML307R音频开发指导】，从软件上进行规避
 */
int32_t cm_audio_player_stream_open(cm_audio_play_format_e format, cm_audio_sample_param_t *sample_param);

/**
 *  @brief 往管道/消息队列中发送要播放的音频数据
 *  
 *  @param [in] data 播放的数据
 *  @param [in] size 播放数据的长度
 *
 *  @return 
 *   >= 0 - 实际写入的数据长度 \n
 *   = -1 - 失败
 *
 *  @details 支持PCM、MP3、AMR格式 \n
 *           对于PCM格式，传输数据须是帧长度的整数倍（例如，8k 16bit的PCM数据，每帧为320字节，传输的数据应是320的整数倍） \n
 *           对于AMR格式，传输数据若包含不完整帧数据可能出现爆破音 \n
 *           模组无内置codec芯片，播放声音的方式为pwm输出音频数据波形驱动扬声器进行播放 \n
 *           因模组无内置codec芯片，不支持音频参数校准 \n
 *           pwm驱动能力有限，无法驱动大功率的扬声器，若用户需要使用大功率扬声器，需自行选型PA芯片并进行软硬件开发 \n
 *           若用户引入PA芯片和PA电路，开启或关闭音频设备时电路可能会出现电压跳变导致爆破音 \n
 *           若出现爆破音且用户的硬件或者使用的PA芯片无法处理爆破音，请参考【ML307R音频开发指导】，从软件上进行规避
 */
int32_t cm_audio_player_stream_push(uint8_t *data, uint32_t size);

/**
 *  @brief 从管道/消息队列中播放音频（关闭）
 *
 *  @details 模组无内置codec芯片，播放声音的方式为pwm输出音频数据波形驱动扬声器进行播放 \n
 *           因模组无内置codec芯片，不支持音频参数校准 \n
 *           pwm驱动能力有限，无法驱动大功率的扬声器，若用户需要使用大功率扬声器，需自行选型PA芯片并进行软硬件开发 \n
 *           若用户引入PA芯片和PA电路，开启或关闭音频设备时电路可能会出现电压跳变导致爆破音 \n
 *           若出现爆破音且用户的硬件或者使用的PA芯片无法处理爆破音，请参考【ML307R音频开发指导】，从软件上进行规避
 */
void cm_audio_player_stream_close(void);

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif

/** @}*/

