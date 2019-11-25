#include "func.h"


#include "stm32f10x.h"
#include "arm_comm.h"
#include "usb_init.h"
#include "sh_exec.h"
#include "ring_buffer.h"

uint8_t RxBuffer[USART_BUFFER_SIZE];
RINGBUFF_T rxBuf;

SHELLINFO g_shell;


void initCDC()
{

  Clk_Init();
  
  RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

  GPIOC->CRL |= GPIO_CRL_CNF2_1;
  GPIOC->CRL &= ~GPIO_CRL_CNF2_0;
  
  initUSB();
 
  RingBuffer_Init(&rxBuf, RxBuffer, 1, USART_BUFFER_SIZE);
  sh_exec_init(&g_shell);
}


