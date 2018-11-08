#include <string.h>

#include "utils.h"
#include "net.h"
#include "list.h"

#include "cjson.h"

//json parse
//get int value from json string.
list *msg_list;


int value_int(char *str, char *key, int *n) 
{
    cJSON *item = NULL;

    /*
    cJSON_Hooks *hooks = (cJSON_Hooks*)malloc(sizeof(cJSON_Hooks));
    hooks->malloc_fn = malloc;
    hooks->free_fn = free;
    cJSON_InitHooks(hooks);
    */

    cJSON *root = cJSON_Parse(str);
    if (root == NULL)
        return -1;

    /*
    item = cJSON_GetObjectItem(root, key);
    if (item == NULL) {
        cJSON_Delete(root);
        return -1;
    }

    if (item->type != cJSON_Number) {
        cJSON_Delete(root);
        cJSON_Delete(item);
        return -1;
    }

    printf("eeeee:%s %s %d %f\n", str, key, item->valueint, item->valuedouble);
    *n = item->valueint;
    //cJSON_Delete(root);
    */

    return cJSON_GetIntItem(root, key, n);
}

void *m_malloc(int len)
{
    char *tmp = malloc(len);
    printf("MMMMM:%p\n", tmp);
    return tmp;
}

//get string value from json string.
char* value_str(char *str, char *key) 
{
    cJSON *node = NULL;
    /*
    cJSON_Hooks *hooks = (cJSON_Hooks*)malloc(sizeof(cJSON_Hooks));
    hooks->malloc_fn = m_malloc;
    hooks->free_fn = free;
    cJSON_InitHooks(hooks);
    */

    cJSON *root = cJSON_Parse(str);

    if (root == NULL)
        return NULL;

    /*
    node = cJSON_GetObjectItem(root, key);
    printf("nnnnnnn:%p %p\n", node, node->valuestring);
    if (node == NULL) {
        cJSON_Delete(root);
        return NULL;
    }

    if (node->type != cJSON_String) {
        cJSON_Delete(root);
        cJSON_Delete(node);
        return NULL;
    }

    printf("F:%s L:%d %p %p %p\n", __func__, __LINE__, str, node->valuestring, node);
    //return strdup(node->valuestring);
    return node->valuestring;
    */
    char *value = cJSON_GetStringItem(root, key);
    return strdup(value);
}
//end json parse

//timeout process
//expedient
msg_record_t *search_record_byUID(int id) {
    msg_record_t *mr;
    listNode *node;

    listSetMatchMethod(msg_list, match_uid);

    node = listSearchKey(msg_list, &id);
    if (node == NULL)
        return NULL;

    return (msg_record_t *)node->value;
}

msg_record_t *search_record_byID(uint16_t id) {
    msg_record_t *mr;
    listNode *node;

    listSetMatchMethod(msg_list, match_id);

    node = listSearchKey(msg_list, &id);
    if (node == NULL)
        return NULL;

    return (msg_record_t *)node->value;
}

void delete_record(msg_record_t *mr) {
    msg_record_t *found;
    listNode *node;

    listSetMatchMethod(msg_list, match_uid);

    node = listSearchKey(msg_list, &mr->msg_uid);
    if (node == NULL)
        return;

    listDelNode(msg_list, node);

    return;
}

//before send state msg, record it for check timeout latter
//if msg exist, return
int msg_record(struct bufferevent *bev, char *data, 
        int msg_len, uint16_t msg_id, int msg_uid) 
{
    time_t t;
    msg_record_t *mr;
    listNode *node;


    //add to msg_list
    mr = malloc(sizeof(msg_record_t));
    memcpy(mr->msg, data, msg_len);
    mr->msg_id = msg_id;
    mr->msg_uid = msg_uid;
    mr->msg_len = msg_len;
    mr->send_time = time(&t);
    mr->send_count++;
    mr->state = pending;
    mr->bev = bev;

    listAddNodeTail(msg_list, mr);

    return 0;
}

void process_msg_timeout() 
{
    time_t t;
    char buf[1024];
    msg_record_t *mr;
    listNode *node;

    LOCK(_state);

    listIter *iter = listGetIterator(msg_list, AL_START_HEAD);

    while ((node = listNext(iter)) != NULL) {
        mr = (msg_record_t*)node->value;
        if (mr->state == pending) {
            if ((time(&t)-mr->send_time) > 4) {
                //if (mr->send_count < 4) {
                //    data_resend(mr->bev, mr->msg, mr->msg_len);
                //    mr->send_count++;

                //    continue;
                //}

                debug("msg[%d] timeout\n", mr->msg_id);
                mr->state = timeout;
                continue;
            }
        }
    }

    listReleaseIterator(iter);
    UNLOCK(_state);
}

int match_uid(void *value, void *key) 
{
    msg_record_t *mr = (msg_record_t*)value;
    //debug("%d %d\n", mr->msg_uid, *((int*)(key)));
    if (mr->msg_uid == *((int*)(key))) 
        return 1;

    return 0;
}

int match_id(void *value, void *key) 
{
    msg_record_t *mr = (msg_record_t*)value;
    //debug("%d %d\n", mr->msg_id, *((uint16_t*)(key)));
    if (mr->msg_id == *((uint16_t*)(key))) 
        return 1;

    return 0;
}

//when recv msg, check if timeout
int check_msg_timeout(int msg_id) 
{
    char buf[1024];
    msg_record_t *mr;
    listNode *node;

    LOCK(_state);
    listSetMatchMethod(msg_list, match_uid);

    node = listSearchKey(msg_list, &msg_id);
    if (node != NULL) {

        mr = (msg_record_t *)node->value;
        if (mr->state == timeout) {
            UNLOCK(_state);
            return 1; 
        }

        mr->state = finish;
        UNLOCK(_state);
        return 0;
    }

    //can't find msg_id, msg timeout
    UNLOCK(_state);
    return 1;
}
//end timeout process
//
