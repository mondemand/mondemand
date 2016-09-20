[![Build Status](https://travis-ci.org/mondemand/mondemand.svg?branch=master)](https://travis-ci.org/mondemand/mondemand)

# MonDemand : a library for sending logs/stats/annotations and traces to the network

## Introduction
The MonDemand library is a library for logging messages of the following
types to the network.

  * Log Messages - text with a log level
  * Stats Messages - counters and gauges
  * Trace Messages - extra debug information with embedded json
  * Performance Trace Messages - spans of time
  * Annotation Messages - for marking events

By default it uses the
[Light Weight Event System](http://www.lwes.org); however,
developers can modify MonDemand to use the mechanism and protocol of their
choice.  This data stream can be examined by developers and administrators
to determine the health and runtime status of systems using the library.

## Initialization
The first step to using MonDemand is to initialize a client object:
```C
  client = mondemand_client_create (...)
```
This should be done at program initialization.  This method only requires
a program ID, which is a string that the user can use in order to
uniquely identify the calling application from other applications that
might be using MonDemand as well.

## Contextual Metadata
Sometimes a single string (the program identifier) is not enough to identify
details about a running system.  For example, a system may be running in
multiple data centers, on multiple networks, or may have multiple clusters or
partitions.  MonDemand supports contextual metadata that can be set at 
runtime in order to identify this data.
```C
  mondemand_set_context(client, key, value)
```
This method allows you set arbitrary key/value pairs at runtime.  For
example, one might set
```C
  mondemand_set_context(client, "cluster", 5)
  mondemand_set_context(client, "partition", 11)
```
which would mean that any events coming from this running system should
include these identifiers.  This is useful for segmenting data by arbitrary
dimensions defined by the user.

Contexts can be added and removed as needed at runtime using various methods
```C
  mondemand_set_context(...)
  mondemand_remove_context(...)
  mondemand_remove_all_contexts(...)
```
and can be fetched using other methods
```C
  mondemand_get_context(...)
  mondemand_get_context_keys(...)
```
The mondemand_get_context_keys method returns a NULL terminated array of
const char * pointers that point to the current context keys.  This array
must be freed by the caller to prevent memory leaks.

## Transports
Transports are methods that are called whenever MonDemand needs to log
its results to the local system or to the network.  For example, one could
enable the 'stderr' transport that would log data to standard error.  One
could also enable the 'lwes' transport that will emit events for log and
statistics messages.

In order to add a transport, one must be created and then attached to the
MonDemand client.
```C
  mondemand_transport_lwes_create(...)
```
will create a transport structure, which can then be attached to the client:
```C
  mondemand_add_transport(client, transport)
```
Once this is done, going forward, all calls to flush logs and statistics
will call this transport to send messages.  As many transports can be added 
as necessary, although obviously this can have a performance impact.

Transports are pluggable.  A user can build their own transport by
implementing the callback methods defined in mondemand_transport.h.  Those
callbacks are invoked by the MonDemand library at runtime.

NOTE: transports must be destroyed separately from the MonDemand objects
themselves.  See the "Shutting Down" section below.

## Logging Messages
Whenever one needs to log a message, one can use one of the available
convenience macros:
```C
  mondemand_emerg(client, trace_id, ...)
  mondemand_alert(client, trace_id, ...)
  mondemand_crit(client, trace_id, ...)
  mondemand_error(client, trace_id, ...)
  mondemand_warning(client, trace_id, ...)
  mondemand_notice(client, trace_id, ...)
  mondemand_info(client, trace_id, ...)
  mondemand_debug(client, trace_id, ...)  
```
Depending on the immediate_send_level and no_send_level settings in the
client, data will be sent according to the following logic:

Log levels start at M_LOG_EMERG (0) and get increasingly large in value, so
messages which are considered greater than or equal to the no_send_level
will not be pushed into events.  This is useful for disabling debug or info
messages.  Messages which are less than or equal to the immediate_send_level
will be sent immediately upon logging.  Any messages in between will be
bundled up into sets of 10 messages, with repeating messages not counting
toward the total.

The log levels match numerically with syslog(2), but this is not guaranteed
to be preserved.

So if a client is configured with the immediate_send_level = M_LOG_CRIT,
and no_send_level = M_LOG_INFO, a call such as
```C
  mondemand_crit(client, MONDEMAND_NULL_TRACE_ID, "foo %d\n", 5);
```
will immediately send an event, while a call such as
```C
  mondemand_debug(client, MONDEMAND_NULL_TRACE_ID, "bar %s\n", "baz");
```
will result in no data being logged.  And a call such as
```C
  mondemand_warning(client, MONDEMAND_NULL_TRACE_ID, "danger!");
```
will be held until 9 other messages are logged, and then emitted.

The levels can be altered at runtime using the following methods:
```C
  mondemand_set_immediate_send_level(...)
  mondemand_set_no_send_level(...)
```
To override this behavior programmatically, this method can be used
```C
  mondemand_flush_logs(...)
```
which will force the client to immediately flush all buffered logs.

## Trace Logging

Each log call has an extra 'trace_id' parameter, which when not set to
MONDEMAND_NULL_TRACE_ID, will do the following:

  * turn on immediate send for this call
  * turn off no send level
  * includes a trace_id in the emitted data

This will allow a downstream listener to aggregate events from a given trace_id
and correlate them for forensic purposes.  This is useful if someone wants
to get very verbose, debug level logging for a single transaction, since
the no send level is disabled just for this transaction, and not for others.
This also is helpful for systems handling high transaction volumes, since
enabling debug logging for such systems could have significant performance
consequences.

To disable trace logging, use the MONDEMAND_NULL_TRACE_ID.  If your
system handles trace ids, when set, use this method to create a trace id
```C
  mondemand_trace_id(...)
```

## Logging Statistics

MonDemand can maintain counters for an application.  This allows useful metrics
to be incremented, decremented, or set programmatically and then periodically
reported to other monitoring systems.

To manipulate counters, one can use any of the following methods:
```C
  mondemand_increment_key(client, key)
  mondemand_decrement_key(client, key)
  mondemand_increment_key_by_val(client, key, value)
  mondemand_decrement_key_by_val(client, key, value)
  mondemand_set_key_by_val(client, key, value)
```
There are also other convenience macros provided.

Statistics are not sent automatically, in order to trigger the library
to send data, call
```C
  mondemand_flush_stats(client)
```
This will send all the current counters and reset all counters to zero.  If 
you prefer monotonically increasing counters, use
```C
  mondemand_flush_stats_no_reset(client)
```
which will NOT reset counters and allow them to grow continually.

## Shutting Down

In order to shut down cleanly, call
```C
  mondemand_destroy(client)
```
This call will flush all pending messages and then clean up the memory
used by the library.

However, transports must handle their own garbage collection, as the
library has no way of knowing what objects to free or sockets to close
on termination.  For example, if using the LWES transport, one must call
```C
  mondemand_transport_lwes_destroy(transport)
```
after the MonDemand client is destroyed. That method will close the LWES
socket and free the allocated LWES data structures. 


## Appendix: Event Format

If using the Light Weight Event System, here are the ESF entries for the
different message types supported
```
MonDemand::LogMessage
{
  string prog_id;    # program identifier

  uint16 num;        # number of log messages in this event

  string f0;         # filename or class logging this message
  uint32 l0;         # line number of the calling filename or class
  uint32 p0;         # priority / log level
  string m0;         # the actual message
  uint16 r0;         # repeat count; used if the client detects repeats
  # repeated for num entries

  uint16 ctxt_num;   # number of contextual key/value dimensions

  string ctxt_k0;    # name of contextual metadata
  string ctxt_v0;    # value of contextual metadata
  # repeated for the number of repeated contextual key/value pairs
}
```
```
MonDemand::StatsMsg
{
  string prog_id;    # program identifier

  uint16 num;        # number of stats messages in this event

  string k0;         # name of the 0th counter
  int64  v0;         # value of the 0th counter
  # repeated for num entries

  uint16 ctxt_num;   # number of contextual key/value dimensions

  string ctxt_k0;    # name of contextual metadata
  string ctxt_v0;    # value of contextual metadata
  # repeated for the number of contextual key/value pairs
}
```
