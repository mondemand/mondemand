
#include "m_mem.h"
#include "m_hash.h"
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

  if( client != NULL )
  {
    client->prog_id = strdup(program_identifier);
    client->immediate_send_level = M_LOG_CRIT;
    client->no_send_level = M_LOG_NOTICE;
    client->contexts = m_hash_table_create();

    /* if any of the memory allocation has failed, bail out */
    if( client->prog_id == NULL ||
        client->contexts == NULL )
    {
      mondemand_client_destroy(client);
      return NULL;
    }
  }

  return client;
}

void
mondemand_client_destroy(struct mondemand_client *client)
{
  if( client != NULL )
  {
    m_free(client->prog_id);
    m_hash_table_destroy(client->contexts);
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

/* returns the value for a given key */
char *
mondemand_client_get_context(struct mondemand_client *client,
                             const char *key)
{
  char *ret = NULL;

  if( client != NULL && key != NULL )
  {
    if( client->contexts != NULL )
    {
      ret = m_hash_table_get( client->contexts, key );
    }
  }

  return ret;
}

/* sets a context, overwriting it if it is already set */
void
mondemand_client_set_context(struct mondemand_client *client,
                             const char *key, const char *value)
{
  char *k = NULL;
  char *v = NULL;

  if( client != NULL && key != NULL && value != NULL )
  {
    if( client->contexts != NULL )
    {
      k = strdup(key);
      v = strdup(value);

      /* if we can't allocate memory for the values, bail out */
      if( k == NULL || v == NULL )
      {
        m_free(k);
        m_free(v);
        return;
      }

      m_hash_table_set( client->contexts, k, v );
    }
  }
}

/* remove a context */
void
mondemand_client_remove_context(struct mondemand_client *client,
                                const char *key)
{
  if( client != NULL && key != NULL )
  {
    if( client->contexts != NULL )
    {
      m_hash_table_remove( client->contexts, key );
    }
  }
}

void
mondemand_client_remove_all_contexts(struct mondemand_client *client)
{
  if( client != NULL )
  {
    m_hash_table_remove_all( client->contexts );
  }
}

