//
//  meta_handler.c
//  YouPlayCore
//
//  Created by 吴建国 on 16/1/7.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

/**

http://host:port/meta?url=ssss
 
*/

#include "meta_handler.h"
#include "../http-server/utility.h"
#include "cdecode.h"
#include <stdlib.h>
#include "you_parser.h"
#include "gzip.h"


enum meta_response_state {
    meta_response_s_idle,
    meta_response_s_parse
};

#define MAX_BODY_LEN (16*1024)
typedef struct {
    char video_url[MAX_URL_LEN];
    int meta;
    char quality[MAX_URL_LEN];
    http_connection *remote;
    char remote_response_data[MAX_BODY_LEN];
    size_t remote_response_off;
    http_request *req;
} meta_request;

static void response_request_error(http_request *req) {
    char error_response_http[] = "HTTP/1.1 200 OK\r\n"
    "Server: YouSir/2\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: 43\r\n"
    "\r\n"
    "{\"msg\":\"request error\", \"err\":1, \"data\":{}}";
    
    http_connection_send(req->conn, error_response_http, strlen(error_response_http));
}

static void response_data(http_request *req, char *data, size_t len) {
    char response_data_format[] = "HTTP/1.1 200 OK\r\n"
    "Server: YouSir/2\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: %zu\r\n"
    "\r\n"
    "%s";

    char response[MAX_BODY_LEN] = {0};
    snprintf(response, MAX_BODY_LEN, response_data_format, len, data);
    http_connection_send(req->conn, response, strlen(response));
}


static void on_connect(http_connection *conn, void *user_data) {
    meta_request *meta = (meta_request*)user_data;
    const char format[] = "GET /api/v1/videos/%s?meta=%d&quality=%s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Connection: close\r\n"
    "Cache-Control: no-cache\r\n"
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
    "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_11_2) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/47.0.2526.106 Safari/537.36\r\n"
    "Accept-Encoding: gzip, deflate\r\n"
    "Accept-Language: zh-CN,zh;q=0.8,en-US;q=0.6,en;q=0.4\r\n"
    "\r\n";
    
    char header[MAX_REQUEST_HEADER_LEN] = {0};
    snprintf(header, MAX_REQUEST_HEADER_LEN, format, meta->video_url, meta->meta, meta->quality, parser_service_host());
    YOU_LOG_DEBUG("%s", header);
    http_connection_send(conn, header, strlen(header));
}

static void on_send(http_connection *conn, void *user_data) {
    YOU_LOG_DEBUG("");
}

static void on_header_complete(http_connection *conn, struct http_header *header, void *user_data) {

}

static void on_body(http_connection *conn, const char *at, size_t length, void *user_data) {
    meta_request *meta = (meta_request*)user_data;
    ASSERT(meta->remote_response_off+length<=MAX_BODY_LEN);
    memcpy(meta->remote_response_data+meta->remote_response_off, at, length);
    meta->remote_response_off+=length;
}

static void on_message_complete(http_connection *conn, void *user_data) {
    meta_request *meta = (meta_request*)user_data;
    size_t out_len = MAX_BODY_LEN;
    char *s = (char*)malloc(out_len);
    memset(s, 0, out_len);
    int ret = httpgzdecompress(meta->remote_response_data, meta->remote_response_off, s, &out_len);
    if (ret == 0) {
        response_data(meta->req, s, out_len);
    } else {
        response_request_error(meta->req);
    }
    free(s);
    free_http_connection(conn);
}

static struct http_connection_settings settings = {
    NULL,
    on_connect,
    on_send,
    on_header_complete,
    on_body,
    on_message_complete,
    NULL,
    NULL
};

void meta_handler_on_header_complete(http_request *req) {
    
}

void meta_handler_on_body(http_request *req, const char *at, size_t length) {
    
}



void meta_handler_on_message_complete(http_request *req) {
    size_t off = 0;
    size_t len = 0;
    get_query_argument(&req->header->url, req->header->url_buf, "url", strlen("url"), &off, &len);
    if (len == 0) {
        response_request_error(req);
        return;
    }
    meta_request *meta = (meta_request*)malloc(sizeof(meta_request));
    memset(meta, 0, sizeof(meta_request));
    meta->req = req;
    req->user_data = meta;
    memcpy(meta->video_url, req->header->url_buf+off, len);

    get_query_argument(&req->header->url, req->header->url_buf, "meta", strlen("meta"), &off, &len);
    if (len != 1) {
        meta->meta = 0;
    } else if (req->header->url_buf[off] == '0') {
        meta->meta = 0;
    } else {
        meta->meta = 1;
    }

    get_query_argument(&req->header->url, req->header->url_buf, "quality", strlen("quality"), &off, &len);
    memcpy(meta->quality, req->header->url_buf+off, len);

    
//    base64_decodestate decode = {0};
//    base64_init_decodestate(&decode);
//    base64_decode_block(req->header->url_buf+off, (int)len, meta->video_url, &decode);

    meta->remote = create_http_connection(req->loop, settings, meta);
    http_connection_connect(meta->remote, parser_service_host(), parser_service_port());
}

void meta_handler_on_send(http_request *req) {
    free(req->user_data);
    req->complete(req);
    
}

void meta_handler_on_error(http_request *req, int err_code) {
    
}

