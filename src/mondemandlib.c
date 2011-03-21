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
#include "config.h"

#include "m_mem.h"
#include "m_hash.h"
#include "mondemand_trace.h"
#include "mondemand_transport.h"
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
  /* trace id for trace messages only */
  char *trace_id;
  /* owner for trace messages only */
  char *owner;
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
  /* hashtable of trace data */
  struct m_hash_table *trace;
  /* array of transports */
  int num_transports;
  struct mondemand_transport **transports;
};


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

/* define an internal structure for keeping stats messages */
struct m_stat_message
{
  MondemandStatType type;
  MondemandStatValue value;
};

/* private forward declarations */
int mondemand_dispatch_logs(struct mondemand_client *client);
int mondemand_dispatch_stats(struct mondemand_client *client);
int mondemand_dispatch_trace(struct mondemand_client *client);

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

  client =
    (struct mondemand_client *) m_try_malloc0(sizeof(struct mondemand_client));

  if( client != NULL )
    {
      client->prog_id = strdup(program_identifier);
      client->trace_id = NULL;
      client->owner = NULL;
      client->immediate_send_level = M_LOG_CRIT;
      client->no_send_level = M_LOG_NOTICE;
      client->contexts = m_hash_table_create();
      client->messages = m_hash_table_create();
      client->stats = m_hash_table_create();
      client->trace = m_hash_table_create();
      client->num_transports = 0;
      client->transports = NULL;

      /* if any of the memory allocation has failed, bail out */
      if( client->prog_id == NULL || client->contexts == NULL 
          || client->messages == NULL || client->stats == NULL
          || client->trace == NULL )
        {
          mondemand_client_destroy(client);
          return NULL;
        }
    }

  return client;
}

void
mondemand_client_destroy (struct mondemand_client *client)
{
  int i=0;
  struct mondemand_transport *transport = NULL;

  if (client != NULL)
    {
      mondemand_flush (client);

      /* destroy all the transports */
      for(i=0; i < client->num_transports; ++i)
        {
          transport = client->transports[i];
          if( transport != NULL )
            {
              transport->destroy_function (transport);
            }
        }

      m_free(client->prog_id);
      m_free(client->trace_id);
      m_free(client->owner);
      m_hash_table_destroy(client->contexts);
      m_hash_table_destroy(client->messages);
      m_hash_table_destroy(client->stats);
      m_hash_table_destroy(client->trace);
      m_free(client->transports);
      client->num_transports = 0;
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
mondemand_get_context (struct mondemand_client *client, const char *key)
{
  const char *ret = NULL;

  if (client != NULL)
    {
      ret = m_hash_table_get (client->contexts, key);
    }

  return ret;
}

const char **
mondemand_get_context_keys(struct mondemand_client *client)
{
  const char **ret = NULL;

  if (client != NULL)
    {
      ret = m_hash_table_keys (client->contexts);
    }

  return ret;
}

/* sets a context, overwriting it if it is already set */
int
mondemand_set_context (struct mondemand_client *client,
                       const char *key, const char *value)
{
  char *k = NULL;
  char *v = NULL;

  if (client != NULL && key != NULL && value != NULL)
    {
      if (client->contexts != NULL)
        {
          k = strdup (key);
          v = strdup (value);

          /* if we can't allocate memory for the values, bail out */
          if (k == NULL || v == NULL)
            {
              return -3;
            }

          if (m_hash_table_set (client->contexts, k, v) != 0)
            {
              m_free (k);
              m_free (v);
              return -3;
            }
        }
    }

  return 0;
}

/* remove a context */
void
mondemand_remove_context (struct mondemand_client *client, const char *key)
{
  if (client != NULL)
    {
      m_hash_table_remove (client->contexts, key);
    }
}

/* remove all contexts */
void
mondemand_remove_all_contexts (struct mondemand_client *client)
{
  if( client != NULL )
    {
      m_hash_table_remove_all (client->contexts);
    }
}

/* returns the value for a given key */
const char *
mondemand_get_trace (struct mondemand_client *client, const char *key)
{
  const char *ret = NULL;

  if( client != NULL)
    {
      ret = m_hash_table_get (client->trace, key);
    }

  return ret;
}

const char **
mondemand_get_trace_keys (struct mondemand_client *client)
{
  const char **ret = NULL;

  if (client != NULL)
    {
      ret = m_hash_table_keys (client->trace);
    }

  return ret;
}

/* sets a trace item, overwriting it if it is already set */
int
mondemand_set_trace (struct mondemand_client *client,
                     const char *key, const char *value)
{
  char *k = NULL;
  char *v = NULL;

  if (client != NULL && key != NULL && value != NULL)
    {
      k = strdup (key);
      v = strdup (value);

      /* if we can't allocate memory for the values, bail out */
      if( k == NULL || v == NULL )
        {
          return -3;
        }

      if (m_hash_table_set (client->trace, k, v) != 0)
        {
          m_free (k);
          m_free (v);
          return -3;
        }
    }

  return 0;
}

/* remove a trace item */
void
mondemand_remove_trace (struct mondemand_client *client, const char *key)
{
  if (client != NULL)
    {
      m_hash_table_remove (client->trace, key);
    }
}

/* remove all trace items */
void
mondemand_remove_all_traces (struct mondemand_client *client)
{
  if (client != NULL)
    {
      m_hash_table_remove_all (client->trace);
    }
}

/* add a transport, by storing a reference to it */
int
mondemand_add_transport(struct mondemand_client *client,
                        struct mondemand_transport *transport)
{
  struct mondemand_transport **data = NULL;

  if( client != NULL && transport != NULL )
    {
      data = (struct mondemand_transport **)
        m_try_realloc(client->transports,
                      (client->num_transports + 1) *
                      sizeof(struct mondemand_transport *));

      if( data == NULL )
        {
          return -3;
        }

      data[client->num_transports] = transport;
      client->num_transports++;
      client->transports = data;
    }

  return 0;
}

/* check if a level is enabled */
int
mondemand_level_is_enabled(struct mondemand_client *client,
                           const int log_level)
{
  if( client != NULL )
    {
      return log_level < client->no_send_level;
    }
  else
    {
      return 0;
    }
}

/* flush the logs to the transports */
int
mondemand_flush_logs(struct mondemand_client *client)
{
  int retval = 0;

  if( client != NULL )
    {
      if( (retval = mondemand_dispatch_logs(client)) != 0 )
        {
          return -1;
        }

      m_hash_table_remove_all( client->messages );
    }

  return 0;
}

/* flush the stats to the transports */
int
mondemand_flush_stats(struct mondemand_client *client)
{
  if( client != NULL )
    {
      if( mondemand_dispatch_stats(client) != 0 )
        {
          return -1;
        }
    }

  return 0;
}

int
mondemand_reset_stats (struct mondemand_client *client)
{
  int i=0;
  const char **keys = NULL;
  struct m_stat_message *stat = NULL;

  if (client != NULL)
    {
      /* reset all the stats */
      keys = m_hash_table_keys (client->stats);
      if (keys != NULL)
        {
          for (i=0; keys[i] != NULL; ++i)
            {
              stat =
                (struct m_stat_message *)
                  m_hash_table_get(client->stats, keys[i]);
              if (stat != NULL)
                {
                  stat->value = 0;
                }
            }
          m_free(keys);
        }
    }

  return 0;
}

/* flush everything */
int
mondemand_flush(struct mondemand_client *client)
{
  int retval = 0;

  retval += mondemand_flush_logs(client);
  retval += mondemand_flush_stats(client);
  retval += mondemand_flush_trace(client);

  return retval;
}

int
mondemand_initialize_trace(struct mondemand_client *client,
                           const char *trace_id,
                           const char *owner)
{
  if( client != NULL )
    {
      client->trace_id = strdup (trace_id);
      client->owner = strdup (owner);
    }
  return 0;
}

int
mondemand_flush_trace(struct mondemand_client *client)
{
  if (client != NULL && client->trace_id != NULL && client->owner != NULL)
    {
      if (mondemand_dispatch_trace (client) != 0)
        {
          return -1;
        }
    }

  return 0;
}

void
mondemand_clear_trace (struct mondemand_client *client)
{
  if (client != NULL)
    {
      mondemand_remove_all_traces (client);
      m_free (client->trace_id);
      m_free (client->owner);
    }
}

int mondemand_log_level_from_string (const char *level)
{
  unsigned int i;

  if (level == NULL)
    {
      return -1;
    }

  for (i = 0 ;
       i < (sizeof (MonDemandLogLevelStrings)
            / sizeof ((MonDemandLogLevelStrings)[0]));
       ++i)
    {
      if (strcmp (level, MonDemandLogLevelStrings[i]) == 0)
        {
          return i;
        }
    }

  return -1;
}

MondemandStatType mondemand_stat_type_from_string (const char *type)
{
  if (strcmp (type, MondemandStatTypeString[MONDEMAND_GAUGE]) == 0)
    {
      return MONDEMAND_GAUGE;
    }
  else if (strcmp (type, MondemandStatTypeString[MONDEMAND_COUNTER]) == 0)
    {
      return MONDEMAND_COUNTER;
    }
  else
    {
      return MONDEMAND_UNKNOWN;
    }
}

/*========================================================================*/
/* Semi-private functions                                                 */
/*========================================================================*/

int
mondemand_log_real(struct mondemand_client *client,
                   const char *filename,
                   const int line,
                   const int level,
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
                      const char *filename,
                      const int line,
                      const int level,
                      const struct mondemand_trace_id trace_id,
                      const char *format,
                      va_list args)
{
  char key[(FILENAME_MAX * 3)];
  struct m_log_message *message = NULL;
  char *hash_key = NULL;

  if( client != NULL && format != NULL )
    {
      /* if the trace ID is NULL or the no send level is too high,
       * give up now */
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
          message =
            (struct m_log_message *) m_hash_table_get(client->messages, key);

          if( message != NULL)
            {
              /* found a duplicate, just increment the counter */
              message->repeat_count++;

              /* if the count is a multiple of 999, force a flush since
               * we might be caught in an infinite loop or tight inner loop */
              if( message->repeat_count % 999 == 0 )
                {
                  mondemand_flush_logs(client);
                }
            }
          else
            {
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
                    }
                  else
                    {
                      m_free(hash_key);
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

int
mondemand_stats_perform_op (struct mondemand_client *client,
                            const char *filename,
                            const int line,
                            const MondemandOp op,
                            const MondemandStatType type,
                            const char *key,
                            const MondemandStatValue value)
{
  char buffer[FILENAME_MAX * 3];
  const char *real_key = key;
  char *new_key = NULL;
  struct m_stat_message *stat = NULL;

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

      /* then get or create a stat */
      stat =
        (struct m_stat_message *) m_hash_table_get (client->stats, real_key);

      if( stat == NULL )
        {
          /* create a new entry */
          new_key = strdup (real_key);
          stat =
            (struct m_stat_message*)
            m_try_malloc0( sizeof(struct m_stat_message) );
          if( stat == NULL || new_key == NULL )
            {
              goto ERROR;
            }
          else
            {
              /* always start with a zeroed stat */
              stat->type = type;
              stat->value = 0;
              /* attempt to add to hashtable */
              if( m_hash_table_set( client->stats, new_key, stat ) != 0 )
                {
                  goto ERROR;
                }
            }
        }

      /* FIXME: should I really reset the type? */
      stat->type = type;
      /* depending on operation change value */
      switch (op)
        {
        case MONDEMAND_INC:
          stat->value += value;
          break;
        case MONDEMAND_DEC:
          stat->value -= value;
          break;
        case MONDEMAND_SET:
          stat->value = value;
          break;
        }
    }

  return 0;

ERROR:
  m_free (new_key);
  m_free (stat);
  return -3;
}

/*========================================================================*/
/* Private functions                                                      */
/*========================================================================*/

/* dispatches log messages to the transports */
int
mondemand_dispatch_logs(struct mondemand_client *client)
{
  int retval = 0;
  int i = 0;
  struct mondemand_transport *transport = NULL;
  const char **message_keys = NULL;
  const char **context_keys = NULL;
  struct m_log_message *message = NULL;
  struct mondemand_log_message *messages = NULL;
  struct mondemand_context *contexts = NULL;

  if( client != NULL )
    {
      if( client->messages != NULL && client->contexts != NULL )
        {
          /* fetch all the messages */
          message_keys = m_hash_table_keys( client->messages );

          /* allocate an array of structs */
          messages = (struct mondemand_log_message *)
            m_try_malloc0(sizeof(struct mondemand_log_message) *
                          client->messages->num);
          for(i=0; i<client->messages->num; ++i)
            {
              /* we copy the values from the hash table since we want an
                 immutable const structure to pass to the transport */
              message =
                (struct m_log_message *) m_hash_table_get(client->messages,
                                                          message_keys[i]);
              messages[i].filename = message->filename;
              messages[i].line = message->line;
              messages[i].level = message->level;
              messages[i].repeat_count = message->repeat_count;
              messages[i].message = message->message;
              messages[i].trace_id = message->trace_id;
            }

          /* fetch the keys to all the contexts */
          context_keys = m_hash_table_keys( client->contexts );

          contexts = (struct mondemand_context *)
            m_try_malloc0(sizeof(struct mondemand_context) *
                          client->contexts->num);
          for(i=0; i<client->contexts->num; ++i)
            {
              /* copy the pointer to a const struct that the transport
                 can copy from */
              contexts[i].key = context_keys[i];
              contexts[i].value = (char *) m_hash_table_get(client->contexts,
                                                            context_keys[i]);
            }

          /* iterate through each transport */
          for(i=0; i<client->num_transports; ++i)
            {
              transport = client->transports[i];
              if( transport != NULL )
                {
                  if( transport->log_sender_function(client->prog_id,
                                                     messages, client->messages->num,
                                                     contexts, client->contexts->num,
                                                     transport->userdata) != 0 )
                    {
                      retval = -1;
                    }
                }
            } /* for(i=0; i<client->num_transports; ++i) */

          /* clean up memory */
          m_free(messages);
          m_free(message_keys);
          m_free(contexts);
          m_free(context_keys);
        } /* if( client->messages != NULL && client->contexts != NULL ) */
    } /* if( client != NULL ) */

  return retval;
}

/* dispatches stats to the transports */
int
mondemand_dispatch_stats(struct mondemand_client *client)
{
  int retval = 0;
  int i=0;
  struct mondemand_transport *transport = NULL;
  const char **message_keys = NULL;
  const char **context_keys = NULL;
  struct mondemand_stats_message *messages = NULL;
  struct mondemand_context *contexts = NULL;

  if( client != NULL )
    {
      if( client->stats != NULL && client->contexts != NULL )
        {
          /* fetch the stats */
          message_keys = m_hash_table_keys( client->stats );

          /* allocate an array of structs */
          messages = (struct mondemand_stats_message *)
            m_try_malloc0(sizeof(struct mondemand_stats_message) *
                          client->stats->num);

          for (i=0; i<client->stats->num; ++i)
            {
              messages[i].key = message_keys[i];
              struct m_stat_message *stat =
                (struct m_stat_message *)
                  m_hash_table_get(client->stats, message_keys[i]);
              messages[i].type = stat->type;
              messages[i].value = stat->value;
            }

          /* fetch the keys to all the contexts */
          context_keys = m_hash_table_keys (client->contexts);

          contexts = (struct mondemand_context *)
            m_try_malloc0 (sizeof (struct mondemand_context) *
                           client->contexts->num);
          for (i=0; i<client->contexts->num; ++i)
            {
              /* copy the pointer to a const struct that the transport
                 can copy from */
              contexts[i].key = context_keys[i];
              contexts[i].value = (char *) m_hash_table_get (client->contexts,
                                                             context_keys[i]);
            }

          /* iterate through each transport */
          for (i=0; i<client->num_transports; ++i)
            {
              transport = client->transports[i];
              if (transport != NULL)
                {
                  if (transport->stats_sender_function
                        (client->prog_id,
                         messages,
                         client->stats->num,
                         contexts,
                         client->contexts->num,
                         transport->userdata) != 0)
                    {
                      retval = -1;
                    }
                }
            }

          m_free (contexts);
          m_free (context_keys);
          m_free (messages);
          m_free (message_keys);
        }
    }

  return retval;
}

int mondemand_dispatch_trace (struct mondemand_client *client)
{
  int retval = 0;
  struct mondemand_transport *transport = NULL;
  const char **trace_keys = NULL;
  struct mondemand_trace *traces = NULL;
  int i;

  if( client != NULL )
    {
      if (client->trace != NULL)
        {
          /* fetch the keys to all the contexts */
          trace_keys = m_hash_table_keys (client->trace);

          traces = (struct mondemand_trace *)
            m_try_malloc0 (sizeof (struct mondemand_trace) *
                           client->trace->num);
          for (i=0; i<client->trace->num; ++i)
            {
              /* copy the pointer to a const struct that the transport
                 can copy from */
              traces[i].key = trace_keys[i];
              traces[i].value = (char *) m_hash_table_get (client->trace,
                                                           trace_keys[i]);
            }

          /* iterate through each transport */
          for (i=0; i<client->num_transports; ++i)
            {
              transport = client->transports[i];
              if (transport != NULL)
                {
                  if (transport->trace_sender_function
                        (client->prog_id,
                         client->trace_id,
                         client->owner,
                         traces,
                         client->trace->num,
                         transport->userdata) != 0)
                    {
                      retval = -1;
                    }
                }
            }

          m_free (traces);
          m_free (trace_keys);
        }
    }

  return retval;
}
