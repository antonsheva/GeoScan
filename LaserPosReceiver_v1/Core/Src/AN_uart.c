#include "AN_uart.h"
#include <stdio.h>
_PRINT_TDC_STRUCT G_printTdcData;
UART_HandleTypeDef huart6;

void APrintData(int param){
  static int cnt = 0;
  int len = 0;
  uint8_t str[128] = {0};

  switch (param){
    case PRINT_PARAM_TDC:
      len = sprintf(str, "%d %d%d%d%d %d %d %d \n\r", PRT.bcs[0], PRT.bcs[1], PRT.bcs[2], PRT.bcs[3], PRT.tm[0], PRT.tm[1], PRT.tm[2]);
      HAL_UART_Transmit_DMA(&huart6, str, len+1);
    break;
    case PRINT_PARAM_CNT:
      len = sprintf(str, "H%d\n\r", G_seconds);
      HAL_UART_Transmit_DMA(&huart6, str, len);
    default:
    break;
  }

 
  while(str[len])len++;
  cnt++;
  // if(!(cnt %100)){
  //   CDC_Transmit_FS((unsigned char*)str, len+1);
  // }

}
