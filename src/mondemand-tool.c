/*======================================================================*
 * Copyright (c) 2009, Anthony Molinaro, All rights reserved.           *
 * Copyright (c) 2009, OpenX Inc. All rights reserved.                  *
 *                                                                      *
 * Licensed under the New BSD License (the "License"); you may not use  *
 * this file except in compliance with the License.  Unless required    *
 * by applicable law or agreed to in writing, software distributed      *
 * under the License is distributed on an "AS IS" BASIS, WITHOUT        *
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.     *
 * See the License for the specific language governing permissions and  *
 * limitations under the License. See accompanying LICENSE file.        *
 *======================================================================*/
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
  "    -T <owner_id>:<trace_id>:<message>"                             "\n"
  "       For trace messages, the owner and trace ids along with a"    "\n"
  "       message, all as strings, no colons in owner or trace ids."   "\n"
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
  "    -c <key>:<value>"                                               "\n"
  "      Add the context <key> with the value <value> to mondemand"    "\n"
  "      log and stats messages.  The key should not contain a ':'."   "\n"
  ""                                                                   "\n"
  "    -c may be specified multiple times"                             "\n"
  ""                                                                   "\n"
  "  Log Options:"                                                     "\n"
  ""                                                                   "\n"
  "    -l <level>:<msg>"                                               "\n"
  "    -l <trace_id>:<level>:<msg>"                                    "\n"
  "       where"                                                       "\n"
  ""                                                                   "\n"
  "       trace_id - optional unsigned integer trace id"               "\n"
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
  "    -s <type>:<name>:<value>"                                       "\n"
  "      Send the value <value> for the statistic named <name>"        "\n"
  "      of the given <type>."                                         "\n"
  "      May be specified multiple times."                             "\n"
  "      If the same name is given, the last value will be the one"    "\n"
  "      sent via mondemand."                                          "\n"
  "      The name should not contain a ':'."                           "\n"
  "      Type should be either 'gauge' or 'counter'"                   "\n"
  ""                                                                   "\n"
  "  Trace Options:"                                                   "\n"
  ""                                                                   "\n"
  "    -t <key>:<value>"                                               "\n"
  "      Add the trace <key> with the value <value> to mondemand"      "\n"
  "      trace messages.  The key should not contain a ':'."           "\n"
  "      All traces are aggregated and sent with the owner and"        "\n"
  "      trace id from the -T option"                                  "\n"
  ""                                                                   "\n"
  "    -t may be specified multiple times"                             "\n"
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

static struct mondemand_transport *
handle_transport_arg (const char *arg)
{
  const char *sep  = ":";
  const char *empty = "";
  char *word;
  const char *words[MAX_WORDS];
  char *buffer;
  char *tofree;
  int count = 0;
  struct mondemand_transport *transport = NULL;
  int i;

  for (i = 0 ; i < MAX_WORDS; i++)
    {
      words[i] = empty;
    }

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

static int
handle_context_arg (const char *arg,
                    struct mondemand_client *client)
{
  const char *sep  = ":";
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

static int
handle_log_arg (const char *arg,
                struct mondemand_client *client,
                int log_count)
{
  const char *sep  = ":";
  char *text_level;
  char *buffer;
  char *tofree;
  int level;
  int ret = -1;
  struct mondemand_trace_id trace_id = MONDEMAND_NULL_TRACE_ID;

  tofree = buffer = strdup (arg);
  if (buffer == NULL)
    {
      return ret;
    }

  text_level = strsep (&buffer, sep);
  level = mondemand_log_level_from_string (text_level);
  if (level < 0)
    {
      /* level didn't parse so assume its the 3 element form and get
       * a trace id
       */
      trace_id = mondemand_trace_id (strtoul (text_level, NULL, 0));
      text_level = strsep (&buffer, sep);
      level = mondemand_log_level_from_string (text_level);
      if (level < 0)
        {
          fprintf (stderr, "ERROR: invalid level '%s'\n", text_level);
          goto END;
        }
    }
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

END:
  free (tofree);
  return ret;
}

static int
handle_stat_arg (const char *arg,
                struct mondemand_client *client,
                int stat_count)
{
  const char *sep  = ":";
  char *text_type;
  char *stat_key;
  char *buffer;
  char *tofree;
  MondemandStatType type;
  int ret = -1;

  tofree = buffer = strdup (arg);
  if (buffer == NULL)
    {
      return ret;
    }

  text_type = strsep (&buffer, sep);
  type = mondemand_stat_type_from_string (text_type);
  if (type == MONDEMAND_UNKNOWN)
    {
      fprintf (stderr, "ERROR: valid types are 'counter' and 'gauge'\n");
    }
  else
    {
      stat_key = strsep (&buffer, sep);
      if (buffer != NULL)
        {
          MondemandStatValue stat_value = atoll (buffer);
          /* need to call the underlying implementation call because
             I need to specify different 'line' numbers so messages
             aren't counted as repeated */
          mondemand_stats_perform_op (client, __FILE__, stat_count,
                                      MONDEMAND_SET,
                                      type,
                                      stat_key,
                                      stat_value);
          ret = 0;
        }
      else
        {
          fprintf (stderr, "WARNING: not sending empty statistic %s\n",
                   stat_key);
        }
    }

  free (tofree);
  return ret;
}

static int
initialize_trace_arg (const char *arg,
                      struct mondemand_client *client)
{
  const char *sep  = ":";
  char *buffer;
  char *tofree;
  char *owner;
  char *id;

  tofree = buffer = strdup (arg);

  if (buffer == NULL)
    {
      return -1;
    }

  owner = strsep (&buffer, sep);
  if (buffer != NULL && strcmp (buffer, "") != 0)
    {
      id = strsep (&buffer, sep);
      if (buffer != NULL && strcmp (buffer, "") != 0)
        {
          assert (mondemand_initialize_trace (client, owner, id, buffer) == 0);
        }
      else
        {
          fprintf (stderr, "ERROR: message is required for trace\n");
        }
    }
  else
    {
      fprintf (stderr, "ERROR: id is required for trace\n");
    }

  free (tofree);
  return 0;
}

static int
handle_trace_arg (const char *arg,
                  struct mondemand_client *client)
{
  const char *sep  = ":";
  char *trace_key;
  char *buffer;
  char *tofree;
  int ret = -1;

  tofree = buffer = strdup (arg);
  if (buffer == NULL)
    {
      return ret;
    }

  trace_key = strsep (&buffer, sep);
  mondemand_set_trace (client, trace_key, buffer);
  ret = 0;

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
  int trace_initialized = 0;
  const char *args = "p:T:o:c:l:s:t:h";
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

          /* deal with these below */
          case 'T':
          case 'o':
          case 'c':
          case 'l':
          case 's':
          case 't':
            break;

          case 'h':
            fprintf (stderr, "%s", help);
            return 1;

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

  /* reset the args list and go through them to get transports and trace meta
   * info
   */
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
          case 'p':
            break;

          case 'T':
            assert (initialize_trace_arg (optarg, client) == 0);
            trace_initialized = 1;
            break;

          case 'o':
            transport = handle_transport_arg (optarg);
            assert (mondemand_add_transport (client, transport) == 0);
            break;

          case 'c':
          case 'l':
          case 's':
          case 't':
          case 'h':
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
          case 'p':
          case 'T':
          case 'o':
            break;

          case 'c':
            handle_context_arg (optarg, client);
            break;

          case 'l':
          case 's':
          case 't':
          case 'h':
            break;

          default:
            fprintf (stderr,
                     "error: unrecognized command line option -%c\n",
                     optopt);
        }
    }

  /* finally get messages, statistics, and traces */
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
          case 'T':
          case 'o':
          case 'c':
            break;

          case 'l':
            if (handle_log_arg (optarg, client, log_count) >= 0)
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

          case 't':
            if (trace_initialized)
              {
                handle_trace_arg (optarg, client);
              }
            else
              {
                fprintf (stderr, "ERROR: can't specify '-t' without '-T'\n"
                                 "       ignoring argument\n");
              }
            break;

          case 'h':
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
