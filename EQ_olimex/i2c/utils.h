#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "stm32f10x_conf.h"

#define BOOL      uint8_t


#define Timed(x) Timeout = 0xFFFF; while (x) { if (Timeout-- == 0) goto errReturn;} 

BOOL writeEQ(uint8_t reg, uint8_t val);
uint8_t askEQ(uint8_t reg, int* stat);

void delay_ms(uint32_t ms);
void printf_log(char * str);

char *strcat_s(char * s1, int numberOfElements, const char * s2);


// лень новый проект пилить

void initGPIO();
void pressPWR(uint32_t del);
void pressRST(uint32_t del);

#endif // _UTILS_H_