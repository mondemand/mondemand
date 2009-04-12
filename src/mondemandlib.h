#ifndef __MONDEMAND_H__
#define __MONDEMAND_H__

/*! \file mondemandlib.h
 *  \brief MonDemand client API file.  This is the entry point for developers
 *         to gather stats and messages and log them to the configured
 *         transport.
 */

#include "m_hash.h"

#include <stdarg.h>

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

/*! \struct mondemand_client
 *  \brief A definition for a mondemand_client structure.
 */
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
};

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

/*!\fn mondemand_client_set_immediate_send_level
 *       (struct mondemand_client *client, const int level)
 * \brief Method to set the immediate send level, which defines the minimum
 *        level where events are sent as soon as they are received (as
 *        opposed to being bundled).  Out of range values are ignored.
 * \param client a mondemand_client object pointer
 * \param level  a log level
 */
void
mondemand_client_set_immediate_send_level(struct mondemand_client *client,
                                          const int level);

/*!\fn mondemand_client_set_no_send_level(struct mondemand_client *client,
 *                                        const int level)
 * \brief Method to set the no send level, which defines the minimum log level
 *        where events are sent at all.  Anything lower than this defined
 *        level will be suppressed.  Out of range values are ignored.
 * \param client a mondemand_client object pointer
 * \param level  a log level
 */
void
mondemand_client_set_no_send_level(struct mondemand_client *client,
                                   const int level);

/*!\fn char *mondemand_client_get_context(struct mondemand_client *client,
 *                                        const char *key)
 * \brief Returns the value for a given key.
 */
char *
mondemand_client_get_context(struct mondemand_client *client,
                             const char *key);

/*!\fn mondemand_client_set_context(struct mondemand_client *client,
 *                                  const char *key, const char *value)
 * \brief Sets a contextual key/value pair to the client, which is sent
 *        out with event event.  If a value is already set, it overwrites it.
 *        This method creates a copy of the key and value internally, so
 *        they can be freed once set.
 */
void
mondemand_client_set_context(struct mondemand_client *client,
                             const char *key, const char *value);

/*!\fn mondemand_client_remove_context(struct mondemand_client *client,
 *                                     const char *key);
 * \brief Removes a contextual key value pair from the client, and
 *        frees that memory.
 */

/*!\fn mondemand_client_remove_all_contexts(struct mondemand_client *client)
 * \brief Method to remove all contextual key/value pairs.  Frees all
 *        related memory.
 * \aram client a mondemand_client object pointer
 */
void
mondemand_client_remove_all_contexts(struct mondemand_client *client);

#endif /* __MONDEMAND_H_ */

