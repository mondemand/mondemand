#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdlib.h>

/* wrap malloc to cause memory problems */
void *my_malloc(size_t size);
static int malloc_fail = 0;

void *my_malloc(size_t size)
{
  void *ret = NULL;
  if( malloc_fail == 0 )
  {
    ret = malloc(size);
  }
  return ret;
}

#define malloc my_malloc
#include "m_mem.c"
#undef malloc

int
main(void)
{
  void *ptr = NULL;

  /* these should fail/return 0 */
  malloc_fail = 1;
  ptr = m_try_malloc(1024);
  ptr = m_try_malloc0(1024);
  m_free(ptr);

  malloc_fail = 0;
  ptr = (void *) m_try_malloc0(1024);
  ptr = m_try_realloc(ptr, 10255);
  m_free(ptr);

  return 0;
}

