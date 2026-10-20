#include "AN_spi.h"

// SPI_HandleTypeDef hspi1;

void ASpiRstPulse(){
  HAL_GPIO_WritePin(TDC_CS_GPIO_Port, TDC_CS_Pin, 1);
  HAL_GPIO_WritePin(TDC_CS_GPIO_Port, TDC_CS_Pin, 0);  
}
void ASpiTransmit(uint8_t data){
  HAL_SPI_Transmit(&hspi1, &data, 1, 5000);  
}
void ASpiReceive(uint8_t *data, int len){
  for(int i=0; i<len; i++)HAL_SPI_Receive(&hspi1, &data[i], 1, 5000);
}