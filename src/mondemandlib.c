
#include "m_mem.h"
#include "m_hash.h"
#include "mondemandlib.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define M_MESSAGE_MAX 2048
#define M_MAX_MESSAGES 10


/* client structure */
struct mondemand_client
{
  /* program identifier */
  char *prog_id;
  /* minimum log level at which to send events immediately */
  int immediate_send_level;
  /* minimum log level at which to send events at all */
  int no_send_level;
  /* hashtable of contextual data */
  struct m_hash_table *contexts;
  /* hashtable of log messages, keyed by 'filename:line' */
  struct m_hash_table *messages;
  /* hashtable of stats */
  struct m_hash_table *stats;
};


/* trace structure and constants */
struct mondemand_trace_id
{
  unsigned long _id;
};
const struct mondemand_trace_id MONDEMAND_NULL_TRACE_ID = { 0 };


/* define an internal structure for keeping log messages */
struct m_log_message
{
  char filename[FILENAME_MAX+1];
  int line;
  int level;
  int repeat_count;
  char message[M_MESSAGE_MAX+1];
  struct mondemand_trace_id trace_id;
};


/* an internal structure for handilng counters */
#if HAVE_LONG_LONG
typedef long long MStatCounter;
#else
typedef long MStatCounter;
#endif


/* ======================================================================== */
/* Public API functions                                                     */
/* ======================================================================== */

struct mondemand_client *
mondemand_client_create(const char *program_identifier)
{
  struct mondemand_client *client = NULL;

  /* if the prog_id is null, don't continue. */
  if( program_identifier == NULL )
  {
    return NULL;
  }

  client = (struct mondemand_client *) 
            m_try_malloc0(sizeof(struct mondemand_client));

  if( client != NULL )
  {
    client->prog_id = strdup(program_identifier);
    client->immediate_send_level = M_LOG_CRIT;
    client->no_send_level = M_LOG_NOTICE;
    client->contexts = m_hash_table_create();
    client->messages = m_hash_table_create();
    client->stats = m_hash_table_create();

    /* if any of the memory allocation has failed, bail out */
    if( client->prog_id == NULL || client->contexts == NULL 
        || client->messages == NULL )
    {
      mondemand_client_destroy(client);
      return NULL;
    }
  }

  return client;
}

void
mondemand_client_destroy(struct mondemand_client *client)
{
  if( client != NULL )
  {
    m_free(client->prog_id);
    m_hash_table_destroy(client->contexts);
    m_hash_table_destroy(client->messages);
    m_hash_table_destroy(client->stats);
    m_free(client);
  }
}

void
mondemand_set_immediate_send_level(struct mondemand_client *client,
                                   const int level)
{
  if( client != NULL )
  {
    if( level >= M_LOG_EMERG && level <= M_LOG_ALL )
    {
      client->immediate_send_level = level;
    }
  }
}

void
mondemand_set_no_send_level(struct mondemand_client *client, const int level)
{
  if( client != NULL )
  {
    if( level >= M_LOG_EMERG && level <= M_LOG_ALL )
    {
      client->no_send_level = level;
    }
  }
}

/* returns the value for a given key */
const char *
mondemand_get_context(struct mondemand_client *client, const char *key)
{
  const char *ret = NULL;

  if( client != NULL && key != NULL )
  {
    if( client->contexts != NULL )
    {
      ret = m_hash_table_get( client->contexts, key );
    }
  }

  return ret;
}

const char **
mondemand_get_context_keys(struct mondemand_client *client)
{
  const char **ret = NULL;

  if( client != NULL )
  {
    if( client->contexts != NULL ) 
    {
      ret = m_hash_table_keys( client->contexts );
    }
  }

  return ret;
}

/* sets a context, overwriting it if it is already set */
int
mondemand_set_context(struct mondemand_client *client,
                      const char *key, const char *value)
{
  char *k = NULL;
  char *v = NULL;

  if( client != NULL && key != NULL && value != NULL )
  {
    if( client->contexts != NULL )
    {
      k = strdup(key);
      v = strdup(value);

      /* if we can't allocate memory for the values, bail out */
      if( k == NULL || v == NULL )
      {
        return -3;
      }

      if( m_hash_table_set( client->contexts, k, v ) != 0 )
      {
        m_free(k);
        m_free(v);
        return -3;
      }
    }
  }

  return 0;
}

/* remove a context */
void
mondemand_remove_context(struct mondemand_client *client, const char *key)
{
  if( client != NULL && key != NULL )
  {
    if( client->contexts != NULL )
    {
      m_hash_table_remove( client->contexts, key );
    }
  }
}

/* remove all contexts */
void
mondemand_remove_all_contexts(struct mondemand_client *client)
{
  if( client != NULL )
  {
    m_hash_table_remove_all( client->contexts );
  }
}

/* generate a trace id */
struct mondemand_trace_id
mondemand_trace_id(unsigned long id)
{
  struct mondemand_trace_id trace_id;
  trace_id._id = id;
  return trace_id;
}

/* compare trace ids */
int
mondemand_trace_id_compare(const struct mondemand_trace_id *a,
                           const struct mondemand_trace_id *b)
{
  if( a == NULL ) return -1;
  if( b == NULL ) return 1;

  if( a ->_id < b->_id ) {
    return -1;
  } else if(a ->_id > b->_id ) {
    return 1;
  }

  return 0;
}

/* flush the logs to the transports */
int
mondemand_flush_logs(struct mondemand_client *client)
{
  if( client != NULL )
  {
    m_hash_table_remove_all( client->messages );
  }

  return 0;
}


/*========================================================================*/
/* Semi-private functions                                                 */
/*========================================================================*/

int
mondemand_log_real(struct mondemand_client *client,
                   const char *filename, const int line, const int level,
                   const struct mondemand_trace_id trace_id,
                   const char *format, ...)
{
  int retval = 0;
  va_list args;
  va_start(args, format);
  retval = mondemand_log_real_va(client, filename, line, level, 
                                 trace_id, format, args);
  va_end(args);
  return retval;
}

int
mondemand_log_real_va(struct mondemand_client *client,
                      const char *filename, const int line, const int level,
                      const struct mondemand_trace_id trace_id,
                      const char *format, va_list args)
{
  char key[(FILENAME_MAX * 3)];
  struct m_log_message *message = NULL;
  char *hash_key = NULL;

  if( client != NULL && format != NULL )
  {
    /* if the trace ID is NULL or the no send level is too high, give up now */
    if( mondemand_trace_id_compare(&trace_id, &MONDEMAND_NULL_TRACE_ID) != 0 
        || level < client->no_send_level )
    {
      /* if we're supposed to send a trace, flush what we have now */
      if( mondemand_trace_id_compare(&trace_id, 
                                     &MONDEMAND_NULL_TRACE_ID) != 0 )
      {
        mondemand_flush_logs(client);
      }

      /* create a lookup key */
      snprintf(key, sizeof(key)-1, "%s:%d", filename, line);

      /* see if there's a duplicate */
      message = (struct m_log_message *) m_hash_table_get(client->messages, 
                                                          key);

      if( message != NULL)
      {
        /* found a duplicate, just increment the counter */
        message->repeat_count++;

        /* if the count is a multiple of 999, force a flush since we might be
         * caught in an infinite loop or tight inner loop */
        if( message->repeat_count % 999 == 0 )
        {
          mondemand_flush_logs(client);
        }
      } else {
        /* there is no duplicate, create a new entry */
        hash_key = strdup(key);
        if( hash_key != NULL )
        {
          message = m_try_malloc0( sizeof(struct m_log_message) );

          if( message != NULL )
          {
            strncpy(message->filename, filename, FILENAME_MAX);
            message->line = line;
            message->level = level;
            message->repeat_count = 1;
            message->trace_id = trace_id;
            vsnprintf( message->message, M_MESSAGE_MAX, format, args );
            m_hash_table_set( client->messages, hash_key, message );
          } else {
            free(hash_key);
            return -3;
          }
        } /* if( hash_key != NULL ) */
      } /* if( message != NULL ) */

      /* if we're in the immediate send level, or the bundle is bigger
       * than M_MAX_MESSAGES, or if a trace ID is set, emit the messages.
       */
      if( level <= client->immediate_send_level 
          || client->messages->num >= M_MAX_MESSAGES
          || mondemand_trace_id_compare(&trace_id, 
                                        &MONDEMAND_NULL_TRACE_ID) != 0 )
      {
        mondemand_flush_logs(client);
      }

    } /* if( mondemand_trace_id_compare ... ) */
  } /* if( client != NULL ... ) */

  return 0;
}

/* increment the value pointed at by key 'key' */
int
mondemand_stats_inc(struct mondemand_client *client, const char *filename,
                    const int line, const char *key, const int value)
{
  char buffer[FILENAME_MAX * 3];
  const char *real_key = key;
  char *new_key = NULL;
  MStatCounter *counter = NULL;

  /* the client must not be null, and either the filename or key must be set */
  if( client != NULL &&
      (filename != NULL || key != NULL) )
  {
    /* if the key wasn't set, use the filename+line number */
    if( real_key == NULL )
    {
      snprintf(buffer, FILENAME_MAX*2, "%s:%d", (char *) filename, line);
      real_key = buffer;
    }

    counter = m_hash_table_get( client->stats, real_key );

    if( counter == NULL )
    { 
      /* create a new entry */
      new_key = strdup(real_key);
      counter = (MStatCounter *) m_try_malloc( sizeof(MStatCounter) );
      if( counter == NULL || new_key == NULL )
      {
        /* malloc failed */
        free(new_key);
        free(counter);
        return -3;
      } else {
        *counter = value;
        if( m_hash_table_set( client->stats, new_key, counter ) != 0 )
        {
          free(new_key);
          free(counter);
          return -3;
        }
      }
    } else {
      /* we found the entry, simply increment */
      *counter += value;
    }
  }

  return 0;
}

int
mondemand_stats_dec(struct mondemand_client *client, const char *filename,
                    const int line, const char *key, const int value)
{
  return mondemand_stats_inc(client, filename, line, key, value * (-1));
}

int
mondemand_stats_set(struct mondemand_client *client, const char *filename,
                    const int line, const char *key, const int value)
{
  char buffer[FILENAME_MAX * 3];
  const char *real_key = key;
  char *new_key = NULL;
  MStatCounter *counter = NULL;

  /* the client must not be null, and either the filename or key must be set */
  if( client != NULL &&
      (filename != NULL || key != NULL) )
  {
    /* if the key wasn't set, use the filename+line number */
    if( real_key == NULL )
    {
      snprintf(buffer, FILENAME_MAX*2, "%s:%d", (char *) filename, line);
      real_key = buffer;
    }

    counter = m_hash_table_get( client->stats, real_key );

    if( counter == NULL )
    { 
      /* create a new entry */
      new_key = strdup(real_key);
      counter = (MStatCounter *) m_try_malloc( sizeof(MStatCounter) );
      if( counter == NULL || new_key == NULL )
      {
        /* malloc failed */
        free(new_key);
        free(counter);
        return -3;
      } else {
        *counter = value;
        if( m_hash_table_set( client->stats, new_key, counter ) != 0 )
        {
          free(new_key);
          free(counter);
          return -3;
        }
      }
    } else {
      /* we found the entry, simply increment */
      *counter = value;
    }
  }

  return 0;
}


/*========================================================================*/
/* Private functions                                                      */
