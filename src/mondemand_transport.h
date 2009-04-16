#ifndef __M_TRANSPORT_H__
#define __M_TRANSPORT_H__

#include "mondemand_trace.h"

/* external structs that callback implementers would use */

struct mondemand_context
{
  const char *key;
  const char *value;
};

struct mondemand_log_message
{
  const char *filename;
  const int line;
  const int level;
  const int repeat_count;
  const char *message;
  const struct mondemand_trace_id trace_id;
};

struct mondemand_stats_message
{
  const char *key;
#if HAVE_LONG_LONG
  const long long counter;
#else
  const long counter;
#endif
};

/* define callback functions */

typedef int (*mondemand_transport_create_t)(void);

typedef int (*mondemand_transport_destroy_t)(void);

typedef int (*mondemand_transport_log_sender_t)
              (const struct mondemand_context **contexts,
               const struct mondemand_log_message **messages);

typedef int (*mondemand_transport_stat_sender_t)
              (const struct mondemand_context **contexts,
               const struct mondemand_stats_message **stats);

struct mondemand_transport
{
  mondemand_transport_create_t create_function;
  mondemand_transport_destroy_t destroy_function;
  mondemand_transport_log_sender_t log_sender_function;
  mondemand_transport_stat_sender_t stat_sender_function;
  void *userdata;
};

#endif
