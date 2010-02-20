/*======================================================================*
 * Copyright (C) 2010 Mondemand                                         *
 * All rights reserved.                                                 *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful,      *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 * GNU General Public License for more details.                         *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program; if not, write to the Free Software          *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,                   *
 * Boston, MA 02110-1301 USA.                                           *
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

