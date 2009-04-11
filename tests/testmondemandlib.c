#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdlib.h>

#include "mondemandlib.c"

int
main(void)
{
  MonDemandClient *client = NULL;

  client = mondemand_client_create("test1234");
  mondemand_client_set_immediate_send_level(client, M_LOG_ALERT);
  mondemand_client_set_immediate_send_level(client, -1);
  mondemand_client_set_immediate_send_level(client, 100000);

  mondemand_client_destroy(client);

  return 0;
}

