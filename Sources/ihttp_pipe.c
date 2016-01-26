//
//  ihttp_pipe.c
//  YouPlayCore
//
//  Created by wujianguo on 16/1/21.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#include <stdio.h>
#include "ihttp_pipe.h"
#include "../http-server/http_connection.h"
#include "ipipe_struct.h"
#include <stdlib.h>

typedef struct {
    idata_pipe ipipe;
    ipipe_callback callback;
    idata_cache *cache;
    struct icache_interface_for_pipe cache_interface;
    void *user_data;
    
    char url_buf[MAX_URL_LEN];
    struct http_parser_url url;
    http_connection *conn;

    uint64_t start_pos;
    uint64_t down_pos;
    uv_loop_t *loop;
    
    uint64_t filesize;
    int index;
    
    int callbacking;
    int destroy;
    
    int connecting;
    uv_timer_t timer;
    int timer_start;
}ihttp_pipe;

static ihttp_pipe * cast_from_ipipe(idata_pipe * p) {
    return (ihttp_pipe *)p;
}

static idata_pipe * cast_to_ipipe(ihttp_pipe * p) {
    return (idata_pipe *)p;
}

static ihttp_pipe * cast_from_void(void * p) {
    return (ihttp_pipe *)p;
}

static int ihttp_pipe_update_url(idata_pipe *pipe, const char *url) {
    ihttp_pipe *p = cast_from_ipipe(pipe);
    if (p->connecting) return 0; // todo: fix it.
    memset(p->url_buf, 0, MAX_URL_LEN);
    strncpy(p->url_buf, url, strlen(url));
    http_parser_url_init(&p->url);
    http_parser_parse_url(url, strlen(url), 0, &p->url);
    return 0;
}

static int ihttp_pipe_destory(idata_pipe *pipe) {
    ihttp_pipe *p = cast_from_ipipe(pipe);
    YOU_LOG_DEBUG("http pipe: %p", p);
    if (p->callbacking) {
        p->destroy = 1;
        return 0;
    }
    if (p->timer_start) {
        uv_timer_stop(&p->timer);
    }
    if (p->conn)
        free_http_connection(p->conn);
    free(p);
    return 0;
}

static ipipe_interface g_interface = {
    ihttp_pipe_update_url,
    ihttp_pipe_destory
};


static void on_error(ihttp_pipe *pipe, int err_code) {
    pipe->callback.on_error(cast_to_ipipe(pipe), err_code, pipe->user_data);
}


static void on_remote_error(http_connection *conn, void *user_data, int err_code) {
    ihttp_pipe *pipe = cast_from_void(user_data);
    YOU_LOG_DEBUG("http pipe:%p", pipe);
    on_error(pipe, err_code);
}

static int send_request(http_connection *conn, ihttp_pipe *pipe) {
    pipe->connecting = 0;
    
    int size = 0;
    if(!pipe->cache_interface.can_download_more(pipe->cache, pipe->index, &size))
        return 1;

    
    const char format[MAX_REQUEST_HEADER_LEN] = "GET %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Connection: keep-alive\r\n"
    "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
    "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_11_2) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/47.0.2526.106 Safari/537.36\r\n"
    "Accept-Encoding: gzip, deflate\r\n"
    "Accept-Language: zh-CN,zh;q=0.8,en-US;q=0.6,en;q=0.4\r\n"
    "Range: bytes=%llu-%d\r\n"
    "\r\n";
    
    char host[MAX_HOST_LEN] = {0};
    memcpy(host, pipe->url_buf + pipe->url.field_data[UF_HOST].off, pipe->url.field_data[UF_HOST].len);
    
    char path[MAX_URL_LEN] = {0};
    memcpy(path, pipe->url_buf + pipe->url.field_data[UF_PATH].off, strlen(pipe->url_buf) - pipe->url.field_data[UF_PATH].off);
    
    char header[MAX_REQUEST_HEADER_LEN] = {0};
    snprintf(header, MAX_REQUEST_HEADER_LEN, format, path, host, pipe->start_pos, size);
    // todo: check if there is a bug about size, especially when drag.
    
    http_connection_send(conn, header, strlen(header));
    return 1;
}

static void on_remote_connect(http_connection *conn, void *user_data) {
    ihttp_pipe *pipe = cast_from_void(user_data);
    YOU_LOG_DEBUG("http pipe:%p", pipe);
    send_request(conn, pipe);
}

static void on_remote_send(http_connection *conn, void *user_data) {
    
}

static void start_connect_to(uv_loop_t *loop, ihttp_pipe *pipe, const char *url);

static void on_remote_header_complete(http_connection *conn, struct http_header *header, void *user_data) {
    ihttp_pipe *pipe = cast_from_void(user_data);
    YOU_LOG_DEBUG("http pipe:%p, content-length:%llu", pipe, header->parser.content_length);
    if (header->parser.status_code == 302) {
        QUEUE *q;
        struct http_header_field_value *head;
        QUEUE_FOREACH(q, &header->headers) {
            head = QUEUE_DATA(q, struct http_header_field_value, node);
            if (strcmp(head->field, "Location") == 0) {
                start_connect_to(pipe->loop, pipe, head->value);
            }
        }
        free_http_connection(conn);
        return;
    }
    if (header->parser.status_code != 200 || header->parser.status_code != 206) {
        pipe->callback.on_error(cast_to_ipipe(pipe), header->parser.status_code, pipe->user_data);
        return;
    }
    pipe->filesize = header->parser.content_length;
    pipe->cache_interface.set_filesize(pipe->cache, pipe->index, header->parser.content_length);
}

static void on_remote_body(http_connection *conn, const char *at, size_t length, void *user_data) {
    ihttp_pipe *pipe = cast_from_void(user_data);
    YOU_LOG_DEBUG("http pipe:%p", pipe);
    range rg = {pipe->down_pos, (uint64_t)length};
    pipe->down_pos += length;

    pipe->callbacking = 1;
    pipe->cache_interface.write_data(pipe->cache, pipe->index, rg, at);
    pipe->callbacking = 0;
    
    if (pipe->destroy) {
        ihttp_pipe_destory(cast_to_ipipe(pipe));
        return;
    }
}

static void on_timer_expire(uv_timer_t *handle) {
    ihttp_pipe *pipe = CONTAINER_OF(handle, ihttp_pipe, timer);
    http_connection *conn = pipe->conn;
    if (send_request(conn, pipe)) {
        pipe->timer_start = 0;
        uv_timer_stop(&pipe->timer);
    }

}

static void on_remote_message_complete(http_connection *conn, void *user_data) {
    ihttp_pipe *pipe = cast_from_void(user_data);
    YOU_LOG_DEBUG("http pipe:%p", pipe);
    
//    pipe->callback.on_complete(cast_to_ipipe(pipe), pipe->user_data);

    range r = {0, 0};
    pipe->cache_interface.downloaded_range(pipe->cache, pipe->index, &r);
    uint64_t filesize = pipe->cache_interface.get_filesize(pipe->cache, pipe->index);
    
    if (RANGE_END(r) == filesize) {
        pipe->callback.on_complete(cast_to_ipipe(pipe), pipe->user_data);
        return;
    }
    if (!send_request(conn, pipe)) {
        pipe->timer_start = 1;
        uv_timer_start(&pipe->timer, on_timer_expire, 500, 1);
    }
}

static struct http_connection_settings g_http_connection_settings = {
    on_remote_error,
    on_remote_connect,
    on_remote_send,
    on_remote_header_complete,
    on_remote_body,
    on_remote_message_complete,
    NULL,
    NULL
};


static void start_connect_to(uv_loop_t *loop, ihttp_pipe *pipe, const char *url) {
    memset(pipe->url_buf, 0, MAX_URL_LEN);
    strncpy(pipe->url_buf, url, strlen(url));
    http_parser_url_init(&pipe->url);
    http_parser_parse_url(url, strlen(url), 0, &pipe->url);
    
    ASSERT(pipe->url.field_set & 1<<UF_HOST);
    
    if (pipe->url.port == 0)
        pipe->url.port = 80;
    
    char host[MAX_HOST_LEN] = {0};
    memcpy(host, url + pipe->url.field_data[UF_HOST].off, pipe->url.field_data[UF_HOST].len);
    memcpy(pipe->url_buf, url, strlen(url));
    
    pipe->conn = create_http_connection(loop, g_http_connection_settings, pipe);
    pipe->connecting = 1;
    http_connection_connect(pipe->conn, host, pipe->url.port);
}

idata_pipe* ihttp_pipe_create(uv_loop_t *loop,
                              ipipe_callback callback,
                              struct icache_interface_for_pipe cache_interface,
                              idata_cache *cache,
                              const char *url,
                              int index,
                              uint64_t start_pos,
                              void *user_data) {
    ihttp_pipe *pipe = (ihttp_pipe*)malloc(sizeof(ihttp_pipe));
    memset(pipe, 0, sizeof(ihttp_pipe));
    pipe->ipipe.interface = &g_interface;
    pipe->callback = callback;
    pipe->cache_interface = cache_interface;
    pipe->cache = cache;
    pipe->user_data = user_data;
    
    pipe->start_pos = start_pos;
    pipe->loop = loop;
    pipe->index = index;
    uv_timer_init(loop, &pipe->timer);
    
    start_connect_to(loop, pipe, url);
    return cast_to_ipipe(pipe);
}