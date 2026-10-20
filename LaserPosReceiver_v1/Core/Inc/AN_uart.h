#ifndef _AN_UART_
#define _AN_UART_

#include "AN_define.h"
#include "main.h"


#include "stm32f7xx_hal.h"

extern UART_HandleTypeDef huart6;

typedef struct{
    uint8_t  bcsQty;
    uint8_t  bcs[PULSE_QTY];
    uint32_t tm[PULSE_QTY];

}_PRINT_TDC_STRUCT;

extern _PRINT_TDC_STRUCT G_printTdcData;

void APrintData(int param);



#endif


