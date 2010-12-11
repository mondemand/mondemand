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
#include <stdlib.h>
#include "mondemand_trace.h"

/* define a NULL value */
const struct mondemand_trace_id MONDEMAND_NULL_TRACE_ID = { 0 };


/* generate a trace id */
struct mondemand_trace_id
mondemand_trace_id(unsigned long id)
{
  struct mondemand_trace_id trace_id;
  trace_id._id = id;
  return trace_id;
}

/* compare trace ids */
int
mondemand_trace_id_compare(const struct mondemand_trace_id *a,
                           const struct mondemand_trace_id *b)
{
  if( a == NULL ) return -1;
  if( b == NULL ) return 1;

  if( a ->_id < b->_id ) {
    return -1;
  } else if(a ->_id > b->_id ) {
    return 1;
  }

  return 0;
}

