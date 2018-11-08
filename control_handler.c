#include "net.h"
#include "utils.h"
#include "handler.h"
#include "event_cb.h"
#include "event_util.h"

static void ctl_register_res(msg_t *msg);
static void ctl_reset_res(msg_t *msg);
static void ctl_play_req(msg_t *msg);
static void ctl_play_res(msg_t *msg);
static void ctl_close_req(msg_t *msg);
static void ctl_background_req(msg_t *msg);
static void ctl_pong_res();
static void ctl_transmit_req(msg_t *msg, int len);

void control_msg(void *data, int len)
{
    int err_code;
    msg_t *msg = (msg_t*)data;

    switch (msg->id) {
        case MsgID_Res_Device_Register:
            ctl_register_res(msg);
            break;
        case MsgID_Res_Device_Reset:
            ctl_reset_res(msg);
            break;
        case MsgID_Noti_DControl_Play:
            ctl_play_req(msg);
            break;
        case MsgID_Res_Device_Play:
            ctl_play_res(msg);
            break;
        case MsgID_Req_Disconnect_Device:
            ctl_close_req(msg);
            break;
        case MsgID_Noti_Background_Transmit:
            ctl_background_req(msg);
            break;
        case MsgID_Res_Pong_C:
            ctl_pong_res();
            break;
        case MsgID_Res_Transmit:
            ctl_transmit_req(msg, len);
            break;
        default:
            break;
    }
}

/*
 * MsgID_Res_Device_Register: 0x401
 * register device info to state server.
 */
static void ctl_register_res(msg_t *msg)
{
    debug("MsgID_Res_Device_Register:%p %s", msg->data, msg->data);
    int err_code;

    if (value_int(msg->data, "errno", &err_code) == -1) {
        debug("MsgID_Res_Device_Register msg error");
        return;
    }
    if (err_code != 0) {
        char *errmsg = value_str(msg->data, "error");
        result(RegisterResult, err_0x301_failed, errmsg);
        return;
    }

    result(RegisterResult, err_code, "Register success");
}

/*
 * MsgID_Res_Device_Reset:
 * notify state server that device had reset succ.
 */
static void ctl_reset_res(msg_t *msg)
{
    debug("MsgID_Res_Device_Reset:%s", msg->data);
    int err_code;

    if (value_int(msg->data, "errno", &err_code) == -1) {
        debug("MsgID_Res_Device_Reset msg error");
        return;
    }
    if (err_code != 0) {
        debug("Device reset:%s", msg->data);
        char *errmsg = value_str(msg->data, "error");
        result(OtherResult, err_0x303_failed, errmsg);
        return;
    }
}

/*
 * MsgID_Noti_DControl_Play:
 * get play request from state server
 * init connect to stream server.
 */
static void ctl_play_req(msg_t *msg)
{
    debug("MsgID_Req_Device_Play:%s", msg->data);

    if (value_int(msg->data, "request_id", &ctx->request_id) == -1) {
        debug("MsgID_Req_Device_Play msg request_id error");
        return;
    }
    if (value_int(msg->data, "server_port", &conns[_stream].port) == -1) {
        debug("MsgID_Req_Device_Play msg server_port error");
        return;
    }

    conns[_stream].addr = value_str(msg->data, "server_ip");
    ctx->app_id = value_str(msg->data, "app_id");
    //ctx->user_id = value_str(msg->data, "user_id");

    if ((ctx->app_id == NULL)
     //   || (ctx->user_id == NULL)
        || (conns[_stream].addr == NULL)) {
        debug("MsgID_Req_Device_Play msg error");
        return;
    }

    //recover();
    init_connect(conns[_stream].addr, 
            conns[_stream].port, stream_readcb, NULL, &_stream);
}

/*
 * MsgID_Res_Device_Play response
 * Do nothing
 * MsgID_Res_Device_Play:
 */
static void ctl_play_res(msg_t *msg)
{
    debug("MsgID_Res_Device_Play:%s", msg->data);
}

/*
 * MsgID_Req_Disconnect_Device: 
 * stream close by user.
 * close connection to stream server
 */
static void ctl_close_req(msg_t *msg)
{
    debug("MsgID_Req_Disconnect_Device:%s", msg->data);

    char msg_buf[256] = {0};
    int len = snprintf(msg_buf, 256, Res_Disconnect_Device_fmt, 0, "");
    msg_send(_stream, msg_buf, len, MsgID_Req_Disconnect_Device);

    play_finish();
    ev_free(_stream);
    
    //backup();
}

/*
 * MsgID_Noti_Background_Transmit:
 */
static void ctl_background_req(msg_t *msg)
{
    debug("MsgID_Noti_Background_Transmit:%s", msg->data);
    update_App(msg->data);
}

/*
 * MsgID_Res_Pong_C:
 */
static void ctl_pong_res()
{
    conns[_state].ht = time(NULL);
}

/*
 * MsgID_Res_Transmit:
 */
static void ctl_transmit_req(msg_t *msg, int len)
{
    int id = msg->id;
    //msg_transmit(msg->data, len-sizeof(msg_t));
    msg_rich_send(_state, "", 0, 0x1, MsgID_Res_Transmit, id);
}
