#include "BUTTON.h"

/**
 * @brief 检测按键状态
 * @param gpio_readpin 接收按键电平状态
 * @param last_state 记录上一次按键状态，1表示未按下，0表示按下
 * @retval 0 表示未按下，1 表示按下
 */
uint8_t Check_Button(uint8_t gpio_readpin, uint8_t *last_state)
{
  // static uint8_t last_state = 1;  // 上一次按键状态，1表示未按下，0表示按下
  // uint8_t current_state = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12);   // 当前按键状态，读取GPIO引脚状态
  
  uint8_t current_state = gpio_readpin;
  uint8_t key_pressed = 0;  // 按键事件标志，1表示按下，0表示未按下

  // 检测按键从未按下到按下的状态变化
  if (*last_state == 1 && current_state == GPIO_PIN_RESET)
  {
    key_pressed = 1; // 按键按下事件
  }

  *last_state = current_state; // 更新上一次按键状态
  return key_pressed;
}