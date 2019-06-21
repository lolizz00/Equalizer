/*******************
 *
 * Copyright 1998-2010 IAR Systems AB.  
 *
 * This is a template implementation of the "__write" function used by
 * the standard library.  Replace it with a system-specific
 * implementation.
 *
 * The "__write" function should output "size" number of bytes from
 * "buffer" in some application-specific way.  It should return the
 * number of characters written, or _LLIO_ERROR on failure.
 *
 * If "buffer" is zero then __write should perform flushing of
 * internal buffers, if any.  In this case "handle" can be -1 to
 * indicate that all handles should be flushed.
 *
 * The template implementation below assumes that the application
 * provides the function "MyLowLevelPutchar".  It should return the
 * character written, or -1 on failure.
 *
 ********************/

#include "includes.h"

#include <yfuns.h>

_STD_BEGIN

#pragma module_name = "?__write"
/*
int MyLowLevelPutchar(char x);

size_t __write(int handle, const unsigned char * buffer, size_t size)
{
  size_t nChars = 0;

  if (buffer == 0)
  {
    return 0;
  }

  if (handle != _LLIO_STDOUT && handle != _LLIO_STDERR)
  {
    return _LLIO_ERROR;
  }

  for (; size != 0; --size)
  {
    if (MyLowLevelPutchar(*buffer++) < 0)
    {
      return _LLIO_ERROR;
    }

    ++nChars;
  }

  return nChars;


  return _LLIO_ERROR;



}

int MyLowLevelPutchar(char x){
  
  UsbCdcWrite(&x, 1);

  return 1;
  
}
*/
_STD_END
