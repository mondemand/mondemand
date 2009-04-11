
#include "m_mem.h"

#include <stdlib.h>
#include <string.h>

/* ======================================================================== */
/* Public API functions                                                     */
/* ======================================================================== */

void *
m_try_malloc(size_t size)
{
  return malloc(size); 
}

void *
m_try_malloc0(size_t size)
{
  void *ptr;

  ptr = m_try_malloc(size);

  if(ptr)
  {
    memset(ptr, 0, size);
  }

  return ptr;
}

void
m_free(void *ptr)
{
  free(ptr);
}

