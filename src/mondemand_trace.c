
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

