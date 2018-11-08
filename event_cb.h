#ifndef EVENT_CB_H_
#define EVENT_CB_H_

#define M 1024*1024

#ifdef __cplusplus
extern "C" {
#endif

void time_cb(int fd, short _event, void *arg);
void stream_readcb(struct bufferevent *bev, void *arg);
void conn_eventcb (struct bufferevent *bev, short events, void *arg);
void signal_cb(evutil_socket_t sig, short events, void *user_data);
void pipe_readcb(struct bufferevent *bev, void *user_data);

#ifdef __cplusplus
}
#endif
#endif
