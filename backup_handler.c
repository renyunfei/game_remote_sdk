#include <string.h>

#include "net.h"
#include "handler.h"

static void backup_res(backup_msg_t *);
static void recover_res(backup_msg_t *);
void recover_path_cb(const char *path, size_t len);
void backup_path_cb(const char *path, size_t len);

void backup_msg(void *data, int len)
{
    header_t *hdr = (header_t*)data;
    backup_msg_t *msg = (backup_msg_t*)hdr->data;

    switch (msg->id) {
        case backup_ack:
            backup_res(msg);
            break;
        case recover_ack:
            recover_res(msg);
            break;
        default:
            break;
    }
}

void document_backup (int app_id)
{
    //get_app_home_path(app_id, backup_path_cb);
}

void recover(int app_id)
{
    //get_app_home_path(app_id, recover_path_cb);
}

void recover_path_cb(const char *path, size_t len)
{
    char buf[1024] = {0};
    if (len == 0)
        return;
    
    header_t *hdr = (header_t*)buf;
    hdr->type = recover_syn;

    if (strlen(ctx->user_id) == 0)
        return;

    if (strlen(ctx->app_id) == 0)
        return;
    
    backup_msg_t *msg = (backup_msg_t*)hdr->data;
    int data_len = snprintf(msg->data, 512, 
                            "%s %s_%s", path, 
                            ctx->user_id, ctx->app_id);
    msg->data_len = data_len;
    hdr->len = sizeof(backup_msg_t)+msg->data_len;
    msg->err = 0;
    
    bufferevent_write(conns[_backup].bev, buf, sizeof(header_t)+hdr->len);
}

void backup_path_cb(const char *path, size_t len)
{
    char buf[1024] = {0};
    if (len == 0)
        return;
    
    header_t *hdr = (header_t*)buf;
    hdr->type = backup_syn;
    
    if (strlen(ctx->user_id) == 0)
        return;

    if (strlen(ctx->app_id) == 0)
        return;

    backup_msg_t *msg = (backup_msg_t*)hdr->data;
    int data_len = snprintf(msg->data, 512, "%s %s_%s", 
            path, ctx->user_id, ctx->app_id);
    msg->data_len = data_len;
    msg->err = 0;
    hdr->len = sizeof(backup_msg_t)+msg->data_len;
    
    bufferevent_write(conns[_backup].bev, buf, sizeof(header_t)+hdr->len);
}

static void backup_res(backup_msg_t *msg)
{
    if (msg->err == Succ)
        debug("backup success");
    else 
        debug("backup failed");
}

static void recover_res(backup_msg_t *msg)
{
    if (msg->err == Succ)
        debug("recover success");
    else 
        debug("recover failed");
}
