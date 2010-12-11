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
#include <stdio.h>
#include <stdlib.h>

#include "lwes.h"
#include "m_mem.h"
#include "mondemandlib.h"
#include "mondemand_trace.h"
#include "mondemand_transport.h"

#define LWES_LOG_MSG "MonDemand::LogMsg"
#define LWES_STATS_MSG "MonDemand::StatsMsg"

/* private method forward declarations */
int mondemand_transport_stderr_log_sender(
                      const char *program_identifier,
                      const struct mondemand_log_message messages[],
                      const int message_count,
                      const struct mondemand_context contexts[],
                      const int context_count,
                      void *userdata);

int mondemand_transport_stderr_stats_sender(
                      const char *program_identifier,
                      const struct mondemand_stats_message stats[],
                      const int message_count,
                      const struct mondemand_context contexts[],
                      const int context_count,
                      void *userdata);

int mondemand_transport_lwes_log_sender(
                      const char *program_identifier,
                      const struct mondemand_log_message messages[],
                      const int message_count,
                      const struct mondemand_context contexts[],
                      const int context_count,
                      void *userdata);

int mondemand_transport_lwes_stats_sender(
                      const char *program_identifier,
                      const struct mondemand_stats_message stats[],
                      const int message_count,
                      const struct mondemand_context contexts[],
                      const int context_count,
                      void *userdata);

/*=========================================================================*/
/* Pubilc API Methods                                                      */
/*=========================================================================*/

struct mondemand_transport *
mondemand_transport_stderr_create(void)
{
  struct mondemand_transport *transport = NULL;

  transport = (struct mondemand_transport *)  
                m_try_malloc0(sizeof(struct mondemand_transport));

  if( transport != NULL )
  {
    transport->log_sender_function = &mondemand_transport_stderr_log_sender;
    transport->stats_sender_function = &mondemand_transport_stderr_stats_sender;
    transport->destroy_function = &mondemand_transport_stderr_destroy;
    transport->userdata = NULL; /* not used */
  }

  return transport;
}

void
mondemand_transport_stderr_destroy(struct mondemand_transport *transport)
{
  m_free(transport);
}

struct mondemand_transport *mondemand_transport_lwes_create(
                               const char *address, const int port,
                               const char *interface, int emit_heartbeat,
                               int heartbeat_frequency)
{
  return mondemand_transport_lwes_create_with_ttl(address, port,
                                                  interface, emit_heartbeat,
                                                  heartbeat_frequency, 1);
}

struct mondemand_transport *mondemand_transport_lwes_create_with_ttl(
                               const char *address, const int port,
                               const char *interface, int emit_heartbeat,
                               int heartbeat_frequency, int ttl)
{
  struct mondemand_transport *transport = NULL;
  struct lwes_emitter *emitter = NULL;

  transport = (struct mondemand_transport *)
                m_try_malloc0(sizeof(struct mondemand_transport));

  if( transport != NULL )
  {
    emitter = lwes_emitter_create_with_ttl((LWES_SHORT_STRING) address,
                                           (LWES_SHORT_STRING) interface,
                                           (LWES_U_INT_32) port, emit_heartbeat,
                                           heartbeat_frequency, ttl);
    if (emitter != NULL)
      {
        transport->log_sender_function = &mondemand_transport_lwes_log_sender;
        transport->stats_sender_function = &mondemand_transport_lwes_stats_sender;
        transport->destroy_function = &mondemand_transport_lwes_destroy;
        transport->userdata = emitter;
      }
    else
      {
        m_free (transport);
        transport = NULL;
      }
  }

  return transport;

}

void mondemand_transport_lwes_destroy(struct mondemand_transport *transport)
{
  if( transport != NULL )
  {
    if(transport->userdata != NULL )
    {
      lwes_emitter_destroy(transport->userdata);
    }
  }

  m_free(transport);
}

/*==========================================================================*/
/* Private API methods                                                      */
/*==========================================================================*/

int
mondemand_transport_stderr_log_sender(
                      const char *program_identifier,
                      const struct mondemand_log_message messages[], 
                      const int message_count,
                      const struct mondemand_context contexts[],
                      const int context_count,
                      void *userdata)
{
  int i=0;
  int j=0;

  for(i = 0; i < message_count; ++i)
    {
      if (messages[i].level >= M_LOG_EMERG
          && messages[i].level <= M_LOG_ALL)
        {
          fprintf (stderr, "[%s]", program_identifier);
          if (mondemand_trace_id_compare (&messages[i].trace_id,
                                          &MONDEMAND_NULL_TRACE_ID) != 0 )
            {
              fprintf (stderr, " : %ld", messages[i].trace_id._id);
            }
          fprintf (stderr, " : %s:%d",
                   messages[i].filename, messages[i].line);
          fprintf (stderr, " : %s : %s",
                   MonDemandLogLevelStrings[messages[i].level],
                   messages[i].message );

          if (context_count > 0)
            {
              for(j = 0; j < context_count; ++j )
                {
                  fprintf( stderr, " : %s=%s", contexts[j].key, contexts[j].value );
                }
            }

          if (messages[i].repeat_count > 1)
            {
              fprintf (stderr, " ... repeats %d times",
                       messages[i].repeat_count);
            }
          fprintf (stderr, "\n");
        } /* if( messages[i].level ... ) */
    } /* for(i=0; i<message_count; ++i) */

  /* we don't need userdata so just satisfy -Wall */
  (void) userdata;

  return 0;
}


int
mondemand_transport_stderr_stats_sender(
                      const char *program_identifier,
                      const struct mondemand_stats_message stats[],
                      const int message_count,
                      const struct mondemand_context contexts[],
                      const int context_count,
                      void *userdata)
{
  int i=0;
  int j=0;

  for(i=0; i<message_count; ++i)
  {
    fprintf( stderr, "[%s]", program_identifier );
    fprintf( stderr, " : %s : %lld", stats[i].key, stats[i].counter );

    if( context_count > 0 )
    {
      for( j=0; j<context_count; ++j )
      {
        fprintf( stderr, " : %s=%s", contexts[j].key, contexts[j].value );
      }
    }

    fprintf( stderr, "\n" );
  } /* for(i=0; i<message_count; ++i) */

  /* we don't need userdata so just satisfy -Wall */
  (void) userdata;

  return 0;
}

int
mondemand_transport_lwes_log_sender(
                      const char *program_identifier,
                      const struct mondemand_log_message messages[], 
                      const int message_count,
                      const struct mondemand_context contexts[],
                      const int context_count,
                      void *userdata)
{
  int i=0;
  int j=0;
  struct lwes_emitter *emitter = userdata;
  struct lwes_event *event = NULL;
  char key_buffer[31];

  if( message_count > 0 )
  {
    event = lwes_event_create(NULL, (LWES_SHORT_STRING) LWES_LOG_MSG);
    lwes_event_set_STRING(event, "prog_id", program_identifier);
    lwes_event_set_U_INT_16(event, "num", message_count);

    for(i=0; i<message_count; ++i)
    {
      if( messages[i].level >= M_LOG_EMERG 
          && messages[i].level <= M_LOG_ALL )
      {
        if( mondemand_trace_id_compare(&messages[i].trace_id,
                                       &MONDEMAND_NULL_TRACE_ID) != 0 )
        {
          snprintf(key_buffer, sizeof(key_buffer), "trace_id%d", i);
          lwes_event_set_U_INT_64(event, key_buffer, messages[i].trace_id._id);
        }

        snprintf(key_buffer, sizeof(key_buffer), "f%d", i);
        lwes_event_set_STRING(event, key_buffer, messages[i].filename);
        snprintf(key_buffer, sizeof(key_buffer), "l%d", i);
        lwes_event_set_U_INT_32(event, key_buffer, messages[i].line);
        snprintf(key_buffer, sizeof(key_buffer), "p%d", i);
        lwes_event_set_U_INT_32(event, key_buffer, messages[i].level);
        snprintf(key_buffer, sizeof(key_buffer), "m%d", i);
        lwes_event_set_STRING(event, key_buffer, messages[i].message);

        if( messages[i].repeat_count > 1 )
        {
          snprintf(key_buffer, sizeof(key_buffer), "r%d", i);
          lwes_event_set_U_INT_16(event, key_buffer, messages[i].repeat_count);
        }
      }
    } /* for(i=0; i<message_count; ++i) */

    if( context_count > 0 )
    {
      lwes_event_set_U_INT_16(event, "ctxt_num", context_count);
      for( j=0; j<context_count; ++j )
      {  
        snprintf(key_buffer, sizeof(key_buffer), "ctxt_k%d", j);
        lwes_event_set_STRING(event, key_buffer, contexts[j].key);
        snprintf(key_buffer, sizeof(key_buffer), "ctxt_v%d", j);
        lwes_event_set_STRING(event, key_buffer, contexts[j].value);
      }
    }

    lwes_emitter_emit(emitter, event);
    lwes_event_destroy(event);
  } /* if( message_count > 0 ) */

  return 0;
}

int
mondemand_transport_lwes_stats_sender(
                      const char *program_identifier,
                      const struct mondemand_stats_message stats[],
                      const int message_count,
                      const struct mondemand_context contexts[],
                      const int context_count,
                      void *userdata)
{
  int i=0;
  int j=0;
  struct lwes_emitter *emitter = userdata;
  struct lwes_event *event = NULL;
  char key_buffer[31];

  if( message_count > 0 )
  {
    event = lwes_event_create(NULL, (LWES_SHORT_STRING) LWES_STATS_MSG);
    lwes_event_set_STRING(event, "prog_id", program_identifier);
    lwes_event_set_U_INT_16(event, "num", message_count);

    for(i=0; i<message_count; ++i)
    {
      snprintf(key_buffer, sizeof(key_buffer), "k%d", i);
      lwes_event_set_STRING(event, key_buffer, stats[i].key);
      snprintf(key_buffer, sizeof(key_buffer), "v%d", i);
      lwes_event_set_INT_64(event, key_buffer, stats[i].counter);
    }

    if( context_count > 0 )
    {
      lwes_event_set_U_INT_16(event, "ctxt_num", context_count);
      for( j=0; j<context_count; ++j )
      {
        snprintf(key_buffer, sizeof(key_buffer), "ctxt_k%d", j);
        lwes_event_set_STRING(event, key_buffer, contexts[j].key);
        snprintf(key_buffer, sizeof(key_buffer), "ctxt_v%d", j);
        lwes_event_set_STRING(event, key_buffer, contexts[j].value);
      }
    }

    lwes_emitter_emit(emitter, event);
    lwes_event_destroy(event);
  }

  return 0;
}




