#ifndef __M_HASH_H__
#define __M_HASH_H__

/*! \file m_hash.h
 *  \brief a stripped down hashtable implementation.  based on glib, but
 *         only using the defaults that we need.
 */

/*! \struct m_hash_node
 *  \brief  individual entries in the hash table
 */
struct m_hash_node
{
  /* the key */
  void *key;
  /* the value */
  void *value;
};

/*! \struct m_hash_table
 *  \brief  a hash table data structure
 */
struct m_hash_table
{
  /* size of the table */
  int size;
  /* array of actual entries */
  struct m_hash_node *nodes;
};

/*!\fn struct m_hash_table *m_hash_table_destroy(void)
 * \brief creates a hash table.
 */
struct m_hash_table *m_hash_table_create(void);

/*!\fn void m_hash_table_destroy(struct m_hash_table *hash_table)
 * \brief destroys a hash table.
 */
void m_hash_table_destroy(struct m_hash_table *hash_table);

#endif
