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
#ifndef __M_TRANSPORT_H__
#define __M_TRANSPORT_H__

#include "mondemand_types.h"
#include "mondemand_trace.h"

/* external structs that callback implementers would use */

/* represents a contextual data pair */
struct mondemand_context
{
  const char *key;
  const char *value;
};

struct mondemand_trace
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
  MondemandStatValue value;
  MondemandStatType type;
};

struct mondemand_timing
{
  char *label;
  long long int start;
  long long int end;
};

/* define callback functions */

/* stderr transport */
struct mondemand_transport *mondemand_transport_stderr_create(void);
void mondemand_transport_stderr_destroy(struct mondemand_transport *transport);

/* lwes transport */
struct mondemand_transport *mondemand_transport_lwes_create(
                               const char *address, const int port,
                               const char *interface, int emit_heartbeat,
                               int heartbeat_frequency);
struct mondemand_transport *mondemand_transport_lwes_create_with_ttl(
                               const char *address, const int port,
                               const char *interface, int emit_heartbeat,
                               int heartbeat_frequency, int ttl);
void mondemand_transport_lwes_destroy(struct mondemand_transport *transport);

/* method called when trying to log messages */
typedef int (*mondemand_transport_log_sender_t)
              (const char *program_identifier,
               const struct mondemand_log_message[],
               const int message_count,
               const struct mondemand_context[],
               const int context_count,
               void *);

/* method called when trying to log statistics */
typedef int (*mondemand_transport_stats_sender_t)
              (const char *program_identifier,
               const struct mondemand_stats_message[],
               const int message_count,
               const struct mondemand_context[],
               const int context_count,
               void *);

/* method called when trying to log traces */
typedef int (*mondemand_transport_trace_sender_t)
              (const char *program_identifier,
               const char *owner,
               const char *trace_id,
               const char *message,
               const struct mondemand_trace traces[],
               const int trace_count,
               void *);

/* method called when trying to log performance traces */
typedef int (*mondemand_transport_perf_sender_t)
              (const char *id,
               const char *caller_label,
               const struct mondemand_timing timings[],
               const int timings_count,
               const struct mondemand_context contexts[],
               const int context_count,
               void *);

/* method called when trying to log annotation traces */
typedef int (*mondemand_transport_annotation_sender_t)
              (const char *id,
               const long long int timestamp,
               const char *description,
               const char *text,
               const char *tags[],
               const int tag_count,
               const struct mondemand_context contexts[],
               const int context_count,
               void *);


typedef void (*mondemand_transport_destroy_t)
               (struct mondemand_transport *transport);

/* a transport struct to encapsulate the data */
struct mondemand_transport
{
  mondemand_transport_log_sender_t        log_sender_function;
  mondemand_transport_stats_sender_t      stats_sender_function;
  mondemand_transport_trace_sender_t      trace_sender_function;
  mondemand_transport_perf_sender_t       perf_sender_function;
  mondemand_transport_annotation_sender_t annotation_sender_function;
  mondemand_transport_destroy_t           destroy_function;
  void *userdata;
};

#endif
