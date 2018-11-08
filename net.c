#include <string.h>
#include <event2/buffer.h>

#include "net.h"
#include "utils.h"
#include "interface.h"

#define MAX_OUTPUT 10*1024*1024

//send stream data
void stream_send(int conn_type, void *data, int len) 
{
    header_t hdr = {0x2, len};
    struct bufferevent *ev = conns[conn_type].bev;
    LOCK(conn_type);
    if (ev == NULL) {
        UNLOCK(conn_type);
        return;
    }

    struct evbuffer *buffer = bufferevent_get_output(ev);
    if (evbuffer_get_length(buffer) > MAX_OUTPUT) {
        //debug("buffer_len gt 10M, throw away the data\n");
        UNLOCK(conn_type);
        return;
    }

    bufferevent_write(ev, &hdr, sizeof(header_t));
    bufferevent_write(ev, data, len);
    UNLOCK(conn_type);
}

//send msg data
void msg_send(int conn_type, void *data, int len, uint16_t id)
{
    int r;
    LOCK(conn_type);
    if ((id!=MsgID_Req_Ping_C) && (id!=MsgID_Req_Ping_S)) {
        /*
        r = msg_record(ev, msg_buf, sizeof(header_t)+hdr->len, id, msg->uid);
        if (r == 1) {
            debug("msg[%x:%d] pending\n", id, msg->uid);
            UNLOCK(conn_type);
            return; 
        }
        */
    }
    UNLOCK(conn_type);

    msg_rich_send(conn_type, data, len, 0x1, id, -1);
}

void msg_rich_send(int conn_type,
        void *data, int len, uint8_t type, uint16_t id, int32_t uid)
{
    char msg_buf[1024] = {0};

    header_t *hdr = (header_t*)msg_buf;
    hdr->type = type;
    hdr->len = sizeof(msg_t) + len;

    msg_t *msg = (msg_t*)hdr->data;
    msg->id = id;
    memcpy(msg->data, data, len);

    LOCK(conn_type);
    struct bufferevent *ev = conns[conn_type].bev;
    if (ev == NULL) {
        UNLOCK(conn_type);
        return;
    }

    ctx->uid++;
    msg->uid = uid;
    if (uid == -1)
        msg->uid = ctx->uid;

    bufferevent_write(ev, msg_buf, sizeof(header_t)+hdr->len);
    UNLOCK(conn_type);
}

//for timeout resend msg
void data_resend(int conn_type, void *data, int len) 
{
    struct bufferevent *ev = conns[conn_type].bev;
    LOCK(conn_type);
    if (bufferevent_write(ev, data, len) == -1) {
        debug("bufferevent_write error\n");
        bufferevent_unlock(ev);
        return;
    }

    UNLOCK(conn_type);
}

//stream data
void handler_stream (char *data, int len) 
{
    //receiveTouchData(data, len);
    device_recv(data, len);
    return;
}
