#ifndef _UTILS_H
#define _UTILS_H

#include "net.h"
#include "defines.h"

#ifdef __cplusplus
extern "C" {
#endif

//
typedef struct {
    char msg[512];
    uint16_t msg_id;
    int msg_uid;
    int msg_len;
    int send_time;
    int send_count;
    int state;
    struct bufferevent *bev;
} msg_record_t;

//
//protocol state
enum {
    //waiting for response
    pending = 0,

    //recv response timeout, delete in time_cb
    timeout,

    //have received response, delete in time_cb
    finish,
};

extern ctx_t *ctx;
extern list *msg_list;

int value_int(char *string, char *key, int *n);
char *value_str(char *string, char *key);

int match_id(void *value, void *key);
int match_uid(void *value, void *key);
int msg_record(struct bufferevent *bev, char *data, int msg_len, uint16_t id, int msg_id);
void process_msg_timeout();
int check_msg_timeout(int msg_id); 
void delete_record(msg_record_t *mr);
msg_record_t *search_record_byID(uint16_t id);
msg_record_t *search_record_byUID(int id);

#ifdef __cplusplus
}
#endif
#endif
