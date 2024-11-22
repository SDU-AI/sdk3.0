/**
 * @file        cm_audio_common.h
 * @brief       Audio 通用接口
 * @copyright   Copyright © 2021 China Mobile IOT. All rights reserved.
 * @author      By WangPeng
 * @date        2021/4/12
 *
 * @defgroup audio_common
 * @ingroup audio_common
 * @{
 */

#ifndef __OPENCPU_AUDIO_COMMON_H__
#define __OPENCPU_AUDIO_COMMON_H__
/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <stdint.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Public Types
 ****************************************************************************/

/** 音频播放格式支持         */
typedef enum
{
    CM_AUDIO_PLAY_FORMAT_PCM = 1,               /*!< PCM格式 */
    CM_AUDIO_PLAY_FORMAT_WAV,                   /*!< WAV格式 */
    CM_AUDIO_PLAY_FORMAT_MP3 = 3,               /*!< MP3格式 */
    CM_AUDIO_PLAY_FORMAT_AMRNB,                 /*!< AMR-NB格式 */
#if 0
    CM_AUDIO_PLAY_FORMAT_AMRWB,                 /*!< AMR-WB格式 */
    CM_AUDIO_PLAY_FORMAT_FR,                    /*!< 预留，暂不支持 */
    CM_AUDIO_PLAY_FORMAT_HR,                    /*!< 预留，暂不支持 */
    CM_AUDIO_PLAY_FORMAT_EFR,                   /*!< 预留，暂不支持 */
    CM_AUDIO_PLAY_FORMAT_AAC,                   /*!< 预留，暂不支持 */
    CM_AUDIO_PLAY_FORMAT_MID,                   /*!< 预留，暂不支持 */
    CM_AUDIO_PLAY_FORMAT_QTY,                   /*!< 预留，暂不支持 */
#endif
} cm_audio_play_format_e; 

/** PCM音频采样格式 */
typedef enum
{
    CM_AUDIO_SAMPLE_FORMAT_8BIT = 1,            /*!< 8位，暂不开放，请使用CM_AUDIO_SAMPLE_FORMAT_16BIT */
    CM_AUDIO_SAMPLE_FORMAT_16BIT,               /*!< 16位, 小端 */
    CM_AUDIO_SAMPLE_FORMAT_24BIT,               /*!< 24位, 本平台不支持 */
    CM_AUDIO_SAMPLE_FORMAT_32BIT,               /*!< 32位, 暂不开放，请使用CM_AUDIO_SAMPLE_FORMAT_16BIT */
} cm_audio_sample_format_e;

/** 音频播放支持的采样率 */
typedef enum
{
    CM_AUDIO_SAMPLE_RATE_8000HZ  =  8000,
#if 0
    CM_AUDIO_SAMPLE_RATE_9600HZ  =  9600,
#endif
    CM_AUDIO_SAMPLE_RATE_11025HZ = 11025,
    CM_AUDIO_SAMPLE_RATE_12000HZ = 12000,
    CM_AUDIO_SAMPLE_RATE_16000HZ = 16000,
    CM_AUDIO_SAMPLE_RATE_22050HZ = 22050,
    CM_AUDIO_SAMPLE_RATE_24000HZ = 24000,
    CM_AUDIO_SAMPLE_RATE_32000HZ = 32000,
    CM_AUDIO_SAMPLE_RATE_44100HZ = 44100,
    CM_AUDIO_SAMPLE_RATE_48000HZ = 48000,
    CM_AUDIO_SAMPLE_RATE_96000HZ = 96000,
#if 0
    CM_AUDIO_SAMPLE_RATE_128000HZ = 128000,
#endif
} cm_audio_sample_rate_e;

/** PCM音频采样通道 */
typedef enum
{
    CM_AUDIO_SOUND_MONO = 1,            /*!< （默认）单通道，录音时默认且只能使用CM_AUDIO_SOUND_MONO */
    CM_AUDIO_SOUND_STEREO = 2,          /*!< 双通道（立体声） */
} cm_audio_sound_channel_e;

/** 音频采样参数结构体 */
typedef struct
{
    cm_audio_sample_format_e sample_format;     /*!< 采样格式（PCM） */
    cm_audio_sample_rate_e rate;                /*!< 采样率（PCM） */
    cm_audio_sound_channel_e num_channels;      /*!< 采样声道（PCM） */
} cm_audio_sample_param_t;

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif
/** @}*/