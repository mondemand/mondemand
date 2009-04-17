#ifndef __M_TRANSPORT_H__
#define __M_TRANSPORT_H__

#include "mondemand_trace.h"

/* external structs that callback implementers would use */

/* represents a contextual data pair */
struct mondemand_context
{
  const char *key;
  const char *value;
};

/* represents a single log message */
struct mondemand_log_message
{
  const char *filename;
  int line;
  int level;
  int repeat_count;
  const char *message;
  struct mondemand_trace_id trace_id;
};

/* represents a single statistic */
struct mondemand_stats_message
{
  const char *key;
#if HAVE_LONG_LONG
  long long counter;
#else
  long counter;
#endif
};

/* define callback functions */

/* method called when trying to log messages */
typedef int (*mondemand_transport_log_sender_t)
              (const struct mondemand_log_message[],
               const int message_count,
               const struct mondemand_context[],
               const int context_count,
               void *);

/* method called when trying to log statistics */
typedef int (*mondemand_transport_stats_sender_t)
              (const struct mondemand_stats_message[],
               const int message_count,
               const struct mondemand_context[],
               const int context_count,
               void *);

/* a transport struct to encapsulate the data */
struct mondemand_transport
{
  mondemand_transport_log_sender_t log_sender_function;
  mondemand_transport_stats_sender_t stats_sender_function;
  void *userdata;
};

#endif
