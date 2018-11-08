#ifndef DEFINES_H_
#define DEFINES_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define STATE_TYPE   0x1
#define STREAM_TYPE  0x2

#define SUCC    0
#define FAIL    -1

#define READY   0
#define PENDING 1
#define TIMEOUT 2

#if DEBUG
#define debug(argc, argv...)                                            \
        do {                                                            \
            fprintf(stderr, "DEBUG %s(%05d)  ", __func__, __LINE__);    \
            fprintf(stderr, argc, ##argv);                              \
            fprintf(stderr, "\n");                                      \
        } while (0)
#else 
#define LOG_EMERG   0   /* system is unusable */
#define LOG_ALERT   1   /* action must be taken immediately */
#define LOG_CRIT    2   /* critical conditions */
#define LOG_ERR     3   /* error conditions */
#define LOG_WARNING 4   /* warning conditions */
#define LOG_NOTICE  5   /* normal but significant condition */
#define LOG_INFO    6   /* informational */
#define LOG_DEBUG   7   /* debug-level messages */

#define debug(argc, argv...)                                            \
    do {                                                                \
        syslog(LOG_NOTICE, "DEBUG %s(%05d)  ", __func__, __LINE__);     \
        syslog(LOG_NOTICE, argc, ##argv);                               \
        syslog(LOG_NOTICE, "\n");                                       \
    } while (0)                                                         \

#endif

#define LOCK(conn_type) pthread_mutex_lock(&conns[conn_type].lock)
#define UNLOCK(conn_type) pthread_mutex_unlock(&conns[conn_type].lock)

/*
err_t *err_new(int code, char *msg)
{
    err_t *err = (err_t*)malloc(sizeof(err_t));
    err->err_code = code;
    strnpy(err->err_msg, msg, strlen(msg));

    return err;
}
*/

enum {
    //network error
    err_stream_close = -11,
    err_state_close = -12,
    err_stream_close_by_user = -13,
    err_connect_to_state = -14,
    err_connect_to_stream = -15,
    err_connection_state = -16,
    err_connection_stream = 17,

    //other error
    err_system = -18,
    err_unkonw = -19,

    //peer return failed
    err_0x301_failed = -0x301,
    err_0x302_failed = -0x302,
    err_0x303_failed = -0x303,

    //response timeout
    err_0x301_timeout = 0x301,
    err_0x302_timeout = 0x302,
    err_0x303_timeout = 0x303,
};

enum {
    ConnectState = 0,
    RegisterResult,
    OtherResult,
};

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)
//timer
typedef struct {
    struct event *ev;
    struct timeval *tv;
} evtimer_t;

//control msg
typedef struct {
    int32_t uid;
    uint16_t id;
    char data[];
} msg_t;

//
//header of all data
typedef struct {
    uint8_t     type;
    uint32_t    len;
    char     data[];
} header_t;

typedef struct {
    int err_code;
    char err_msg[128];
} err_t;

//
// connection
typedef struct {
    char *addr;
    int port;
    pthread_mutex_t lock;
    struct bufferevent *bev;

    int state;
    int type;
    time_t ht;
    int recount;
} conn_t;

//
// global var
struct sdk {
    pthread_t       tid;
    int             state;
    char            *rbuffer;
    int             rnotify;
    int             wnotify;
    struct event_base *base;
    struct bufferevent *pipe_bev;
    struct event *signal_ev;
};

//
// context, record play context info
typedef struct context {
    char  *app_id;
    int   uid;
    char  *user_id;
    char  *device_id;
    int   session_id;
    int   request_id;
    int   state;
} ctx_t;

extern conn_t *conns;
extern struct sdk *SDK;
extern ctx_t *ctx;

extern int _state;
extern int _stream;
extern int _backup;

#ifdef __cplusplus
}
#endif
#endif
