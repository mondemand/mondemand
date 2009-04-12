#ifndef __M_HASH_H__
#define __M_HASH_H__

/*! \file m_hash.h
 *  \brief a stripped down hashtable implementation.  based on glib, but
 *         only using the defaults that we need, e.g. we only need basic types.
 */

/*! \struct m_hash_node
 *  \brief  individual entries in the hash table
 */
struct m_hash_node
{
  /* the key */
  char *key;
  /* the value */
  void *value;
  /* next element if this bin has more than one element */
  struct m_hash_node *next;
};

/*! \struct m_hash_table
 *  \brief  a hash table data structure
 */
struct m_hash_table
{
  /* size of table */
  int size;
  /* number of actual entries */
  int num;
  /* array of actual entries */
  void **nodes;
};

/*!\fn struct m_hash_table *m_hash_table_destroy(void)
 * \brief creates a hash table.
 */
struct m_hash_table *m_hash_table_create(void);

/*!\fn void m_hash_table_destroy(struct m_hash_table *hash_table)
 * \brief destroys a hash table.
 */
void m_hash_table_destroy(struct m_hash_table *hash_table);

/*!\fn void *m_hash_table_get(struct m_hash_table *hash_table,
 *                            const char *key)
 * \brief gets a value in the hash table.
 */
void *m_hash_table_get(struct m_hash_table *hash_table, const char *key);

/*!\fn void m_hash_table_set(struct m_hash_table *hash_table,
 *                           char *key, void *value)
 * \brief sets a value in the hash table, creating or overwriting if necessary.
 */
void m_hash_table_set(struct m_hash_table *hash_table, char *key, void *value);

/*!\fn void m_hash_table_remove(struct m_hash_table *hash_table, 
                                const char *key)
 * \brief removes a key/value pair in the hash table.
 */
void m_hash_table_remove(struct m_hash_table *hash_table, const char *key);

/*!\fn void m_hash_table_remove_all(struct m_hash_table *hash_table)
 * \brief removes all the elements.
 */
void m_hash_table_remove_all(struct m_hash_table *hash_table);

#endif
