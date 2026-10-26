#include "AN_uart.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
_PRINT_TDC_STRUCT G_printTdcData;
UART_HandleTypeDef huart6;




void APrintData(int param, char *msg){
  static int cnt = 0;
  int len = 0;
  uint8_t str[128] = {0};

  switch (param){
    case PRINT_PARAM_TDC:
      len = sprintf(str, "%d %d%d%d%d %d %d %d \n\r", PRT.bcsQty , PRT.bcs[0], PRT.bcs[1], PRT.bcs[2], PRT.bcs[3], PRT.tm[0], PRT.tm[1], PRT.tm[2]);
      HAL_UART_Transmit_DMA(&huart6, str, len+1);
      // CDC_Transmit_FS((unsigned char*)str, len+1);
    break;
    case PRINT_PARAM_CNT:
      len = sprintf(str, "H%d\n\r", G_seconds);
      HAL_UART_Transmit_DMA(&huart6, str, len);
      // CDC_Transmit_FS((unsigned char*)str, len+1);
    break;

    case PRINT_PARAM_MSG:
      len = sprintf(str, "%s\n\r", msg);
      HAL_UART_Transmit_DMA(&huart6, str, len);
      // CDC_Transmit_FS((unsigned char*)str, len+1);
    break;    
  }

 
  while(str[len])len++;
  cnt++;
  // if(!(cnt %100)){
  //   CDC_Transmit_FS((unsigned char*)str, len+1);
  // }

}

 
int AParseVal(char *data, int *val, int shft){
  char tmpStr[8] = {'\0'};
  int cnt = 0;
  int hasVal = 0;
  int error = 0;
  
  for(int i=0; i<8; i++){
    if(isdigit(data[i+shft])){
      tmpStr[cnt++] = data[i+shft];
      if(data[i+shft] != '0')hasVal = 1;
    }
    if((data[i+shft] == '\0')||(data[i+shft] == '\n')||(data[i+shft] == '\r'))break;
  }

  if(cnt == 0){
    error = UART_ERR_BAD_DATA;
    *val = 0;
     
  }else{
    *val = atoi(tmpStr);
    if((val == 0) && hasVal) error = UART_ERR_BAD_DATA;    
  }  
  return error;
}

void ACmd_cmpset(char *data){
  int val;
  int error = AParseVal(data, &val, 6);
  char str[16] = {'\0'};
  if(error){
    APrintData(PRINT_PARAM_MSG, "ERROR VAL");
    return;
  }
  G_dacVal = val;
  sprintf(str, "CMP %d", G_dacVal);
  APrintData(PRINT_PARAM_MSG, str);
}
void ACmd_cmpget(){
  char str[16] = {'\0'};
  sprintf(str, "CMP %d", G_dacVal);
  APrintData(PRINT_PARAM_MSG, str);
}
void ACmd_fpsset(char *data){
  int val = 0;
  int error = AParseVal(data, &val, 6);
  char str[16] = {'\0'};
  if(error){
    APrintData(PRINT_PARAM_MSG, "ERROR VAL");
    return;
  }
  G_uartPackFrq = val;
  sprintf(str, "FSP %d", G_uartPackFrq);
  APrintData(PRINT_PARAM_MSG, str);
}
void ACmd_fpsget(){
  char str[16] = {'\0'};
  sprintf(str, "FSP %d", G_uartPackFrq);
  APrintData(PRINT_PARAM_MSG, str);
}

int AParseUartData(char *data){
  char *ptr;
  char *ss_cmpset = "CMPSET";
  char *ss_cmpget = "CMPGET";
  char *ss_fpsset = "FPSSET";
  char *ss_fpsget = "FPSGET";  

  ptr = strstr(data, ss_cmpset); 
  if (ptr != NULL){ACmd_cmpset(data); return CMD_CMPSET;}
  ptr = strstr(data, ss_cmpget); 
  if (ptr != NULL){ACmd_cmpget(); return CMD_CMPGET;}
 
  ptr = strstr(data, ss_fpsset); 
  if (ptr != NULL){ACmd_fpsset(data); return CMD_FPSSET;}
  ptr = strstr(data, ss_fpsget); 
  if (ptr != NULL){ACmd_fpsget(); return CMD_FPSGET;} 

   
  APrintData(PRINT_PARAM_MSG, "ERROR DATA");
  return UART_ERR_BAD_DATA;
}



