#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdlib.h>

#include "mondemandlib.c"

int
main(void)
{
  struct mondemand_client *client = NULL;

  client = mondemand_client_create(NULL);
  assert( client == NULL );

  client = mondemand_client_create("test1234");
  assert( client != NULL );

  mondemand_client_set_immediate_send_level(client, M_LOG_ALERT);
  mondemand_client_set_immediate_send_level(client, -1);
  mondemand_client_set_immediate_send_level(client, 100000);
  assert( client->immediate_send_level == M_LOG_ALERT );

  mondemand_client_set_no_send_level(client, M_LOG_NOTICE);
  mondemand_client_set_no_send_level(client, -100);
  mondemand_client_set_no_send_level(client, 56494949);
  assert( client->no_send_level == M_LOG_NOTICE );

  mondemand_client_destroy(client);

  return 0;
}

