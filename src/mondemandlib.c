
#include "m_mem.h"
#include "mondemandlib.h"

#include <stdlib.h>
#include <string.h>

/* ======================================================================== */
/* Public API functions                                                     */
/* ======================================================================== */

struct mondemand_client *
mondemand_client_create(const char *program_identifier)
{
  struct mondemand_client *client = NULL;

  /* if the prog_id is null, don't continue. */
  if( program_identifier == NULL )
  {
    return NULL;
  }

  client = (struct mondemand_client *) 
            m_try_malloc0(sizeof(struct mondemand_client));

  if( client != NULL)
  {
    client->prog_id = strdup(program_identifier);
    client->immediate_send_level = M_LOG_CRIT;
    client->no_send_level = M_LOG_NOTICE;
  }

  return client;
}

void
mondemand_client_destroy(struct mondemand_client *client)
{
  if( client != NULL )
  {
    m_free(client->prog_id);
    m_free(client);
  }
}

void
mondemand_client_set_immediate_send_level(struct mondemand_client *client,
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

void
mondemand_client_set_no_send_level(struct mondemand_client *client,
                                   const int level)
{
  if( client != NULL )
  {
    if( level >= M_LOG_EMERG && level <= M_LOG_ALL )
    {
      client->no_send_level = level;
    }
  }
}

