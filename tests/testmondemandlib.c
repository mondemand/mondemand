#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* wrap malloc to cause memory problems */
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

/* wrap strdup to cause memory problems */
static int strdup_fail = 0;
char *my_strdup(const char *s)
{
  void *ret = NULL;
  if( strdup_fail == 0 )
  {
    ret = strdup(s);
  }
  return ret;
}

#define m_try_malloc0 my_malloc
#define strdup my_strdup
#include "mondemandlib.c"
#undef m_try_malloc0
#undef strdup


int
main(void)
{
  int i=0;
  struct mondemand_client *client = NULL;
  char buf[512];

  /* test NULL parameter */
  client = mondemand_client_create(NULL);
  assert( client == NULL );

  /* this should work fine */
  client = mondemand_client_create("test1234");
  assert( client != NULL );

  /* set a valid send level then some bogus ones */
  mondemand_client_set_immediate_send_level(client, M_LOG_ALERT);
  mondemand_client_set_immediate_send_level(client, -1);
  mondemand_client_set_immediate_send_level(client, 100000);
  assert( client->immediate_send_level == M_LOG_ALERT );

  /* set a valid send level then some bogus ones */
  mondemand_client_set_no_send_level(client, M_LOG_NOTICE);
  mondemand_client_set_no_send_level(client, -100);
  mondemand_client_set_no_send_level(client, 56494949);
  assert( client->no_send_level == M_LOG_NOTICE );

  /* destroy it */
  mondemand_client_destroy(client);

  /* create a new one, but with memory allocation problems */
  strdup_fail = 1;
  client = mondemand_client_create("this_should_fail");
  assert( client == NULL );
  strdup_fail = 0;

  malloc_fail = 1;
  client = mondemand_client_create("this_also_should_fail");
  assert( client == NULL );
  malloc_fail = 0;

  /* this should do nothing since the client is NULL */
  mondemand_client_destroy(client);

  /* now create a valid one */
  client = mondemand_client_create("test1234");
  assert( client != NULL );

  /* set some contexts */
  mondemand_client_set_context(client, "context-key-1", "context-value-1");

  /* try it with strdup errors */
  strdup_fail = 1;
  mondemand_client_set_context(client, "abc", "123");
  strdup_fail = 0;

  /* create a grip of contexts */
  for( i=0; i<1000; ++i )
  {
    sprintf(buf, "grip-%d", i);
    mondemand_client_set_context(client, buf, "this is a grip of data");
  }

  /* remove some of them */
  mondemand_client_remove_context(client, "grip-101");
  mondemand_client_remove_context(client, "grip-381");

  /* remove the rest */
  mondemand_client_remove_all_contexts(client);
  assert( client->contexts->num == 0 );
  

  /* leave some dangling contexts for destroy to clean up */
  for( i=0; i<1000; ++i )
  {
    sprintf(buf, "test-%d", i);
    mondemand_client_set_context(client, buf, "hi there");
  }

  /* free it up */
  mondemand_client_destroy(client);

  return 0;
}


