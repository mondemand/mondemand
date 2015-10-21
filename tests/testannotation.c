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
  const char *tags[] = { "tag1", "tag2" };
  int tag_count = sizeof (tags) / sizeof (const char *);


  client = mondemand_client_create ("test1234");
  assert ( client != NULL );
  transport = mondemand_transport_lwes_create ("127.0.0.1", 20502, NULL, 0, 60);
  assert (transport != NULL );
  assert (mondemand_add_transport (client, transport) == 0);

  /* test some error cases */

  /* no id */
  assert (mondemand_flush_annotation (NULL,
                                      1234567,
                                      "something happened",
                                      NULL,
                                      NULL,
                                      0,
                                      client) == -2);

  /* bad timestamp */
  assert (mondemand_flush_annotation ("foo",
                                      -1,
                                      "something happened",
                                      NULL,
                                      NULL,
                                      0,
                                      client) == -2);

  /* no description */
  assert (mondemand_flush_annotation ("foo",
                                      1234567,
                                      NULL,
                                      NULL,
                                      NULL,
                                      0,
                                      client) == -2);


  /* test with no text, no tags*/
  assert (mondemand_flush_annotation ("foo",
                                      1234567,
                                      "something happened",
                                      NULL,
                                      NULL,
                                      0,
                                      client) == 0);

  /* test with text, no tags */
  assert (mondemand_flush_annotation ("foo",
                                      1234567,
                                      "something happened",
                                      "with details here",
                                      NULL,
                                      0,
                                      client) == 0);

  /* test with no text, tags */
  assert (mondemand_flush_annotation ("foo",
                                      1234567,
                                      "something happened",
                                      NULL,
                                      tags,
                                      tag_count,
                                      client) == 0);

  /* test with text, tags */
  assert (mondemand_flush_annotation ("foo",
                                      1234567,
                                      "something happened",
                                      "with details here",
                                      tags,
                                      tag_count,
                                      client) == 0);

  /* test with text, tags */
  assert (mondemand_flush_annotation ("foo",
                                      1234567,
                                      "something happened",
                                      "with details here",
                                      tags,
                                      1,
                                      client) == 0);


  mondemand_client_destroy (client);

  return 0;
}
