#include "VL53L0X_LaserRanging.h"

/* 声明 VL53L0X 的全局设备结构体实体与句柄指针 */
VL53L0X_Dev_t MyDevice;
VL53L0X_DEV pMyDevice = &MyDevice;

VL53L0X_Error status = VL53L0X_ERROR_NONE;

/**
 * @brief  初始化 VL53L0X 激光测距传感器 (基础快速开机流)
 * @note   对应流程图中的前4步：DataInit -> StaticInit -> SPADs -> Temp Cal
 * @retval 0 代表初始化成功，非 0 代表对应的错误步骤
 */
uint8_t VL53L0X_Init(void)
{
    uint32_t refSpadCount;
    uint8_t isApertureSpads;
    uint8_t VhvSettings;
    uint8_t PhaseCal;
    
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET); // 拉低，强制让传感器复位
    HAL_Delay(50);                                        // 稳一下
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);   // 拉高，唤醒传感器
    HAL_Delay(50);                                        // 等待传感器内部启动完成
    
    // 【步骤 0】：设定设备的物理 I2C 地址 (出厂默认为 0x52)
    pMyDevice->I2cDevAddr = 0x52;
    
    // 【步骤 1】：软件数据结构初始化 (DataInit) -> 耗时 ~40ms
    status = VL53L0X_DataInit(pMyDevice);
    if (status != VL53L0X_ERROR_NONE) return 1; // 此时返回 1 是完全合法的（因为函数返回类型是 uint8_t）

    // 【步骤 2】：传感器静态初始化 (StaticInit) -> 与 DataInit 共耗时 ~40ms
    status = VL53L0X_StaticInit(pMyDevice);
    if (status != VL53L0X_ERROR_NONE) return 2;
    
    // 【步骤 3】：SPAD 阵列参考管理 (PerformRefSpadManagement) -> 耗时 ~10ms
    status = VL53L0X_PerformRefSpadManagement(pMyDevice, &refSpadCount, &isApertureSpads);
    if (status != VL53L0X_ERROR_NONE) return 3;
    
    // 【步骤 4】：温度与暗电流参考校准 (PerformRefCalibration) -> 耗时 ~40ms
    status = VL53L0X_PerformRefCalibration(pMyDevice, &VhvSettings, &PhaseCal);
    if (status != VL53L0X_ERROR_NONE) return 4;

    // 【步骤 5】：配置工作模式为连续测距
    status = VL53L0X_SetDeviceMode(pMyDevice, VL53L0X_DEVICEMODE_CONTINUOUS_RANGING);
    if (status != VL53L0X_ERROR_NONE) return 5;

    // 设定 Sigma 噪声硬性限制放宽至 45mm
    status = VL53L0X_SetLimitCheckValue(pMyDevice, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, (FixPoint1616_t)(18 * 65536));
    if (status != VL53L0X_ERROR_NONE) return 6;

    // 设定最小返回信号强度放宽至 0.1 Mcps
    status = VL53L0X_SetLimitCheckValue(pMyDevice, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, (FixPoint1616_t)(0.25 * 65536));
    if (status != VL53L0X_ERROR_NONE) return 7;

    // 将单次测距的时间预算拉长到 200ms 
    status = VL53L0X_SetMeasurementTimingBudgetMicroSeconds(pMyDevice, 200000);
    if (status != VL53L0X_ERROR_NONE) return 8;
    
    return 0; // 无报错，返回 0 
}


/**
 * @brief  开始测距
 * @retval 0 代表初始化成功，非 0 代表对应的错误
 */
uint8_t VL53L0X_Start(void)
{
    status = VL53L0X_StartMeasurement(pMyDevice);
    if (status != VL53L0X_ERROR_NONE) return 9;
    
    return 0;
}

/**
 * @brief  停止测距
 * @retval 0 代表初始化成功，非 0 代表对应的错误
 */
uint8_t VL53L0X_Stop(void)
{
    status = VL53L0X_StopMeasurement(pMyDevice);
    if (status != VL53L0X_ERROR_NONE) return 10;
    
    return 0;
}

/**
 * @brief  获取最新的测距值
 * @retval 距离 (mm)，范围在 30mm ~ 2000mm 之间。若出错返回 0xFFFF
 */
uint16_t VL53L0X_GetDistance(void)
{
/*  VL53L0X_GetMeasurementDataReady(); 获取测量状态
    VL53L0X_GetRangingMeasurementData(); 获取测量数据
    RangeMilliMeter：以mm为单位的测量距离。
    RangeDMaxMilliMeter:最大测量距离，mm为单位。
    SignalRateRtnMegaCps：返回的信号大小（signal rate (MCPS)），由目标物的反射率决定。
    AmbientRateRtnMegaCps：返回的环境信号大小（ambient rate (MCPS)），由环境光决定。 */
    
    VL53L0X_RangingMeasurementData_t RangingMeasurementData;

    status = VL53L0X_GetRangingMeasurementData(pMyDevice, &RangingMeasurementData);
    
    if (status == VL53L0X_ERROR_NONE)
    {
        if (RangingMeasurementData.RangeStatus == 0)
        {
            uint16_t distance = RangingMeasurementData.RangeMilliMeter;
            VL53L0X_ClearInterruptMask(pMyDevice, VL53L0X_REG_SYSTEM_INTERRUPT_GPIO_NEW_SAMPLE_READY);
            return distance;
        }
    }

    VL53L0X_ClearInterruptMask(pMyDevice, VL53L0X_REG_SYSTEM_INTERRUPT_GPIO_NEW_SAMPLE_READY);
    return 0xFFFF; 
}

/**
 * @brief  检查报错并处理
 * @param error_status 接收错误参数
 */
void VL53L0X_Errors(uint8_t error_status)
{
  if (error_status != 0)
  {
    // 如果初始化失败，在 OLED 第三行输出故障码，方便 debug
    char error_msg[16];
    sprintf(error_msg, "INIT ERR: %d", error_status);
    OLED_ShowString(3, 1, error_msg);
    
    while (1)
    {
      // 初始化失败，卡死在这里提示排查
      HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0); // 闪烁板载 LED 报警
      HAL_Delay(200);
    }
    
  }
}
