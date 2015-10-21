/*======================================================================*
 * Copyright (c) 2008, Yahoo! Inc. All rights reserved.                 *
 *                                                                      *
 * Licensed under the New BSD License (the "License"); you may not use  *
 * this file except in compliance with the License.  Unless required    *
 * by applicable law or agreed to in writing, software distributed      *
 * under the License is distributed on an "AS IS" BASIS, WITHOUT        *
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.     *
 * See the License for the specific language governing permissions and  *
 * limitations under the License. See accompanying LICENSE file.        *
 *======================================================================*/
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

/*!\fn int m_hash_table_destroy(struct m_hash_table *hash_table)
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
 * \return 0 on success, non-zero on failure
 */
int m_hash_table_set(struct m_hash_table *hash_table, char *key, void *value);

/*!\fn void m_hash_table_remove(struct m_hash_table *hash_table, 
                                const char *key)
 * \brief removes a key/value pair in the hash table.
 */
void m_hash_table_remove(struct m_hash_table *hash_table, const char *key);

/*!\fn void m_hash_table_remove_all(struct m_hash_table *hash_table)
 * \brief removes all the elements.
 */
void m_hash_table_remove_all(struct m_hash_table *hash_table);

/*!\fn char **m_hash_table_keys(struct m_hash_table *hash_table)
 * \brief returns an array of char * elements representing the keys in the
 *        hash table.  The last element is NULL to mark the end of the
 *        array.  The list must be freed after usage to prevent memory
 *        leaks.
 */
const char **
m_hash_table_keys(struct m_hash_table *hash_table);

/*!\fn int m_hash_num (struct m_hash_table *hash_table)
 * \brief return the number of entries currently in the table
 */
int
m_hash_table_num (struct m_hash_table *hash_table);

#endif
