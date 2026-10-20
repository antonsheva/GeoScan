#ifndef _AN_SPI_
#define _AN_SPI_

#include "main.h"

extern SPI_HandleTypeDef hspi1;

void ASpiRstPulse();
void ASpiTransmit(uint8_t data);
void ASpiReceive(uint8_t *data, int len);

#endif


