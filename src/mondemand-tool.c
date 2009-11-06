/*======================================================================*
 * Copyright (C) 2009 Mondemand                                         *
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
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mondemand_transport.h"
#include "mondemandlib.h"

static const char help[] =
  "mondemand-tool [options]"                                           "\n"
  ""                                                                   "\n"
  "  where options are:"                                               "\n"
  ""                                                                   "\n"
  "  General Options:"                                                 "\n"
  ""                                                                   "\n"
  "    -p <program identifier>"                                        "\n"
  "       Specify a string as a <program identifier>."                 "\n"
  ""                                                                   "\n"
  "    -t <trace_id>"                                                  "\n"
  "       Specify an unsigned integer trace id."                       "\n"
  ""                                                                   "\n"
  "  Transport Options:"                                               "\n"
  ""                                                                   "\n"
  "    -o lwes:<iface>:<ip>:<port> | stderr"                           "\n"
  "       Specify a place to send messages."                           "\n"
  "         lwes - send message via lwes"                              "\n"
  "           iface - ethernet interface to send via, defaults to"     "\n"
  "                   system configured default interface."            "\n"
  "           ip    - ip address to send datagrams to."                "\n"
  "           port  - port to send datagrams to."                      "\n"
  ""                                                                   "\n"
  "           if ip is a multicast ip, then datagrams are sent via"    "\n"
  "           multicast, otherwise they are sent via UDP."             "\n"
  ""                                                                   "\n"
  "         stderr - send messages to stderr"                          "\n"
  ""                                                                   "\n"
  "    -o may be specified multiple times"                             "\n"
  ""                                                                   "\n"
  "  Context Options:"                                                 "\n"
  ""                                                                   "\n"
  "    -c <key>=<value>"                                               "\n"
  "      Add the context <key> with the value <value> to all mondemand""\n"
  "      messages.  The key should not contain a '='."                 "\n"
  ""                                                                   "\n"
  "    -c may be specified multiple times"                             "\n"
  ""                                                                   "\n"
  "  Log Options:"                                                     "\n"
  ""                                                                   "\n"
  "    -l <level>:<msg>"                                               "\n"
  "       where"                                                       "\n"
  ""                                                                   "\n"
  "       level - the log level as one of the following strings"       "\n"
  "                 emerg    alert    crit"                            "\n"
  "                 error    warning  notice"                          "\n"
  "                 info     debug"                                    "\n"
  "       msg   - the message to send"                                 "\n"
  ""                                                                   "\n"
  "    -l man be specified multiple times."                            "\n"
  ""                                                                   "\n"
  "  Stats Options:"                                                   "\n"
  ""                                                                   "\n"
  "    -s <name>=<value>"                                              "\n"
  "      Send the value <value> for the statistic named <name>."       "\n"
  "      May be specified multiple times."                             "\n"
  "      If the same name is given, the last value will be the one"    "\n"
  "      sent via mondemand."                                          "\n"
  "      The name should not contain a '='."                           "\n"
  ""                                                                   "\n"
  "  Other Options:"                                                   "\n"
  ""                                                                   "\n"
  "    -h"                                                             "\n"
  "       show this message"                                           "\n"
  ""                                                                   "\n"
  "  arguments are specified as -option value or -optionvalue"         "\n"
  ""                                                                   "\n";

/* given a transport argument, return a transport object or NULL if there
 * is an error
 */
#define MAX_WORDS 10

struct mondemand_transport *
handle_transport_arg (const char *arg)
{
  const char *sep  = ":";
  char *word;
  char *words[MAX_WORDS];
  char *buffer;
  char *tofree;
  int count = 0;
  struct mondemand_transport *transport = NULL;

  tofree = buffer = strdup (arg);
  if (buffer == NULL)
    {
      return NULL;
    }

  /* get all the words between the ':'s */
  while ((word = strsep (&buffer, sep)) && count < MAX_WORDS)
    {
      words[count++]=word;
    }

  /* now figure out what to do with them words */
  if (strcmp (words[0], "stderr") == 0)
    {
      transport = mondemand_transport_stderr_create ();
    }
  else
    {
      if (strcmp (words[0], "lwes") == 0)
        {
          if (count != 4)
            {
              fprintf (stderr, "ERROR: lwes transport requires 3 parts\n");
              fprintf (stderr, "       lwes:<iface>:<ip>:<port>\n");
            }
          else
            {
              const char *iface = NULL;
              const char *ip = NULL;
              int port = -1;
              if (strcmp (words[1], "") != 0)
                {
                  iface = words[1];
                }
              if (strcmp (words[2], "") != 0)
                {
                  ip = words[2];
                  if (strcmp (words[3], "") != 0)
                    {
                      port = atoi (words[3]);
                      transport =
                        mondemand_transport_lwes_create (ip, port, iface,
                                                         0, 60);
                    }
                  else
                    {
                      fprintf (stderr,
                               "ERROR: lwes transport requires "
                               "non-empty port\n");
                    }
                }
              else
                {
                  fprintf (stderr,
                           "ERROR: lwes transport requires non-empty ip\n");
                }
            }
        }
      else
        {
          fprintf (stderr, "ERROR: unrecognized transport %s\n",words[0]);
        }
    }

  free (tofree);

  return transport;
}

int
handle_context_arg (const char *arg,
                    struct mondemand_client *client)
{
  const char *sep  = "=";
  char *ctxt_key;
  char *buffer;
  char *tofree;
  int ret = -1;

  tofree = buffer = strdup (arg);
  if (buffer == NULL)
    {
      return ret;
    }

  ctxt_key = strsep (&buffer, sep);
  mondemand_set_context (client, ctxt_key, buffer);
  ret = 0;

  free (tofree);
  return ret;
}

int
handle_log_arg (const char *arg,
                struct mondemand_client *client,
                struct mondemand_trace_id trace_id,
                int log_count)
{
  const char *sep  = ":";
  char *text_level;
  char *buffer;
  char *tofree;
  int level;
  int ret = -1;

  tofree = buffer = strdup (arg);
  if (buffer == NULL)
    {
      return ret;
    }

  text_level = strsep (&buffer, sep);
  level = mondemand_log_level_from_string (text_level);
  if (level < 0)
    {
      fprintf (stderr, "ERROR: invalid level '%s'\n", text_level);
    }
  else
    {
      if (buffer != NULL && strcmp (buffer, "") != 0)
        {
          /* need to call the underlying implementation call because
             I need to specify different 'line' numbers so messages
             aren't counted as repeated */
          mondemand_log_real(client, __FILE__, log_count,
                             level, trace_id, "%s", buffer);

          ret = 0;
        }
      else
        {
          fprintf (stderr, "WARNING: not sending empty log message\n");
        }
    }

  free (tofree);
  return ret;
}

int
handle_stat_arg (const char *arg,
                struct mondemand_client *client,
                int stat_count)
{
  const char *sep  = "=";
  char *stat_key;
  char *buffer;
  char *tofree;
  int ret = -1;

  tofree = buffer = strdup (arg);
  if (buffer == NULL)
    {
      return ret;
    }

  stat_key = strsep (&buffer, sep);
  if (buffer != NULL)
    {
      int stat_value = atoi (buffer);
      /* need to call the underlying implementation call because
         I need to specify different 'line' numbers so messages
         aren't counted as repeated */
      mondemand_stats_set (client, __FILE__, stat_count,
                           stat_key, stat_value);

      ret = 0;
    }
  else
    {
      fprintf (stderr, "WARNING: not sending empty statistic %s\n",stat_key);
    }

  free (tofree);
  return ret;

}

int main (int   argc,
          char *argv[])
{
  int stat_count = 0;
  int log_count = 0;
  struct mondemand_client *client = NULL;
  struct mondemand_transport *transport = NULL;
  struct mondemand_trace_id trace_id = MONDEMAND_NULL_TRACE_ID;
  const char *args = "p:t:o:l:c:s:h";
  const char *prog_id = "mondemand-tool";

  /* turn off error messages, I'll handle them */
  opterr = 0;

  /* get the program id */
  while (1)
    {
      char c = getopt (argc, argv, args);

      if (c == -1)
        {
          break;
        }
      switch (c)
        {
          case 'p':
            prog_id = optarg;
            break;

          case 'h':
            fprintf (stderr, "%s", help);
            return 1;

          case 't':
            trace_id = mondemand_trace_id (strtoul (optarg, NULL, 0));
            break;

          /* deal with these below */
          case 'o':
          case 'c':
          case 'l':
          case 's':
            break;

          default:
            fprintf (stderr,
                     "error: unrecognized command line option -%c\n",
                     optopt);
        }
    }

  /* create the client */
  client = mondemand_client_create (prog_id);
  assert (client != NULL);

  /* have it keep all the messages */
  mondemand_set_no_send_level (client, M_LOG_ALL);
  mondemand_set_immediate_send_level (client, M_LOG_EMERG);

  /* reset the args list and go through them to get transports */
  optind = 1;
  while (1)
    {
      char c = getopt (argc, argv, args);

      if (c == -1)
        {
          break;
        }

      switch (c)
        {
          /* taken care of above */
          case 'p':
          case 'h':
          case 't':
          case 'c':
          case 'l':
          case 's':
            break;

          case 'o':
            transport = handle_transport_arg (optarg);
            assert (mondemand_add_transport (client, transport) == 0);
            break;

          default:
            fprintf (stderr,
                     "error: unrecognized command line option -%c\n",
                     optopt);
        }
    }

  /* reset again and get contexts */
  optind = 1;
  while (1)
    {
      char c = getopt (argc, argv, args);

      if (c == -1)
        {
          break;
        }

      switch (c)
        {
          /* taken care of above */
          case 'p':
          case 'h':
          case 't':
          case 'o':
          case 'l':
          case 's':
            break;

          case 'c':
            handle_context_arg (optarg, client);
            break;

          default:
            fprintf (stderr,
                     "error: unrecognized command line option -%c\n",
                     optopt);
        }
    }

  /* finally get messages and statistics */
  optind = 1;
  while (1)
    {
      char c = getopt (argc, argv, args);

      if (c == -1)
        {
          break;
        }

      switch (c)
        {
          /* taken care of above */
          case 'p':
          case 'h':
          case 't':
          case 'o':
          case 'c':
            break;

          case 'l':
            if (handle_log_arg (optarg, client, trace_id, log_count) >= 0)
              {
                log_count++;
              }
            break;

          case 's':
            if (handle_stat_arg (optarg, client, stat_count) >= 0)
              {
                stat_count++;
              }
            break;

          default:
            fprintf (stderr,
                     "error: unrecognized command line option -%c\n",
                     optopt);
        }
    }
  mondemand_client_destroy (client);

  return 0;
}
