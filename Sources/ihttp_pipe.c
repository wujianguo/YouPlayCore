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

static int ihttp_pipe_destory(idata_pipe *pipe) {
    return 0;
}

static ipipe_interface g_interface = {
    ihttp_pipe_destory
};


static void on_error(ihttp_pipe *pipe) {

}


static void on_remote_error(http_connection *conn, void *user_data, int err_code) {
    ihttp_pipe *pipe = (ihttp_pipe*)user_data;
    YOU_LOG_DEBUG("http pipe:%p", pipe);
    on_error(pipe);
}

static void on_remote_connect(http_connection *conn, void *user_data) {
    ihttp_pipe *pipe = (ihttp_pipe*)user_data;
    YOU_LOG_DEBUG("http pipe:%p", pipe);

}

static void on_remote_send(http_connection *conn, void *user_data) {
    
}

static void on_remote_header_complete(http_connection *conn, struct http_header *header, void *user_data) {
    ihttp_pipe *pipe = (ihttp_pipe*)user_data;
    YOU_LOG_DEBUG("http pipe:%p", pipe);

}

static void on_remote_body(http_connection *conn, const char *at, size_t length, void *user_data) {
    ihttp_pipe *pipe = (ihttp_pipe*)user_data;
    YOU_LOG_DEBUG("http pipe:%p", pipe);
}

static void on_remote_message_complete(http_connection *conn, void *user_data) {
    ihttp_pipe *pipe = (ihttp_pipe*)user_data;
    YOU_LOG_DEBUG("http pipe:%p", pipe);

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


idata_pipe* ihttp_pipe_create(ipipe_callback callback,
                              struct idata_cache_interface cache_interface,
                              idata_cache *cache,
                              const char *url,
                              range rg,
                              void *user_data) {
    ihttp_pipe *pipe = (ihttp_pipe*)malloc(sizeof(ihttp_pipe));
    memset(pipe, 0, sizeof(ihttp_pipe));
    pipe->ipipe.interface = &g_interface;
    return cast_to_ipipe(pipe);
}