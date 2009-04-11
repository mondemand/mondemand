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

#define m_try_malloc0 my_malloc
#include "m_hash.c"
#undef m_try_malloc0

int
main(void)
{
  struct m_hash_table *hash_table;

  malloc_fail = 1;
  hash_table = m_hash_table_create();
  assert( hash_table == NULL );

  malloc_fail = 0;
  hash_table = m_hash_table_create();
  m_hash_table_destroy(hash_table);

  m_hash_table_destroy(NULL);

  return 0;
}

