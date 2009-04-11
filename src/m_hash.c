
#include "m_mem.h"
#include "m_hash.h"

#include <stdlib.h>

struct m_hash_table *
m_hash_table_create(void)
{
  struct m_hash_table *hash_table;

  hash_table = (struct m_hash_table *) 
               m_try_malloc0(sizeof(struct m_hash_table)); 

  if( hash_table != NULL )
  {
    hash_table->size = 0;
    hash_table->nodes = NULL;
  }

  return hash_table;
}

void
m_hash_table_destroy(struct m_hash_table *hash_table)
{
  if( hash_table != NULL )
  {
    m_free(hash_table);
  }
}

