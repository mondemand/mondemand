
#include "mem.h"
#include "mondemandlib.h"

#include <stdlib.h>
#include <string.h>

/*
 * Internal client structure, hidden from the header.
 */
struct _MonDemandClient
{
  char *prog_id;
  int immediate_send_level;
  int no_send_level;
};

/* ======================================================================== */
/* Public API functions                                                     */
/* ======================================================================== */

MonDemandClient *
mondemand_client_create(const char *program_identifier)
{
  MonDemandClient *client = NULL;

  client = (MonDemandClient *) m_try_malloc0(sizeof(MonDemandClient));

  if( client != NULL)
  {
    client->prog_id = strdup(program_identifier);
    client->immediate_send_level = M_LOG_CRIT;
    client->no_send_level = M_LOG_NOTICE;
  }

  return client;
}

void
mondemand_client_destroy(MonDemandClient *client)
{
  if( client != NULL )
  {
    m_free(client->prog_id);
    m_free(client);
  }
}

void
mondemand_client_set_immediate_send_level(MonDemandClient *client,
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

