/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
  NTC,
  VL53L0X
} Device_t;   // 设备类型枚举

typedef struct {
  Device_t dev;     // 设备类型
  
  union {
    int val_int;   // 当类型是距离时，用这个存整数
    float val_float; // 当类型是温度时，用这个存浮点数
  } value;           // 联合体，用于存储不同类型的值
  
} ScreenMsg_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
uint8_t flag = 0;
uint8_t error_status = 0;
/* USER CODE END Variables */
/* Definitions for ADC_Task */
osThreadId_t ADC_TaskHandle;
const osThreadAttr_t ADC_Task_attributes = {
  .name = "ADC_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for VL53L0X_Task */
osThreadId_t VL53L0X_TaskHandle;
const osThreadAttr_t VL53L0X_Task_attributes = {
  .name = "VL53L0X_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal1,
};
/* Definitions for Button_Task */
osThreadId_t Button_TaskHandle;
const osThreadAttr_t Button_Task_attributes = {
  .name = "Button_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal2,
};
/* Definitions for OLED_Task */
osThreadId_t OLED_TaskHandle;
const osThreadAttr_t OLED_Task_attributes = {
  .name = "OLED_Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Screen_Queue */
osMessageQueueId_t Screen_QueueHandle;
const osMessageQueueAttr_t Screen_Queue_attributes = {
  .name = "Screen_Queue"
};
/* Definitions for I2C_Mutex */
osMutexId_t I2C_MutexHandle;
const osMutexAttr_t I2C_Mutex_attributes = {
  .name = "I2C_Mutex"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartTask_ADC(void *argument);
void StartTask_VL53L0X(void *argument);
void StartTask_Button(void *argument);
void StartTask_OLED(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of I2C_Mutex */
  I2C_MutexHandle = osMutexNew(&I2C_Mutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of Screen_Queue */
  Screen_QueueHandle = osMessageQueueNew (4, sizeof(ScreenMsg_t), &Screen_Queue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of ADC_Task */
  ADC_TaskHandle = osThreadNew(StartTask_ADC, NULL, &ADC_Task_attributes);

  /* creation of VL53L0X_Task */
  VL53L0X_TaskHandle = osThreadNew(StartTask_VL53L0X, NULL, &VL53L0X_Task_attributes);

  /* creation of Button_Task */
  Button_TaskHandle = osThreadNew(StartTask_Button, NULL, &Button_Task_attributes);

  /* creation of OLED_Task */
  OLED_TaskHandle = osThreadNew(StartTask_OLED, NULL, &OLED_Task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartTask_ADC */
/**
  * @brief  Function implementing the ADC_Task thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartTask_ADC */
void StartTask_ADC(void *argument)
{
  /* USER CODE BEGIN StartTask_ADC */
  float temperature = 0.0;
  ScreenMsg_t tx_msg;
  /* Infinite loop */
  for(;;)
  {
/*     temperature = NTC_Temperature(NTC_DMA_Value(0));
    
    if (osMutexAcquire(I2C_MutexHandle, osWaitForever) == osOK)
    {
      OLED_ShowPointNum(1, 5, temperature, 2, 2);
      
      osMutexRelease(I2C_MutexHandle);
    }
    
    osDelay(500); */
    
    // 获取ADC并算出温度
    temperature = NTC_Temperature(NTC_DMA_Value(0));
    
    /* 准备发送到OLED任务的消息 */
    tx_msg.dev = NTC;
    tx_msg.value.val_float = temperature;
    
    // 将温度数据发送到OLED任务队列
    osMessageQueuePut(Screen_QueueHandle, &tx_msg, 0, 0); 
    
    osDelay(200);
  }
  /* USER CODE END StartTask_ADC */
}

/* USER CODE BEGIN Header_StartTask_VL53L0X */
/**
* @brief Function implementing the VL53L0X_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask_VL53L0X */
void StartTask_VL53L0X(void *argument)
{
  /* USER CODE BEGIN StartTask_VL53L0X */
  int distance_mm = 0;
  // char display_buffer[32];
  ScreenMsg_t tx_msg;
  /* Infinite loop */
  for(;;)
  {
/*     if (flag == 1)
    {
      if (osMutexAcquire(I2C_MutexHandle, osWaitForever) == osOK)
      {
        // 抓取最新的绝对毫米距离 (ToF 时间转换值)
        distance_mm = VL53L0X_GetDistance();

        if (distance_mm != 0xFFFF)
        {
          // 如果数据有效 (RangeStatus == 0)
          // 格式化输出字符串，例如 "125 mm" 或 "2000 mm"
          sprintf(display_buffer, "%4d mm         ", distance_mm);
          
          // 将数据实时刷写到 OLED 屏幕的第 2, 3 页（高度 16 像素）
          OLED_ShowString(3, 1, display_buffer);
        }
        else
        {
          // 如果数据无效（比如超量程、环境光干扰、或手挡住传感器太近了）
          // 在屏幕上闪烁提示 "OUT " (代表 Out of range)
          OLED_ShowString(3, 1, "OUT mm    ");
        }
        
        osMutexRelease(I2C_MutexHandle);
      }
    }
    
    else
    {
      if (osMutexAcquire(I2C_MutexHandle, osWaitForever) == osOK)
      {
        OLED_ShowString(3, 1, "Stop            ");
        osMutexRelease(I2C_MutexHandle);
      }
    }
    
    osDelay(200); */
    
    
    if (flag == 1)
    {
      if (osMutexAcquire(I2C_MutexHandle, osWaitForever) == osOK)
      {
        // 抓取最新的绝对毫米距离 (ToF 时间转换值)
        distance_mm = VL53L0X_GetDistance();
        osMutexRelease(I2C_MutexHandle);
        
        /* 准备发送到OLED任务的消息 */
        tx_msg.dev = VL53L0X;
        tx_msg.value.val_int = distance_mm;
        
        // 将距离数据发送到OLED任务队列
        osMessageQueuePut(Screen_QueueHandle, &tx_msg, 0, 0);
      }
    }
    
    else
    {
      if (osMutexAcquire(I2C_MutexHandle, osWaitForever) == osOK)
      {
        OLED_ShowString(3, 1, "Stop            ");
        osMutexRelease(I2C_MutexHandle);
      }
    }
    
    osDelay(200);
  }
  /* USER CODE END StartTask_VL53L0X */
}

/* USER CODE BEGIN Header_StartTask_Button */
/**
* @brief Function implementing the Button_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask_Button */
void StartTask_Button(void *argument)
{
  /* USER CODE BEGIN StartTask_Button */
  uint8_t button_12 = 1;
  uint8_t button_15 = 1;
  /* Infinite loop */
  for(;;)
  {
    // 开启激光测量
    if (Check_Button(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12), &button_12))
    {
      if (osMutexAcquire(I2C_MutexHandle, osWaitForever) == osOK)
      {
        error_status = VL53L0X_Start();
        VL53L0X_Errors(error_status);   // 检测开启过程是否错误
        flag = 1; // 开启
        
        osMutexRelease(I2C_MutexHandle);
      }
    }
    
    // 关闭激光测量
    if (Check_Button(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15), &button_15))
    {
      if (osMutexAcquire(I2C_MutexHandle, osWaitForever) == osOK)
      {
        error_status = VL53L0X_Stop();
        VL53L0X_Errors(error_status); // 检测关闭过程是否错误
        flag = 0; // 关闭
        
        osMutexRelease(I2C_MutexHandle);
      }
    }
    osDelay(20);
  }
  /* USER CODE END StartTask_Button */
}

/* USER CODE BEGIN Header_StartTask_OLED */
/**
* @brief Function implementing the OLED_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask_OLED */
void StartTask_OLED(void *argument)
{
  /* USER CODE BEGIN StartTask_OLED */
  ScreenMsg_t rx_msg;
  char display_buffer[32];
  /* Infinite loop */
  for(;;)
  {
    if (osMessageQueueGet(Screen_QueueHandle, &rx_msg, NULL, osWaitForever) == osOK)
    {
      switch (rx_msg.dev)
      {
        case NTC:
          if (osMutexAcquire(I2C_MutexHandle, osWaitForever) == osOK)
          {
            OLED_ShowPointNum(1, 5, rx_msg.value.val_float, 2, 2);
            osMutexRelease(I2C_MutexHandle);
          }
          break;
          
        case VL53L0X:
            if (rx_msg.value.val_int != 0xFFFF)
            {
              // 如果数据有效 (RangeStatus == 0)
              // 格式化输出字符串，例如 "125 mm" 或 "2000 mm"
              sprintf(display_buffer, "%4d mm         ", rx_msg.value.val_int);
              
              // 将数据实时刷写到 OLED 屏幕
              if (osMutexAcquire(I2C_MutexHandle, osWaitForever) == osOK)
              {
                OLED_ShowString(3, 1, display_buffer);
                osMutexRelease(I2C_MutexHandle);
              }
            }
            else
            {
              // 如果数据无效（比如超量程、环境光干扰、或手挡住传感器太近了）
              // 在屏幕上闪烁提示 "OUT " (代表 Out of range)
              if (osMutexAcquire(I2C_MutexHandle, osWaitForever) == osOK)
              {
                OLED_ShowString(3, 1, "OUT mm    ");
                osMutexRelease(I2C_MutexHandle);
              }
            }
            
          break;
      
        default:
          break;
      }
    }
    osDelay(20);
  }
  /* USER CODE END StartTask_OLED */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

