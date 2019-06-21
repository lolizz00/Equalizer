#include "usb_init.h"
#include "includes.h"

void initUSB(void)
{

  ENTR_CRT_SECTION();
  UsbCdcInit();
  USB_ConnectRes(TRUE);
  EXT_CRT_SECTION();
  
  while(!IsUsbCdcConfigure());

}

void Clk_Init (void){
  // 1. Clocking the controller from internal HSI RC (8 MHz)
  RCC_HSICmd(ENABLE);
  // wait until the HSI is ready
  while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);
  RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);
  // 2. Enable ext. high frequency OSC
  RCC_HSEConfig(RCC_HSE_ON);
  // wait until the HSE is ready
  while(RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET);
  // 3. Init PLL
  RCC_PLLConfig(RCC_PLLSource_HSE_Div1,RCC_PLLMul_9); // 72MHz
  RCC_PLLCmd(ENABLE);
  // wait until the PLL is ready
  while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
  // 4. Set system clock dividers
  RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
  RCC_ADCCLKConfig(RCC_PCLK2_Div8);
  RCC_PCLK2Config(RCC_HCLK_Div1);
  RCC_PCLK1Config(RCC_HCLK_Div2);
  RCC_HCLKConfig(RCC_SYSCLK_Div1);
  // 5. Init Embedded Flash
  // Zero wait state, if 0 < HCLK 24 MHz
  // One wait state, if 24 MHz < HCLK 56 MHz
  // Two wait states, if 56 MHz < HCLK 72 MHz
  // Flash wait state
  FLASH_SetLatency(FLASH_Latency_2);
  // Half cycle access
  FLASH_HalfCycleAccessCmd(FLASH_HalfCycleAccess_Disable);
  // Prefetch buffer
  FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
  // 5. Clock system from PLL
  RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
  
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
}

