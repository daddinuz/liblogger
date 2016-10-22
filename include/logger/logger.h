/*
 *  C Header File
 *
 *  Author: Davide Di Carlo
 *  Date:   Octorber 19, 2016
 *  email:  daddinuz@gmail.com
 */

#include <stdio.h>


#ifndef __LOGGER_H__
#define __LOGGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NDEBUG
#define NDEBUG 0
#endif

#ifndef NCOLOR
#define NCOLOR 0
#endif

/*
 * log_level_t declaration
 */
typedef enum log_level_t {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_NOTICE,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL,
} log_level_t;

/*
 * log_policy_t declaration
 */
typedef enum log_policy_t {
    LOG_POLICY_NONE = 0,
    LOG_POLICY_ROTATE,
    LOG_POLICY_OVERWRITE,
} log_policy_t;

/*
 * log_mode_t declaration
 */
typedef enum log_mode_t {
    LOG_MODE_WRITE = 0,
    LOG_MODE_APPEND,
} log_mode_t;

/*
 * logger_t functions declaration
 */
typedef struct logger_t logger_t;

extern logger_t * stream_logger_new(const char *identifier, log_level_t level, FILE *stream);
extern logger_t * file_logger_new(const char *identifier, log_level_t level, const char *filepath, log_mode_t mode, log_policy_t policy, size_t bytes);
extern void logger_delete(logger_t **logger);

/*
 * log function declaration
 */
extern void log_debug   (logger_t *logger, const char *format, ...);
extern void log_notice  (logger_t *logger, const char *format, ...);
extern void log_info    (logger_t *logger, const char *format, ...);
extern void log_warning (logger_t *logger, const char *format, ...);
extern void log_error   (logger_t *logger, const char *format, ...);
extern void log_fatal   (logger_t *logger, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* __LOGGER_H__ */
