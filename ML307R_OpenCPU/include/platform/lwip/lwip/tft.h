#ifndef __TFT_H__
#define __TFT_H__

#include "lwipopts.h"

#if IS_LWIP_PLATFORM_CRANE
#include "pdpdef.h"  /*define TftInfoList_CM*/
#else
#include "Dialer_Task.h"
#include "teldef.h"

typedef struct TftInfoList_st   TftInfoList_CM;
#endif

int tft_new_tft(TftInfoList_CM *);
err_t tft_ulrate_control(u8_t protocol, struct netif *inp, struct pbuf *p, int* psendFlag);
err_t tft_route(struct pbuf *p, struct netif *inp);
err_t tft_route_ip6(struct pbuf *p, struct netif *inp);

#endif

