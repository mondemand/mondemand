#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "m_hash.h"
#include "m_mem.h"

/* wrap malloc to cause memory problems */
static int malloc_fail = 0;

void *my_malloc0(size_t size)
{
  void *ret = NULL;
  if( malloc_fail == 0 )
  {
    ret = m_try_malloc0(size);
  }
  return ret;
}

void *my_malloc(size_t size)
{
  void *ret = NULL;
  if( malloc_fail == 0 )
  {
    ret = m_try_malloc(size);
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

/* wrap m_hash_table_set to make it fail */
static int m_hash_fail = 0;
int
my_m_hash_table_set(struct m_hash_table *hash_table, char *key, void *value)
{
  int ret = -3;
  if( m_hash_fail == 0 )
  {
    ret = m_hash_table_set(hash_table, key, value);
  } 
  return ret;
}


#define m_try_malloc0 my_malloc0
#define m_try_malloc my_malloc
#define strdup my_strdup
#define m_hash_table_set my_m_hash_table_set
#include "mondemandlib.c"
#undef m_try_malloc0
#undef m_try_malloc
#undef strdup
#undef m_hash_table_set


int
main(void)
{
  int i=0;
  const char *data = NULL;
  const char **list = NULL;
  struct mondemand_client *client = NULL;
  struct mondemand_trace_id trace_id, trace_id2;
  char buf[512];

  /* test NULL parameter */
  client = mondemand_client_create(NULL);
  assert( client == NULL );

  /* this should work fine */
  client = mondemand_client_create("test1234");
  assert( client != NULL );

  /* set a valid send level then some bogus ones */
  mondemand_set_immediate_send_level(client, M_LOG_ALERT);
  mondemand_set_immediate_send_level(client, -1);
  mondemand_set_immediate_send_level(client, 100000);
  assert( client->immediate_send_level == M_LOG_ALERT );

  /* set a valid send level then some bogus ones */
  mondemand_set_no_send_level(client, M_LOG_NOTICE);
  mondemand_set_no_send_level(client, -100);
  mondemand_set_no_send_level(client, 56494949);
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
  mondemand_set_context(client, "context-key-1", "context-value-1");

  /* try it with strdup errors */
  strdup_fail = 1;
  mondemand_set_context(client, "abc", "123");
  strdup_fail = 0;

  /* create a grip of contexts */
  for( i=0; i<1000; ++i )
  {
    sprintf(buf, "grip-%d", i);
    mondemand_set_context(client, buf, "this is a grip of data");
  }

  /* fetch some of them */
  data = mondemand_get_context(client, "grip-100");
  assert( strcmp(data, "this is a grip of data") == 0 );

  /* remove some of them */
  mondemand_remove_context(client, "grip-101");
  mondemand_remove_context(client, "grip-381");

  /* get some of the data that isn't there */
  data = mondemand_get_context(client, "grip-101");
  assert( data == NULL );

  /* get a list of keys */
  list = mondemand_get_context_keys(client); 
  free(list);

  /* overwrite some of it */
  mondemand_set_context(client, "grip-200", "new value");
  data = mondemand_get_context(client, "grip-200");
  assert( strcmp(data, "new value") == 0 ); 

  /* set some contexts where the hash table fails */
  m_hash_fail = 1; 
  assert( mondemand_set_context(client, "bogus123", "silly345") != 0 );
  m_hash_fail = 0;

  /* remove the rest */
  mondemand_remove_all_contexts(client);
  assert( client->contexts->num == 0 );
  

  /* leave some dangling contexts for destroy to clean up */
  for( i=0; i<1000; ++i )
  {
    sprintf(buf, "test-%d", i);
    mondemand_set_context(client, buf, "hi there");
  }

  /* make sure we've got data */
  list = mondemand_get_context_keys(client);
  for( i=0; list[i] != NULL; ++i );
  assert( i > 999 );
  free(list);

  /* create a trace ID */
  trace_id = mondemand_trace_id(12345);  
  trace_id2 = mondemand_trace_id(34567);

  /* compare them */
  assert( mondemand_trace_id_compare(&trace_id, &trace_id2) == -1 );
  assert( mondemand_trace_id_compare(&trace_id2, &trace_id) == 1 );
  assert( mondemand_trace_id_compare(&trace_id, &trace_id) == 0 );
  assert( mondemand_trace_id_compare(&MONDEMAND_NULL_TRACE_ID,
                                     &MONDEMAND_NULL_TRACE_ID) == 0 );

  /* log some stuff */
  mondemand_log_real(client, __FILE__, __LINE__, M_LOG_EMERG, trace_id,
                     "this is a test");
  mondemand_log_real(client, __FILE__, __LINE__, M_LOG_ERR,
                     MONDEMAND_NULL_TRACE_ID, "another test");


  /* lower the no send threshold */
  mondemand_set_no_send_level(client, M_LOG_DEBUG);

  /* log a lot of stuff, as if we were in an inner loop */
  for( i=0; i<1000; ++i )
  {
    mondemand_log_real(client, __FILE__, __LINE__, M_LOG_INFO,
                       MONDEMAND_NULL_TRACE_ID, "spamspamspam");
  }

  /* this should fail since we're unable to allocate the message */
  malloc_fail = 1;
  mondemand_log_real(client, __FILE__, __LINE__, M_LOG_INFO,
                     MONDEMAND_NULL_TRACE_ID, "memoryerrors");
  malloc_fail = 0;

  /* change some stats */
  mondemand_stats_inc(client, __FILE__, __LINE__, "speed", 999999);
  mondemand_stats_inc(client, __FILE__, __LINE__, NULL, 11111);
  mondemand_stats_inc(client, __FILE__, __LINE__, "speed", 111111);
  mondemand_stats_dec(client, __FILE__, __LINE__, "speed", 111111);
  mondemand_stats_set(client, __FILE__, __LINE__, "speed", 888888);
  mondemand_stats_set(client, __FILE__, __LINE__, NULL, 101010);

  malloc_fail = 1;
  mondemand_stats_inc(client, __FILE__, __LINE__, "failboat", 101);
  mondemand_stats_set(client, __FILE__, __LINE__, "foolgle", 404);
  malloc_fail = 0;

  m_hash_fail = 1;
  mondemand_stats_inc(client, __FILE__, __LINE__, "failwhale", 202);
  mondemand_stats_set(client, __FILE__, __LINE__, "windoesnot", 500);
  m_hash_fail = 0;

  /* free it up */
  mondemand_client_destroy(client);

  return 0;
}

