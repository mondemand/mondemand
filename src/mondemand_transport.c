
#include <stdio.h>
#include <stdlib.h>

#include "m_mem.h"
#include "mondemandlib.h"
#include "mondemand_trace.h"
#include "mondemand_transport.h"

int mondemand_transport_stderr_log_sender(
                      const struct mondemand_log_message messages[],
                      const int message_count,
                      const struct mondemand_context contexts[],
                      const int context_count,
                      void *userdata);

int mondemand_transport_stderr_stats_sender(
                      const struct mondemand_stats_message stats[],
                      const int message_count,
                      const struct mondemand_context contexts[],
                      const int context_count,
                      void *userdata);


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
    transport->userdata = NULL; /* not used */
  }

  return transport;
}

void
mondemand_transport_stderr_destroy(struct mondemand_transport *transport)
{
  m_free(transport);
}

int
mondemand_transport_stderr_log_sender(
                      const struct mondemand_log_message messages[], 
                      const int message_count,
                      const struct mondemand_context contexts[],
                      const int context_count,
                      void *userdata)
{
  int i=0;
  int j=0;

  for(i=0; i<message_count; ++i)
  {
    if( messages[i].level >= M_LOG_EMERG 
        && messages[i].level <= M_LOG_ALL )
    {

      if( mondemand_trace_id_compare(&messages[i].trace_id,
                                     &MONDEMAND_NULL_TRACE_ID) != 0 )
      {
        fprintf( stderr, "0x%016lx : %s:%d : %s : %s\n",
                 messages[i].trace_id._id, messages[i].filename,
                 messages[i].line, 
                 MonDemandLogLevelStrings[messages[i].level],
                 messages[i].message );
      } else {
        fprintf( stderr, "%s:%d : %s : %s\n",
                 messages[i].filename, messages[i].line,
                 MonDemandLogLevelStrings[messages[i].level],
                 messages[i].message );
      }

      if( context_count > 0 )
      {
        for( j=0; j<context_count; ++j )
        {  
          fprintf( stderr, "%s = %s ", contexts[i].key, contexts[i].value );
        }
        fprintf( stderr, "\n" );
      }

      if( messages[i].repeat_count > 1 )
      {
        fprintf( stderr, " ... repeats %d times\n",
                 messages[i].repeat_count );
      }
    } /* if( messages[i].level ... ) */
  } /* for(i=0; i<message_count; ++i) */

  /* we don't need userdata so just satisfy -Wall */
  (void) userdata;

  return 0;
}


int
mondemand_transport_stderr_stats_sender(
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
    fprintf( stderr, " %s = %016ld ", stats[i].key, stats[i].counter );

    if( context_count > 0 )
    {
      for( j=0; j<context_count; ++j )
      {  
        fprintf( stderr, "%s = %s ", contexts[j].key, contexts[j].value );
      }
      fprintf( stderr, "\n" );
    }
  } /* for(i=0; i<message_count; ++i) */

  /* we don't need userdata so just satisfy -Wall */
  (void) userdata;

  return 0;
}

