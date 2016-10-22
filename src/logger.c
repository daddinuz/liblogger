/*
 *  C Source File
 *
 *  Author: Davide Di Carlo
 *  Date:   Octorber 19, 2016
 *  email:  daddinuz@gmail.com
 */

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#ifdef _WIN32
#include "ansicolor-w32/ansicolor-w32.h"
#endif

#include "logger/logger.h"


/*
 * Colors
 */
#define COLOR_NORMAL    (NCOLOR != 0) ? "" : "\x1B[00m"
#define COLOR_RED       (NCOLOR != 0) ? "" : "\x1B[31m"
#define COLOR_GREEN     (NCOLOR != 0) ? "" : "\x1B[32m"
#define COLOR_YELLOW    (NCOLOR != 0) ? "" : "\x1B[33m"
#define COLOR_BLUE      (NCOLOR != 0) ? "" : "\x1B[34m"
#define COLOR_MAGENTA   (NCOLOR != 0) ? "" : "\x1B[35m"

/*
 * Helper functions
 */
static const char * _level2color(log_level_t level) {
    switch(level) {
        case LOG_LEVEL_DEBUG:
            return COLOR_BLUE;
        case LOG_LEVEL_NOTICE:
            return COLOR_NORMAL;
        case LOG_LEVEL_INFO:
            return COLOR_GREEN;
        case LOG_LEVEL_WARNING:
            return COLOR_YELLOW;
        case LOG_LEVEL_ERROR:
            return COLOR_RED;
        case LOG_LEVEL_FATAL:
            return COLOR_MAGENTA;
        default:
            abort();
    }
}

static const char * _level2string(log_level_t level) {
    switch(level) {
        case LOG_LEVEL_DEBUG:
            return "DEBUG";
        case LOG_LEVEL_NOTICE:
            return "NOTICE";
        case LOG_LEVEL_INFO:
            return "INFO";
        case LOG_LEVEL_WARNING:
            return "WARNING";
        case LOG_LEVEL_ERROR:
            return "ERROR";
        case LOG_LEVEL_FATAL:
            return "FATAL";
        default:
            abort();
    }
}

/*
 * String utils
 */
static char *_string_new(const char *data) {
    size_t length = strlen(data) + 1;
    char *str = calloc(length, sizeof(char));
    strcpy(str, data);
    return str;
}

static char *_string_cat(const char *a, const char *b) {
    size_t a_length = strlen(a);
    size_t b_length = strlen(b);
    char *str = calloc(a_length + b_length + 1, sizeof(char));
    strcpy(str, a);
    str += a_length;
    strcpy(str, b);
    str -= a_length;
    return str;
}

/*
 * File utils
 */
static const char *_file_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) {
        return "";
    }
    return dot + 1;
}

/*
 * logger_t definition
 */
struct logger_t {
    FILE *_fd;
    char *_filepath;    /** if NULL is a stream logger otherwise is a file logger **/
    char *_identifier;
    log_level_t _level;
    log_policy_t _policy;
    size_t _policy_bytes;
    size_t _written_bytes;
};

/*
 * Stream logger constructor
 */
logger_t * stream_logger_new(const char *identifier, log_level_t level, FILE *stream) {
    logger_t *logger = malloc(sizeof(logger_t));
    logger->_fd = (NULL != stream) ? stream : stderr;
    logger->_filepath = NULL;
    logger->_identifier = (NULL != identifier) ? _string_new(identifier) : "unknown";
    logger->_level = (LOG_LEVEL_DEBUG == level && NDEBUG != 0) ? LOG_LEVEL_NOTICE : level;
    logger->_policy = LOG_POLICY_NONE;
    logger->_policy_bytes = 0;
    logger->_written_bytes = 0;
    return logger;
}

/*
 * File logger utils
 */
#define IS_FILE_LOGGER(_Logger)     ((NULL == (_Logger)->_filepath) ? 0 : 1)

static void _logger_file_open(logger_t *logger, log_mode_t mode, const char *filepath) {
    assert(NULL != logger);
    logger->_fd = fopen(filepath, ((LOG_MODE_APPEND == mode) ? "a" : "w"));
    assert(NULL != logger->_fd);    /** TODO: better error handling **/
    logger->_filepath = _string_new(filepath);
    logger->_written_bytes = 0;
}

static void _logger_file_close(logger_t *logger) {
    assert(NULL != logger && IS_FILE_LOGGER(logger));
    fclose(logger->_fd);
    free(logger->_filepath);
    logger->_written_bytes = 0;
}

/*
 * File logger constructor
 */
logger_t * file_logger_new(const char *identifier, log_level_t level, const char *filepath, log_mode_t mode, log_policy_t policy, size_t bytes) {
    logger_t *logger = malloc(sizeof(logger_t));
    _logger_file_open(logger, mode, filepath);
    logger->_identifier = (NULL != identifier) ? _string_new(identifier) : "unknown";
    logger->_level = (LOG_LEVEL_DEBUG == level && NDEBUG != 0) ? LOG_LEVEL_NOTICE : level;
    logger->_policy = policy;
    logger->_policy_bytes = bytes;
    logger->_written_bytes = 0;
    return logger;
}

/*
 * Common logger destructor
 */
void logger_delete(logger_t **logger) {
    if (NULL != logger && NULL != *logger) {
        if (IS_FILE_LOGGER(*logger)) {
            _logger_file_close(*logger);
        }
        free((*logger)->_identifier);
        free(*logger);
        *logger = NULL;
    }
}

/*
 * Logging function internals
 */
static const char *_timestring(void) {
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = gmtime(&rawtime);
    char *timestring = asctime(timeinfo);
    size_t len = strlen(timestring);
    timestring[len - 1] = '\0';
    return timestring;
}

static void _log(logger_t *logger, log_level_t level, const char *format, va_list args) {
    logger->_written_bytes += fprintf(logger->_fd, "%s%-7s [%s UTC]%s -- (%s): ",
                                      _level2color(level), _level2string(level), _timestring(), COLOR_NORMAL, logger->_identifier);
    logger->_written_bytes += vfprintf(logger->_fd, format, args);
    fflush(logger->_fd);
}

/*
 * None Policy
 */
static void _apply_none_policy(logger_t *logger, log_level_t level, const char *format, va_list args) {
    assert(NULL != logger);
    _log(logger, level, format, args);
}

/*
 * Rotate Policy
 */
static void _rotate_file(logger_t *logger) {
    assert(NULL != logger && IS_FILE_LOGGER(logger));

    char ext[128];
    int c = atoi(_file_ext(logger->_filepath));
    sprintf(ext, ".%d", c);

    char *start = NULL;
    if (NULL == (start = strstr(logger->_filepath, ext))) {
        sprintf(ext, ".%d", c);
    } else {
        strncpy(start, "\0", 1);
        sprintf(ext, ".%d", c + 1);
    }

    char *tmp = _string_cat(logger->_filepath, ext);    /** TODO: better memory handling (avoid malloc here and in _logger_file_open) **/
    _logger_file_close(logger);
    _logger_file_open(logger, LOG_MODE_WRITE, tmp);
    free(tmp);
}

static void _apply_rotate_policy(logger_t *logger, log_level_t level, const char *format, va_list args) {
    assert(NULL != logger && IS_FILE_LOGGER(logger));

    if (logger->_written_bytes >= logger->_policy_bytes) {
        _rotate_file(logger);
    }

    _log(logger, level, format, args);
}

/*
 * Overwrite Policy
 */
static void _overwrite_file(logger_t *logger) {
    char *tmp = _string_new(logger->_filepath);         /** TODO: better memory handling (avoid malloc here and in _logger_file_open) **/
    _logger_file_close(logger);
    _logger_file_open(logger, LOG_MODE_WRITE, tmp);
    free(tmp);
}

static void _apply_overwrite_policy(logger_t *logger, log_level_t level, const char *format, va_list args) {
    assert(NULL != logger && IS_FILE_LOGGER(logger));

    if (logger->_written_bytes >= logger->_policy_bytes) {
        _overwrite_file(logger);
    }

    _log(logger, level, format, args);
}

/*
 * Logging functions entrypoint
 */
static void _apply_policy(logger_t *logger, log_level_t level, const char *format, va_list args) {
    assert(NULL != logger);

    if (level < logger->_level) {
        return;
    }

    switch (logger->_policy) {
        case LOG_POLICY_NONE:
            return _apply_none_policy(logger, level, format, args);
        case LOG_POLICY_ROTATE:
            return _apply_rotate_policy(logger, level, format, args);
        case LOG_POLICY_OVERWRITE:
            return _apply_overwrite_policy(logger, level, format, args);
        default:
            abort();
    }
}

/*
 * Define public logging functions
 */
#define DEFINE_LOGGER(_Identifier, _Level)                              \
    void log_##_Identifier(logger_t *logger, const char *format, ...) { \
        va_list args;                                                   \
        va_start(args, format);                                         \
        _apply_policy(logger, LOG_LEVEL_##_Level, format, args);        \
        va_end(args);                                                   \
    }

DEFINE_LOGGER(debug, DEBUG);
DEFINE_LOGGER(notice, NOTICE);
DEFINE_LOGGER(info, INFO);
DEFINE_LOGGER(warning, WARNING);
DEFINE_LOGGER(error, ERROR);
DEFINE_LOGGER(fatal, FATAL);

#undef DEFINE_LOGGER
