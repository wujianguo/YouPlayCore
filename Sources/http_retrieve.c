//
//  http_retrieve.c
//  YouPlayCore
//
//  Created by 吴建国 on 16/1/18.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#include "http_retrieve.h"
#include <stdlib.h>
#include "gzip.h"
#include "../http-server/utility.h"

#define MAX_BODY_LEN (128*1024)

typedef struct http_retrieve {
    char url_buf[MAX_URL_LEN];
    struct http_parser_url url;
    http_connection *conn;
    char remote_response_data[MAX_BODY_LEN];
    size_t remote_response_off;
    struct http_header *response_header;
    http_retrieve_complete_cb complete_cb;
    http_retrieve_error_cb error_cb;
    void *user_data;
    int callbacking;
    int destroy;
}http_retrieve;


static void on_connect(http_connection *conn, void *user_data) {
    http_retrieve *retrieve = (http_retrieve*)user_data;
    
    const char format[] = "GET %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Connection: close\r\n"
    "Cache-Control: no-cache\r\n"
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
    "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_11_2) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/47.0.2526.106 Safari/537.36\r\n"
    "Accept-Encoding: gzip, deflate\r\n"
    "Accept-Language: zh-CN,zh;q=0.8,en-US;q=0.6,en;q=0.4\r\n"
    "\r\n";
    
    char host[MAX_HOST_LEN] = {0};
    memcpy(host, retrieve->url_buf + retrieve->url.field_data[UF_HOST].off, retrieve->url.field_data[UF_HOST].len);
    
    char path[MAX_URL_LEN] = {0};
    memcpy(path, retrieve->url_buf + retrieve->url.field_data[UF_PATH].off, strlen(retrieve->url_buf) - retrieve->url.field_data[UF_PATH].off);
    
    char header[MAX_REQUEST_HEADER_LEN] = {0};
    snprintf(header, MAX_REQUEST_HEADER_LEN, format, path, host);
    http_connection_send(conn, header, strlen(header));
}

static void on_error(http_connection *conn, void *user_data, int err_code) {
    http_retrieve *retrieve = (http_retrieve*)user_data;
    retrieve->callbacking = 1;
    if (retrieve->error_cb)
        retrieve->error_cb(retrieve, err_code, retrieve->user_data);
    retrieve->callbacking = 0;
    if (retrieve->destroy)
        free_http_retrieve(retrieve);

}

static void on_send(http_connection *conn, void *user_data) {

}

static void on_header_complete(http_connection *conn, struct http_header *header, void *user_data) {
    http_retrieve *retrieve = (http_retrieve*)user_data;
    retrieve->response_header = header;
}

static void on_body(http_connection *conn, const char *at, size_t length, void *user_data) {
    http_retrieve *retrieve = (http_retrieve*)user_data;
    ASSERT(retrieve->remote_response_off+length<=MAX_BODY_LEN);
    memcpy(retrieve->remote_response_data+retrieve->remote_response_off, at, length);
    retrieve->remote_response_off+=length;
}

static void on_message_complete(http_connection *conn, void *user_data) {
    http_retrieve *retrieve = (http_retrieve*)user_data;
    if (is_response_content_encoding_gzip(retrieve->response_header)) {
        size_t out_len = MAX_BODY_LEN;
        char *s = (char*)malloc(out_len);
        memset(s, 0, out_len);
        int ret = httpgzdecompress(retrieve->remote_response_data, retrieve->remote_response_off, s, &out_len);
        if (ret == 0) {
            retrieve->callbacking = 1;
            if (retrieve->complete_cb)
                retrieve->complete_cb(retrieve, s, out_len, retrieve->user_data);
            free(s);
            retrieve->callbacking = 0;
            if (retrieve->destroy)
                free_http_retrieve(retrieve);
            return;
        }
        free(s);
    }
    retrieve->callbacking = 1;
    if (retrieve->complete_cb)
        retrieve->complete_cb(retrieve, retrieve->remote_response_data, retrieve->remote_response_off, retrieve->user_data);
    retrieve->callbacking = 0;
    if (retrieve->destroy)
        free_http_retrieve(retrieve);
}

static struct http_connection_settings settings = {
    on_error,
    on_connect,
    on_send,
    on_header_complete,
    on_body,
    on_message_complete,
    NULL,
    NULL
};

http_retrieve* create_http_retrieve(uv_loop_t *loop, const char url[MAX_URL_LEN], http_retrieve_complete_cb complete_cb, http_retrieve_error_cb error_cb, void *user_data) {
    http_retrieve *retrieve = (http_retrieve*)malloc(sizeof(http_retrieve));
    memset(retrieve, 0, sizeof(http_retrieve));
    retrieve->conn = create_http_connection(loop, settings, retrieve);
    strncpy(retrieve->url_buf, url, MAX_URL_LEN);
    retrieve->complete_cb = complete_cb;
    retrieve->error_cb = error_cb;
    retrieve->user_data = user_data;
    http_parser_url_init(&retrieve->url);
    http_parser_parse_url(url, strlen(url), 0, &retrieve->url);
    ASSERT(retrieve->url.field_set & 1<<UF_HOST);
    
    if (retrieve->url.port == 0)
        retrieve->url.port = 80;
    
    char host[MAX_HOST_LEN] = {0};
    memcpy(host, url + retrieve->url.field_data[UF_HOST].off, retrieve->url.field_data[UF_HOST].len);
    memcpy(retrieve->url_buf, url, strlen(url));
    
    http_connection_connect(retrieve->conn, host, retrieve->url.port);
    return retrieve;
}

void free_http_retrieve(http_retrieve *retrieve) {
    if (retrieve->callbacking) {
        retrieve->destroy = 1;
        return;
    }
    free_http_connection(retrieve->conn);
    free(retrieve);
}
