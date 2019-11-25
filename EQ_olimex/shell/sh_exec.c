#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

#include "includes.h"
#include "cd_class.h"
#include "func.h"

#include "utils.h"

char *g_shell_argv[SH_MAX_ARG];
char g_shell_cl_buf[SH_CL_SIZE];

char * g_shell_argv_hsb[SH_MAX_ARG];
char g_shell_cl_buf_hsb[SH_CL_SIZE];

extern uint8_t smbus_en;
extern uint8_t psu_address;
static char console_buf[SH_CL_SIZE];
static unsigned int g_max_cmd_len = 0; 

uint32_t comm_time = 0;



const SHELLCMD g_shell_cmd_arr[] =
{
   
  {
    "help",
    "Print list of supported commands/commands description",
    "help [cmd] []",
    sh_exec_cmd_help,
    sh_print_cmd_generic, 
    CMD_USER
  },
  {
    "conn",
    "",
    "",
    sh_exec_cmd_conn,
    sh_print_cmd_generic, 
    CMD_USER
  },
  {
    "read",
    "",
    "",
    sh_exec_cmd_read,
    sh_print_cmd_generic, 
    CMD_USER
  },
  {
    "write",
    "",
    "",
    sh_exec_cmd_write,
    sh_print_cmd_generic, 
    CMD_USER
  },
    {
    "reset",
    "",
    "",
    sh_exec_cmd_reset,
    sh_print_cmd_generic, 
    CMD_USER
  },
  {
    "addr",
    "",
    "",
    sh_exec_cmd_addr,
    sh_print_cmd_generic, 
    CMD_USER
  },
   {
    "find",
    "",
    "",
    sh_exec_cmd_find,
    sh_print_cmd_generic, 
    CMD_USER
  },
   {
    "press",
    "",
    "",
    sh_exec_cmd_press,
    sh_print_cmd_generic, 
    CMD_USER
  }
    
};

// ---


int sh_exec_cmd_press(SHELLINFO * sh_info, SHELLCMD *cmd)
{
  
  if(!strcmp("pwr", sh_info->argv[1]))
  {
    pressPWR(atoi(sh_info->argv[2]));
  }
  else if(!strcmp("rst", sh_info->argv[1]))
  {
    pressRST(atoi(sh_info->argv[2]));
  }
  
  return 0;
}

int sh_exec_cmd_addr(SHELLINFO * sh_info, SHELLCMD *cmd)
{
  char buff[100] = {0};
  unsigned addr = strtol(sh_info->argv[1], NULL, 16);
  
  EQ_ADDR = addr;
  
  
  return 0;
}


int sh_exec_cmd_find(SHELLINFO * sh_info, SHELLCMD *cmd)
{
  I2C_search(MAIN_I2C, sh_info);
  
  
  return 0;
}

int sh_exec_cmd_reset(SHELLINFO * sh_info, SHELLCMD *cmd)
{
  I2C_clockReset(MAIN_I2C);
  I2C_init(MAIN_I2C);
  
  sh_info->sh_send_str("Reseted\n\r");
}

int sh_exec_cmd_read(SHELLINFO * sh_info, SHELLCMD *cmd)
{
   char buff[100] = {0};
   unsigned reg = strtol(sh_info->argv[1], NULL, 16);
   int stat = FALSE;
   
   unsigned val = askEQ(reg, &stat);
   
   if(stat)
   {
    sprintf(buff, "read OK 0x%X\n\r", val);
   }
   else
   {
    sprintf(buff, "read ERR\n\r");
   }
   
   sh_info->sh_send_str(buff);
}

int sh_exec_cmd_write(SHELLINFO * sh_info, SHELLCMD *cmd)
{
  unsigned reg = strtol(sh_info->argv[1], NULL, 16);
  unsigned val = strtol(sh_info->argv[2], NULL, 16);
   
  char buff[100] = {0};
  
  if(writeEQ(reg, val))
  {
    sprintf(buff, "write OK\n\r");
  }
  else
  {
      sprintf(buff, "write ERR\n\r");
  }
  
  sh_info->sh_send_str(buff);
}

int sh_exec_cmd_conn(SHELLINFO * sh_info, SHELLCMD *cmd)
{
  
  if(I2C_checkAddr(MAIN_I2C, EQ_ADDR))
  {
   sh_info->sh_send_str("Connection : OK\n\r");
  }
  else
  {
   sh_info->sh_send_str("Connection : ERR\n\r");
  }
  
  return 0;
}

// ----

uint32_t USB_RecvBuf(uint8_t *Buf, uint32_t BufSize)
{
  uint32_t count = 0;
  if ((count = RingBuffer_GetCount(&rxBuf)) > 0)
  {
    count = RingBuffer_PopMult(&rxBuf, Buf, count); 
  }
  return count;
}
  

uint32_t USB_SendBuf(uint8_t *Buf, uint32_t BufSize)
{
  UsbCdcWrite(Buf, BufSize);
  return BufSize;
}

void USB_SendStr(const char *str)
{
   unsigned long len, send = 0;
   len = strlen(str);
   
   while(len != send)
   {
    send += USB_SendBuf((uint8_t *)&str[send], len - send);
   }
        
    
}

void USB_SendChar(char ch)
{
   USB_SendBuf((uint8_t *)&ch, 1);
}


// ----

int sh_exec_init(SHELLINFO * sh)
{
  int len = 0;
  memset(sh, 0, sizeof(SHELLINFO));
  
  (*sh).cl_size      = SH_CL_SIZE;
  (*sh).cl_buf       = &g_shell_cl_buf_hsb[0];
  (*sh).argv         = (char **)&g_shell_argv_hsb[0];
  (*sh).num_cmd      = sizeof(g_shell_cmd_arr)/sizeof(SHELLCMD);
  (*sh).sh_cmd_arr   = (SHELLCMD *)&g_shell_cmd_arr[0];
  (*sh).sh_send_str  = USB_SendStr;
  (*sh).sh_send_char = USB_SendChar;  
  (*sh).current_permission = PER_USER;
  
  for(int ix = 0; ix < sizeof (g_shell_cmd_arr)/sizeof(SHELLCMD); ix++)
  {
    if (len < strlen(g_shell_cmd_arr[ix].cmd_name))
    {
      len = strlen(g_shell_cmd_arr[ix].cmd_name);
    }
  }
  g_max_cmd_len = len;  
  
  return 0;
}

int sh_exec_cmd_help(SHELLINFO * sh_info, SHELLCMD *cmd)
{
 
  uint8_t ix, len;
  if (NULL == sh_info->argv[1])
  {
    len = ((0 == g_max_cmd_len) && (g_max_cmd_len < SH_CL_SIZE)) ? (uint8_t)g_max_cmd_len : 20; //-V560
    for(ix = 0; ix < sizeof (g_shell_cmd_arr)/sizeof(SHELLCMD); ix++)
    {
      
      switch(g_shell_cmd_arr[ix].cmd_type){
      case CMD_USER:
        sprintf(console_buf, "%-*s\t", len, g_shell_cmd_arr[ix].cmd_name);
        strcat_s(console_buf, sizeof(console_buf), g_shell_cmd_arr[ix].cmd_descr);
        strcat_s(console_buf, sizeof(console_buf), "\n\r");
        sh_info->sh_send_str(console_buf);
        break;
        
      case CMD_ROOT:
        if(PER_ROOT == sh_info->current_permission){
           sprintf(console_buf, "%-*s\t", len, g_shell_cmd_arr[ix].cmd_name);
            strcat_s(console_buf, sizeof(console_buf), g_shell_cmd_arr[ix].cmd_descr);
            strcat_s(console_buf, sizeof(console_buf), "\n\r");
            sh_info->sh_send_str(console_buf);
        }
        break;
        
      case CMD_SERVICE:
        // просто не отображаем
        break;
      }
      
    }
    sh_info->sh_send_str("Type help <command> for detailed description\r\n");
  }
  else
  {
    for(ix = 0; ix < sizeof (g_shell_cmd_arr)/sizeof(SHELLCMD); ix++)
    {
      if (0 == strcasecmp((const char *)sh_info->argv[1], g_shell_cmd_arr[ix].cmd_name))
      {
        g_shell_cmd_arr[ix].print_cmd_help(sh_info, (struct _SHELLCMD *)&g_shell_cmd_arr[ix]);
        return 0;
      }
    }
    sh_info->sh_send_str("Wrong command\r\n");
  }
  
  return 0;
}

int sh_print_cmd_generic(SHELLINFO * sh_info, SHELLCMD *cmd)
{
  strcpy(console_buf, cmd->cmd_descr);
  strcat(console_buf, "\r\nCommand line syntax:\r\n\t");
  strcat(console_buf, cmd->cmd_syntax);
  strcat(console_buf, "\r\n");
  sh_info->sh_send_str(console_buf);    
  return 0;
}

static void sh_print_no_argument(SHELLINFO * sh, SHELLCMD *cmd)
{
  strcpy(console_buf, cmd->cmd_name);
  strcat_s(console_buf, SH_CL_SIZE, ": No argument(s)\r\n");
  sh->sh_send_str(console_buf);
}

void sh_print_invalid_argument(SHELLINFO * sh, SHELLCMD *cmd)
{
  strcpy(console_buf, cmd->cmd_name);
  strcat_s(console_buf, SH_CL_SIZE, ": Invalid argument(s)\r\n");
  sh->sh_send_str(console_buf);
}

static void sh_print_help(SHELLINFO * sh, SHELLCMD *cmd)
{
  strcpy(console_buf, cmd->cmd_name);
  strcat_s(console_buf, SH_CL_SIZE, ": Type \"help ");
  strcat_s(console_buf, SH_CL_SIZE, cmd->cmd_name);
  strcat_s(console_buf, SH_CL_SIZE, "\" for command description\r\n");
  sh->sh_send_str(console_buf);
}