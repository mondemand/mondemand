#ifndef __MONDEMAND_H__
#define __MONDEMAND_H__

/*! \file mondemandlib.h
 *  \brief MonDemand client API file.  This is the entry point for developers
 *         to gather stats and messages and log them to the configured
 *         transport.
 */

#include "config.h"
#include "m_hash.h"
#include "mondemand_trace.h"
#include "mondemand_transport.h"

#include <stdarg.h>

#ifdef __cplusplus
#define HAVE_ISO_VARARGS HAVE_ISO_CXX_VARARGS
#define HAVE_GNUC_VARARGS HAVE_GNUC_CXX_VARARGS
#else
#define HAVE_ISO_VARARGS HAVE_ISO_C_VARARGS
#define HAVE_GNUC_VARARGS HAVE_GNUC_C_VARARGS
#endif

/*
 * priorities map to the syslog priorities.
 */
#define M_LOG_EMERG       0       /* system is unusable */
#define M_LOG_ALERT       1       /* action must be taken immediately */
#define M_LOG_CRIT        2       /* critical conditions */
#define M_LOG_ERR         3       /* error conditions */
#define M_LOG_WARNING     4       /* warning conditions */
#define M_LOG_NOTICE      5       /* normal but significant condition */
#define M_LOG_INFO        6       /* informational */
#define M_LOG_DEBUG       7       /* debug-level messages */
#define M_LOG_ALL         8       /* all messages, including traces */

/* convenience */
#define M_LOG_WARN M_LOG_WARNING
#define M_LOG_ERROR M_LOG_ERR


/* forward declaration of opaque structs */
struct mondemand_client;


/*!\fn mondemand_client_create(const char *program_identifier)
 * \brief Constructor used to create a new logging client.
 *
 * \param program_identifier a string identifying this program to the network
 * \return a mondemand_client object pointer
 */
struct mondemand_client *
mondemand_client_create(const char *program_identifier);


/*!\fn mondemand_client_destroy(mondemand_client *client)
 *
 * \brief Destructor, used to clean up memory used by the client.  This
 *        automatically flushes any unsent events.
 *
 * \param client mondemand_client object to be deleted.
 */
void mondemand_client_destroy(struct mondemand_client *client);


/*!\fn mondemand_set_immediate_send_level(struct mondemand_client *client,
 *                                        const int level)
 * \brief Method to set the immediate send level, which defines the minimum
 *        level where events are sent as soon as they are received (as
 *        opposed to being bundled).  Out of range values are ignored.
 * \param client a mondemand_client object pointer
 * \param level  a log level
 */
void
mondemand_set_immediate_send_level(struct mondemand_client *client,
                                   const int level);

/*!\fn mondemand_set_no_send_level(struct mondemand_client *client,
 *                                 const int level)
 * \brief Method to set the no send level, which defines the minimum log level
 *        where events are sent at all.  Anything lower than this defined
 *        level will be suppressed.  Out of range values are ignored.
 * \param client a mondemand_client object pointer
 * \param level  a log level
 */
void
mondemand_set_no_send_level(struct mondemand_client *client, const int level);


/*!\fn const char *mondemand_get_context(struct mondemand_client *client,
 *                                       const char *key)
 * \brief Returns the value for a given key.
 */
const char *
mondemand_get_context(struct mondemand_client *client, const char *key);


/*!\fn const char **mondemand_get_context_keys(
 *         struct mondemand_client *client)
 * \brief returns an array of keys.  The array is NULL terminated, and must
 *        be freed by the caller when finished with it.
 */
const char **
mondemand_get_context_keys(struct mondemand_client *client);


/*!\fn mondemand_set_context(struct mondemand_client *client,
 *                           const char *key, const char *value)
 * \brief Sets a contextual key/value pair to the client, which is sent
 *        out with event event.  If a value is already set, it overwrites it.
 *        This method creates a copy of the key and value internally, so
 *        they can be freed once set.
 * \return zero on success, non-zero on failure
 */
int
mondemand_set_context(struct mondemand_client *client,
                      const char *key, const char *value);


/*!\fn mondemand_remove_context(struct mondemand_client *client,
 *                              const char *key);
 * \brief Removes a contextual key value pair from the client, and
 *        frees that memory.
 */
void
mondemand_remove_context(struct mondemand_client *client, const char *key);


/*!\fn mondemand_remove_all_contexts(struct mondemand_client *client)
 * \brief Method to remove all contextual key/value pairs.  Frees all
 *        related memory.
 * \aram client a mondemand_client object pointer
 */
void
mondemand_remove_all_contexts(struct mondemand_client *client);


/*!\fn mondemand_add_transport(struct mondemand_client *client,
 *                             struct mondemand_transport *transport)
 * \brief adds a transport, which are configurable callbacks used to
 *        send log and stat messages.
 */
int
mondemand_add_transport(struct mondemand_client *client,
                        struct mondemand_transport *transport);

/*@\fn mondemand_initialize_transports(struct mondemand_client *client)
 * \brief initializes all transports.  this calls the create function
 *        for each transport.
 */
int
mondemand_initialize_transports(struct mondemand_client *client);


/*\fn mondemand_level_is_enabled(struct mondemand_client *client,
 *                               const int log_level)
 * \brief this checks if a log level is enabled, which is useful to callers
 *        in case they want to check whether or not to bother to log.
 */
int
mondemand_level_is_enabled(struct mondemand_client *client,
                           const int log_level);

/*!\fn mondemand_flush_logs
 * \brief flushes logs to the transports.
 */
int mondemand_flush_logs(struct mondemand_client *client);

/*!\fn mondemand_flush_stats
 * \brief flushes stats to the transports.
 */
int mondemand_flush_stats(struct mondemand_client *client);

/*!\fn mondemand_flush_stats_no_reset
 * \brief flushes stats to the transports, but does NOT reset the counters.
 *        this is useful for monotonically increasing counters.
 */
int mondemand_flush_stats_no_reset(struct mondemand_client *client);

/*!\fn mondemand_flush
 * \brief flushes logs and stats.
 */
int mondemand_flush(struct mondemand_client *client);


/*=====================================================================*/
/* Useful Macros                                                       */
/*=====================================================================*/

#if HAVE_ISO_VARARGS

/* macros for ISO style varargs */

#define mondemand_log(m, level, tid, ...) \
  mondemand_log_real(m, __FILE__, __LINE__, level, tid, __VA_ARGS__)

#define mondemand_emerg(m, tid, ...) \
  mondemand_log_real(m, __FILE__, __LINE__, M_LOG_EMERG, tid, __VA_ARGS__)

#define mondemand_alert(m, tid, ...) \
  mondemand_log_real(m, __FILE__, __LINE__, M_LOG_ALERT, tid, __VA_ARGS__)

#define mondemand_crit(m, tid, ...) \
  mondemand_log_real(m, __FILE__, __LINE__, M_LOG_CRIT, tid, __VA_ARGS__)

#define mondemand_error(m, tid, ...) \
  mondemand_log_real(m, __FILE__, __LINE__, M_LOG_ERR, tid, __VA_ARGS__)

#define mondemand_warning(m, tid, ...) \
  mondemand_log_real(m, __FILE__, __LINE__, M_LOG_WARNING, tid, __VA_ARGS__)

#define mondemand_notice(m, tid, ...) \
  mondemand_log_real(m, __FILE__, __LINE__, M_LOG_NOTICE, tid, __VA_ARGS__)

#define mondemand_info(m, tid, ...) \
  mondemand_log_real(m, __FILE__, __LINE__, M_LOG_INFO, tid, __VA_ARGS__)

#define mondemand_debug(m, tid, ...) \
  mondemand_log_real(m, __FILE__, __LINE__, M_LOG_DEBUG, tid, __VA_ARGS__)


#elif HAVE_GNUC_VARARGS

/* macros for gnuc style varargs */

#define mondemand_log(m, level, tid, format...) \
  mondemand_log_real(m, __FILE__, __LINE__, level, tid, format)

#define mondemand_emerg(m, tid, format...) \
  mondemand_log_real(m, __FILE__, __LINE__, M_LOG_EMERG, tid, format)

#define mondemand_alert(m, tid, format...) \
  mondemand_log_real(m, __FILE__, __LINE__, M_LOG_ALERT, tid, format)

#define mondemand_crit(m, tid, format...) \
  mondemand_log_real(m, __FILE__, __LINE__, M_LOG_CRIT, tid, format)

#define mondemand_error(m, tid, format...) \
  mondemand_log_real(m, __FILE__, __LINE__, M_LOG_ERR, tid, format)

#define mondemand_warning(m, tid, format...) \
  mondemand_log_real(m, __FILE__, __LINE__, M_LOG_WARNING, tid, format)

#define mondemand_notice(m, tid, format...) \
  mondemand_log_real(m, __FILE__, __LINE__, M_LOG_NOTICE, tid, format)

#define mondemand_info(m, tid, format...) \
  mondemand_log_real(m, __FILE__, __LINE__, M_LOG_INFO, tid, format)

#define mondemand_debug(m, tid, format...) \
  mondemand_log_real(m, __FILE__, __LINE__, M_LOG_DEBUG, tid, format)

#endif

/* increments a counter by 1 using the filename:line */
#define mondemand_increment(m) \
  mondemand_stats_inc(m, __FILE__, __LINE__, NULL, 1)

/* increments a counter by value using the filename:line */
#define mondemand_increment_value(m, v) \
  mondemand_stats_inc(m, __FILE__, __LINE__, NULL, v)

/* increments the specified key by 1 */
#define mondemand_increment_key(m, k) \
  mondemand_stats_inc(m, __FILE__, __LINE__, k, 1);

/* increments the specified key by value */
#define mondemand_increment_key_by_val(m, k, v) \
  mondemand_stats_inc(m, __FILE__, __LINE__, k, v)

/* decrements a counter by 1 using filename:line */
#define mondemand_decrement(m) \
  mondemand_stats_dec(m, __FILE__, __LINE__, NULL, 1)

/* decrements a counter by value using filename:line */
#define mondemand_decrement_value(m, v) \
  mondemand_stats_dec(m, __FILE__, __LINE__, NULL, v)

/* decrements the named key by 1 */
#define mondemand_decrement_key(m, k) \
  mondemand_stats_dec(m, __FILE__, __LINE__, k, 1);

/* decrements the named key by value */
#define mondemand_decrement_key_by_val(m, k, v) \
  mondemand_stats_dec(m, __FILE__, __LINE__, k, v)

/* sets a counter by value using filename:line */
#define mondemand_set_by_val(m, v) \
  mondemand_stats_set(m, __FILE__, __LINE__, NULL, v)

/* set the specified key to the value specified */
#define mondemand_set_key_by_val(m, k, v) \
  mondemand_stats_set(m, __FILE__, __LINE__, k, v)


/*=====================================================================*/
/* Semi-private functions                                              */
/*=====================================================================*/

/*!\fn mondemand_log_real(struct mondemand_client *client,
 *                        const char *filename, const int line,
 *                        const int level,
 *                        const struct mondemand_trace_id trace_id,
 *                        const char *format, ...)
 * \brief full-featured logging function, usually called by macros.
 */
int mondemand_log_real(struct mondemand_client *client,
                       const char *filename, const int line, const int level,
                       const struct mondemand_trace_id trace_id,
                       const char *format, ...);

/*!\fn mondemand_log_real_va(struct mondemand_client *client,
 *                           const char *filename, const int line,
 *                           const int level,
 *                           const struct mondemand_trace_id trace_id,
 *                           const char *format, va_list args)
 * \brief full-featured logging function, usually called by macros.  This
 *        function takes a va_list so that calling functions can pass
 *        ellipsis style arguments
 */
int mondemand_log_real_va(struct mondemand_client *client,
                          const char *filename, const int line, 
                          const int level,
                          const struct mondemand_trace_id trace_id,
                          const char *format, va_list args);

/*!\fn int mondemand_stats_inc(struct mondemand_client *client,
 *                             const char *filename, const int line,
 *                             const char *key, const int value)
 * \brief lower-level stats function that increments a named key with
 *        the given value
 */
int mondemand_stats_inc(struct mondemand_client *client,
                        const char *filename, const int line,
                        const char *key, const int value);

/*!\fn int mondemand_stats_dec(struct mondemand_client *client,
 *                             const char *filename, const int line,
 *                             const char *key, const int value)
 * \brief lower-level stats function that decrements a named key with
 *        the given value
 */
int mondemand_stats_dec(struct mondemand_client *client,
                        const char *filename, const int line,
                        const char *key, const int value);

/*!\fn int mondemand_stats_set(struct mondemand_client *client,
 *                             const char *filename, const int line,
 *                             const char *key, const int value)
 * \brief lower-level stats function that sets a named key with
 *        the given value
 */
int mondemand_stats_set(struct mondemand_client *client,
                        const char *filename, const int line,
                        const char *key, const int value);


#endif /* __MONDEMAND_H_ */

