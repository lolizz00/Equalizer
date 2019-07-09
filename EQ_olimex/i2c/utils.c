#include "includes.h"

char *strcat_s(char * s1, int numberOfElements, const char * s2)
{
  if (strlen(s1) + strlen(s2) < numberOfElements)
  {
    strcat(s1, s2);
  }
  else
  {
    int freespace;
    freespace = numberOfElements - strlen(s1) - 1;
    strncpy(&s1[strlen(s1)], s2, freespace);
  }
  return s1;
}

void delay_ms(uint32_t ms)
{
  volatile uint32_t nCount;
  RCC_ClocksTypeDef RCC_Clocks;
  RCC_GetClocksFreq (&RCC_Clocks);
  nCount=(RCC_Clocks.HCLK_Frequency/10000)*ms;
  for (; nCount!=0; nCount--);
}

BOOL writeEQ(uint8_t reg, uint8_t val)
{
   uint8_t wr_buff[2] = { reg, val };
   
   char buff[50] = {0};
   
   if(I2C_write(MAIN_I2C, EQ_ADDR, 2, wr_buff))
   {
       sprintf(buff, "Succ. write register 0x%X\n", reg);
       printf_log(buff);
       return TRUE;
   }
   else
    {
      sprintf(buff, "Cannot write register 0x%X\n", reg);
      printf_log(buff);
      return FALSE;
    }
  
}

uint8_t askEQ(uint8_t reg, int* stat)
{
  uint8_t wr_buff[1] = { reg };
  uint8_t rd_buff[1] = { 0 };
  
   char buff[50] = {0};
  
  if(I2C_ask(MAIN_I2C, EQ_ADDR, 1, wr_buff, 1, rd_buff))
  {
   *stat = TRUE;
  }
  else
  {
   *stat = FALSE;
  }
  
  
  
  return rd_buff[0];
}

//#define LOG_JTAG



void printf_log(char * str)
{

#ifdef LOG_JTAG
  printf("LOG : %s", str);
#endif
}

  
