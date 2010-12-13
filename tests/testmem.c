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
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdlib.h>

/* wrap malloc to cause memory problems */
void *my_malloc(size_t size);
static int malloc_fail = 0;

void *my_malloc(size_t size)
{
  void *ret = NULL;
  if( malloc_fail == 0 )
  {
    ret = malloc(size);
  }
  return ret;
}

#define malloc my_malloc
#include "m_mem.c"
#undef malloc

int
main(void)
{
  void *ptr = NULL;

  /* these should fail/return 0 */
  malloc_fail = 1;
  ptr = m_try_malloc(1024);
  ptr = m_try_malloc0(1024);
  m_free(ptr);

  malloc_fail = 0;
  ptr = (void *) m_try_malloc0(1024);
  ptr = m_try_realloc(ptr, 10255);
  m_free(ptr);

  return 0;
}

