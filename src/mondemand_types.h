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
#ifndef __MONDEMAND_TYPES_H__
#define __MONDEMAND_TYPES_H__

/* statistic value */
typedef long long MondemandStatValue;

/* statistic types */
typedef enum {
  MONDEMAND_UNKNOWN = 0,
  MONDEMAND_GAUGE = 1,
  MONDEMAND_COUNTER = 2
} MondemandStatType;

/* lookup table for strings */
static const char * const MondemandStatTypeString[] = {
  "unknown",
  "gauge",
  "counter"
};

/* statistic operation */
typedef enum {
  MONDEMAND_INC = 0,
  MONDEMAND_DEC = 1,
  MONDEMAND_SET = 2
} MondemandOp;

#endif /* __MONDEMAND_TYPES_H__ */
