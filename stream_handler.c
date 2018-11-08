#include <string.h>

#include "net.h"
#include "utils.h"
#include "handler.h"
#include "event_util.h"

static void stm_heartbeat_res();
static void stm_connect_res(msg_t *);
static void stm_disconnect_res(msg_t *);

void stream_msg(void *data, int len)
{
    msg_t *msg = (msg_t*)data;

    switch (msg->id) {
        case MsgID_Res_Device_Connect:
            stm_connect_res(msg);
            break;
        case MsgID_Res_Pong_S:
            stm_heartbeat_res();
            break;
        case MsgID_Res_Disconnect_Device:
            stm_disconnect_res(msg);
            break;
        default:
            break;
    }
}

/*
 * MsgID_Res_Pong_C
 * 0x0002
 */
static void stm_heartbeat_res()
{
    conns[_stream].ht = time(NULL);
}

/*
 * MsgID_Res_Device_Connect
 * 0x0402
 */
static void stm_connect_res(msg_t *msg)
{
    debug("MsgID_Res_Device_Connect:%s", msg->data);

    char msg_buf[256] = {0};
    int err_code;

    if (value_int(msg->data, "errno", &err_code) == -1) {
        debug("MsgID_Res_Device_Connect msg error");
        return;
    }
    if (err_code != 0) {
        char *errmsg = value_str(msg->data, "error");
        result(OtherResult, err_0x302_failed, errmsg);
    }
    if (value_int(msg->data, "session_id", &ctx->session_id)) {
        debug("MsgID_Res_Device_Connect msg error");
        return;
    }

    memset(msg_buf, 0, 256);
    snprintf(msg_buf, 256, "{\"app_id\":\"%s\"}", ctx->app_id);
    play_start(msg_buf);
}

/*
 * MsgID_Res_Disconnect_Device
 * 0x0801
 */
static void stm_disconnect_res(msg_t *msg)
{
    debug("MsgID_Req_Disconnect_Device:%s", msg->data);

    char msg_buf[256] = {0};
    int len = snprintf(msg_buf, 256, Res_Disconnect_Device_fmt, 0, "");
    msg_send(_stream, msg_buf, len, MsgID_Req_Disconnect_Device);
    ev_free(_stream);

    play_finish();
}
