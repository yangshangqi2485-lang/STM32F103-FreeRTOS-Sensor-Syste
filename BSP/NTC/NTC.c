#include "NTC.h"
#include "adc.h"
#include <math.h>

#define B_VALUE      3950.0   // 行业通用B值
#define R25_VALUE    10000.0  // 25度标准阻值 10k
#define T25_KELVIN   298.15   // 开尔文常数 (273.15 + 25)

#define ADC_HANDLE &hadc1 // 选择adc通道
#define VCC 3.3

int value = 0;
float voltage = 0.0;
uint16_t adc_buffer[4];

void NTC_DMA_Init(void)
{
    // 1. 硬件校准
    HAL_ADCEx_Calibration_Start(ADC_HANDLE);
    
    // 2. 开启 DMA 循环采集，把数据直接丢给 adc_buffer 数组，长度写 3
    // 只要这句话执行了，adc_buffer 里的数值就会在后台高频自动更新
    HAL_ADC_Start_DMA(ADC_HANDLE, (uint32_t*)adc_buffer, 3);
}

/**
 * @brief  获取指定通道的最新 ADC 采样值
 * @param  channel_index: 0=NTC, 1=内部温度, 2=Vrefint
 * @retval 对应通道的 0~4095 原始值
 */
int NTC_DMA_Value(uint8_t channel_index)
{
    // 防御性编程，防止数组越界
    if(channel_index >= 3) return 0;
    
    // 直接返回数组里的值！后台 DMA 早就帮你把最新的值放进去了
    return adc_buffer[channel_index];
}

/**
 * @brief  将 ADC 采样值转换电压
 * @param  channel_index: 0=NTC, 1=内部温度, 2=Vrefint
 * @retval 计算得出的电压，带一位小数 (float)
 */
float NTC_DMA_Voltage(uint8_t channel_index)
{
    // 防御性编程，防止数组越界
    if(channel_index >= 3) return 0;
    
    value = adc_buffer[channel_index];   // 获取ADC采样值 
    voltage = (value / 4095.0) * VCC;
    
    return voltage;
}

/**
 * @brief  将 ADC 采样值转换为摄氏度
 * @param  adValue: HAL_ADC_GetValue() 读出的 0~4095 的原始值
 * @retval 计算得出的摄氏温度，带小数 (float)
 */
float NTC_Temperature(int adValue)
{
    if (adValue >= 4095 || adValue == 0) return -99.0; // 异常处理（断线或短路）
    
    // 1. 根据分压公式计算当前 NTC 电阻值 (R1 = 10000.0)
    float r_ntc = ((float)adValue * 10000.0) / (4095.0 - (float)adValue);
    
    // 2. 代入 B 值公式计算开尔文温度
    float kelvin = 1.0 / ( (1.0 / T25_KELVIN) + (1.0 / B_VALUE) * log(r_ntc / R25_VALUE) );
    
    // 3. 转为摄氏度并进行单点校准修正（减去自热和器件带来的约 2.2°C 偏差）
    float temperature = kelvin - 273.15;
    
    return temperature;
}
