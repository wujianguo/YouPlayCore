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


const static char error_response_http[] = "HTTP/1.1 200 OK\r\n"
"Server: YouSir/2\r\n"
"Content-Type: application/json\r\n"
"Content-Length: 43\r\n"
"\r\n"
"{\"msg\":\"request error\", \"err\":1, \"data\":{}}";


enum meta_response_state {
    meta_response_s_idle,
    meta_response_s_parse
};

typedef struct {
    char video_url[MAX_URL_LEN];
    http_connection *remote;
    http_request *req;
} meta_request;

static void response_request_error(http_request *req) {
    http_connection_send(req->conn, error_response_http, strlen(error_response_http));
}


static void on_connect(http_connection *conn, void *user_data) {
    meta_request *meta = (meta_request*)user_data;
    const char format[] = "GET /api/v1/videos/%s?meta=1 HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Connection: close\r\n"
    "Cache-Control: no-cache\r\n"
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
    "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_11_2) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/47.0.2526.106 Safari/537.36\r\n"
    "Accept-Encoding: gzip, deflate\r\n"
    "Accept-Language: zh-CN,zh;q=0.8,en-US;q=0.6,en;q=0.4\r\n"
    "\r\n";
    
    char header[MAX_REQUEST_HEADER_LEN] = {0};
    snprintf(header, MAX_REQUEST_HEADER_LEN, format, meta->video_url, parser_service_host());
    YOU_LOG_DEBUG("%s", header);
    http_connection_send(conn, header, strlen(header));
}

static void on_send(http_connection *conn, void *user_data) {
    YOU_LOG_DEBUG("");
}

static void on_header_complete(http_connection *conn, struct http_header *header, void *user_data) {
    YOU_LOG_DEBUG("");
}

static void on_body(http_connection *conn, const char *at, size_t length, void *user_data) {
    size_t out_len = 1024*4;
    char *s = (char*)malloc(out_len);
    memset(s, 0, out_len);
    YOU_LOG_DEBUG("%lu", length);
    int ret = httpgzdecompress(at, length, s, &out_len);
    if (ret == 0) {
        YOU_LOG_DEBUG("%s", s);
    } else {
        YOU_LOG_ERROR("%d", ret);
    }
}

static void on_message_complete(http_connection *conn, void *user_data) {
    free_http_connection(conn);
}

static struct http_connection_settings settings = {
    on_connect,
    on_send,
    on_header_complete,
    on_body,
    on_message_complete
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
    memcpy(meta->video_url, req->header->url_buf+off, len);
    
//    base64_decodestate decode = {0};
//    base64_init_decodestate(&decode);
//    base64_decode_block(req->header->url_buf+off, (int)len, meta->video_url, &decode);

    meta->remote = create_http_connection(req->loop, settings, meta);
    http_connection_connect(meta->remote, parser_service_host(), parser_service_port());
}

void meta_handler_on_send(http_request *req) {
    req->complete(req);
    
}
