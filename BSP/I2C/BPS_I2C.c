#include "BSP_I2C.h"

/*引脚配置*/
#define SCL(x)		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, (x))  // PB10 SCL(0):低电平 SCL(0):高电平
#define SDA(x)		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, (x))  // PB11 SDA(0):低电平 SDA(0):高电平
#define READ_SDA    HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11)        // 读取 PB11 数据电平

/**
  * @brief  引脚初始化
  * @note   当前适合于72Mhz频率，注意根据MCU主频调整
  */
static void I2C_Delay(void)
{
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
    __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP();
}

/**
  * @brief  引脚初始化
  * @param  无
  * @retval 无
  */
void BSP_I2C_Init(void)
{
	/*注意要在MX配置GPIO为开漏输出，设置上拉*/
	SCL(1);
	SDA(1);
}

/**
  * @brief  I2C开始
  * @param  无
  * @retval 无
  */
void BSP_I2C_Start(void)
{
	SDA(1);
	SCL(1);
    I2C_Delay();
	SDA(0);
    I2C_Delay();
	SCL(0);
}

/**
  * @brief  I2C停止
  * @param  无
  * @retval 无
  */
void BSP_I2C_Stop(void)
{
	SDA(0);
	SCL(1);
    I2C_Delay();
	SDA(1);
    I2C_Delay();
}

/**
  * @brief  主机产生应答 (ACK)
  * @param  无
  */
void BSP_I2C_Ack(void)
{
    SCL(0);
    SDA(0);
    I2C_Delay();
    SCL(1);
    I2C_Delay();
    SCL(0);
}

/**
 * @brief  主机产生非应答 (NACK)
 * @param  无
 */
void BSP_I2C_NAck(void)
{
    SCL(0);
    SDA(1);
    I2C_Delay();
    SCL(1);
    I2C_Delay();
    SCL(0);
}

/**
  * @brief  等待从机应答 (ACK)
  * @param  无
  * @retval 返回 0 代表有应答，返回 1 代表无应答 (NACK)
  */
uint8_t BSP_I2C_WaitAck(void)
{
    uint8_t timeOut = 0;
    SDA(1); // 释放数据线准备接收
    I2C_Delay();
    SCL(1);
    I2C_Delay();
    
    while (READ_SDA)
    {
        timeOut++;
        if (timeOut > 250)
        {
            BSP_I2C_Stop();
            return 1; // 超时无应答
        }
    }
    SCL(0);
    return 0;
}

/**
  * @brief  I2C发送一个字节
  * @param  Byte 要发送的一个字节
  * @retval 无
  */
void BSP_I2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		SDA(!!(Byte & (0x80 >> i)));    // 进行转换两次逻辑非，强制 0 或 1，消除非零数值的多样性，强制进行二值化
		SCL(1);
        I2C_Delay();
		SCL(0);
	}
}

/**
  * @brief  I2C读取一个字节
  * @param  ack 读取完后应答 0:应答(Ack)  1:非应答(NAck)
  * @retval 返回读取到一个字节
  */
uint8_t BSP_I2C_ReadByte(uint8_t ack)
{
    uint8_t dataByte = 0x00;
    
    // 1. 释放数据线，准备接收
    SDA(1);
    I2C_Delay();
    
    // 2. 循环读取 8 位数据
    for (uint8_t i = 0; i < 8; i++)
    {
        SCL(0);         // 拉低时钟，允许从机改变数据
        I2C_Delay();
        SCL(1);         // 拉高时钟，锁定数据
        I2C_Delay();
        
        dataByte <<= 1;  // 左移一位，腾出最低位
        if (READ_SDA) 
        {
            dataByte |= 0x01; // 如果接收到高电平，最低位置1
        }
    }
    
    // 3. 接收完毕后，由主机向从机发送应答信号
    // 注意：这里的 ack 参数通常由外部决定（如：0表示继续读，1表示读最后一个字节）
    if (ack)
        BSP_I2C_Ack();   // 主机发送应答
    else
        BSP_I2C_NAck();  // 主机发送非应答
    
    return dataByte;
}

/**
 * @brief  I2C发送数据
 * @param  slaveAddress: 从机地址
 * @param  regAddress: 寄存器地址
 * @param  data: 数据缓冲区
 * @param  len: 数据长度
 * @retval 0:成功 1:失败
 */
uint8_t BSP_I2C_SendData(uint8_t slaveAddress, uint8_t regAddress, uint8_t *data, uint8_t len)
{
    BSP_I2C_Start();
    
    BSP_I2C_SendByte(slaveAddress);
    if (BSP_I2C_WaitAck()) return 1;
    
    BSP_I2C_SendByte(regAddress);
    if (BSP_I2C_WaitAck()) return 1;
    
    for (uint8_t i = 0; i < len; i++) 
    {
        BSP_I2C_SendByte(data[i]);
        if (BSP_I2C_WaitAck()) return 1;
    }
    
    BSP_I2C_Stop();
    return 0;
}

/**
 * @brief  I2C读取数据
 * @param  slaveAddress: 从机地址
 * @param  regAddress: 寄存器地址
 * @param  data: 数据缓冲区
 * @param  len: 数据长度
 * @retval 0:成功 1:失败
 */
uint8_t BSP_I2C_ReadData(uint8_t slaveAddress, uint8_t regAddress, uint8_t *data, uint8_t len) 
{
    BSP_I2C_Start();
    
    BSP_I2C_SendByte(slaveAddress);
    if (BSP_I2C_WaitAck()) return 1;
    
    BSP_I2C_SendByte(regAddress);
    if (BSP_I2C_WaitAck()) return 1;
    
    BSP_I2C_Start();
    
    BSP_I2C_SendByte(slaveAddress + 1);
    if (BSP_I2C_WaitAck()) return 1;
    
    for (uint8_t i = 0; i < len; i++)
    {
        if (i == len - 1)
            data[i] = BSP_I2C_ReadByte(0);
        else
            data[i] = BSP_I2C_ReadByte(1);
    }
    
    BSP_I2C_Stop();
    return 0;
}
