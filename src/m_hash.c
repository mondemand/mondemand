
#include "m_mem.h"
#include "m_hash.h"

#include <stdlib.h>
#include <string.h>

/* it should be rare for people to have hundreds of contexts and
   keys in an event, but then again 640k ought to be enough for anybody */
#define DEFAULT_SIZE 100


/* internal structure for iterating */
struct m_hash_iterator
{
  int index;
  struct m_hash_node *current;
};


/* forward declaration of private functions */
int m_hash_function(const char *key);
void m_hash_free(void *value);
struct m_hash_node *m_hash_table_get_node(struct m_hash_table *hash_table,
                                          const char *key);

/* ======================================================================== */
/* Public API functions                                                     */
/* ======================================================================== */

/* creates the hash table */
struct m_hash_table *
m_hash_table_create(void)
{
  int i = 0;
  struct m_hash_table *hash_table = NULL;

  hash_table = (struct m_hash_table *) 
               m_try_malloc0(sizeof(struct m_hash_table)); 

  if( hash_table != NULL )
  {
    hash_table->size = DEFAULT_SIZE;
    hash_table->num = 0;
    hash_table->nodes = 
      (void **) m_try_malloc0( sizeof(void *) * hash_table->size );
    if( hash_table->nodes != NULL )
    {
      for( i=0; i < hash_table->size; ++i )
      {
        hash_table->nodes[i] = NULL;
      }
    } else {
      /* we couldn't allocate the bins, free and return NULL */
      m_free(hash_table);
      return NULL;
    }
  }

  return hash_table;
}

/* destroys the hash table */
void
m_hash_table_destroy(struct m_hash_table *hash_table)
{
  if( hash_table != NULL )
  {
    m_hash_table_remove_all(hash_table);
    m_free(hash_table->nodes);
    m_free(hash_table);
  }
}

/* gets pointer to a value out of the hash table */
void *
m_hash_table_get(struct m_hash_table *hash_table, const char *key)
{
  struct m_hash_node *node = NULL;

  node = m_hash_table_get_node(hash_table, key);

  if( node == NULL )
  {
    return NULL;
  } else {
    return node->value;
  }
}

/* sets a value, creating or overwriting if necessary */
int
m_hash_table_set(struct m_hash_table *hash_table, char *key, void *value)
{
  int index = 0;
  struct m_hash_node *node = NULL;
  struct m_hash_node *iterator = NULL;
  struct m_hash_node *tmp = NULL;

  if( hash_table != NULL && key != NULL && value != NULL )
  {
    /* see if the node exists */
    tmp = m_hash_table_get_node(hash_table, key);

    if( tmp != NULL ) /* we found an existing object at this key */
    {
      /* free the object and replace it with the new one */
      m_hash_free(tmp->key);
      m_hash_free(tmp->value);
      tmp->key = key;
      tmp->value = value;
      return 0;
    }

    /* allocate the new node */
    node = (struct m_hash_node *) m_try_malloc0(sizeof(struct m_hash_node)); 
    if( node != NULL )
    {
      node->key = key;
      node->value = value;
      node->next = NULL;

      index = m_hash_function(key) % hash_table->size;
      if( hash_table->nodes[index] == NULL )
      {
        /* this bucket is empty so just add it */
        hash_table->nodes[index] = (void *) node;
      } else {
        /* get to the end of the bucket and append it */
        iterator = (struct m_hash_node *) hash_table->nodes[index];
        while( iterator->next != NULL ) 
        {
          iterator = iterator->next;
        }
        iterator->next = node;
      }

      /* increment the count */
      hash_table->num++;
    } else {
      /* malloc failed */
      return -3;
    }
  }

  return 0;
}

void
m_hash_table_remove(struct m_hash_table *hash_table, const char *key)
{
  int index = 0;
  struct m_hash_node *head = NULL;
  struct m_hash_node *iterator = NULL;
  struct m_hash_node *prev = NULL;

  if(hash_table != NULL && key != NULL)
  {
    /* look for the element to remove */
    index = m_hash_function(key) % hash_table->size;
    head = (struct m_hash_node *) hash_table->nodes[index];
    if( head == NULL )
    {
      return;
    }

    iterator = head;
    prev = head;
    while( iterator != NULL && strcmp(iterator->key, key) != 0 )
    {
      prev = iterator;
      iterator = iterator->next;
    }

    /* if we don't find anything just stop */
    if( iterator == NULL )
    {
      return;
    }

    if( iterator == head )
    {
      /* if first element in the list, set the first element to the next */
      hash_table->nodes[index] = (void *) iterator->next;
    } else {
      /* otherwise, remove ourselves by pointing the previous entry at 
         the next */
      prev->next = iterator->next;
    }

    /* remove this entry */
    iterator->next = NULL;
    m_hash_free(iterator->key);
    m_hash_free(iterator->value);
    m_hash_free(iterator);
    
    hash_table->num--;
  }
}

void
m_hash_table_remove_all(struct m_hash_table *hash_table)
{
  int i=0;
  struct m_hash_node *iterator = NULL;
  struct m_hash_node *marked = NULL;

  if( hash_table != NULL )
  {
    if( hash_table->nodes != NULL )
    {
      /* go through all buckets */
      for( i=0; i<hash_table->size; ++i)
      {
        /* go throuh all the entries */
        iterator = hash_table->nodes[i];
        while( iterator != NULL )
        {
          marked = iterator;
          iterator = iterator->next;

          m_hash_free(marked->key);
          m_hash_free(marked->value);
          m_hash_free(marked);
        }

        /* wipe out the reference */
        hash_table->nodes[i] = NULL;
      }

      /* reset the num */
      hash_table->num = 0;
    }
  }
}

/* return an immutable list of keys */
const char **
m_hash_table_keys(struct m_hash_table *hash_table)
{
  int i=0;
  int j=0;
  char **keys = NULL;
  struct m_hash_node *iterator = NULL;

  if( hash_table != NULL )
  {
    keys = (char **) m_try_malloc0( sizeof(char *) * (hash_table->num + 1) ); 

    if( keys != NULL )
    {
      for( i=0; i<hash_table->size; ++i )
      {
        iterator = hash_table->nodes[i];
        while( iterator != NULL )
        {
          keys[j++] = (char *) iterator->key;
          iterator = iterator->next;
        } 
      }
      keys[j] = NULL;

      return (const char **) keys;
    }
  } /* if( hash_table != NULL ) */

  return NULL;
}


/* ======================================================================== */
/* Private API functions                                                     */
/* ======================================================================== */
int
m_hash_function(const char *key)
{
  int sum = 0;
  int i = 0;

  if( key == NULL )
  {
    return 0;
  }

  for( i=0; key[i] != '\0'; ++i )
  {
    sum += ((int) key[i]) * ((int) key[i]);
  }

  return sum;
}

/* this is our free function. we could add a callback later but since
   we're only dealing with basic types, a call to free  */
void
m_hash_free(void *ptr)
{
  /* a simple wrapper to m_mem */
  m_free(ptr);
}

/* get a node from the hash table */
struct m_hash_node *
m_hash_table_get_node(struct m_hash_table *hash_table, const char *key)
{
  int index = 0;
  struct m_hash_node *bucket = NULL;
  struct m_hash_node *iterator = NULL;

  if( hash_table != NULL && key != NULL )
  {
    index = m_hash_function(key) % hash_table->size;
    bucket = hash_table->nodes[index];
    if( bucket == NULL )
    {
      return NULL;
    }

    iterator = bucket;
    while( iterator != NULL && strcmp(iterator->key, key) != 0 )
    {
      iterator = iterator->next;
    }

    if( iterator == NULL )
    {
      return NULL;
    }

    return iterator;
  } else {
    return NULL;
  }
}
