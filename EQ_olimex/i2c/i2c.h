#ifndef _I2C_H_
#define _I2C_H_

#include "stm32f10x_i2c.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

#include "utils.h"

// --- user

#define MAIN_I2C I2C2


#define MAIN_I2C_Port GPIOB
#define MAIN_I2C_SCL_Pin  GPIO_Pin_10
#define MAIN_I2C_SDA_Pin  GPIO_Pin_11


#define EQ_ADDR 0xB0

// --- 

void initMainI2C();

void I2C_search(I2C_TypeDef* I2Cx);
BOOL I2C_checkAddr(I2C_TypeDef* I2Cx, uint8_t addr);
BOOL I2C_init(I2C_TypeDef* I2Cx);
void I2C_clockReset(I2C_TypeDef* I2Cx);
BOOL I2C_write(I2C_TypeDef* I2Cx, uint8_t addr, int len, uint8_t* bytes);
BOOL I2C_ask(I2C_TypeDef* I2Cx, uint8_t addr, int comm_len, uint8_t* comm, int res_len, uint8_t* res);
#endif // _I2C_H_