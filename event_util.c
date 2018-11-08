#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <event2/listener.h>
#include <event2/thread.h>
#include <event2/event.h>
#include <event2/util.h>
#include "net.h"
#include "utils.h"
#include "handler.h"
#include "event_util.h"

ctx_t *ctx;
conn_t *conns;
struct sdk *SDK;
int _state = 0x0;
int _stream = 0x1;
int _backup = 0x2;

//
//set util event
int init_event() 
{
    SDK->pipe_bev = bufferevent_socket_new(SDK->base, 
            SDK->rnotify, BEV_OPT_CLOSE_ON_FREE|BEV_OPT_THREADSAFE);
    if (SDK->pipe_bev == NULL) {
        debug("new pipe event error\n");
        return -1;
    }

	bufferevent_setcb(SDK->pipe_bev, pipe_readcb, 
            NULL, conn_eventcb, NULL);
	if (bufferevent_enable(SDK->pipe_bev, EV_READ) == -1) {
        debug("bufferevent_setcb error\n");
        return -1;
    }
    
	SDK->signal_ev = evsignal_new(SDK->base, SIGINT, 
            signal_cb, (void *)SDK->base);
    if (SDK->signal_ev == NULL) {
        debug("new signal event error\n");
        return -1;
    }

	if (event_add(SDK->signal_ev, NULL) != 0) {
        debug("add signal event error\n");
        return -1;
    }

    return 0;
}

//
//connect to addr:port
struct bufferevent* init_connect(char *addr, 
        int port, bufferevent_data_cb rcb, 
        bufferevent_data_cb wcb, void *arg) 
{
    struct timeval tv = {2, 0};
	struct sockaddr_in sin;

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    inet_aton(addr, &sin.sin_addr);

    struct bufferevent *be = bufferevent_socket_new(SDK->base, -1, 
            BEV_OPT_CLOSE_ON_FREE|BEV_OPT_THREADSAFE|BEV_OPT_DEFER_CALLBACKS);
    if (be == NULL) {
        debug("bufferevent_socket_new error\n");
        return NULL;
    }

    bufferevent_setcb(be, rcb, wcb, conn_eventcb, arg);
    bufferevent_set_timeouts(be, NULL, &tv);
    bufferevent_enable(be, EV_READ|EV_WRITE);

    if (bufferevent_socket_connect(be, 
                (struct sockaddr*)&sin, sizeof(sin)) == -1) {
        debug("bufferevent_socket_connect error\n");
        //bufferevent_free(be) in conn_eventcb
        return NULL;
    }

    return be;
}

//
//init timer
void init_timer(time_t sec, long msec)
{
    struct event *time_ev = NULL;
    evtimer_t *tm = (evtimer_t*)malloc(sizeof(evtimer_t));
    struct timeval *tv = (struct timeval*)malloc(sizeof(struct timeval));
    tv->tv_sec = sec;
    tv->tv_usec = msec;

    time_ev = evtimer_new(SDK->base, time_cb, tm);
    tm->ev = time_ev;
    tm->tv = tv;
    evtimer_add(time_ev, tv);
}

//
//run libevent base
void* event_run(void *arg) 
{
    pthread_detach(pthread_self());

	event_base_dispatch(SDK->base);
	event_base_free(SDK->base);

    return 0;
}

void keepalive(int conn_type)
{
    time_t now = time(NULL);
    if (conn_type == _state) {
        if ((now-conns[_state].ht) > 5) {
            debug("state heartbeat timeout");
            ev_free(_state);

            conns[conn_type].bev = init_connect (conns[_state].addr, 
                                    conns[_state].port, 
                                    stream_readcb, 
                                    NULL, &_state);
            conns[_state].ht = now;
        }
        else {
            heartbeat(conn_type);
        }
    }

    if ((conn_type == _stream) && conns[_stream].bev) {
        if ((now-conns[_stream].ht) > 5) {
            debug("stream heartbeat timeout");
            ev_free(_stream);

            //document_backup(atoi(ctx->app_id));
            result(OtherResult, err_connect_to_stream, "");
            play_finish();
        }
        else {
            heartbeat(conn_type);
        }
    }
}

//
//free ev
void ev_free(int conn_type)
{
    if (conns[conn_type].bev == NULL)
        return;

    LOCK(conn_type);
    bufferevent_free(conns[conn_type].bev);
    conns[conn_type].bev = NULL;
    UNLOCK(conn_type);
}

void heartbeat(int conn_type)
{
    uint16_t proto;
    struct bufferevent *bev = conns[conn_type].bev;
    if (bev == NULL)
        return;

    if (conn_type == _state)
        proto = MsgID_Req_Ping_C;
    if (conn_type == _stream)
        proto = MsgID_Req_Ping_S;

    msg_send(conn_type, Noti_Heartbeat_fmt, strlen(Noti_Heartbeat_fmt), proto);
}

