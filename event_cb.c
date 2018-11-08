#include <string.h>
#include <event2/util.h>
#include <event2/buffer.h>
#include <event2/event.h>
#include <event2/bufferevent.h>

#include "defines.h"
#include "event_cb.h"
#include "event_util.h"
#include "handler.h"
#include "net.h"
//
//timer
void time_cb(int fd, short _event, void *arg)
{
    evtimer_t *tt = (evtimer_t *)arg;
    struct event *ev = tt->ev;
    struct timeval *tv = tt->tv;

    //check msg timeout
    //process_msg_timeout();

    //heartbeat
    keepalive(_state);
    keepalive(_stream);

    evtimer_add(ev, tv);
}

//libevent read callback
void stream_readcb(struct bufferevent *bev, void *arg)
{
    char *data = SDK->rbuffer;
    memset(data, 0, 1*M);
    int conn_type = *((int*)arg);
    header_t hdr;

    struct evbuffer *buffer = bufferevent_get_input(bev);
    for (;;) {
        size_t len = evbuffer_get_length(buffer);
        if (len < sizeof(hdr)) {
            return;
        }

        evbuffer_copyout(buffer, &hdr, sizeof(hdr));
        if (len < (hdr.len + sizeof(hdr))) {
            return;
        }

        evbuffer_remove(buffer, data, sizeof(hdr)+hdr.len);
        header_t *h = (header_t*)data;

        if (hdr.type == STREAM_TYPE) {
            handler_stream(h->data, h->len);
            continue;
        }

        if (hdr.type == STATE_TYPE) {
            switch (conn_type) {
                case 0x0:
                    h->data[h->len] = '\0';
                    control_msg(h->data, h->len);
                    break;
                case 0x1:
                    stream_msg(h->data, h->len);
                    break;
                case 0x2:
                    h->data[h->len] = '\0';
                    backup_msg(h->data, h->len);
                    break;
                default:
                    break;
            }
            continue;
        }
    }
}

//event callback
void conn_eventcb (struct bufferevent *bev, short events, void *arg)
{
    int conn_type = *((int*)arg);

    if (conn_type == _stream) {
        if (events & BEV_EVENT_CONNECTED) {
            debug("connect to stream server");
            conns[_stream].bev = bev;

            msg_send(_stream, Req_Device_Connect_fmt, 
                    strlen(Req_Device_Connect_fmt), 
                    MsgID_Req_Device_Connect);
            conns[_stream].ht = time(NULL);
            return;
        }

        if ((events & BEV_EVENT_TIMEOUT)
                || (events & BEV_EVENT_EOF) 
                || (events & BEV_EVENT_ERROR)
                || (events & BEV_EVENT_WRITING)) {
            debug("connect to stream server error");

            return;
        }
    }

    if (conn_type == _state) {
        if (events & BEV_EVENT_CONNECTED) {
            debug("connect to state server");

            conns[_state].bev = bev;
            result(ConnectState, SUCC, "");
            return;
        }

        if ((events & BEV_EVENT_TIMEOUT)
                || (events & BEV_EVENT_EOF) 
                || (events & BEV_EVENT_ERROR)
                || (events & BEV_EVENT_WRITING)) {
            debug("connect to state server error");
            return;
        }
    }

    if (conn_type == _backup) {
    }
}

//signal event callback
void signal_cb(evutil_socket_t sig, short events, void *user_data)
{
	struct event_base *base = user_data;
	struct timeval delay = {0,100};
	debug("Caught an interrupt signal; exiting cleanly in two seconds.");

    if (conns[_state].bev) {
        ev_free(_state);
    }

    if (conns[_stream].bev) {
        ev_free(_stream);
    }

	event_base_loopexit(base, &delay);
    free(SDK->rbuffer);
}

void pipe_readcb(struct bufferevent *bev, void *user_data)
{
}
