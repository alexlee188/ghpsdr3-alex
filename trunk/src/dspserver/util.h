/* Copyright 2012 Ethan Blanton */
/**
 * @file util.h
 * @brief Definitions of utility functions for dspserver.
 * @author Ethan Blanton, KB8OJH
 */
#ifndef _UTIL_H_
#define _UTIL_H_

#include <pthread.h>

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

/**
 * @defgroup threads Thread debugging facilities
 *
 * This group contains structures and functions useful for debugging
 * thread interactions in dspserver.
 */

/**
 * @brief Thread tracking structure for synchronization-less structures.
 * @ingroup threads
 *
 * This structure is used to ensure that synchronization-less global
 * data is always accessed by the same thread.  To use the
 * synchronization assertions, globally allocate an instance of this
 * structure with a static initializer of DSPSERVER_THREAD_ID, like:
 *
 * struct dspserver_thread_id tid = DSPSERVER_THREAD_ID;
 *
 * The members of this structure should not be modified by any code
 * other than the dspserver thread debugging code.
 *
 * @see DSPSERVER_THREAD_ID
 * @see dspserver_thread_assert_id()
 */
struct dspserver_thread_id {
    volatile int initialized;
    pthread_mutex_t lock;
    pthread_t id;
};

/**
 * @brief Static initializer for dspserver_thread_id
 * @ingroup threads
 *
 * @see struct dspserver_thread_id
 */
#define DSPSERVER_THREAD_ID { 0, PTHREAD_MUTEX_INITIALIZER }

volatile int dspserver__threads_debug;

void dspserver_log(enum DSPServerLogLevel level, char *fmt, ...);

void dspserver_threads_init();
void dspserver_thread_register(const char *name);
void dspserver_thread_unregister();
void dspserver_threads_debug(int enabled);

void dspserver__thread_assert_id(struct dspserver_thread_id *tid,
                                 char *file, int line);

#ifdef THREAD_DEBUG
/**
 * @brief Assert that the current thread is the only thread which has
 *        yet accessed the associated struct dspserver_thread_id.
 */
#define dspserver_thread_assert_id(tid) do {                            \
    if (dspserver__threads_debug)                                       \
        dspserver__thread_assert_id(tid, __FILE__, __LINE__);           \
    } while (0)
#else /* THREAD_DEBUG */
#define dspserver_thread_assert_id(tid)
#endif /* THREAD_DEBUG */

#endif /* _UTIL_H_ */
