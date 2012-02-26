/* Copyright 2012 Ethan Blanton */
/**
 * @file util.h
 * @brief Definitions of utility functions for dspserver.
 * @author Ethan Blanton, KB8OJH
 */
#ifndef _UTIL_H_
#define _UTIL_H_

/**
 * @brief Severity of debug message.
 *
 * Values in this enum indicate the severity of a debug message, in
 * increasing order from top (lesser values) to bottom (greater values).
 * When verbosity is implemented, more severe levels will be logged at
 * lower verbosity levels.
 */
enum DSPServerLogLevel {
    DSP_LOG_INFO,
    DSP_LOG_WARNING,
    DSP_LOG_ERROR
};

void dspserver_log(enum DSPServerLogLevel level, char *fmt, ...);

#endif /* _UTIL_H_ */
