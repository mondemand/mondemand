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
#include <unistd.h>

#include "lwes.h"
#include "m_mem.h"
#include "mondemandlib.h"
#include "mondemand_trace.h"
#include "mondemand_transport.h"

#define LWES_LOG_MSG "MonDemand::LogMsg"
#define LWES_STATS_MSG "MonDemand::StatsMsg"
#define LWES_TRACE_MSG "MonDemand::TraceMsg"
#define LWES_PERF_MSG "MonDemand::PerfMsg"
#define LWES_ANNOTATION_MSG "MonDemand::AnnotationMsg"

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

int mondemand_transport_stderr_trace_sender(
                      const char *program_identifier,
                      const char *owner,
                      const char *trace_id,
                      const char *message,
                      const struct mondemand_trace traces[],
                      const int trace_count,
                      void *userdata);

int mondemand_transport_stderr_perf_sender(
               const char *id,
               const char *caller_label,
               const struct mondemand_timing timings[],
               const int timings_count,
               const struct mondemand_context contexts[],
               const int context_count,
               void *userdata);

int mondemand_transport_stderr_annotation_sender(
               const char *id,
               const long long int timestamp,
               const char *description,
               const char *text,
               const char *tags[],
               const int tag_count,
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

int mondemand_transport_lwes_trace_sender(
                      const char *program_identifier,
                      const char *owner,
                      const char *trace_id,
                      const char *message,
                      const struct mondemand_trace traces[],
                      const int trace_count,
                      void *userdata);

int mondemand_transport_lwes_perf_sender(
               const char *id,
               const char *caller_label,
               const struct mondemand_timing[],
               const int timings_count,
               const struct mondemand_context[],
               const int context_count,
               void *userdata);

int mondemand_transport_lwes_annotation_sender(
               const char *id,
               const long long int timestamp,
               const char *description,
               const char *text,
               const char *tags[],
               const int num_tags,
               const struct mondemand_context[],
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
      transport->log_sender_function =
        &mondemand_transport_stderr_log_sender;
      transport->stats_sender_function =
        &mondemand_transport_stderr_stats_sender;
      transport->trace_sender_function =
        &mondemand_transport_stderr_trace_sender;
      transport->perf_sender_function =
        &mondemand_transport_stderr_perf_sender;
      transport->annotation_sender_function =
        &mondemand_transport_stderr_annotation_sender;
      transport->destroy_function =
        &mondemand_transport_stderr_destroy;
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
                                                  heartbeat_frequency, 3);
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
      emitter =
        lwes_emitter_create_with_ttl ((LWES_SHORT_STRING) address,
                                      (LWES_SHORT_STRING) interface,
                                      (LWES_U_INT_32) port,
                                      emit_heartbeat,
                                      heartbeat_frequency,
                                      ttl);
      if (emitter != NULL)
        {
          transport->log_sender_function =
            &mondemand_transport_lwes_log_sender;
          transport->stats_sender_function =
            &mondemand_transport_lwes_stats_sender;
          transport->trace_sender_function =
            &mondemand_transport_lwes_trace_sender;
          transport->perf_sender_function =
            &mondemand_transport_lwes_perf_sender;
          transport->annotation_sender_function =
            &mondemand_transport_lwes_annotation_sender;
          transport->destroy_function =
            &mondemand_transport_lwes_destroy;
          transport->userdata =
            emitter;
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
                  fprintf ( stderr, " : %s=%s", contexts[j].key,
                            contexts[j].value );
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
      fprintf( stderr, " %s : %s : %lld",
               MondemandStatTypeString[stats[i].type],
               stats[i].key,
               stats[i].value);

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
mondemand_transport_stderr_trace_sender
  (const char *program_identifier,
   const char *owner,
   const char *trace_id,
   const char *message,
   const struct mondemand_trace traces[],
   const int trace_count,
   void *userdata)
{
  int j;

  fprintf (stderr, "[%s] %s:%s : %s", program_identifier, owner, trace_id,
           message);
  if (trace_count > 0)
    {
      for (j=0; j < trace_count ; ++j)
        {
          fprintf (stderr, " : %s=%s", traces[j].key, traces[j].value);
        }
    }
  fprintf (stderr, "\n");

  /* we don't need userdata so just satisfy -Wall */
  (void) userdata;

  return 0;
}

int mondemand_transport_stderr_perf_sender(
               const char *id,
               const char *caller_label,
               const struct mondemand_timing timings[],
               const int timings_count,
               const struct mondemand_context contexts[],
               const int context_count,
               void *userdata)
{
  int t = 0;
  int c = 0;

  for (t = 0; t < timings_count; ++t)
    {
      fprintf (stderr, "[%s]", id);
      if (context_count > 0)
        {
          for(c = 0; c < context_count; ++c )
            {
              fprintf (stderr, " : %s=%s", contexts[c].key,
                        contexts[c].value );
            }
        }
      fprintf (stderr, " : %s -> %s : %lld -> %lld\n", caller_label,
               timings[t].label, timings[t].start, timings[t].end);
    }

  /* we don't need userdata so just satisfy -Wall */
  (void) userdata;

  return 0;
}

int mondemand_transport_stderr_annotation_sender(
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
  int t = 0;
  int c = 0;

  fprintf (stderr, "[%s]", id);
  if (context_count > 0)
    {
      for(c = 0; c < context_count; ++c )
        {
          fprintf (stderr, " : %s=%s", contexts[c].key,
                    contexts[c].value );
        }
    }
  fprintf (stderr, " : %lld", timestamp);
  if (tag_count > 0)
    {
      fprintf (stderr, " : ");
      for (t = 0 ; t < tag_count - 1; ++t)
        {
          fprintf (stderr, "%s,", tags[t]);
        }
      fprintf (stderr, "%s", tags[t]);
    }
  fprintf (stderr, " : %s", description);
  if (text != NULL)
    {
      fprintf (stderr, " : %s", text);
    }
  fprintf (stderr, "\n");

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
                  lwes_event_set_U_INT_64(event, key_buffer,
                                          messages[i].trace_id._id);
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
                  lwes_event_set_U_INT_16(event, key_buffer,
                                          messages[i].repeat_count);
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
          snprintf (key_buffer, sizeof(key_buffer), "t%d", i);
          lwes_event_set_STRING (event, key_buffer,
                                 MondemandStatTypeString[stats[i].type]);
          snprintf (key_buffer, sizeof(key_buffer), "k%d", i);
          lwes_event_set_STRING (event, key_buffer, stats[i].key);
          snprintf (key_buffer, sizeof(key_buffer), "v%d", i);
          lwes_event_set_INT_64 (event, key_buffer, stats[i].value);
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

int
mondemand_transport_lwes_trace_sender
  (const char *program_identifier,
   const char *owner,
   const char *trace_id,
   const char *message,
   const struct mondemand_trace traces[],
   const int trace_count,
   void *userdata)
{
  struct lwes_emitter *emitter = userdata;
  struct lwes_event *event = NULL;
  char hostname[1024];
  int j;

  hostname[1023] = '\0';
  gethostname (hostname, 1023);

  event = lwes_event_create (NULL, (LWES_SHORT_STRING) LWES_TRACE_MSG);
  lwes_event_set_STRING (event, "mondemand.prog_id", program_identifier);
  lwes_event_set_STRING (event, "mondemand.trace_id", trace_id);
  lwes_event_set_STRING (event, "mondemand.owner", owner);
  lwes_event_set_STRING (event, "mondemand.src_host", hostname);
  lwes_event_set_STRING (event, "mondemand.message", message);

  if (trace_count > 0)
    {
      for (j=0; j < trace_count; ++j)
        {
          lwes_event_set_STRING (event, traces[j].key, traces[j].value);
        }
    }

  lwes_emitter_emit(emitter, event);
  lwes_event_destroy(event);
  return 0;
}

int mondemand_transport_lwes_perf_sender(
               const char *id,
               const char *caller_label,
               const struct mondemand_timing timings[],
               const int timings_count,
               const struct mondemand_context contexts[],
               const int context_count,
               void *userdata)
{
  struct lwes_emitter *emitter = userdata;
  struct lwes_event *event = NULL;
  char key_buffer[31];
  int t = 0;
  int c = 0;

  if (timings_count > 0)
    {
      event = lwes_event_create (NULL, (LWES_SHORT_STRING) LWES_PERF_MSG);
      lwes_event_set_STRING(event, "id", id);
      lwes_event_set_STRING(event, "caller_label", caller_label);
      lwes_event_set_U_INT_16(event, "num", timings_count);
      for (t = 0; t < timings_count; ++t)
        {
          snprintf (key_buffer, sizeof(key_buffer), "label%d", t);
          lwes_event_set_STRING (event, key_buffer, timings[t].label);
          snprintf (key_buffer, sizeof(key_buffer), "start%d", t);
          lwes_event_set_INT_64 (event, key_buffer, timings[t].start);
          snprintf (key_buffer, sizeof(key_buffer), "end%d", t);
          lwes_event_set_INT_64 (event, key_buffer, timings[t].end);
        }
      if( context_count > 0 )
        {
          lwes_event_set_U_INT_16(event, "ctxt_num", context_count);
          for(c = 0; c < context_count; ++c )
            {
              snprintf(key_buffer, sizeof(key_buffer), "ctxt_k%d", c);
              lwes_event_set_STRING(event, key_buffer, contexts[c].key);
              snprintf(key_buffer, sizeof(key_buffer), "ctxt_v%d", c);
              lwes_event_set_STRING(event, key_buffer, contexts[c].value);
            }
        }

      lwes_emitter_emit(emitter, event);
      lwes_event_destroy(event);
  }

  return 0;
}

int mondemand_transport_lwes_annotation_sender(
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
  struct lwes_emitter *emitter = userdata;
  struct lwes_event *event = NULL;
  char key_buffer[31];
  int t = 0;
  int c = 0;

  event = lwes_event_create (NULL, (LWES_SHORT_STRING) LWES_ANNOTATION_MSG);
  lwes_event_set_STRING(event, "id", id);
  lwes_event_set_INT_64(event, "timestamp", timestamp);
  lwes_event_set_STRING(event, "description", description);
  if (text != NULL)
    {
      lwes_event_set_STRING(event, "text", text);
    }

  if (tag_count > 0)
    {
      lwes_event_set_U_INT_16 (event, "tag_num", tag_count);
      for (t = 0; t < tag_count; ++t)
        {
          snprintf (key_buffer, sizeof(key_buffer), "tag%d", t);
          lwes_event_set_STRING (event, key_buffer, tags[t]);
        }
    }

  if( context_count > 0 )
    {
      lwes_event_set_U_INT_16(event, "ctxt_num", context_count);
      for(c = 0; c < context_count; ++c )
        {
          snprintf(key_buffer, sizeof(key_buffer), "ctxt_k%d", c);
          lwes_event_set_STRING(event, key_buffer, contexts[c].key);
          snprintf(key_buffer, sizeof(key_buffer), "ctxt_v%d", c);
          lwes_event_set_STRING(event, key_buffer, contexts[c].value);
        }
    }

  lwes_emitter_emit(emitter, event);
  lwes_event_destroy(event);

  return 0;
}




