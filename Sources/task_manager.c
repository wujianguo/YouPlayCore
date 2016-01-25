//
//  task_manager.c
//  YouPlayCore
//
//  Created by 吴建国 on 16/1/19.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#include "task_manager.h"
#include <stdlib.h>
#include "extractor.h"

static QUEUE g_task_list = {0};
static int g_task_init = 0;

enum vod_task_state {
    vod_task_s_parse,
    vod_task_s_proxy_1,
    vod_task_s_proxy_2,
    vod_task_s_close
};

#define TASK_CACHE_BUF_LEN (2*1024*1024)
typedef struct vod_task {
//    int reference;
    enum vod_task_state state;
    uv_loop_t *loop;
    int index;
    http_connection *remote;
    char remote_url_buf[MAX_URL_LEN];
    struct http_parser_url remote_url;
    vod_data_read_cb read_cb;
    vod_data_header_cb header_cb;
    void *user_data;
    
    char *buf;
    uint64_t cur_pos;
    size_t cur_len;

    uint64_t want_pos;
    size_t want_len;
    
    int callbacking;
    int destroy;
}vod_task;


static void on_error(vod_task *task) {
    YOU_LOG_DEBUG("task:%p", task);
    return;
    ASSERT(task->read_cb);
    task->callbacking = 1;
    task->read_cb(task, NULL, 0, task->user_data);
    task->callbacking = 0;
    if (task->destroy) {
        stop_vod_task(task);
    } else {
        free_http_connection(task->remote);
        task->remote = NULL;
    }
}

static void on_remote_error(http_connection *conn, void *user_data, int err_code) {
    vod_task *task = (vod_task*)user_data;
    YOU_LOG_DEBUG("task:%p", task);
    on_error(task);
}

static void on_remote_connect(http_connection *conn, void *user_data) {
    vod_task *task = (vod_task*)user_data;
    YOU_LOG_DEBUG("task:%p", task);
    /*
    char range[MAX_HTTP_VALUE_LEN] = {0};
    QUEUE *q = NULL;
    QUEUE_FOREACH(q, &task->req->header->headers) {
        struct http_header_field_value *head = QUEUE_DATA(q, struct http_header_field_value, node);
        if (strcmp(head->field, "Range")==0)
            strncpy(range, head->value, MAX_HTTP_VALUE_LEN);
    }
    */
    const char format[] = "GET %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Connection: keep-alive\r\n"
    "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
    "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_11_2) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/47.0.2526.106 Safari/537.36\r\n"
    "Accept-Encoding: gzip, deflate\r\n"
    "Accept-Language: zh-CN,zh;q=0.8,en-US;q=0.6,en;q=0.4\r\n"
    "Range: bytes=0-\r\n"
    "\r\n";
    
    char host[MAX_HOST_LEN] = {0};
    memcpy(host, task->remote_url_buf + task->remote_url.field_data[UF_HOST].off, task->remote_url.field_data[UF_HOST].len);
    
    char path[MAX_URL_LEN] = {0};
    memcpy(path, task->remote_url_buf + task->remote_url.field_data[UF_PATH].off, strlen(task->remote_url_buf) - task->remote_url.field_data[UF_PATH].off);
    
    char header[MAX_REQUEST_HEADER_LEN] = {0};
    snprintf(header, MAX_REQUEST_HEADER_LEN, format, path, host);
    http_connection_send(conn, header, strlen(header));
}

static void on_remote_send(http_connection *conn, void *user_data) {
    
}

static void on_remote_header_complete(http_connection *conn, struct http_header *header, void *user_data) {
    vod_task *task = (vod_task*)user_data;
    YOU_LOG_DEBUG("task:%p", task);
    ASSERT(task->header_cb);
    task->header_cb(task, header, task->user_data);
//    http_connection_stop_read(conn);
}

static void on_remote_body(http_connection *conn, const char *at, size_t length, void *user_data) {
    vod_task *task = (vod_task*)user_data;
    YOU_LOG_DEBUG("task:%p", task);
    ASSERT(task->cur_len+length <= TASK_CACHE_BUF_LEN);
    memcpy(task->buf + task->cur_len, at, length);
    task->cur_len += length;
    ASSERT(task->cur_pos==task->want_pos);
    if (task->cur_pos == task->want_pos && task->cur_pos + task->cur_len >= task->want_pos + task->want_len) {
        ASSERT(task->read_cb);
        http_connection_stop_read(conn);
        task->read_cb(task, task->buf, task->want_len, task->user_data);
    }
}

static void on_remote_message_complete(http_connection *conn, void *user_data) {
    vod_task *task = (vod_task*)user_data;
    on_error(task);
}

static struct http_connection_settings settings = {
    on_remote_error,
    on_remote_connect,
    on_remote_send,
    on_remote_header_complete,
    on_remote_body,
    on_remote_message_complete,
    NULL,
    NULL
};

static void free_vod_task(vod_task *task) {
    YOU_LOG_DEBUG("task:%p", task);
    if (task->remote)
        free_http_connection(task->remote);
    free(task);
}

static void on_extractor_complete(extractor_result *ret, void *user_data) {
    vod_task *task = (vod_task*)user_data;
    YOU_LOG_DEBUG("task:%p", task);
    if (task->state == vod_task_s_close) {
        free_vod_task(task);
        return;
    }
    ASSERT(task->state == vod_task_s_parse);
    task->state = vod_task_s_proxy_1;
    QUEUE *q = NULL;
    YOU_LOG_DEBUG("extractor_result:%p", ret);
    QUEUE_FOREACH(q, &ret->qualities) {
        YOU_LOG_DEBUG("");
        extract_quality_item *quality = QUEUE_DATA(q, extract_quality_item, node);
        QUEUE *q2 = NULL;
        QUEUE_FOREACH(q2, &quality->clips) {
            extract_clips *clip = QUEUE_DATA(q2, extract_clips, node);
            YOU_LOG_DEBUG("%d,%s", clip->index, clip->url);
            if (clip->index == task->index) {
                YOU_LOG_DEBUG("%s", clip->url);
                strncpy(task->remote_url_buf, clip->url, MAX_URL_LEN);
                break;
            }
        }
        break;
    }
    http_parser_url_init(&task->remote_url);
    http_parser_parse_url(task->remote_url_buf, strlen(task->remote_url_buf), 0, &task->remote_url);
    
    if (task->remote_url.port == 0) {
        task->remote_url.port = 80;
    }
    
    char host[MAX_HOST_LEN] = {0};
    memcpy(host, task->remote_url_buf + task->remote_url.field_data[UF_HOST].off, task->remote_url.field_data[UF_HOST].len);
    YOU_LOG_DEBUG("%s", task->remote_url_buf);
    YOU_LOG_DEBUG("%s", host);
    task->remote = create_http_connection(task->loop, settings, task);
    http_connection_connect(task->remote, host, task->remote_url.port);
}

static vod_task* find_vod_task(const char url[MAX_URL_LEN]) {
    return NULL;
}

vod_task* create_vod_task(uv_loop_t *loop, const char url[MAX_URL_LEN], enum you_media_quality quality, int cur_index, vod_data_read_cb read_cb, vod_data_header_cb header_cb, void *user_data) {
    if (!g_task_init) {
        g_task_init = 1;
        QUEUE_INIT(&g_task_list);
    }
    vod_task *task = (vod_task*)malloc(sizeof(vod_task));
    memset(task, 0, sizeof(vod_task));
    task->state = vod_task_s_parse;
    task->loop = loop;
    task->buf = (char*)malloc(TASK_CACHE_BUF_LEN);
//    task->reference++;
    task->read_cb = read_cb;
    task->header_cb = header_cb;
    task->want_pos = 0;
    task->want_len = 256*1024;
    task->index = cur_index;
    task->user_data = user_data;
    memset(task->buf, 0, TASK_CACHE_BUF_LEN);
    extractor_result *extract = find_extract_result(url);
    if (extract)
        on_extractor_complete(extract, task);
    else
        execute_extractor(loop, url, quality, on_extractor_complete, task);
    return task;
}

int read_vod_data(vod_task *task, int index, uint64_t pos, int len) {
    task->want_pos = pos;
    task->want_len = len;
    http_connection_start_read(task->remote);
    return 0;
}

int stop_vod_task(vod_task *task) {
    if (task->callbacking) {
        task->destroy = 1;
        return 0;
    }
    if (task->state == vod_task_s_parse) {
        task->state = vod_task_s_close;
        return 0;
    }
    free_vod_task(task);
    return 0;
}




#include "task_manager.h"
#include "idata_cache.h"
#include "dispatch.h"
#include "idata_cache_for_dispatch.h"

static QUEUE g_media_task_list = {0};

static int g_media_task_init = 0;

typedef struct {
    void *user_data;
    media_task_meta_info_cb meta_cb;
    QUEUE node;
} media_task_user;

typedef struct media_task {
    int reference;
    char url[MAX_URL_LEN];
    enum you_media_quality quality;
    uv_loop_t *loop;
    void *user_data;
    int callbacking;
    int destroy;
    QUEUE node;
    
    QUEUE user;
    
    dispatch *dis;
    data_cache *cache;
} media_task;

static media_task* find_media_task(const char url[MAX_URL_LEN], enum you_media_quality quality) {
    QUEUE *q;
    media_task *task = NULL;
    QUEUE_FOREACH(q, &g_media_task_list) {
        task = QUEUE_DATA(q, media_task, node);
        if (task->quality==quality && strcmp(task->url, url)==0)
            return task;
    }
    return NULL;
}

static void add_media_task_user(media_task *task, media_task_meta_info_cb meta_cb, void *user) {
    task->reference++;
    media_task_user *u = (media_task_user*)malloc(sizeof(media_task_user));
    memset(u, 0, sizeof(media_task_user));
    u->user_data = user;
    u->meta_cb = meta_cb;
    QUEUE_INIT(&u->node);
    QUEUE_INSERT_TAIL(&task->user, &u->node);
}

media_task* create_media_task(uv_loop_t *loop,
                              const char url[MAX_URL_LEN],
                              const char full_path[MAX_NAME_LEN],
                              enum you_media_quality quality,
                              media_task_meta_info_cb meta_cb,
                              void *user) {
    if (!g_media_task_init) {
        g_media_task_init = 1;
        QUEUE_INIT(&g_media_task_list);
    }
    media_task *task = find_media_task(url, quality);
    if (task) {
        add_media_task_user(task, meta_cb, user);
    } else {
        task = (media_task*)malloc(sizeof(media_task));
        memset(task, 0, sizeof(media_task));
        QUEUE_INIT(&task->user);
        task->loop = loop;
        strncpy(task->url, url, MAX_URL_LEN);
        task->quality = quality;
        add_media_task_user(task, meta_cb, user);
        
        task->media = open_media_cache(loop, full_path, url, task);
        struct dispatch_interface interface = {media_cache_set_clips_num, media_cache_set_filesize, media_cache_write_data, media_cache_undownload_range_queue};
        create_dispatch(loop, url, quality, task->media, interface);
    }
    return task;
}

int media_task_read_data(media_task *task, int index, range rg, media_task_read_cb read_cb, void *user) {
    ASSERT(g_media_task_init);
    return 0;
}

int media_task_get_clips_num(media_task *task, int *num) {
    ASSERT(g_media_task_init);
    return 0;
}

int media_task_get_meta_info(media_task *task, int index, uint64_t *filesize, double *duration) {
    ASSERT(g_media_task_init);
    return 0;
}

int free_media_task(media_task *task, void *user) {
    ASSERT(g_media_task_init);
    return 0;
}
