//
//  media_handler.c
//  YouPlayCore
//
//  Created by wujianguo on 16/1/1.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#include "media_handler.h"
#include "../http-server/utility.h"
#include <stdlib.h>
#include "task_manager.h"

#define MAX_BODY_LEN (16*1024)

typedef struct {
    vod_task *task;
    http_request *req;
    int index;
    uint64_t pos;
} media_request;

static char *http_status_name(int code) {
    if (code == 200)
        return "OK";
    else if (code == 206)
        return "Partial Content";
    else if (code == 403)
        return "Forbidden";
    else
        return "OK";
}

static void free_media_request(media_request *media) {
    stop_vod_task(media->task);
    media->req->complete(media->req);
    free(media);
}

static void on_vod_data_read(vod_task *task, const char *buf, size_t len, void *user_data) {
    media_request *media = (media_request*)user_data;
    if (buf==NULL) {
        free_media_request(media);
        return;
    }
    media->pos += len;
    http_connection_send(media->req->conn, buf, len);
}

static void on_vod_data_header(vod_task *task, struct http_header *header, void *user_data) {
    char format[] = "HTTP/%d.%d %d %s\r\n";
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
    struct http_header_field_value *head;
    QUEUE_FOREACH(q, &header->headers) {
        head = QUEUE_DATA(q, struct http_header_field_value, node);
        strcat(response_header, head->field);
        strcat(response_header, ": ");
        strcat(response_header, head->value);
        strcat(response_header, "\r\n");
    }
    strcat(response_header, "\r\n");
    YOU_LOG_DEBUG("%s", response_header);
    media_request *media = (media_request*)user_data;
    http_connection_send(media->req->conn, response_header, strlen(response_header));
    
    if (header->parser.status_code == 302) {
        free_media_request(media);
    }
}

void media_handler_on_error(http_request *req, int err_code) {
    media_request *media = (media_request*)req->user_data;
    free_media_request(media);
}

void media_handler_on_send(http_request *req) {
    media_request *media = (media_request*)req->user_data;
    read_vod_data(media->task, media->index, media->pos, 256*1024);
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
    int index = atoi(index_str);
    media->index = index;
    req->user_data = media;
    
    media->task = create_vod_task(req->loop, url_base64, you_media_quality_1080p, index, on_vod_data_read, on_vod_data_header, media);
}

