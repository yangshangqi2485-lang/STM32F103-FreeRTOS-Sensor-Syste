#ifndef BSP_I2C_H
#define BSP_I2C_H

#include "main.h"

void BSP_I2C_Init(void);

void BSP_I2C_Start(void);

void BSP_I2C_Stop(void);

void BSP_I2C_Ack(void);

void BSP_I2C_NAck(void);

uint8_t BSP_I2C_WaitAck(void);

void BSP_I2C_SendByte(uint8_t Byte);

uint8_t BSP_I2C_ReadByte(uint8_t ack);

uint8_t BSP_I2C_SendData(uint8_t slaveAddress, uint8_t regAddress, uint8_t *data, uint8_t len);

uint8_t BSP_I2C_ReadData(uint8_t slaveAddress, uint8_t regAddress, uint8_t *data, uint8_t len);

#endif