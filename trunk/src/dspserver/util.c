/* Copyright 2012 Ethan Blanton */
/**
 * @file util.c
 * @brief Utility functions for dspserver.
 * @author Ethan Blanton, KB8OJH
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

#include "util.h"

#ifdef THREAD_DEBUG
/**
 * @brief Structure for list of thread ID -> name mappings, for debugging.
 *
 * @see sdr_threads_init() 
 * @see sdr_thread_register()
 * @see sdr_thread_unregister()
 */
struct threadlist {
    pthread_t id;               /**< Thread ID for this mapping       */
    char *name;                 /**< Free-form name of this thread    */
    struct threadlist *next;    /**< Next registered thread           */
};
static struct threadlist *threadlist;

/**
 * @brief Mutex to protect threadlist
 * @ingroup threads
 */
static sem_t threadlist_sem;

/**
 * @brief Enable (costly) thread debugging if true
 *
 * This variable should not by modified or used by anything but the
 * sdr_thread_* functions and macros.
 */
volatile int sdr__threads_debug;
#endif /* THREAD_DEBUG */

/**
 * @brief Log a message to stderr with timestamp.
 *
 * Log a message to stderr with timestamp.  The debug level argument is
 * currently ignored, but will be used to control verbosity in the
 * future.
 *
 * @param level severity of the message
 * @param fmt printf-style format string
 */
void sdr_log(enum SDRLogLevel level, char *fmt, ...)
{
    va_list args;
    time_t now;
    struct tm tm;
    char ts[24];
    
    now = time(NULL);
    localtime_r(&now, &tm);
    strftime(ts, sizeof(ts), "[%Y-%m-%d %H:%M:%S] ", &tm);

    /* Ideally we'd do this in one write, but for now two will do. */
    fprintf(stderr, "%s", ts);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}


/**
 * @brief Initialize the sdr thread debugging facilities
 * @ingroup threads
 *
 * This function must be called before calling any of the other thread
 * debugging functions.  It should ideally be called before any threads
 * are created, and MUST COMPLETE before any thread debugging functions
 * are called.
 */
void sdr_threads_init()
{
#ifdef THREAD_DEBUG
    sem_init(&threadlist_sem, 0, 1);
    sdr_log(SDR_LOG_INFO, "sdr thread debugging initialized\n");
#endif /* THREAD_DEBUG */
}

/**
 * @brief Register a thread with the thread debugging facility.
 * @ingroup threads
 *
 * @param name The name of this thread 
 */
void sdr_thread_register(const char *name)
{
#ifdef THREAD_DEBUG
    struct threadlist *newentry, *cur;
    pthread_t self = pthread_self();

    sdr_log(SDR_LOG_INFO, "Registering thread '%s'\n", name);

    sem_wait(&threadlist_sem);
    for (cur = threadlist; cur; cur = cur->next) {
        if (pthread_equal(cur->id, self)) {
            sdr_log(SDR_LOG_ERROR, "Thread ID being registered already exists!");
            sem_post(&threadlist_sem);
            return; /* Not clear what to do here */
        }
    }
    sem_post(&threadlist_sem);

    newentry = malloc(sizeof(struct threadlist));
    newentry->id = self;
    newentry->name = strdup(name);

    sem_wait(&threadlist_sem);
    newentry->next = threadlist;
    threadlist = newentry;
    sem_post(&threadlist_sem);
#endif /* THREAD_DEBUG */
}

/**
 * @brief Unregister a thread from the sdr thread debugging facility.
 * @ingroup threads
 */
void sdr_thread_unregister()
{
#ifdef THREAD_DEBUG
    struct threadlist *cur, *prev;

    sem_wait(&threadlist_sem);

    prev = cur = NULL;
    for (cur = threadlist; cur; cur = cur->next) {
        if (pthread_equal(cur->id, pthread_self())) {
            sdr_log(SDR_LOG_INFO, "Unregistering thread '%s'\n",
                    cur->name);
            free(cur->name);
            if (prev) {
                prev->next = cur->next;
            } else {
                threadlist = cur->next;
            }
            free(cur);
            break;
        }
        prev = cur;
    }

    sem_post(&threadlist_sem);

    if (cur == NULL) {
        sdr_log(SDR_LOG_ERROR, "Thread was not found to unregister!\n");
    }
#endif /* THREAD_DEBUG */
}

/**
 * @brief Change the status of thread debugging.
 * @ingroup threads
 *
 * This function does not do any synchronization, so calling it
 * mid-stream may result in the loss of assertions or additional
 * execution of assertions near the time of the call.
 *
 * @param enabled TRUE to enable debugging, FALSE to disable
 */
void sdr_threads_debug(int enabled)
{
#ifdef THREAD_DEBUG
    sdr__threads_debug = !!enabled;
#endif /* THREAD_DEBUG */
}


void sdr__thread_assert_id(struct sdr_thread_id *tid,
                           char *file, int line)
{
#ifdef THREAD_DEBUG
    pthread_t self = pthread_self();

    if (!tid->initialized) {
        pthread_mutex_lock(&tid->lock);
        if (!tid->initialized) {
            tid->id = self;
            tid->initialized = 1;
            pthread_mutex_unlock(&tid->lock);
            return;
        }
        /* Simultaneous initialization checks, the other thread won */
        pthread_mutex_unlock(&tid->lock);
    }
    if (!pthread_equal(tid->id, self)) {
        struct threadlist *cur;
        const char *me = NULL, *other = NULL;

        sem_wait(&threadlist_sem); /* Break here for debugging */
        for (cur = threadlist; cur; cur = cur->next) {
            if (pthread_equal(cur->id, self)) {
                me = cur->name;
            } else if (pthread_equal(cur->id, tid->id)) {
                other = cur->name;
            }
            if (me && other) {
                break;
            }
        }
        sem_post(&threadlist_sem);
        sdr_log(SDR_LOG_ERROR, "thread '%s' attempted to manipulate state owned by thread '%s' at %s:%d\n",
                me ? me : "unknown", other ? other : "unknown",
                file, line);
    }
#endif /* THREAD_DEBUG */
}
