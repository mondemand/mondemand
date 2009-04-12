#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* wrap malloc to cause memory problems */
static int malloc_count = 0;
static int malloc_fail_at = -1;

void *my_malloc(size_t size)
{
  void *ret = NULL;
  malloc_count++;
  if( malloc_fail_at != malloc_count )
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
  int i = 0;
  char *key = NULL;
  char *value = NULL;
  char *test_value = NULL;
  char buf[1024];

  /* try a memory allocation error */
  malloc_count = 0;
  malloc_fail_at = 1;
  hash_table = m_hash_table_create();
  assert( hash_table == NULL );
  malloc_fail_at = -1;

  /* this should work normally */
  hash_table = m_hash_table_create();
  assert( hash_table != NULL );
  m_hash_table_destroy(hash_table);

  /* fail the bucket allocation */
  malloc_fail_at = 2;
  malloc_count = 0;
  hash_table = m_hash_table_create();
  assert( hash_table == NULL );
  malloc_fail_at = -1;

  /* destroy a bogus hash */
  m_hash_table_destroy(NULL);

  /* re-create */
  hash_table = m_hash_table_create();
  assert( hash_table != NULL );

  /* create some keys */
  key = strdup("key1");
  value = strdup("value1");
  m_hash_table_set(hash_table, key, value);

  /* make sure we can get it back */
  test_value = m_hash_table_get(hash_table, key);
  assert( strcmp(test_value, value) == 0 );

  /* look up keys that don't exist */
  test_value = m_hash_table_get(hash_table, "i_do_not_exist");
  assert( test_value == NULL );

  /* over write the keys, which should free the old values in memory */
  key = strdup("key1");
  value = strdup("newvalue1");
  m_hash_table_set(hash_table, key, value);

  /* create and remove a grip of entries to check for leaks */
  for( i=0; i<1000; ++i )
  {
    sprintf(buf, "newkey%d", i);
    key = strdup(buf);
    value = strdup("a grip of data is a lot");
    m_hash_table_set(hash_table, key, value);
  }

  for( i=0; i<1000; ++i )
  {
    sprintf(buf, "newkey%d", i);
    m_hash_table_remove(hash_table, buf);
  }

  /* do it again but use remove all */
  for( i=0; i<1000; ++i )
  {
    sprintf(buf, "newkey%d", i);
    key = strdup(buf);
    value = strdup("a grip of data is a lot");
    m_hash_table_set(hash_table, key, value);
  }

  /* this should destroy all 1000 keys/values */ 
  m_hash_table_remove_all(hash_table);

  /* remove more anyways to see what happens */
  m_hash_table_remove(hash_table, "i_dont_exist");


  /* create a bunch of entries and try to remove one by one */
  for( i=0; i<1000; ++i )
  {
    sprintf(buf, "newkey%d", i);
    key = strdup(buf);
    value = strdup("a grip of data is a lot");
    m_hash_table_set(hash_table, key, value);
  }

  /* start a little higher so the bucket search finds nothing */
  for( i=1010; i>=0; --i )
  {
    sprintf(buf, "newkey%d", i);
    m_hash_table_remove(hash_table, buf);
  }

  assert( m_hash_function(NULL) == 0 );
  assert( m_hash_table_get_node(NULL, NULL) == NULL );

  m_hash_table_destroy(hash_table);

  return 0;
}

