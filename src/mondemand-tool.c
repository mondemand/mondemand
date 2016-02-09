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
  "  Transport Options:"                                               "\n"
  ""                                                                   "\n"
  "    -o lwes:<iface>:<ip>:<port> | lwes:<iface>:<ip>:<port>:<ttl> | stderr""\n"
  "       Specify a place to send messages."                           "\n"
  "         lwes - send message via lwes"                              "\n"
  "           iface - ethernet interface to send via, defaults to"     "\n"
  "                   system configured default interface."            "\n"
  "           ip    - ip address to send datagrams to."                "\n"
  "           port  - port to send datagrams to."                      "\n"
  "           ttl   - ttl for multicast datagrams"                     "\n"
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
  "      -l may be specified multiple times."                          "\n"
  ""                                                                   "\n"
  "  Stats Options:"                                                   "\n"
  ""                                                                   "\n"
  "    -s <type>:<name>:<value>"                                       "\n"
  "      Send the value <value> for the statistic named <name>"        "\n"
  "      of the given <type>."                                         "\n"
  "      The name should not contain a ':'."                           "\n"
  "      Type should be either 'gauge' or 'counter'"                   "\n"
  ""                                                                   "\n"
  "      -s may be specified multiple times."                          "\n"
  "      If the same name is given, the last value will be the one"    "\n"
  "      sent via mondemand."                                          "\n"
  ""                                                                   "\n"
  "  Trace Options:"                                                   "\n"
  ""                                                                   "\n"
  "    -T <owner_id>:<trace_id>:<message>"                             "\n"
  "       For trace messages, the owner and trace ids along with a"    "\n"
  "       message, all as strings, no colons in owner or trace ids."   "\n"
  ""                                                                   "\n"
  "    -t <key>:<value>"                                               "\n"
  "      Add the trace <key> with the value <value> to mondemand"      "\n"
  "      trace messages.  The key should not contain a ':'."           "\n"
  "      All traces are aggregated and sent with the owner and"        "\n"
  "      trace id from the -T option"                                  "\n"
  ""                                                                   "\n"
  "      -t may be specified multiple times"                           "\n"
  ""                                                                   "\n"
  "  Performance Trace Options:"                                       "\n"
  ""                                                                   "\n"
  "    -X <id>:<caller_label>"                                         "\n"
  "       For Performance Trace messages, the id for the performance"  "\n"
  "       trace, as well as a label for the caller."                   "\n"
  ""                                                                   "\n"
  "    -x <label>:<start>:<end>"                                       "\n"
  "       Add a performance trace for the given label.  The start and" "\n"
  "       end should be given in milliseconds since epoch."            "\n"
  ""                                                                   "\n"
  "       -x may be specified multiple times."                         "\n"
  ""                                                                   "\n"
  "  Annotation Options:"                                              "\n"
  ""                                                                   "\n"
  "    -a <id>:<timestamp>:<description>[:<tag1>,<tag2>...]"           "\n"
  "       The <id> should be unique for each annotation."              "\n"
  "       The timestamp should be in UTC seconds since epoch."         "\n"
  "       The <desc> and <tag> values should not contain a ':' and"    "\n"
  "       the <tag> values should not contain a ','."                  "\n"
  ""                                                                   "\n"
  "    -A <text>"                                                      "\n"
  "       For annotations, -A is separate as it may contain various"   "\n"
  "       characters not allowed in -a."                               "\n"
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
#define MAX_WORDS 100


static struct mondemand_transport *
handle_transport_arg (const char *arg)
{
  const char *sep  = ":";
  const char *empty = "";
  char *word;
  const char *words[MAX_WORDS];
  int count = 0;
  char *buffer;
  char *tofree;
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
          if (count < 4 || count > 5)
            {
              fprintf (stderr, "ERROR: lwes transport requires 3 or 4 parts\n");
              fprintf (stderr, "       lwes:<iface>:<ip>:<port>\n");
              fprintf (stderr, "       lwes:<iface>:<ip>:<port>:<ttl>\n");
            }
          else
            {
              const char *iface = NULL;
              const char *ip = NULL;
              int port = -1;
              int ttl = 3;
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
                      if (strcmp (words[4], "") != 0)
                        {
                          ttl = atoi (words[4]);
                          if (ttl < 0 || ttl > 32)
                            {
                              fprintf (stderr, "WARNING: ttl must be between 0 "
                                               "and 32, defaulting to 3\n");
                              ttl = 3;
                            }
                        }
                      transport =
                        mondemand_transport_lwes_create_with_ttl
                          (ip, port, iface, 0, 60, ttl);
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
  const char *empty = "";
  char *word;
  const char *words[MAX_WORDS];
  int count = 0;
  int i;

  char *buffer;
  char *tofree;
  int ret = -1;

  for (i = 0 ; i < MAX_WORDS; i++)
    {
      words[i] = empty;
    }

  tofree = buffer = strdup (arg);
  if (buffer == NULL)
    {
      return ret;
    }

  /* get all the words between the ':'s */
  while ((word = strsep (&buffer, sep)) && count < MAX_WORDS)
    {
      words[count++]=word;
    }

  if (count == 2)
    {
      ret = mondemand_set_context (client, words[0], words[1]);
    }

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
      ret = mondemand_log_real(client, __FILE__, log_count,
                              level, trace_id, "%s", buffer);
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
      ret = 1;
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
          ret = mondemand_stats_perform_op (client, __FILE__, stat_count,
                                            MONDEMAND_SET,
                                            type,
                                            stat_key,
                                            stat_value);
        }
      else
        {
          fprintf (stderr, "WARNING: not sending empty statistic %s\n",
                   stat_key);
          ret = 1;
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
  int ret = -1;

  tofree = buffer = strdup (arg);

  if (buffer == NULL)
    {
      return ret;
    }

  owner = strsep (&buffer, sep);
  if (buffer != NULL && strcmp (buffer, "") != 0)
    {
      id = strsep (&buffer, sep);
      if (buffer != NULL && strcmp (buffer, "") != 0)
        {
          if ((ret = mondemand_initialize_trace (client, owner, id, buffer)) != 0)
            {
              fprintf (stderr, "ERROR: can't initialize trace\n");
            }
        }
      else
        {
          fprintf (stderr, "ERROR: message is required for trace\n");
          ret = 1;
        }
    }
  else
    {
      fprintf (stderr, "ERROR: id is required for trace\n");
      ret = 1;
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
  ret = mondemand_set_trace (client, trace_key, buffer);

  free (tofree);
  return ret;
}

static int
initialize_perf_arg (const char *arg,
                     struct mondemand_client *client)
{
  const char *sep  = ":";
  char *buffer;
  char *tofree;
  char *id;
  char *caller_label;
  int ret = 0;

  tofree = buffer = strdup (arg);

  if (buffer == NULL)
    {
      return -1;
    }

  id = strsep (&buffer, sep);
  if (buffer != NULL && strcmp (buffer, "") != 0)
    {
      caller_label = strsep (&buffer, sep);
      assert (mondemand_initialize_performance_trace
                (client, id, caller_label) == 0);
    }
  else
    {
      fprintf (stderr,
               "ERROR: id and caller_label are required for perf events\n");
      ret = 1;
    }

  free (tofree);
  return ret;
}

static int
handle_perf_arg (const char *arg,
                 struct mondemand_client *client)
{
  const char *sep  = ":";
  const char *empty = "";
  char *word;
  const char *words[MAX_WORDS];
  int count = 0;

  const char *label;
  long long int start;
  long long int end;
  char *buffer;
  char *tofree;
  char *tofree2=NULL;
  int ret = -1;
  int i;

  for (i = 0 ; i < MAX_WORDS; i++)
    {
      words[i] = empty;
    }

  tofree = buffer = strdup (arg);
  if (buffer == NULL)
    {
      return ret;
    }

  /* get all the words between the ':'s */
  while ((word = strsep (&buffer, sep)) && count < MAX_WORDS)
    {
      words[count++]=word;
    }

  /* support labels with ':' in them, by using the last 2 as the start
   * and end, and putting the rest back together
   */
  if (count > 3)
    {
      int j;
      tofree2 = strdup(arg);
      tofree2[0] = '\0';
      for (j = 0; j < count - 2; j++)
        {
          strcat (tofree2,words[j]);
          if (j != count -3)
            {
              strcat (tofree2,":");
            }
        }
      label = tofree2;
      start = atoll(words[count-2]);
      end = atoll(words[count-1]);
      ret = 0;
    }
  else if (count == 3)
    {
      label = words[0];
      start = atoll(words[1]);
      end = atoll(words[2]);
      ret = 0;
    }
  else
    {
      fprintf (stderr, "ERROR: -x requires 3 parts\n");
    }
  if (ret == 0)
    {
      ret = mondemand_add_performance_trace_timing (client, label, start, end);
    }
  if (tofree2 != NULL)
    {
      free(tofree2);
    }
  free (tofree);
  return ret;
}

static int
handle_annotation_arg (const char *arg,
                       const char *annotation_text,
                       struct mondemand_client *client)
{
  const char *sep  = ":";
  const char *empty = "";
  char *word;
  const char *words[MAX_WORDS];
  char *buffer;
  char *tofree;
  int count = 0;
  int i;
  int ret = 0;

  for (i = 0 ; i < MAX_WORDS; i++)
    {
      words[i] = empty;
    }

  tofree = buffer = strdup (arg);
  if (buffer == NULL)
    {
      return -1;
    }

  /* get all the words between the ':'s */
  while ((word = strsep (&buffer, sep)) && count < MAX_WORDS)
    {
      words[count++]=word;
    }

  if (count < 3 || count > 4)
    {
      fprintf (stderr, "ERROR: annotation (-a) arg requires 3 or 4 parts\n");
      fprintf (stderr, "       <id>:<timestamp>:<description>[:<tag1>,<tag2>...]\n");
      ret = 1;
    }
  else
    {
      const char *id = words[0];
      const long long int timestamp = atoll (words[1]);
      const char *description = words[2];
      if (count > 3)
        {
          char *tag;
          const char *tags[MAX_WORDS];
          char *tagbuffer;
          char *tagtofree;
          int j;
          int tagcount = 0;

          for (j = 0 ; j < MAX_WORDS; j++)
            {
              tags[j] = empty;
            }
          tagtofree = tagbuffer = strdup (words[3]);
          if (tagbuffer == NULL)
            {
              return -1;
            }
          /* get all the words between the ','s */
          while ((tag= strsep (&tagbuffer, ",")) && tagcount < MAX_WORDS)
            {
              tags[tagcount++]=tag;
            }

          /* deal with tags */
          if ((ret = mondemand_flush_annotation (id,
                                                 timestamp,
                                                 description,
                                                 annotation_text,
                                                 tags,
                                                 tagcount,
                                                 client)) != 0)
            {
              fprintf (stderr, "ERROR: unable to send annotation %d\n",ret);
            }
          free (tagtofree);
        }
      else
        {
          if ((ret = mondemand_flush_annotation (id,
                                                 timestamp,
                                                 description,
                                                 annotation_text,
                                                 NULL,
                                                 0,
                                                 client)) != 0)
            {
              fprintf (stderr, "ERROR: unable to send annotation %d\n",ret);
            }
        }
    }
  free (tofree);

  return ret;
}

int main (int   argc,
          char *argv[])
{
  const char *prog_id = "mondemand-tool";
  const char *args = "p:T:o:c:l:s:t:X:x:A:a:h";

  struct mondemand_client *client = NULL;
  struct mondemand_transport *transport = NULL;

  int stat_count = 0;
  int stat_error = 0;
  int log_count = 0;
  int trace_initialized = 0;
  int transport_count = 0;
  int perf_count = 0;

  int performance_trace_initialized = 0;

  unsigned short annotation_count = 0;
  unsigned short annotation_text_count = 0;
  char *annotation_text = NULL;

  int ret = 0;

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

          case 'a':
            /* capture a count so we can error if there are too many */
            annotation_count++;
            break;

          case 'A':
            annotation_text = optarg;
            /* capture a count so we can error if there are too many */
            annotation_text_count++;
            break;

          /* deal with these below */
          case 'T':
          case 'o':
          case 'c':
          case 'l':
          case 's':
          case 't':
          case 'X':
          case 'x':
            break;

          case 'h':
            fprintf (stderr, "%s", help);
            return 1;

          default:
            fprintf (stderr,
                     "WARNING: unrecognized command line option -%c\n",
                     optopt);
        }
    }
  if (annotation_count > 1)
    {
      fprintf (stderr,
               "ERROR: can't specify '-a' more than once\n");
      exit (1);
    }
  if (annotation_text_count > 1)
    {
      fprintf (stderr,
               "ERROR: can't specify '-A' more than once\n");
      exit (1);
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

          case 'X':
            if ((ret = initialize_perf_arg (optarg, client)) != 0)
              {
                fprintf (stderr, "ERROR: error initializing perf trace\n");
                goto cleanup;
              }
            performance_trace_initialized = 1;
            break;

          case 'T':
            if ((ret = initialize_trace_arg (optarg, client)) != 0)
              {
                fprintf (stderr, "ERROR: error initializing trace\n");
                goto cleanup;
              }
            trace_initialized = 1;
            break;

          case 'o':
            transport = handle_transport_arg (optarg);
            if ((ret = mondemand_add_transport (client, transport)) != 0)
              {
                fprintf (stderr, "WARNING: unable to add transport %s\n", optarg);
              }
            else
              {
                transport_count++;
              }
            break;

          case 'c':
          case 'l':
          case 's':
          case 't':
          case 'h':
          case 'x':
          case 'A':
          case 'a':
            break;

          default:
            break;
        }
    }
  if (transport_count == 0)
    {
      fprintf (stderr, "ERROR: must specify at least one transport with '-o'\n");
      ret = 1;
      goto cleanup;
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
            if (handle_context_arg (optarg, client) < 0)
              {
                fprintf (stderr,
                         "WARNING: parsing of context %s failed\n", optarg);
              }
            break;

          case 'l':
          case 's':
          case 't':
          case 'h':
          case 'X':
          case 'x':
          case 'A':
          case 'a':
            break;

          default:
            break;
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
            if (handle_log_arg (optarg, client, log_count) == 0)
              {
                log_count++;
              }
            break;

          case 's':
            if (handle_stat_arg (optarg, client, stat_count) == 0)
              {
                stat_count++;
              }
            else
              {
                stat_error++;
              }
            break;

          case 't':
            if (trace_initialized)
              {
                ret = handle_trace_arg (optarg, client);
              }
            else
              {
                fprintf (stderr, "ERROR: can't specify '-t' without '-T'\n"
                                 "       ignoring argument\n");
                ret = 1;
              }
            break;

          case 'X':
            break;

          case 'x':
            if (performance_trace_initialized)
              {
                if ((ret = handle_perf_arg (optarg, client)) != 0)
                  {
                    fprintf (stderr, "ERROR: issue with '-x' arg %s\n",optarg);
                    goto cleanup;
                  }
                perf_count++;
              }
            else
              {
                fprintf (stderr, "ERROR: can't specify '-x' without '-X'\n"
                                 "       ignoring argument\n");
                ret = 1;
              }
            break;

          case 'A':
            break;

          case 'a':
            ret = handle_annotation_arg (optarg, annotation_text, client);
            break;

          case 'h':
            break;

          default:
            break;
        }
    }
  if (performance_trace_initialized && perf_count == 0)
    {
      fprintf (stderr, "ERROR: perf trace started with '-X' but not sent as"
                       " no '-x' was given\n");
      ret = 1;
    }
  if (stat_error > 0)
    {
      fprintf (stderr, "ERROR: had issues sending some stats\n");
      ret = 1;
    }

cleanup:
  mondemand_client_destroy (client);

  return ret;
}
