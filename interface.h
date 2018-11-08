
#ifndef INTERFACE_H_
#define INTERFACE_H_

#define DEVICE

#ifdef __cplusplus
extern "C" {
#endif

extern void result(int which, int result, char *errmsg);

extern void init_sdk(char *addr, int port);
extern void release_sdk();
extern void play_start();

extern void play_finish();
extern void general_status(int result, char *errmsg);
extern void msg_proxy(void *data, int len);

#ifdef DEVICE
//device
extern void device_register(char *info, int len);
extern void device_send(void *data, int len);
extern void device_ready(int status, int err);
extern void device_play_ready(int status, char *err);
extern void connect_status(int state, char *msg);
extern void void device_release();

extern void update_App(char *appid);
extern void register_status(int status, char *err);
extern void device_recv(void *data, int len);

#else
//user
extern void user_send(void *data, int len);

extern void connect_state(int status, char *err);
extern void user_recv(void *data, int len);
extern void play_status(int status, char *err);
#endif

#ifdef __cplusplus
}
#endif
#endif
