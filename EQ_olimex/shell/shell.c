/*****************************************************************************
*
*  Copyright (c) 2011 Yuri Tiomkin
*  All Rights Reserved
*
******************************************************************************
*
*  THIS SOFTWARE IS PROVIDED BY YURI TIOMKIN "AS IS" AND ANY EXPRESSED OR
*  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
*  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
*  IN NO EVENT SHALL YURI TIOMKIN OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
*  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
*  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
*  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
*  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
*  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
*  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
*  THE POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************/

#include <string.h>
#include "includes.h"

#define _RING_HISTORY_LEN (SH_CL_SIZE)

// direction of history navigation
#define _HIST_UP    1
#define _HIST_LAST  0
#define _HIST_DOWN  -1

typedef struct {
	char ring_buf [_RING_HISTORY_LEN];
	int begin;
	int end;
	int cur;
} ring_history_t;

static ring_history_t shell_history;
static unsigned char shell_history_buf[SH_CL_SIZE] = {0, };
static uint8_t escape;
static BOOL esc_seq;

int sh_get_command_line_args(char * cmd_line, char ** argv);
void sh_cmd_end(SHELLINFO * sh_info, int use_CRLF);
int sh_send(SHELLINFO * sh_info, char * str_to_send);
int sh_stcasecmp(char * s1, char * s2);
int sh_strncasecmp(char * s1, char * s2, int len);
int sh_escape_process (SHELLINFO * sh_info, unsigned char ch);
int sh_end_escape_process(SHELLINFO * sh_info);
int is_sh_escape_process(SHELLINFO * sh_info);

static void shell_history_erase_older (ring_history_t * pThis);
static int shell_history_is_space_for_new (ring_history_t * pThis, int len);
static void shell_history_save_line (ring_history_t * pThis, char * line, int len);
static int shell_history_restore_line (ring_history_t * pThis, char * line, int dir);

#ifdef __cplusplus
extern "C"  {
#endif

#if defined(__CROSSWORKS_ARM)
int strcasecmp(const char *s1, const char *s2);
#endif

#ifdef __cplusplus
}  /* extern "C" */
#endif

int _sh_input(SHELLINFO * sh_info, char ch)
{

   if(sh_info->cmd_run == FALSE)
   {
      if(ch != CR_SYM && sh_info->cl_pos < sh_info->cl_size) // CR or Buffer is full.
      {
         if (escape)
         {
           sh_escape_process(sh_info, ch);
         }
         else
         {
           switch(ch)
           {
              case ESC_SYM:
                escape = TRUE;
                break;
              case BS_SYM:
              case DEL_SYM:

                if(sh_info->cl_pos > 0)
                {
                   sh_info->cl_pos--;
                   sh_echo(sh_info, BS_SYM);
                   sh_putchar(sh_info, ' ');
                   sh_putchar(sh_info, BS_SYM);
                }
                break;

              default:

                if(sh_info->cl_pos + 1 < sh_info->cl_size)
                {
                   // Only printable characters.
                   if(ch >= SPACE_SYM && ch <= DEL_SYM)
                   {
                      if((sh_info->cl_pos + 1) < SH_CL_SIZE)
                      {
                        sh_info->cl_buf[sh_info->cl_pos++] = (unsigned char)ch;
                      }
                      else
                      {
                        sh_info->cl_pos = 0;
                      }
                   //   sh_echo(sh_info, ch);
                   }
                }
                break;
           }
         }
      }
      else //-- cmd received - to process
      {
         sh_info->cl_buf[sh_info->cl_pos] = '\0';

         if (strlen((char *)sh_info->cl_buf) > 0)
         {
          // shell_history_save_line(&shell_history, (char *)sh_info->cl_buf, sh_info->cl_pos);
         }
         
         sh_echo(sh_info, CR_SYM);
         sh_echo(sh_info, LF_SYM);

         sh_info->cmd_run = TRUE;
         sh_start_cmd_exec(sh_info); // Just signal on
      }
   }
   else // cmd_run == TRUE
   {
      if(ch == CTRLC_SYM)
        sh_stop_cmd_exec(sh_info); //-- // Just signal on, inside ->set cmd_run = FALSE when actually finished
   }

   return 0;
}

//----------------------------------------------------------------------------
int sh_input(SHELLINFO * sh_info, char ch)
{

   if(sh_info->cmd_run == FALSE)
   {
      if(ch != CR_SYM && sh_info->cl_pos < sh_info->cl_size) // CR or Buffer is full.
      {
         if (escape)
         {
           sh_escape_process(sh_info, ch);
         }
         else
         {
           switch(ch)
           {
              case ESC_SYM:
                escape = TRUE;
                break;
              case BS_SYM:
              case DEL_SYM:

                if(sh_info->cl_pos > 0)
                {
                   sh_info->cl_pos--;
                  // sh_echo(sh_info, BS_SYM);
                   sh_putchar(sh_info, ' ');
                   sh_putchar(sh_info, BS_SYM);
                }
                break;

              default:

                if(sh_info->cl_pos + 1 < sh_info->cl_size)
                {
                   // Only printable characters.
                   if(ch >= SPACE_SYM && ch <= DEL_SYM)
                   {
                      if((sh_info->cl_pos + 1) < SH_CL_SIZE)
                      {
                       sh_info->cl_buf[sh_info->cl_pos++] = (unsigned char)ch;
                       //sh_echo(sh_info, ch);
                      }
                     else
                      {
                        sh_info->cl_pos = 0;
                      }
                      
                   }
                }
                break;
           }
         }
      }
      else //-- cmd received - to process
      {
         sh_info->cl_buf[sh_info->cl_pos] = '\0';

         if (strlen((char *)sh_info->cl_buf) > 0)
         {
           //shell_history_save_line(&shell_history, (char *)sh_info->cl_buf, sh_info->cl_pos);
         }
         
         //sh_echo(sh_info, CR_SYM);
         //sh_echo(sh_info, LF_SYM);

         sh_info->cmd_run = TRUE;
         sh_start_cmd_exec(sh_info); // Just signal on
      }
   }
   else // cmd_run == TRUE
   {
      if(ch == CTRLC_SYM)
        sh_stop_cmd_exec(sh_info); //-- // Just signal on, inside ->set cmd_run = FALSE when actually finished
   }

   return 0;
}


//----------------------------------------------------------------------------
int sh_start_cmd_exec(SHELLINFO * sh_info)
{
   return 0;
}
//----------------------------------------------------------------------------
int sh_do_cmd_exec(SHELLINFO * sh_info)
{
   int i;
   int argc;
   SHELLCMD * cmd_ptr;

   argc = sh_get_command_line_args(sh_info->cl_buf, sh_info->argv);
   
   sh_info->argc = argc;
   
   if(argc <= 0)
   {
      sh_cmd_end(sh_info, FALSE);
      return -1;
   }

   for(i=0; i < sh_info->num_cmd; i++)
   {
      cmd_ptr = &sh_info->sh_cmd_arr[i];
      if(strcasecmp((char*)sh_info->argv[0], cmd_ptr->cmd_name) == 0)
      {
              //-- recognize cmd
        
        /* --- FIX --- */
        
         int ret = cmd_ptr->exec_cmd(sh_info, cmd_ptr);
         
         
           
            //sh_send(sh_info, "Wrong args!\n\r");
           
   
         
         
        /* --- END FIX --- */
         while(argc)
         {
           sh_info->argv[--argc] = NULL;
         }
         

         sh_cmd_end(sh_info, TRUE);
         return 0;
      }
   }
    // If here - cmd not found
    
      //sh_send(sh_info, "Wrong command!\n\r");
    
   
   sh_cmd_end(sh_info, TRUE);
   while(argc)
   {
     sh_info->argv[--argc] = NULL;
   }
   
   return -1;
}

//----------------------------------------------------------------------------
int sh_send(SHELLINFO * sh_info, char * str_to_send)
{
   if(sh_info->sh_send_str == NULL)
     return -1;
   sh_info->sh_send_str(str_to_send);

   return 0; //-- OK
}

//----------------------------------------------------------------------------
int sh_prompt(SHELLINFO * sh_info, int send_CR_LF)
{
   if(sh_info->sh_send_str == NULL)
      return -1;
   
   //sh_info->sh_send_str("SHELL>");
   
   

   return 0; //-- OK
}

//----------------------------------------------------------------------------
int sh_stop_cmd_exec(SHELLINFO * sh_info)
{
   sh_info->stop_exec = TRUE;

   return 0;
}

//----------------------------------------------------------------------------
int sh_echo(SHELLINFO * sh_info, char ch)
{
   if(sh_info->sh_send_char == NULL)
      return -1;
   sh_info->sh_send_char(ch);

   return 0;
}

//----------------------------------------------------------------------------
int sh_putchar(SHELLINFO * sh_info, char ch)
{
   if(sh_info->sh_send_char == NULL)
      return -1;
   sh_info->sh_send_char(ch);

   return 0;
}

//----------------------------------------------------------------------------
void sh_cmd_end(SHELLINFO * sh_info, int use_CRLF)
{
   sh_info->cl_pos = 0;
   sh_info->cl_buf[sh_info->cl_pos] = '\0';

   sh_prompt(sh_info, use_CRLF);

   __disable_interrupt();
   sh_info->cmd_run = FALSE;
   __enable_interrupt();
}
//----------------------------------------------------------------------------
//  Args with whitespaces does not supported !!!
//----------------------------------------------------------------------------
int sh_get_command_line_args(char * cmd_line, char ** argv)
{
   int ch;
   int argc = 0;
   int now_arg = FALSE;
   char * ptr = cmd_line;
   char * start_ptr = cmd_line;

   for(;;)
   {
      ch = *ptr;
      switch(ch)
      {
         case '\0':
         case ' ':
         case '\t':
            if(now_arg)
            {
               *ptr = '\0';
               argv[argc] = start_ptr;
               argc++;
               if(argc >= SH_MAX_ARG)
                  return argc;

               now_arg = FALSE;
            }
            if(ch == '\0')
               return argc;

            break;

         default:
            if(now_arg == FALSE)
            {
               now_arg = TRUE;
               start_ptr = ptr;
            }
            break;
      }
      ptr++;
   }
}

//----------------------------------------------------------------------------
#if 0
int sh_strcasecmp(char * s1, char * s2)
{
    char ch1, ch2;

    for(;;)
    {
       ch1 = *s1++;
       if(ch1 >= 'A' && ch1 <= 'Z')
          ch1 += 0x20;

       ch2 = *s2++;
       if(ch2 >= 'A' && ch2 <= 'Z')
          ch2 += 0x20;

        if(ch1 < ch2)
           return -1;
        if(ch1 > ch2)
           return 1;
        if(ch1 == 0)
           return 0;
    }
}
#endif
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
int sh_strncasecmp(char * s1, char * s2, int len)
{
    char ch1, ch2;
    int idx; 
    for(idx = 0; idx < len; idx++)
    {
       ch1 = *s1++;
       if(ch1 >= 'A' && ch1 <= 'Z')
          ch1 += 0x20;

       ch2 = *s2++;
       if(ch2 >= 'A' && ch2 <= 'Z')
          ch2 += 0x20;

        if(ch1 < ch2)
           return -1;
        if(ch1 > ch2)
           return 1;
        if(ch1 == 0)
           return 0;
    }
    return 0;
}

//----------------------------------------------------------------------------
int sh_escape_process (SHELLINFO * sh_info, unsigned char ch)
{
  int len;
//  sh_echo(sh_info, ch);
  if (ch == '[')
  {
    esc_seq = _ESC_BRACKET;    
    return 0;
  }
  else if (_ESC_BRACKET == esc_seq)
  {
    switch(ch)
    {
    case 'A':
      len = shell_history_restore_line(&shell_history, (char *)shell_history_buf/*(char *)sh_info->cl_buf*/, _HIST_UP);
      if (len > 0)
      //if (strlen((char *)shell_history_buf))
      {
        while(sh_info->cl_pos > 0)
        {
          sh_info->cl_pos--;
          sh_echo(sh_info, BS_SYM);
          sh_putchar(sh_info, ' ');
          sh_putchar(sh_info, BS_SYM);
        }
        //strcpy((char *)sh_info->cl_buf, (char *)shell_history_buf);
        strncpy((char *)sh_info->cl_buf, (char *)shell_history_buf, len);
        sh_info->cl_buf[len] = 0;
        sh_send(sh_info, (char *)sh_info->cl_buf);
        /*uart1_send_str((char *)sh_info->cl_buf);*/
        sh_info->cl_pos = strlen((char *)sh_info->cl_buf);
      }
      break;
    case 'B':
      len = shell_history_restore_line(&shell_history, (char *)shell_history_buf/*(char *)sh_info->cl_buf*/, _HIST_DOWN);
      if (len > 0)
      //if (strlen((char *)shell_history_buf))
      {
        while(sh_info->cl_pos > 0)
        {
          sh_info->cl_pos--;
          sh_echo(sh_info, BS_SYM);
          sh_putchar(sh_info, ' ');
          sh_putchar(sh_info, BS_SYM);
        }
        //strcpy((char *)sh_info->cl_buf, (char *)shell_history_buf);
        strncpy((char *)sh_info->cl_buf, (char *)shell_history_buf, len);
        sh_info->cl_buf[len] = 0;
        sh_send(sh_info, (char *)sh_info->cl_buf);
        /*uart1_send_str((char *)sh_info->cl_buf);*/
        sh_info->cl_pos = strlen((char *)sh_info->cl_buf);
      }
      break;
    case 'C':
      break;
    case 'D':
      break;
    default:
      break;
    }
    esc_seq = 0;
    escape = FALSE;
  }
  else
  {
    escape = FALSE;
  }
  return 1;
}

//----------------------------------------------------------------------------
int sh_end_escape_process(SHELLINFO * sh_info)
{
  if (escape)
  {
    escape = 0;
    if (_ESC_BRACKET == esc_seq)
    {
      esc_seq = 0;
    }
    // TODO: распознавание escape-последовательностей
    while(sh_info->cl_pos > 0)
    {
      sh_info->cl_pos--;
      sh_echo(sh_info, BS_SYM);
      sh_putchar(sh_info, ' ');
      sh_putchar(sh_info, BS_SYM);
    }
    sh_info->cl_buf[0] = '\0';
    return 1;
  }
  return 0;
}

//----------------------------------------------------------------------------
int is_sh_escape_process(SHELLINFO * sh_info)
{
  return escape;
}





//*****************************************************************************
// remove older message from ring buffer
static void shell_history_erase_older (ring_history_t * pThis)
{
	int new_pos = pThis->begin + pThis->ring_buf [pThis->begin] + 1;
	if (new_pos >= _RING_HISTORY_LEN)
		new_pos = new_pos - _RING_HISTORY_LEN;
	
	pThis->begin = new_pos;
}

//*****************************************************************************
// check space for new line, remove older while not space
static int shell_history_is_space_for_new (ring_history_t * pThis, int len)
{
	if (pThis->ring_buf [pThis->begin] == 0)
		return TRUE;
	if (pThis->end >= pThis->begin) {
		if (_RING_HISTORY_LEN - pThis->end + pThis->begin - 1 > len)
			return TRUE;
	}	else {
		if (pThis->begin - pThis->end - 1> len)
			return TRUE;
	}
	return FALSE;
}

//*****************************************************************************
// put line to ring buffer
static void shell_history_save_line (ring_history_t * pThis, char * line, int len)
{
  int prev_cmd_len;
  if (pThis->ring_buf [pThis->begin] != 0)
  {
	int cnt, header = pThis->begin;
	while (header < pThis->end) {
		cnt = header;
		header += pThis->ring_buf [header] + 1;
		if (header >= _RING_HISTORY_LEN)
			header -= _RING_HISTORY_LEN; 
	}
        
    prev_cmd_len = shell_history_restore_line(&shell_history, (char *)shell_history_buf, _HIST_LAST);    
    if ((prev_cmd_len == len) && (0 == sh_strncasecmp(line, (char *)shell_history_buf, len)))
    {
      pThis->cur = 0;
      return;
    }
  }
    if (len > _RING_HISTORY_LEN - 2)
		return;
	while (!shell_history_is_space_for_new (pThis, len)) {
		shell_history_erase_older (pThis);
	}
	// if it's first line
	if (pThis->ring_buf [pThis->begin] == 0) 
		pThis->ring_buf [pThis->begin] = len;
	
	// store line
	if (len < _RING_HISTORY_LEN-pThis->end-1)
		memcpy (pThis->ring_buf + pThis->end + 1, line, len);
	else {
		int part_len = _RING_HISTORY_LEN-pThis->end-1;
		memcpy (pThis->ring_buf + pThis->end + 1, line, part_len);
		memcpy (pThis->ring_buf, line + part_len, len - part_len);
	}
	pThis->ring_buf [pThis->end] = len;
	pThis->end = pThis->end + len + 1;
	if (pThis->end >= _RING_HISTORY_LEN)
		pThis->end -= _RING_HISTORY_LEN;
	pThis->ring_buf [pThis->end] = 0;
	pThis->cur = 0;
#ifdef _HISTORY_DEBUG
	shell_history_print (pThis);
#endif
}

//*****************************************************************************
// copy saved line to 'line' and return size of line
static int shell_history_restore_line (ring_history_t * pThis, char * line, int dir)
{
	int cnt = 0;
	// count history record	
	int header = pThis->begin;
    char *last = &pThis->ring_buf[header];
	while (pThis->ring_buf [header] != 0) {
        last = &pThis->ring_buf [header];
		header += pThis->ring_buf [header] + 1;
		if (header >= _RING_HISTORY_LEN)
			header -= _RING_HISTORY_LEN; 
		cnt++;
	}
    if (_HIST_LAST == dir)
    {
      if (last - pThis->ring_buf + last[0] < _RING_HISTORY_LEN) {
        memset (line, 0, SH_CL_SIZE);
        memcpy (line, last + 1, last[0]);
      } else {
        int part0 = _RING_HISTORY_LEN - (last - pThis->ring_buf) - 1;
        memset (line, 0, SH_CL_SIZE);
        memcpy (line, last + 1, part0);
        memcpy (line + part0, pThis->ring_buf, last[0] - part0);
      }
      return last[0];
    }

	if (dir == _HIST_UP) {
		if (cnt >= pThis->cur) {
			int header = pThis->begin;
			int j = 0;
			// found record for 'pThis->cur' index
			while ((pThis->ring_buf [header] != 0) && (cnt - j -1 != pThis->cur)) {
				header += pThis->ring_buf [header] + 1;
				if (header >= _RING_HISTORY_LEN)
					header -= _RING_HISTORY_LEN;
				j++;
			}
			if (pThis->ring_buf[header]) {
					pThis->cur++;
				// obtain saved line
				if (pThis->ring_buf [header] + header < _RING_HISTORY_LEN) {
					memset (line, 0, SH_CL_SIZE);
					memcpy (line, pThis->ring_buf + header + 1, pThis->ring_buf[header]);
				} else {
					int part0 = _RING_HISTORY_LEN - header - 1;
					memset (line, 0, SH_CL_SIZE);
					memcpy (line, pThis->ring_buf + header + 1, part0);
					memcpy (line + part0, pThis->ring_buf, pThis->ring_buf[header] - part0);
				}
				return pThis->ring_buf[header];
			}
		}
	} else if (_HIST_DOWN == dir){
		if (pThis->cur > 0) {
				pThis->cur--;
			int header = pThis->begin;
			int j = 0;

			while ((pThis->ring_buf [header] != 0) && (cnt - j != pThis->cur)) {
				header += pThis->ring_buf [header] + 1;
				if (header >= _RING_HISTORY_LEN)
					header -= _RING_HISTORY_LEN;
				j++;
			}
			if (pThis->ring_buf [header] + header < _RING_HISTORY_LEN) {
				memcpy (line, pThis->ring_buf + header + 1, pThis->ring_buf[header]);
			} else {
				int part0 = _RING_HISTORY_LEN - header - 1;
				memcpy (line, pThis->ring_buf + header + 1, part0);
				memcpy (line + part0, pThis->ring_buf, pThis->ring_buf[header] - part0);
			}
			return pThis->ring_buf[header];
		} else {
			/* empty line */
			return 0;
		}
	}
	return -1;
}
