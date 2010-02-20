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
#ifndef __M_MEM_H__
#define __M_MEM_H__

/*! \file m_mem.h
 *  \brief Memory management routines.  Some prototypes are copied from glib,
 *         but we don't link to glib in case a user doesn't want to have to
 *         link to glib in their program.
 */

#include <stdlib.h>

/*! \fn m_try_malloc(size_t size)
 *  \brief  Attempd to allocate size bytes, and return NULL on failure.  Does
 *          not initialize the memory.
 *  \param  size number of bytes to allocate
 *  \return the allocated memory, or NULL
 */
void *m_try_malloc(size_t size);

/*! \fn m_try_malloc0(size_t size)
 *  \brief  Attempt to allocate size bytes, initialized to 0's, and return
 *          NULL on failure.
 *  \param  size number of bytes to allocate
 *  \return the allocated memory, or NULL
 */
void *m_try_malloc0(size_t size);

/*! \fn m_try_realloc(void *ptr, size_t size)
 *  \brief reallocate memory, the new memory is uninitialized.
 */
void *m_try_realloc(void *ptr, size_t size);

/*! \fn m_free(void *ptr)
 *  \brief Free the memory referenced by ptr.  If ptr is NULL, no operation
 *         is performed.
 *  \param ptr pointer to memory
 */
void m_free(void *ptr);

#endif

