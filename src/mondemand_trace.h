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
#ifndef __M_TRACE__
#define __M_TRACE__

/* defines a trace_id structure. should not be accessed directly in case
 * the type changes. */
struct mondemand_trace_id
{
  unsigned long _id;
};
extern const struct mondemand_trace_id MONDEMAND_NULL_TRACE_ID;


/*!\fn mondemand_trace_id
 * \brief creates a trace ID from an unsigned long.
 */
struct mondemand_trace_id mondemand_trace_id(unsigned long id);

/*!\fn mondemand_trace_id_compare
 * \brief compares one trace_id to another.
 */
int mondemand_trace_id_compare(const struct mondemand_trace_id *a,
                               const struct mondemand_trace_id *b);


#endif

