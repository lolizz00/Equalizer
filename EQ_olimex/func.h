#ifndef _FUNC_H_
#define _FUNC_H_

#include <string.h>
#include <assert.h>
#include <stdint.h>

void initCDC();

#include "shell.h"


extern SHELLINFO g_shell;
extern RINGBUFF_T rxBuf;

#define USART_BUFFER_SIZE       (1024)

#endif  //_FUNC_H_