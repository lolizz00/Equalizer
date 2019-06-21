#ifndef __SH_EXEC_H
#define __SH_EXEC_H



#define SH_CL_SIZE          (150)
#define SH_ROW_SIZE         (25)
#define SCREEN_BUFFER_SIZE  (SH_CL_SIZE*SH_ROW_SIZE)

int sh_exec_init(SHELLINFO*);
int sh_print_cmd_generic(SHELLINFO * sh_info, SHELLCMD *cmd);
int sh_exec_cmd_help(SHELLINFO * sh_info, SHELLCMD *cmd);


int sh_exec_cmd_conn(SHELLINFO * sh_info, SHELLCMD *cmd);
int sh_exec_cmd_read(SHELLINFO * sh_info, SHELLCMD *cmd);
int sh_exec_cmd_write(SHELLINFO * sh_info, SHELLCMD *cmd);
int sh_exec_cmd_reset(SHELLINFO * sh_info, SHELLCMD *cmd);

#endif
