#ifndef __FLYMODEM_H__
#define __FLYMODEM_H__

#ifndef BOOL
#define BOOL  int
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef FM_CONFIG_FILE
#define FM_CONFIG_FILE  "flymodem.cfg"
#endif

#ifndef DEFAULT_CARRIER
#define DEFAULT_CARRIER TelecomCarrier_ChinaMobile
#endif

typedef enum {
    TelecomCarrier_ChinaMobile,
    TelecomCarrier_ChinaTelecom,
    TelecomCarrier_ChinaUnion,
    TelecomCarrier_Invalid
} TelecomCarrier;

typedef enum {
    RETURN_OK,              // 返回成功
    RETURN_NOSIM,           // 未插入SIM卡
    RETURN_UNKNOWNSIM,      // 未知卡
    RETURN_SIMCHANGED,      // 检测到卡更换
    RETURN_SYSERR,          // CMSIS-RTOS接口相关错误
    RETURN_ATERR,           // cm_virt_at_**相关接口相关错误
    RETURN_FSERR,           // 读写文件系统错误
    RETURN_CARRIERERR,      // 未知运营商
    RETURN_MODEERR,         // 非法工作模式
    RETURN_ILLEGALRESPONSE, // at命令返回错误
    RETURN_ERR,             // 其它一般错误
} RETURN_CODE;

typedef enum {
    WorkingMode_DefaultCarrier  = 1,
    WorkingMode_LastUsedCarrier = 2,
    WorkingMode_Invalid
} WorkingMode;

typedef int (*log_cb_t)(const char *fmt, ...);

void fm_init(log_cb_t write_log);

int fm_get_sim_card_info(BOOL *is_flymodem_card, BOOL *is_trans_carrier);

int fm_get_telecom_carrier_info(TelecomCarrier *carrier, char *iccid);

int fm_set_telecom_carrier(TelecomCarrier carrier);

int fm_set_working_mode_prefer_last_used_carrier();

int fm_set_working_mode_power_on_default_carrier(TelecomCarrier carrier);

int fm_get_current_working_mode(WorkingMode *mode);

int fm_get_power_on_telecom_carrier(TelecomCarrier *carrier, char *iccid);

#endif
