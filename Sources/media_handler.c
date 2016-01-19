//
//  media_handler.c
//  YouPlayCore
//
//  Created by wujianguo on 16/1/1.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#include "media_handler.h"
#include "extractor.h"
#include "../http-server/utility.h"
#include <stdlib.h>

#define MAX_BODY_LEN (16*1024)

enum media_state {
    media_s_idle,
    media_s_parse,
    media_s_remote_connecting,
    media_s_proxy,
    media_s_close
};


typedef struct {
    enum media_state state;
    http_connection *remote;
    http_request *req;
    int index;
    char remote_url_buf[MAX_URL_LEN];
    struct http_parser_url remote_url;
} media_request;


static void on_remote_send(http_connection *conn, void *user_data) {
    media_request *media = (media_request*)user_data;
    if (media->state != media_s_remote_connecting) {
        ASSERT(media->state==media_s_proxy);
        return;
    }
    media->state = media_s_proxy;
    char range[MAX_HTTP_VALUE_LEN] = {0};
    QUEUE *q = NULL;
    QUEUE_FOREACH(q, &media->req->header->headers) {
        struct http_header_field_value *head = QUEUE_DATA(q, struct http_header_field_value, node);
        if (strcmp(head->field, "Range")==0)
            strncpy(range, head->value, MAX_HTTP_VALUE_LEN);
    }
    const char format[] = "GET %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Connection: keep-alive\r\n"
    "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
    "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_11_2) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/47.0.2526.106 Safari/537.36\r\n"
    "Accept-Encoding: gzip, deflate\r\n"
    "Accept-Language: zh-CN,zh;q=0.8,en-US;q=0.6,en;q=0.4\r\n"
    "Range: %s\r\n"
    "\r\n";
    
    char host[MAX_HOST_LEN] = {0};
    memcpy(host, media->remote_url_buf + media->remote_url.field_data[UF_HOST].off, media->remote_url.field_data[UF_HOST].len);
    
    char path[MAX_URL_LEN] = {0};
    memcpy(path, media->remote_url_buf + media->remote_url.field_data[UF_PATH].off, strlen(media->remote_url_buf) - media->remote_url.field_data[UF_PATH].off);

    char header[MAX_REQUEST_HEADER_LEN] = {0};
    snprintf(header, MAX_REQUEST_HEADER_LEN, format, path, host, range);
    http_connection_send(conn, header, strlen(header));
}

static char *http_status_name(int code) {
    if (code == 200)
        return "OK";
    else if (code == 206)
        return "Partial Content";
    else
        return "OK";
}

static void on_remote_header_complete(http_connection *conn, struct http_header *header, void *user_data) {
    char format[] = "HTTP/%d.%d %d %s\4\n";
    char response_header[MAX_RESPONSE_HEADER_LEN] = {0};
    
    snprintf(response_header,
             MAX_RESPONSE_HEADER_LEN,
             format,
             header->parser.http_major,
             header->parser.http_minor,
             header->parser.status_code,
             http_status_name(header->parser.status_code)
             );
    
    QUEUE *q = NULL;
    QUEUE_FOREACH(q, &header->headers) {
        struct http_header_field_value *head = QUEUE_DATA(q, struct http_header_field_value, node);
        strncpy(response_header+strlen(response_header), head->field, MAX_RESPONSE_HEADER_LEN);
        strncpy(response_header+strlen(response_header), ": ", MAX_RESPONSE_HEADER_LEN);
        strncpy(response_header+strlen(response_header), head->value, MAX_RESPONSE_HEADER_LEN);
        strncpy(response_header+strlen(response_header), "\r\n", MAX_RESPONSE_HEADER_LEN);
    }
    strncpy(response_header+strlen(response_header), "\r\n", MAX_RESPONSE_HEADER_LEN);
    YOU_LOG_DEBUG("%s", response_header);
    media_request *media = (media_request*)user_data;
    http_connection_send(media->req->conn, response_header, strlen(response_header));
}

static void on_remote_body(http_connection *conn, const char *at, size_t length, void *user_data) {
    media_request *media = (media_request*)user_data;
    http_connection_send(media->req->conn, at, length);
}

static void on_remote_message_complete(http_connection *conn, void *user_data) {

}

static struct http_connection_settings settings = {
    NULL,
    NULL,
    on_remote_send,
    on_remote_header_complete,
    on_remote_body,
    on_remote_message_complete,
    NULL,
    NULL
};



static void on_extractor_complete(extractor_result *ret, void *user_data) {
    media_request *media = (media_request*)user_data;
    if (media->state == media_s_close) {
        media->req->complete(media->req);
    } else if (media->state == media_s_parse) {
        media->state = media_s_remote_connecting;
        QUEUE *q = NULL;
        QUEUE_FOREACH(q, &ret->qualities) {
            extract_quality_item *quality = QUEUE_DATA(q, extract_quality_item, node);
            QUEUE *q2 = NULL;
            QUEUE_FOREACH(q2, &quality->clips) {
                extract_clips *clip = QUEUE_DATA(q2, extract_clips, node);
                if (clip->index == media->index) {
                    strncpy(media->remote_url_buf, clip->url, MAX_URL_LEN);
                    break;
                }
            }
            break;
        }
        http_parser_url_init(&media->remote_url);
        http_parser_parse_url(media->remote_url_buf, strlen(media->remote_url_buf), 0, &media->remote_url);

        if (media->remote_url.port == 0) {
            media->remote_url.port = 80;
        }
        
        char host[MAX_HOST_LEN] = {0};
        memcpy(host, media->remote_url_buf + media->remote_url.field_data[UF_HOST].off, media->remote_url.field_data[UF_HOST].len);
        YOU_LOG_DEBUG("%s", media->remote_url_buf);
        YOU_LOG_DEBUG("%s", host);
        http_connection *conn = create_http_connection(media->req->loop, settings, media);
        http_connection_connect(conn, host, media->remote_url.port);
        
    } else {
        UNREACHABLE();
    }
}

void media_handler_on_error(http_request *req, int err_code) {
    media_request *media = (media_request*)req->user_data;
    if (media->state == media_s_parse) {
        media->state = media_s_close;
    } else {
        media->req->complete(media->req);
    }
}


void media_handler_on_header_complete(http_request *req) {
    
}

void media_handler_on_body(http_request *req, const char *at, size_t length) {
    
}

void media_handler_on_message_complete(http_request *req) {
    size_t off = 0;
    size_t len = 0;
    get_query_argument(&req->header->url, req->header->url_buf, "url", strlen("url"), &off, &len);
    if (len == 0) {
        ASSERT(0);
        return;
    }
    
    media_request *media = (media_request*)malloc(sizeof(media_request));
    memset(media, 0, sizeof(media_request));
    media->req = req;
    
    char url_base64[MAX_URL_LEN] = {0};
    memcpy(url_base64, req->header->url_buf+off, len);
    
    
    get_query_argument(&req->header->url, req->header->url_buf, "index", strlen("index"), &off, &len);
    char index_str[MAX_URL_LEN] = {0};
    memcpy(index_str, req->header->url_buf+off, len);
    media->index = atoi(index_str);
    media->state = media_s_parse;
    req->user_data = media;
    
    execute_extractor(req->loop, url_base64, you_media_quality_720p, on_extractor_complete, media);
}

void media_handler_on_send(http_request *req) {

}
