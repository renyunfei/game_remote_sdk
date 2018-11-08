#ifndef EVENT_UTIL_H_
#define EVENT_UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

int init_event();
struct bufferevent* init_connect(char *addr, 
        int port, bufferevent_data_cb rcb, 
        bufferevent_data_cb wcb, void *arg);
void init_timer(time_t sec, long msec);
void* event_run(void *arg);
void keepalive(int conn_type);
void ev_free(int conn_type);
void heartbeat(int conn_type);

#ifdef __cplusplus
}
#endif
#endif
