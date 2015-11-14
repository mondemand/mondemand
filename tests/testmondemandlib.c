/*======================================================================*
 * Copyright (c) 2008, Yahoo! Inc. All rights reserved.                 *
 *                                                                      *
 * Licensed under the New BSD License (the "License"); you may not use  *
 * this file except in compliance with the License.  Unless required    *
 * by applicable law or agreed to in writing, software distributed      *
 * under the License is distributed on an "AS IS" BASIS, WITHOUT        *
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.     *
 * See the License for the specific language governing permissions and  *
 * limitations under the License. See accompanying LICENSE file.        *
 *======================================================================*/
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "m_hash.h"
#include "m_mem.h"
#include "mondemand_transport.h"

/* wrap malloc to cause memory problems */
static int malloc_fail = 0;

static void *my_malloc0(size_t size)
{
  void *ret = NULL;
  if( malloc_fail == 0 )
  {
    ret = m_try_malloc0(size);
  }
  return ret;
}

static int realloc_fail = 0;
static void *my_realloc(void *ptr, size_t size)
{
  void *ret = NULL;
  if( realloc_fail == 0 )
  {
    ret = m_try_realloc(ptr, size);
  }
  return ret;
}

/* wrap strdup to cause memory problems */
static int strdup_fail = 0;
static int strdup_count = 0;
static int strdup_fail_at = -1;
static char *my_strdup(const char *s)
{
  void *ret = NULL;
  strdup_count++;
  if( strdup_fail == 0 && strdup_fail_at != strdup_count )
  {
    ret = strdup(s);
  }
  return ret;
}

/* wrap m_hash_table_set to make it fail */
static int m_hash_fail = 0;
static int
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
#define m_try_realloc my_realloc
#undef strdup
#define strdup my_strdup
#define m_hash_table_set my_m_hash_table_set
#include "mondemandlib.c"
#undef m_try_malloc0
#undef m_try_realloc
#undef strdup
#undef m_hash_table_set

/* define some basic callback functions */

static int fail_log_callback = 0;
static int fail_stats_callback = 0;
static int fail_trace_callback = 0;
static int fail_perf_callback = 0;
static int fail_annotation_callback = 0;

static int
log_sender_callback(const char *prog_id,
                    const struct mondemand_log_message messages[],
                    const int message_count,
                    const struct mondemand_context contexts[],
                    const int context_count,
                    void *userdata)
{
  int i=0;

  for(i=0; i<message_count; ++i)
  {
    (void) messages[i];
  }

  for(i=0; i<context_count; ++i)
  {
    (void) contexts[i];
  }

  (void) prog_id;
  (void) userdata;

  if( fail_log_callback != 0 )
  {
    return -1;
  }

  return 0;
}

static int
stats_sender_callback(const char *prog_id,
                      const struct mondemand_stats_message stats[],
                      const int message_count,
                      const struct mondemand_context contexts[],
                      const int context_count,
                      void *userdata)
{
  int i=0;

  for(i=0; i<message_count; ++i)
  {
    (void) stats[i];
  }

  for(i=0; i<context_count; ++i)
  {
    (void) contexts[i];
  }

  (void) userdata;
  (void) prog_id;

  if(fail_stats_callback != 0)
  {
    return -1;
  }

  return 0;
}

static int
trace_sender_callback (const char *program_identifier,
                       const char *owner,
                       const char *trace_id,
                       const char *message,
                       const struct mondemand_trace traces[],
                       const int trace_count,
                       void *userdata)
{
  int i=0;

  (void) userdata;
  (void) program_identifier;
  (void) owner;
  (void) trace_id;
  (void) message;
  for(i=0; i<trace_count; ++i)
  {
    (void) traces[i];
  }

  if(fail_trace_callback != 0)
  {
    return -1;
  }

  return 0;
}
static int perf_sender_callback (
               const char *id,
               const char *caller_label,
               const struct mondemand_timing timings[],
               const int timings_count,
               const struct mondemand_context contexts[],
               const int context_count,
               void *userdata)
{
  int i=0;

  for(i=0; i<timings_count; ++i)
  {
    (void) timings[i];
  }

  for (i=0; i<context_count; ++i)
  {
    (void) contexts[i];
  }

  (void) id;
  (void) caller_label;
  (void) userdata;

  if (fail_perf_callback!= 0)
  {
    return -1;
  }

  return 0;
}

static int annotation_sender_callback(
               const char *id,
               const long long int timestamp,
               const char *description,
               const char *text,
               const char *tags[],
               const int tag_count,
               const struct mondemand_context contexts[],
               const int context_count,
               void *userdata)
{
  int i=0;

  (void) id;
  (void) timestamp;
  (void) description;
  (void) text;
  for (i=0; i<tag_count; ++i)
    {
      (void) tags[i];
    }
  for (i=0; i<context_count; ++i)
    {
      (void) contexts[i];
    }
  (void) userdata;

  if (fail_annotation_callback!= 0)
  {
    return -1;
  }

  return 0;
}

static void
destroy_callback (struct mondemand_transport *transport)
{
  free(transport);
}

static struct mondemand_transport *
make_test_transport (void)
{
  struct mondemand_transport *transport = NULL;
  transport = (struct mondemand_transport *) 
                malloc(sizeof(struct mondemand_transport));

  transport->log_sender_function = &log_sender_callback;
  transport->stats_sender_function = &stats_sender_callback;
  transport->trace_sender_function = &trace_sender_callback;
  transport->perf_sender_function = &perf_sender_callback;
  transport->annotation_sender_function = &annotation_sender_callback;
  transport->destroy_function = &destroy_callback;
  transport->userdata = NULL;

  return transport;
}


static void trace_test (void)
{
  struct mondemand_client *client = NULL;
  struct mondemand_transport *transport = NULL;
  struct mondemand_trace_id trace_id;

  client = mondemand_client_create ("test1234");
  assert ( client != NULL );
  transport = mondemand_transport_lwes_create ("127.0.0.1", 20502, NULL, 0, 60);
  assert (transport != NULL );
  assert (mondemand_add_transport (client, transport) == 0);

  trace_id = mondemand_trace_id (12345);
  (void)trace_id; /* appease -Werror */

  assert (mondemand_initialize_trace (client, "owner", "id", "message 1") == 0);
  assert (mondemand_set_trace (client, "foo1", "bar1") == 0);
  strdup_count = 0; strdup_fail_at=1;
  assert (mondemand_set_trace (client, "foo1", "bar1") == -3);
  strdup_count = 0; strdup_fail_at=2;
  assert (mondemand_set_trace (client, "foo1", "bar1") == -3);
  strdup_count = 0; strdup_fail_at=-1;
  m_hash_fail = 1;
  assert (mondemand_set_trace (client, "foo1", "bar1") == -3);
  m_hash_fail = 0;
  assert (strcmp (mondemand_get_trace (client, "foo1"), "bar1") == 0);
  {
    const char **keys = mondemand_get_trace_keys (client);
    assert (strcmp (*keys, "foo1") == 0);
    free (keys);
  }
  assert (mondemand_flush_trace (client) == 0);
  mondemand_clear_trace (client);

  assert (mondemand_initialize_trace (client, "owner", "id", "message 2") == 0);
  assert (mondemand_set_trace (client, "foo2", "bar2") == 0);
  assert (mondemand_flush_trace (client) == 0);

  assert (mondemand_initialize_trace (client, "owner", "id", "message 3") == 0);
  assert (mondemand_set_trace (client, "foo3", "bar3") == 0);
  assert (mondemand_flush_trace (client) == 0);
  mondemand_clear_trace (client);

  assert (mondemand_initialize_trace (client, "owner", "id", "message 4") == 0);
  assert (mondemand_set_trace (client, "foo1", "bar1") == 0);
  assert (mondemand_set_trace (client, "foo2", "bar2") == 0);
  mondemand_remove_trace (client, "foo1");
  assert (mondemand_flush_trace (client) == 0);

  mondemand_client_destroy (client);
}

static void perf_test (void)
{
  struct mondemand_client *client = NULL;
  struct mondemand_transport *test_transport = NULL;
  struct mondemand_transport *lwes_transport = NULL;
  struct mondemand_transport *stderr_transport = NULL;


  client = mondemand_client_create ("test1234");
  assert ( client != NULL );
  lwes_transport = mondemand_transport_lwes_create ("127.0.0.1", 20502, NULL, 0, 60);
  assert (lwes_transport != NULL );
  assert (mondemand_add_transport (client, lwes_transport) == 0);
  stderr_transport = mondemand_transport_stderr_create();
  assert (mondemand_add_transport(client, stderr_transport) == 0);
  test_transport = make_test_transport ();
  assert (mondemand_add_transport(client, test_transport) == 0);

  /* set a context */
  assert (mondemand_set_context(client, "context-key-1", "context-value-1") == 0);

  assert (mondemand_initialize_performance_trace
           (client, "id", "caller_label") == 0);

  assert (mondemand_add_performance_trace_timing (client, "step1",
                                                  1445285994, 1445285996) == 0);
  assert (mondemand_add_performance_trace_timing (client, "step2",
                                                  1445285997, 1445286011) == 0);

  /* various failure cases */
  realloc_fail = 1;
  assert (mondemand_add_performance_trace_timing (client, "step3",
                                                  1445285997, 1445286011) == -3);
  realloc_fail = 0;

  assert (mondemand_add_performance_trace_timing
            (client, NULL, 1445285997, 1445286011) == -2);
  assert (mondemand_add_performance_trace_timing
            (client, "step4", -1, 1445286011) == -2);
  assert (mondemand_add_performance_trace_timing
            (client, "step4", 1445286011, -1) == -2);

  /* first fail to send */
  fail_perf_callback = 1;
  assert (mondemand_flush_performance_trace (client) == -1);
  fail_perf_callback = 0;
  /* then succeed */
  assert (mondemand_flush_performance_trace (client) == 0);

  /* finally clear and destroy */
  mondemand_clear_performance_trace (client);
  mondemand_client_destroy (client);
}

static void annotation_test (void)
{
  struct mondemand_client *client = NULL;
  struct mondemand_transport *test_transport = NULL;
  struct mondemand_transport *lwes_transport = NULL;
  struct mondemand_transport *stderr_transport = NULL;

  const char *tags[] = { "tag1", "tag2" };
  int tag_count = sizeof (tags) / sizeof (const char *);


  client = mondemand_client_create ("test1234");
  assert ( client != NULL );
  lwes_transport = mondemand_transport_lwes_create ("127.0.0.1", 20502, NULL, 0, 60);
  assert (lwes_transport != NULL );
  assert (mondemand_add_transport (client, lwes_transport) == 0);
  stderr_transport = mondemand_transport_stderr_create();
  assert (mondemand_add_transport(client, stderr_transport) == 0);
  test_transport = make_test_transport ();
  assert (mondemand_add_transport(client, test_transport) == 0);

  /* set a context */
  assert (mondemand_set_context(client, "context-key-1", "context-value-1") == 0);
  /* test some error cases */

  /* no id */
  assert (mondemand_flush_annotation (NULL,
                                      1234567,
                                      "something happened",
                                      NULL,
                                      NULL,
                                      0,
                                      client) == -2);

  /* bad timestamp */
  assert (mondemand_flush_annotation ("foo",
                                      -1,
                                      "something happened",
                                      NULL,
                                      NULL,
                                      0,
                                      client) == -2);

  /* no description */
  assert (mondemand_flush_annotation ("foo",
                                      1234567,
                                      NULL,
                                      NULL,
                                      NULL,
                                      0,
                                      client) == -2);


  /* test with no text, no tags*/
  assert (mondemand_flush_annotation ("foo",
                                      1234567,
                                      "something happened",
                                      NULL,
                                      NULL,
                                      0,
                                      client) == 0);

  /* test with text, no tags */
  assert (mondemand_flush_annotation ("foo",
                                      1234567,
                                      "something happened",
                                      "with details here",
                                      NULL,
                                      0,
                                      client) == 0);

  /* test with no text, tags */
  assert (mondemand_flush_annotation ("foo",
                                      1234567,
                                      "something happened",
                                      NULL,
                                      tags,
                                      tag_count,
                                      client) == 0);

  /* test with text, tags */
  assert (mondemand_flush_annotation ("foo",
                                      1234567,
                                      "something happened",
                                      "with details here",
                                      tags,
                                      tag_count,
                                      client) == 0);

  /* test with text, tags */
  assert (mondemand_flush_annotation ("foo",
                                      1234567,
                                      "something happened",
                                      "with details here",
                                      tags,
                                      1,
                                      client) == 0);

  /* test lower layer failure */
  fail_annotation_callback = 1;
  assert (mondemand_flush_annotation ("foo",
                                      1234567,
                                      "something happened",
                                      "with details here",
                                      tags,
                                      1,
                                      client) == -1);
  fail_annotation_callback = 0;

  mondemand_client_destroy (client);
}

static void other_test (void)
{
  int i;
  for (i=0; i < 8; i++)
    {
      assert (
        mondemand_log_level_from_string (MonDemandLogLevelStrings[i]) == i
      );
    }
  assert (mondemand_log_level_from_string (NULL) == -1);
  assert (mondemand_log_level_from_string ("unknown") == -1);

  assert (MONDEMAND_GAUGE == mondemand_stat_type_from_string ("gauge"));
  assert (MONDEMAND_COUNTER == mondemand_stat_type_from_string ("counter"));
  assert (MONDEMAND_UNKNOWN == mondemand_stat_type_from_string ("unknown"));
}

int
main(void)
{
  int i=0;
  const char *data = NULL;
  const char **list = NULL;
  struct mondemand_client *client = NULL;
  struct mondemand_trace_id trace_id, trace_id2;
  struct mondemand_transport *transport = NULL;
  struct mondemand_transport *stderr_transport = NULL;
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

  /* add some transports */
  transport = make_test_transport ();
  mondemand_add_transport(client, transport); 

  realloc_fail = 1;
  mondemand_add_transport(client, transport);
  realloc_fail = 0;

  /* set some contexts */
  assert (mondemand_set_context(client, "context-key-1", "context-value-1") == 0);

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
  assert( m_hash_table_num (client->contexts) == 0 );

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

  /* check the log levels */
  assert( mondemand_level_is_enabled(client, M_LOG_INFO) > 0 );
  mondemand_level_is_enabled(NULL, -5);

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
  mondemand_stats_perform_op (client, __FILE__, __LINE__,
                              MONDEMAND_INC, MONDEMAND_COUNTER,
                              "speed", 999999);
  mondemand_stats_perform_op (client, __FILE__, __LINE__,
                              MONDEMAND_INC, MONDEMAND_COUNTER,
                              NULL, 11111);
  mondemand_stats_perform_op (client, __FILE__, __LINE__,
                              MONDEMAND_INC, MONDEMAND_COUNTER,
                              "speed", 111111);
  mondemand_stats_perform_op (client, __FILE__, __LINE__,
                              MONDEMAND_DEC, MONDEMAND_COUNTER,
                              "speed", 111111);
  mondemand_stats_perform_op (client, __FILE__, __LINE__,
                              MONDEMAND_SET, MONDEMAND_COUNTER,
                              "speed", 888888);
  mondemand_stats_perform_op (client, __FILE__, __LINE__,
                              MONDEMAND_SET, MONDEMAND_COUNTER,
                              NULL, 101010);

  malloc_fail = 1;
  mondemand_stats_perform_op (client, __FILE__, __LINE__,
                              MONDEMAND_INC, MONDEMAND_COUNTER,
                              "failboat", 101);
  mondemand_stats_perform_op (client, __FILE__, __LINE__,
                              MONDEMAND_SET, MONDEMAND_COUNTER,
                              "foolgle", 404);
  malloc_fail = 0;

  m_hash_fail = 1;
  mondemand_stats_perform_op (client, __FILE__, __LINE__,
                              MONDEMAND_INC, MONDEMAND_COUNTER,
                              "failwhale", 202);
  mondemand_stats_perform_op (client, __FILE__, __LINE__,
                              MONDEMAND_SET, MONDEMAND_COUNTER,
                              "windoesnot", 500);
  m_hash_fail = 0;

  /* fail */
  fail_log_callback = 1;
  mondemand_flush(client);
  fail_log_callback = 0;

  /* setup the stderr transport */
  mondemand_remove_all_contexts(client);
  mondemand_set_context(client, "test1", "test1");
  stderr_transport = mondemand_transport_stderr_create();
  /* adding the transport means the client will destroy it when its time
     comes */
  mondemand_add_transport(client, stderr_transport);

  mondemand_stats_perform_op (client, __FILE__, __LINE__,
                              MONDEMAND_SET, MONDEMAND_COUNTER,
                              "hlep", 4949494);
  fail_stats_callback = 1;
  mondemand_flush(client);
  mondemand_flush_stats(client);
  fail_stats_callback = 0;

  fail_trace_callback = 1;
  mondemand_initialize_trace (client,"owner","id","message");
  mondemand_set_trace (client, "foo", "bar");
  mondemand_flush_trace (client);
  fail_trace_callback = 0;
  mondemand_flush_trace (client);
  mondemand_clear_trace (client);

  realloc_fail = 1;
  mondemand_flush(client);
  realloc_fail = 0;

  mondemand_flush_stats(client);
  mondemand_flush(client);

  assert (mondemand_reset_stats (client) == 0);

  /* free up the client and transports */
  mondemand_client_destroy(client);

  trace_test ();
  perf_test ();
  annotation_test ();
  other_test ();

  return 0;
}
