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
#include "m_mem.h"

#include <stdlib.h>
#include <string.h>

/* ======================================================================== */
/* Public API functions                                                     */
/* ======================================================================== */

void *
m_try_malloc(size_t size)
{
  return malloc(size); 
}

void *
m_try_malloc0(size_t size)
{
  void *ptr;

  ptr = m_try_malloc(size);

  if(ptr)
  {
    memset(ptr, 0, size);
  }

  return ptr;
}

void *m_try_realloc(void *ptr, size_t size)
{
  return realloc(ptr, size);
}


void
m_free(void *ptr)
{
  free(ptr);
}

