#include <signal.h>
#include <string.h>
#include <event2/thread.h>
#include <event2/event.h>

#include "net.h"
#include "defines.h"
#include "utils.h"
#include "interface.h"
#include "event_util.h"

void init_sdk(char *addr, int port) 
{
    sigset_t signal_mask; 
    sigemptyset(&signal_mask); 
    sigaddset(&signal_mask, SIGPIPE); 
    pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);

    SDK = (struct sdk*)malloc(sizeof(struct sdk));
    SDK->rbuffer = malloc(1024*1024);

    //control stream backup
    conns = (conn_t*)malloc(3*sizeof(conn_t));
    for (int i = 0; i < 3; i++) {
        pthread_mutex_init(&conns[i].lock, NULL);
        conns[i].type = i;
        conns[i].ht = time(NULL);
        conns[i].recount = 0;
        conns[i].bev = NULL;
        conns[i].state = 0;

        if (_state == i) {
            conns[i].addr = strdup(addr);
            conns[i].port = port;
        }
    }

    msg_list = listCreate();
    listSetFreeMethod(msg_list, free);

    evthread_use_pthreads();
	SDK->base = event_base_new();
    if (SDK->base == NULL) {
        debug("create event base error");
        return;
    }

    init_timer(2, 0);
    init_event();

    init_connect(conns[_state].addr, 
           conns[_state].port, stream_readcb, NULL, &_state);

    //init_connect("127.0.0.1", 
    //       10006, stream_readcb, NULL, &_backup);

    pthread_create(&SDK->tid, NULL, event_run, NULL);
}

void device_release() {
    ev_free(_stream);
}

void device_register(char *msg, int len) 
{
    if (ctx != NULL) free(ctx);
    ctx = (ctx_t*)malloc(sizeof(ctx_t));
    if (ctx == NULL) {
        result(OtherResult, -1, "new ctx error");
        return;
    }
    ctx->device_id = value_str(msg, "device_id");
    if (ctx->device_id == NULL) {
        result(OtherResult, -1, "Registre need revice_id");
        return;
    }

    msg_send(_state, msg, len, MsgID_Req_Device_Register);
}

void device_ready(int status, int err) 
{
    char msg_buf[128];
    if (strlen(ctx->device_id) != 0) {
        return;
    }
    size_t len = snprintf(msg_buf, 128, "{\"device_id\":\"%s\"}", ctx->device_id);
    msg_send(_state, msg_buf, len, MsgID_Req_Device_Reset);
}

void device_send(void *data, int len) 
{
    stream_send(_stream, data, len);
}

void device_play_ready(int result, char* errorMsg) 
{
    char msg_buf[256] = {0};
    int len = snprintf(msg_buf, 256, 
            Res_Device_Play_fmt, 
            ctx->request_id, 
            ctx->device_id,
            ctx->session_id,
            result, errorMsg);

    msg_send(_state, msg_buf, len, MsgID_Req_Device_Play);
}

void result(int which, int result, char *errmsg)
{
    switch (which) {
        case RegisterResult:
            register_status(result, errmsg);
            break;
        case ConnectState:
            connect_status(result, errmsg);
            break;
        default:
            general_status(result, errmsg);
    }
}

