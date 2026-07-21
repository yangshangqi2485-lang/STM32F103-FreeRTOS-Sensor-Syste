#ifndef VL53L0X_LaserRanging_H
#define VL53L0X_LaserRanging_H

#include "main.h"
#include "vl53l0x_api.h"
#include "vl53l0x_platform.h"

uint8_t VL53L0X_Init(void);

uint8_t VL53L0X_Start(void);

uint8_t VL53L0X_Stop(void);

uint16_t VL53L0X_GetDistance(void);

void VL53L0X_Errors(uint8_t error_status);

#endif

