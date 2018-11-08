#ifndef _NET_H
#define _NET_H

#include <event2/bufferevent.h>

#ifdef __ANDROID__
#include <android/log.h>
#endif

#include "list.h"
#include "defines.h"
#include "handler.h"

#ifdef __cplusplus
extern "C" {
#endif

void handler_msg(char *data, int len);
void handler_stream(char *data, int len);
void msg_send(int conn_type, void *data, int len, uint16_t id);
void stream_send(int conn_type, void *data, int len);
void data_resend(int conn_type, void *data, int len);
void msg_rich_send(int conn_type, void *data, 
        int len, uint8_t type, uint16_t id, int32_t uid);

#ifdef __cplusplus
}
#endif
#endif
