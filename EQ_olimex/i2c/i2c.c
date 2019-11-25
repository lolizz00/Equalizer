#include "i2c.h"

uint32_t Timeout = 0; 


unsigned EQ_ADDR;



void initMainI2C()
{
  GPIO_InitTypeDef GPIO_InitStruct;
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
  
  GPIO_InitStruct.GPIO_Pin = MAIN_I2C_SCL_Pin | MAIN_I2C_SDA_Pin; 
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;		
  GPIO_Init(MAIN_I2C_Port, &GPIO_InitStruct);	
    
  for(int i = 0; i < 5;i++)
  {
    
    if(I2C_init(MAIN_I2C))
    {
      printf_log("initMainI2C --- OK\n");
      return;
    }
    
  }
}

BOOL I2C_ask(I2C_TypeDef* I2Cx, uint8_t addr, int comm_len, uint8_t* comm, int res_len, uint8_t* res)
{
  
  I2C_GenerateSTART(I2Cx, ENABLE);
  Timed(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));
  
  I2C_Send7bitAddress(I2Cx, addr, I2C_Direction_Transmitter);
  Timed(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
  
  for(int i =0; i < comm_len;i++)
  {
     I2C_SendData(I2Cx, comm[i]); 
     Timed (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
  }
  
  I2C_GenerateSTART(I2Cx, ENABLE); 
  Timed(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));
   
  I2C_Send7bitAddress(I2Cx, addr, I2C_Direction_Receiver);
  Timed(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
  
  for(int i =0; i < res_len;i++)
  {
    
    if(i == (res_len-1))
    {
      I2C_AcknowledgeConfig(I2Cx, DISABLE);
    }
    else
    {
      I2C_AcknowledgeConfig(I2Cx, ENABLE);
    }
    
    Timed(!(I2Cx->SR1 & I2C_FLAG_RXNE));
    res[i]= I2C_ReceiveData(I2Cx);
    
  }
  
  I2C_GenerateSTOP(I2Cx, ENABLE);


   I2C_clockReset(I2Cx); // костыль
   I2C_init(I2Cx);
  
  return TRUE;
  
   errReturn:
    
     
     return FALSE;
  
}

void I2C_clockReset(I2C_TypeDef* I2Cx)
{
  
  if(I2Cx == MAIN_I2C)
  {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
  }
}

void I2C_search(I2C_TypeDef* I2Cx, SHELLINFO * sh_info)
{
  char buff[100]= {0};
  int flg = 0;
  
  sh_info->sh_send_str("Begin scaning I2C...\n\r");
  
  for(int i =0; i < 0xFF;i++)
  {
    
     I2C_GenerateSTART(I2Cx, ENABLE);
     Timed(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));
     
     I2C_Send7bitAddress(I2Cx, i, I2C_Direction_Transmitter);
     Timed(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    
     I2C_GenerateSTOP(I2Cx, ENABLE);
     
     sprintf(buff, "I2C addr: 0x%X -YES\n\r", i);
     sh_info->sh_send_str(buff);  
     flg = TRUE;
     continue;
     
 errReturn:
      if(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY))
      {
       I2C_clockReset(I2Cx);
       I2C_init(I2Cx);
      }
      
       sprintf(buff, "I2C addr: 0x%X -NO\n", i);
       //sh_info->sh_send_strprintf_log(buff);  
     
     
  }
  
   if(!flg)
  {
    sh_info->sh_send_str("Nothing found!\n\r");
  }
  
   sh_info->sh_send_str("End scaning I2C.\n\r");
  
  
}

BOOL I2C_checkAddr(I2C_TypeDef* I2Cx, uint8_t addr)
{
  I2C_GenerateSTART(I2Cx, ENABLE);
  Timed(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));
  
  I2C_Send7bitAddress(I2Cx, addr, I2C_Direction_Transmitter);
  Timed(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
  
  I2C_GenerateSTOP(I2Cx, ENABLE);
  
  return TRUE;
  
   errReturn:
     /* --- TODO: обработка флагов "ломания" и перезагрузка I2C --- */
     return FALSE;
}

BOOL I2C_init(I2C_TypeDef* I2Cx)
{
  
  I2C_clockReset(I2Cx);
  
  I2C_InitTypeDef I2C_InitStruct;
  
  I2C_InitStruct.I2C_ClockSpeed = 10000; 		
  I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;			
  I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;	
  I2C_InitStruct.I2C_OwnAddress1 = 0x05;			
  I2C_InitStruct.I2C_Ack = I2C_Ack_Disable;	
  I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; 
    
  I2C_DeInit(I2Cx);
    
  I2C_Init(I2Cx, &I2C_InitStruct);	
  I2C_Cmd(I2Cx, ENABLE);
  
  Timed(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));
  
  I2C_AcknowledgeConfig(I2Cx, ENABLE);
  
  return TRUE;
  
  errReturn:
    return FALSE;
    
}

BOOL I2C_write(I2C_TypeDef* I2Cx, uint8_t addr, int len, uint8_t* bytes)
{
  I2C_GenerateSTART(I2Cx, ENABLE);
  Timed(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));
  
  I2C_Send7bitAddress(I2Cx, addr, I2C_Direction_Transmitter);
  Timed(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
  
  for(int i =0; i < len;i++)
  {  
    I2C_SendData(I2Cx, bytes[i]);
    Timed (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
  }
  
  I2C_GenerateSTOP(I2Cx, ENABLE);
  
   return TRUE;
  
   errReturn:
     I2C_clockReset(I2Cx);
     I2C_init(I2Cx);
     return FALSE;
}
