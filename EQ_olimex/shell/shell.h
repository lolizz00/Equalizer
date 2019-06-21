#ifndef  _SHELL_H_
#define  _SHELL_H_

extern const char version_full [];
extern const char version_logo [];

#include "ring_buffer.h"


struct _SHELLINFO;
struct _SHELLCMD;

typedef int(*sh_exec_cmd_handler)(struct _SHELLINFO * sh_info, struct _SHELLCMD *cmd);
typedef void (*sh_send_str_handler)(const char * buf);
typedef void (*sh_send_char_handler)(char ch);

typedef enum{  //тип команды
  CMD_SERVICE,
  CMD_USER,
  CMD_ROOT
}shell_cmd_type;

typedef enum{
  PER_USER,
  PER_ROOT
}shell_permission_level;

typedef struct _SHELLCMD
{
   char * cmd_name;
   char * cmd_descr;
   char * cmd_syntax;   
   sh_exec_cmd_handler exec_cmd;
   sh_exec_cmd_handler print_cmd_help;
   
   shell_cmd_type cmd_type; //!!!
}SHELLCMD;

struct _SHELLINFO
{
   int cmd_run;
   int cl_pos;
   int cl_size;
   char * cl_buf;
   int stop_exec;

   char ** argv;
   unsigned argc;

   int num_cmd;
   SHELLCMD * sh_cmd_arr;

   sh_send_str_handler  sh_send_str;
   sh_send_char_handler sh_send_char;
   
   shell_permission_level current_permission;
};

typedef struct _SHELLINFO SHELLINFO;

#define  SH_EXECUTE     8
#define  SH_BREAK       3
#define  SH_ESCAPE      27

#define  CTRLC_SYM      3
#define  BS_SYM         8
#define  LF_SYM        10
#define  CR_SYM        13
#define  ESC_SYM       27
#define  SPACE_SYM     32
#define  DEL_SYM     0x7F

#define LEFTARROW_SYM   37
#define RIGHTARROW_SYM  39
#define UPARROW_SYM     38
#define DOWNARROW_SYM   40

#define _ESC_BRACKET    1

#define  SH_MAX_ARG     6

int sh_input(SHELLINFO * sh_info, char ch);
int _sh_input(SHELLINFO * sh_info, char ch);
int sh_start_cmd_exec(SHELLINFO * sh_info);
int sh_stop_cmd_exec(SHELLINFO * sh_info);
int sh_do_cmd_exec(SHELLINFO * sh_info);

int sh_echo(SHELLINFO * sh_info, char ch);
int sh_putchar(SHELLINFO * sh_info, char ch);

int sh_prompt(SHELLINFO * sh_info, int send_CR_LF);

#endif /*  _SHELL_H_  */
