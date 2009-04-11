#ifndef __MONDEMAND_H__
#define __MONDEMAND_H__

/*! \file mondemandlib.h
 *  \brief MonDemand client API file.  This is the entry point for developers
 *         to gather stats and messages and log them to the configured
 *         transport.
 */

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

/*! \var typedef struct _mondemand_client mondemand_client
 *  \brief A type definition for a mondemand_client structure.
 * 
 *  Forward declaration.
 */

typedef struct _MonDemandClient MonDemandClient;

/*!\fn mondemand_client_create(const char *program_identifier)
 * \brief Constructor used to create a new logging client.
 *
 * \param program_identifier a string identifying this program to the network
 * \return a MonDemandClient object pointer
 */
MonDemandClient *mondemand_client_create(const char *program_identifier);


/*!\fn mondemand_client_destroy(MonDemandClient *client)
 *
 * \brief Destructor, used to clean up memory used by the client.  This
 *        automatically flushes any unsent events.
 *
 * \param MonDemandClient client object to be deleted.
 */
void mondemand_client_destroy(MonDemandClient *client);

/*!\fn mondemand_client_set_immediate_send_level(MonDemandClient *client,
 *                                               const int level)
 * \brief Method to set the immediate send level, which defines the minimum
 *        level where events are sent as soon as they are received (as
 *        opposed to being bundled).  Out of range values are ignored.
 * \param client a MonDemandClient object pointer
 * \param level  a log level
 */
void mondemand_client_set_immediate_send_level(MonDemandClient *client,
                                               const int level);


#endif /* __MONDEMAND_H_ */

