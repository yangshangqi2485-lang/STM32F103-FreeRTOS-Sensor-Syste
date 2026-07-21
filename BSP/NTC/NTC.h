#ifndef NTC_H
#define NTC_H

#include "main.h"

void NTC_DMA_Init(void);

int NTC_DMA_Value(uint8_t channel_index);

float NTC_DMA_Voltage(uint8_t channel_index);

float NTC_Temperature(int ADValue);

#endif
