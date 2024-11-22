 
#include "custom_main.h"
#include "cm_uart.h"
 
#include "cm_iomux.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "cm_os.h"
#include "cm_mem.h"
#include "cm_sys.h"
#include "test.h"
#include "bsp_uart.h"
#include "cJSON.h"

int cm_opencpu_entry(void *param)
{
    (void)param;
    //bsp_uart_init();
    test_printf();
	
	return 0;
}
