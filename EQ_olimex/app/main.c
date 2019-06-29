#include "includes.h"
#include "func.h"
Int32U CriticalSecCntr;

uint8_t USB_Buf[SH_CL_SIZE];


void handleShell()
{
  int rc = 0;
  uint32_t count= 0;  
  
  
  
  if ((count = UsbCdcRead(USB_Buf, SH_CL_SIZE)) > 0)
    {
      for (int i = 0; i < count; i++)
      {
        rc = sh_input(&g_shell, USB_Buf[i]);
        if(rc == SH_EXECUTE)
          sh_start_cmd_exec(&g_shell);
        else if(rc == SH_BREAK)
          sh_stop_cmd_exec(&g_shell);
        else if (rc == SH_ESCAPE)
        {
        }        
      }      
    }
    
    
    if (g_shell.cmd_run)
    {
      sh_do_cmd_exec(&g_shell);
    }
}

void main(void)
{
  
  initCDC();
  initMainI2C();
 
  //I2C_search(MAIN_I2C);
  
  
    
 /*
  writeEQ(0x26, 0x1);
  tmp = askEQ(0x26);
  sprintf(buff, "Val: 0x%X\n", tmp);
  printf_log(buff); 
 */
 
  
  while(1)
  {
    handleShell();
  }

}
