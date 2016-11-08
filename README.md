# liblogger

A logging library written in ANSI C.

### Installation

Install with [clib](https://github.com/clibs/clib):
```bash
$ clib install daddinuz/liblogger
```

### Description

Currently liblogger supports 4 types of loggers:

- stream logger (prints to an out stream such as stderr or stdout) 
- file logger (prints to a file without applying any policy)
- rotating logger (prints to a file and rotate every n bytes written)
- buffer logger (prints to a file and overwrites it every n bytes written)

liblogger **will not truncate your logs** this means that if the specified numbers of 
bytes for applying a policy are reached while performing a log function, the policy 
will be applied the next time a log function is called, preserving the integrity of you logs. 

By default, if a stream logger is set to stdout it will use colors;
in order to disable colors define at compile time the macro **NCOLORS** to 1

### Log levels

- **LOG_LEVEL_DEBUG**: 0
- **LOG_LEVEL_NOTICE**: 1
- **LOG_LEVEL_INFO**: 2
- **LOG_LEVEL_WARNING**: 3
- **LOG_LEVEL_ERROR**: 4
- **LOG_LEVEL_FATAL**: 5

Setting NDEBUG will not log debug messages no matter what the log level is.

### Example

```C
#include "logger.h"

int main(void) {
    char *logger_name = "ExampleStreamLogger";
    logger_t *logger = stream_logger_new(logger_name, LOG_LEVEL_DEBUG, stdout);
    
    log_debug   (logger, "Debug log\n\n");
    log_notice  (logger, "Notice log\n\n");
    log_info    (logger, "Info log\n\n");
    log_warning (logger, "Warning log\n\n");
    log_error   (logger, "Error log\n\n");
    log_fatal   (logger, "Fatal log\n\n");
    
    logger_delete(&logger);
    return 0;
}
```

### License 

MIT
