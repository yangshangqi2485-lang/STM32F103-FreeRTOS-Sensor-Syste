/*
 * COPYRIGHT (C) STMicroelectronics 2015. All rights reserved.
 *
 * This software is the confidential and proprietary information of
 * STMicroelectronics ("Confidential Information").  You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered into
 * with STMicroelectronics
 *
 * Programming Golden Rule: Keep it Simple!
 *
 */

/*!
 * \file   VL53L0X_platform.c
 * \brief  Code function defintions for Ewok Platform Layer
 *
 */


#include <stdio.h>    // sprintf(), vsnprintf(), printf()
#include <stdbool.h>

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

#include "vl53l0x_i2c_platform.h"
#include "vl53l0x_def.h"

#include "vl53l0x_platform_log.h"

/* * 1. 引入 STM32 HAL 库必需的头文件
 * main.h 包含了 HAL 库的核心定义，i2c.h 包含了 CubeMX 生成的 I2C 句柄声明
 */
#include "main.h"
#include "i2c.h"
#include "BSP_I2C.h"

#ifdef VL53L0X_LOG_ENABLE
#define trace_print(level, ...) trace_print_module_function(TRACE_MODULE_PLATFORM, level, TRACE_FUNCTION_NONE, ##__VA_ARGS__)
#define trace_i2c(...) trace_print_module_function(TRACE_MODULE_NONE, TRACE_LEVEL_NONE, TRACE_FUNCTION_I2C, ##__VA_ARGS__)
#endif



// char debug_string[VL53L0X_MAX_STRING_LENGTH_PLT];


#define MIN_COMMS_VERSION_MAJOR     1
#define MIN_COMMS_VERSION_MINOR     8
#define MIN_COMMS_VERSION_BUILD     1
#define MIN_COMMS_VERSION_REVISION  0

#define STATUS_OK              0x00
#define STATUS_FAIL            0x01

bool_t _check_min_version(void)
{
    return true;
}

int32_t VL53L0X_comms_initialise(uint8_t comms_type, uint16_t comms_speed_khz)
{
    int32_t status = STATUS_OK;
    return status;
}

int32_t VL53L0X_comms_close(void)
{
    int32_t status = STATUS_OK;
    return status;
}

/*
 * 核心接口：连续写多个字节 (Multi-Byte Write)
 * -------------------------------------------------------------------------
 * 原理对齐：
 * 官方的核心算法算好了数据，通过此函数发送。
 * 我们需要将参数：器件地址、寄存器地址、数据缓冲区、长度，映射到 HAL_I2C_Mem_Write。
 */
int32_t VL53L0X_write_multi(uint8_t address, uint8_t index, uint8_t *pdata, int32_t count)
{

#ifdef VL53L0X_LOG_ENABLE
    int32_t i = 0;
    char value_as_str[VL53L0X_MAX_STRING_LENGTH_PLT];
    char *pvalue_as_str;

    pvalue_as_str =  value_as_str;

    for(i = 0 ; i < count ; i++)
    {
        sprintf(pvalue_as_str,"%02X", *(pdata+i));

        pvalue_as_str += 2;
    }
    trace_i2c("Write reg : 0x%04X, Val : 0x%s\n", index, value_as_str);
#endif

    // write 8-bit standard V2W8 write (not paging mode)
    HAL_StatusTypeDef status;

    // 进行硬件 I2C 搬运
    // status = HAL_I2C_Mem_Write(&hi2c1,           // 使用你的硬件 I2C1 接口
    //                            address,          // 写入器件地址 (0x52)
    //                            index,            // 写入内部寄存器地址
    //                            I2C_MEMADD_SIZE_8BIT, // 寄存器是 8 位宽
    //                            pdata,            // 数据的源头指针
    //                            (uint16_t)count,  // 写入的数据长度
    //                            100               // 100ms 超时等待时间
    // );
    
    status = BSP_I2C_SendData(address, index, pdata, (uint8_t)count); // 软件i2c

    // 将 HAL 库的状态翻译成官方 API 认识的返回值
    if (status == HAL_OK)
    {
        return STATUS_OK; // 成功，返回 0 (在官方头文件中 STATUS_OK 通常就是 0)
    }
    else
    {
        return STATUS_FAIL; // 失败，返回非 0 值
    }
}

int32_t VL53L0X_read_multi(uint8_t address, uint8_t index, uint8_t *pdata, int32_t count)
{
    HAL_StatusTypeDef status;

#ifdef VL53L0X_LOG_ENABLE
    int32_t      i = 0;
    char   value_as_str[VL53L0X_MAX_STRING_LENGTH_PLT];
    char *pvalue_as_str;
#endif

    // read 8-bit standard V2W8 write (not paging mode)
    // 进行硬件 I2C 搬运
    // status = HAL_I2C_Mem_Read(&hi2c1,           // 使用你的硬件 I2C1 接口
    //                            address,          // 写入器件地址 (0x52)
    //                            index,            // 写入内部寄存器地址
    //                            I2C_MEMADD_SIZE_8BIT, // 寄存器是 8 位宽
    //                            pdata,            // 数据的源头指针
    //                            (uint16_t)count,  // 写入的数据长度
    //                            100               // 100ms 超时等待时间
    // );
    
    status = BSP_I2C_ReadData(address, index, pdata, (uint8_t)count); // 软件i2c

    // 将 HAL 库的状态翻译成官方 API 认识的返回值
    if (status == HAL_OK) // status == HAL_OK
    {
        return STATUS_OK; // 成功，返回 0 (在官方头文件中 STATUS_OK 通常就是 0)
    }
    else
    {
        return STATUS_FAIL; // 失败，返回非 0 值
    }

#ifdef VL53L0X_LOG_ENABLE
    // Build  value as string;
    pvalue_as_str =  value_as_str;

    for(i = 0 ; i < count ; i++)
    {
        sprintf(pvalue_as_str, "%02X", *(pdata+i));
        pvalue_as_str += 2;
    }

    trace_i2c("Read  reg : 0x%04X, Val : 0x%s\n", index, value_as_str);
#endif

    return status;
}


int32_t VL53L0X_write_byte(uint8_t address, uint8_t index, uint8_t data)
{
    int32_t status = STATUS_OK;
    const int32_t cbyte_count = 1;

    status = VL53L0X_write_multi(address, index, &data, cbyte_count);

    return status;

}


int32_t VL53L0X_write_word(uint8_t address, uint8_t index, uint16_t data)
{
    int32_t status = STATUS_OK;

    uint8_t  buffer[BYTES_PER_WORD];

    // Split 16-bit word into MS and LS uint8_t
    buffer[0] = (uint8_t)(data >> 8);
    buffer[1] = (uint8_t)(data &  0x00FF);

    status = VL53L0X_write_multi(address, index, buffer, BYTES_PER_WORD);

    return status;

}


int32_t VL53L0X_write_dword(uint8_t address, uint8_t index, uint32_t data)
{
    int32_t status = STATUS_OK;
    uint8_t  buffer[BYTES_PER_DWORD];

    // Split 32-bit word into MS ... LS bytes
    buffer[0] = (uint8_t) (data >> 24);
    buffer[1] = (uint8_t)((data &  0x00FF0000) >> 16);
    buffer[2] = (uint8_t)((data &  0x0000FF00) >> 8);
    buffer[3] = (uint8_t) (data &  0x000000FF);

    status = VL53L0X_write_multi(address, index, buffer, BYTES_PER_DWORD);

    return status;

}


int32_t VL53L0X_read_byte(uint8_t address, uint8_t index, uint8_t *pdata)
{
    int32_t status = STATUS_OK;
    int32_t cbyte_count = 1;

    status = VL53L0X_read_multi(address, index, pdata, cbyte_count);

    return status;

}


int32_t VL53L0X_read_word(uint8_t address, uint8_t index, uint16_t *pdata)
{
    int32_t  status = STATUS_OK;
	uint8_t  buffer[BYTES_PER_WORD];

    status = VL53L0X_read_multi(address, index, buffer, BYTES_PER_WORD);
	*pdata = ((uint16_t)buffer[0]<<8) + (uint16_t)buffer[1];

    return status;

}

int32_t VL53L0X_read_dword(uint8_t address, uint8_t index, uint32_t *pdata)
{
    int32_t status = STATUS_OK;
	uint8_t  buffer[BYTES_PER_DWORD];

    status = VL53L0X_read_multi(address, index, buffer, BYTES_PER_DWORD);
    *pdata = ((uint32_t)buffer[0]<<24) + ((uint32_t)buffer[1]<<16) + ((uint32_t)buffer[2]<<8) + (uint32_t)buffer[3];

    return status;

}

int32_t VL53L0X_platform_wait_us(int32_t wait_us)
{
    int32_t status = STATUS_OK;

    /*
     * Use stm32f103c8t6 72mhz event handling to perform non-blocking wait.
     */
    while (wait_us--)
    {
        __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
        __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
    }

#ifdef VL53L0X_LOG_ENABLE
    trace_i2c("Wait us : %6d\n", wait_us);
#endif

    return status;

}


int32_t VL53L0X_wait_ms(int32_t wait_ms)
{
    int32_t status = STATUS_OK;

    /*
     * Use stm32 event handling to perform non-blocking wait.
     */
    HAL_Delay(wait_ms);

#ifdef VL53L0X_LOG_ENABLE
    trace_i2c("Wait ms : %6d\n", wait_ms);
#endif

    return status;

}


int32_t VL53L0X_set_gpio(uint8_t level)
{
    int32_t status = STATUS_OK;
    //status = VL53L0X_set_gpio_sv(level);
#ifdef VL53L0X_LOG_ENABLE
    trace_i2c("// Set GPIO = %d;\n", level);
#endif

    return status;

}


int32_t VL53L0X_get_gpio(uint8_t *plevel)
{
    int32_t status = STATUS_OK;
#ifdef VL53L0X_LOG_ENABLE
    trace_i2c("// Get GPIO = %d;\n", *plevel);
#endif
    return status;
}


int32_t VL53L0X_release_gpio(void)
{
    int32_t status = STATUS_OK;
#ifdef VL53L0X_LOG_ENABLE
    trace_i2c("// Releasing force on GPIO\n");
#endif
    return status;

}

int32_t VL53L0X_cycle_power(void)
{
    int32_t status = STATUS_OK;
#ifdef VL53L0X_LOG_ENABLE
    trace_i2c("// cycle sensor power\n");
#endif
	return status;
}


int32_t VL53L0X_get_timer_frequency(int32_t *ptimer_freq_hz)
{
       *ptimer_freq_hz = 0;
       return STATUS_FAIL;
}


int32_t VL53L0X_get_timer_value(int32_t *ptimer_count)
{
       *ptimer_count = 0;
       return STATUS_FAIL;
}
