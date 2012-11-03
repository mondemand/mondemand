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
  struct mondemand_trace_id trace_id;

  client = mondemand_client_create ("test1234");
  assert ( client != NULL );
  transport = mondemand_transport_lwes_create ("127.0.0.1", 20502, NULL, 0, 60);
  assert (transport != NULL );
  assert (mondemand_add_transport (client, transport) == 0);

  trace_id = mondemand_trace_id (12345);

  assert (mondemand_initialize_trace (client, "owner", "id", "message 1") == 0);
  assert (mondemand_set_trace (client, "foo1", "bar1") == 0);
  assert (mondemand_flush_trace (client) == 0);
  mondemand_clear_trace (client);

  assert (mondemand_initialize_trace (client, "owner", "id", "message 2") == 0);
  assert (mondemand_set_trace (client, "foo2", "bar2") == 0);
  assert (mondemand_flush_trace (client) == 0);

  assert (mondemand_initialize_trace (client, "owner", "id", "message 3") == 0);
  assert (mondemand_set_trace (client, "foo3", "bar3") == 0);
  assert (mondemand_flush_trace (client) == 0);
  mondemand_clear_trace (client);

  mondemand_client_destroy (client);

  return 0;
}
