#ifndef HANDLER_H_
#define HANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

//#include "net.h"
#include "defines.h"
#include "interface.h"
#include "event_cb.h"

//
//json format
#define Noti_Heartbeat_fmt              "{}"
#define Req_User_Register_fmt           "{\"device_id\":\"%s\",\"resolution\":\"%s\",\"product\":\"%s\"}"
#define Res_User_Register_fmt           "{}"
#define Req_User_Connect_fmt            "{\"connect_id\":%d,\"device_ip\":\"%s\"}"
#define Res_User_Connect_fmt            "{}"
#define Req_User_Disconnect_fmt         "{}"
#define Res_User_Disconnect_fmt         "{}"
#define Req_Device_Register_fmt         "{}"
#define Res_Device_Register_fmt         "{}"   
#define Req_Device_Connect_fmt          "{}"
#define Res_Device_Connect_fmt          "{}"
#define Req_Device_Reset_fmt            "{\"connect_id\":%d}"
#define Res_Device_Reset_fmt            "{\"errno\":%d,\"error\":\"%s\"}"
#define Req_Device_Play_fmt             "{}"
#define Res_Device_Play_fmt             "{\"request_id\":%d,\"device_id\":\"%s\",\"session_id\":%d,\"errno\":%d,\"error\":\"%s\"}"
#define Req_Disconnect_Device_fmt       "{\"connect_id\":%d}"
#define Res_Disconnect_Device_fmt       "{\"errno\":%d,\"error\":\"%s\"}"

//
//protocol cmd
enum {
    MsgID_Req_Ping_C                = 0x0001,
    MsgID_Res_Pong_C                = 0x0002,
    MsgID_Req_Ping_S                = 0x0003,
    MsgID_Res_Pong_S                = 0x0004,
	MsgID_Noti_Heartbeat            = 0x0f00,	// headtbeat 	{}
    MsgID_Noti_DControl_Play        = 0x0f01,
    MsgID_Req_User_Apply            = 0x0101, 	// user register  {device_id, resolution, product}
    MsgID_Res_User_Apply            = 0x0201,	// user register response	{errno, error, server_ip, server_port, connect_id}
    MsgID_Req_User_Connect          = 0x0102, 	// user connect to stream server {connect_id, device_ip}
    MsgID_Res_User_Connect          = 0x0202,	// user connect to stream server response {errno, error}
    MsgID_Req_User_Disconnect       = 0x0103, 	// user close {}
    MsgID_Res_User_Disconnect       = 0x0203,	// user close response {errno, error}
    MsgID_Req_Device_Register       = 0x0301,  // device register {device_id, resolution, product, used, app_id}
    MsgID_Res_Device_Register       = 0x0401,	// {errno, error}
    MsgID_Req_Device_Connect        = 0x0302,  // 
    MsgID_Res_Device_Connect        = 0x0402,  //
    MsgID_Req_Device_Reset          = 0x0303, 	// 
    MsgID_Res_Device_Reset          = 0x0403,  //
    MsgID_Req_Device_Play           = 0x0304, 	//
    MsgID_Res_Device_Play           = 0x0404, 	//
    MsgID_Req_Disconnect_Device     = 0x0701,  //
    MsgID_Res_Disconnect_Device     = 0x0801,  //
    MsgID_Noti_Background_Transmit  = 0x0f02,
    MsgID_Req_Transmit              = 0x0005, // 转发给指定设备
    MsgID_Res_Transmit              = 0x0006,
};

//
//backup state
enum {
    Succ = 0x0,
    Init,
    Read,
    Write,
    Fini,
};

enum {
    upload_syn = 0x11,
    upload_ack,
    download_syn,
    download_ack,

    backup_syn,
    backup_ack,
    recover_syn,
    recover_ack,
};

//
//backup proto
typedef struct {
    int err;
    int id;
    char filename[128];
    int data_len;
    char md5[16];
    char data[];
} backup_msg_t;

extern int _state;
extern int _stream;
extern int _backup;

//
//global context
extern ctx_t *ctx;

void control_msg(void *data, int len);
void stream_msg(void *data, int len);
void backup_msg(void *data, int len);

#ifdef __cplusplus
}
#endif
#endif
