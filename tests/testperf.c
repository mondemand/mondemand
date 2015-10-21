#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mondemandlib.h"

int
main(void)
{
  struct mondemand_client *client = NULL;
  struct mondemand_transport *transport = NULL;

  client = mondemand_client_create ("test1234");
  assert ( client != NULL );
  transport = mondemand_transport_lwes_create ("127.0.0.1", 20502, NULL, 0, 60);
  assert (transport != NULL );
  assert (mondemand_add_transport (client, transport) == 0);

  assert (mondemand_initialize_performance_trace
           (client, "id", "caller_label") == 0);

  assert (mondemand_add_performance_trace_timing (client, "step1",
                                                  1445285994, 1445285996) == 0);
  assert (mondemand_add_performance_trace_timing (client, "step2",
                                                  1445285997, 1445286011) == 0);

  mondemand_flush_performance_trace (client);
  mondemand_clear_performance_trace (client);
  mondemand_client_destroy (client);

  return 0;
}
