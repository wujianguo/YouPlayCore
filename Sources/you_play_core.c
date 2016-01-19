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
#include "you_parser.h"
#include "media_handler.h"
#include "meta_handler.h"
#include "extractor.h"

typedef struct {
    char parser_script_url[MAX_URL_LEN];
    unsigned short service_port;
    unsigned short parser_port;
    uv_loop_t *loop;
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
        setting.on_error = custom_handler##_on_error;                       \
        QUEUE_INIT(&setting.node);                                          \
        QUEUE_INSERT_TAIL(queue, &setting.node);                            \
    }                                                                       \
    while(0)



static void* you_play_service_thread(void* params) {
    http_server_config config;
    
    memset(&config, 0, sizeof(config));
    config.bind_host = "127.0.0.1";
    config.bind_port = sv.service_port;
    config.idle_timeout = 30*1000;
    
    QUEUE_INIT(&config.handlers);
    HTTP_SERVER_ADD_HANDLER(&config.handlers, "/media", media_handler);
    HTTP_SERVER_ADD_HANDLER(&config.handlers, "/meta", meta_handler);
    
    uv_loop_t *loop = sv.loop;
    if (loop == NULL) {
        loop = (uv_loop_t*)malloc(sizeof(uv_loop_t));
        memset(loop, 0, sizeof(uv_loop_t));
        uv_loop_init(loop);
    }

//    start_you_parser_service(&loop, sv.parser_script_url, sv.parser_port, NULL);
    http_server_run(&config, loop);

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
                            unsigned short parser_port,
                            uv_loop_t *loop)
{
    memcpy(sv.parser_script_url, parser_script_url, strlen(parser_script_url));
    sv.service_port = service_port;
    sv.parser_port = parser_port;
    sv.loop = loop;
    you_play_service_thread(NULL);
}


int create_download_task(char *url, size_t url_len, enum you_media_quality quality, int *task_id) {
    return 0;
}

int resume_download_task(int task_id) {
    return 0;
}

int pause_download_task(int task_id) {
    return 0;
}

int delete_download_task(int task_id) {
    return 0;
}

