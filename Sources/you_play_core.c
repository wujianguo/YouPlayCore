//
//  you_play_core.c
//  YouPlayCore
//
//  Created by wujianguo on 16/1/1.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#include "you_play_core.h"
#include <stdlib.h>
#include <pthread.h>
#include "../http-server/http_server.h"
#include "you_parser.h"

typedef struct {
    char parser_script_url[MAX_URL_LEN];
    unsigned short service_port;
    unsigned short parser_port;
} you_play_service;

static you_play_service sv = {0};


#define HTTP_SERVER_ADD_HANDLER(queue, custom_path, custom_handler)         \
    do {                                                                    \
        http_handler_setting setting = {0};                                 \
        strncpy(setting.path, custom_path, sizeof(setting.path));           \
        setting.on_send = custom_handler##_on_send;                         \
        setting.on_body = custom_handler##_on_body;                         \
        setting.on_message_complete = custom_handler##_on_message_complete; \
        setting.on_header_complete = custom_handler##_on_header_complete;   \
        QUEUE_INIT(&setting.node);                                          \
        QUEUE_INSERT_TAIL(queue, &setting.node);                            \
    }                                                                       \
    while(0)


const static char default_http[] = "HTTP/1.1 200 OK\r\n"
"Server: YouSir/2\r\n"
"Content-type: text/plain\r\n"
"Content-Length: 5\r\n"
"\r\n"
"hello";

// /
static void root_handler_on_header_complete(http_request *req) {
    
}

static void root_handler_on_body(http_request *req, const char *at, size_t length) {
    
}

static void root_handler_on_message_complete(http_request *req) {
    http_connection_send(req->conn, default_http, strlen(default_http));
}

static void root_handler_on_send(http_request *req) {
    req->complete(req);
}


static void* you_play_service_thread(void* params) {
    http_server_config config;
    
    memset(&config, 0, sizeof(config));
    config.bind_host = "127.0.0.1";
    config.bind_port = sv.service_port;
    config.idle_timeout = 30*1000;
    
    QUEUE_INIT(&config.handlers);
    HTTP_SERVER_ADD_HANDLER(&config.handlers, "/", root_handler);
    
    uv_loop_t loop = {0};
    uv_loop_init(&loop);
    start_you_parser(&loop, sv.parser_script_url, sv.parser_port, NULL);
    
    http_server_run(&config, &loop);

    return NULL;
}

void start_you_play_service_in_new_thread(unsigned short service_port,
                                          const char parser_script_url[1024],
                                          unsigned short parser_port)
{
    memcpy(sv.parser_script_url, parser_script_url, strlen(parser_script_url));
    sv.service_port = service_port;
    sv.parser_port = parser_port;
    pthread_t tid;
    pthread_create(&tid, NULL, you_play_service_thread, NULL);

}

void start_you_play_service(unsigned short service_port,
                            const char parser_script_url[1024],
                            unsigned short parser_port)
{
    memcpy(sv.parser_script_url, parser_script_url, strlen(parser_script_url));
    sv.service_port = service_port;
    sv.parser_port = parser_port;
    you_play_service_thread(NULL);
}