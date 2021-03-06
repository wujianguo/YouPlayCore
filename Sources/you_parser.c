//
//  you_parser.c
//  YouPlayCore
//
//  Created by wujianguo on 16/1/1.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#include "you_parser.h"
#include "../http-server/http_connection.h"
#include "../http-server/http-parser/http_parser.h"
#include <stdlib.h>
#include <pthread.h>
#include <Python/Python.h>

typedef struct {
    char url_buf[MAX_URL_LEN];
    struct http_parser_url url;
    
    int parser_port;
    
    you_parser_ready_cb complete;
    
    char *script;
    size_t pos;
    
} you_parser_struct;


static void* python_script_thread(void* params) {
    you_parser_struct *y = (you_parser_struct*)params;
    // todo: notify server start
    Py_Initialize();
    PyObject *p_code = Py_CompileString(y->script, "you_parser", Py_file_input);
    if (p_code) {
        PyObject *p_module = PyImport_ExecCodeModule("you_parser", p_code);
        if (p_module) {
            PyObject *p_func = PyObject_GetAttrString(p_module, "start_server");
            if (p_func && PyCallable_Check(p_func)) {
                PyObject *p_port = PyInt_FromLong(y->parser_port);
                PyObject *p_args = PyTuple_New(1);
                PyTuple_SetItem(p_args, 0, p_port);
                PyObject_CallObject(p_func, p_args);
                
                Py_DecRef(p_port);
                Py_DecRef(p_args);
            }
            Py_XDECREF(p_func);
            Py_DecRef(p_module);
        }
        Py_DecRef(p_code);
    }
    
    Py_Finalize();
    free(y->script);
    free(y);
    YOU_LOG_ERROR("");
    // todo: notify server stop or error
    return NULL;
}

static void run_python_script(you_parser_struct *you_parser) {
    pthread_t tid;
    pthread_create(&tid, NULL, python_script_thread, (void*)you_parser);
    if (you_parser->complete)
        you_parser->complete(you_parser->parser_port);
}


static void on_connect(http_connection *conn, void *user_data) {
    YOU_LOG_DEBUG("");
    you_parser_struct *y = (you_parser_struct*)user_data;
    
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
    memcpy(host, y->url_buf + y->url.field_data[UF_HOST].off, y->url.field_data[UF_HOST].len);
    
    char path[MAX_URL_LEN] = {0};
    memcpy(path, y->url_buf + y->url.field_data[UF_PATH].off, strlen(y->url_buf) - y->url.field_data[UF_PATH].off);
    
    char header[MAX_REQUEST_HEADER_LEN] = {0};
    snprintf(header, MAX_REQUEST_HEADER_LEN, format, path, host);
    http_connection_send(conn, header, strlen(header));
}

static void on_send(http_connection *conn, void *user_data) {
    YOU_LOG_DEBUG("");
}

static void on_header_complete(http_connection *conn, struct http_header *header, void *user_data) {
    YOU_LOG_DEBUG("");
    you_parser_struct *y = (you_parser_struct*)user_data;
    y->script = (char*)malloc(header->parser.content_length + 1);
    memset(y->script, 0, header->parser.content_length + 1);
}

static void on_body(http_connection *conn, const char *at, size_t length, void *user_data) {
    you_parser_struct *y = (you_parser_struct*)user_data;
    memcpy(y->script + y->pos, at, length);
    y->pos += length;
    YOU_LOG_DEBUG("%zu, %zu", length, y->pos);
}

static void on_message_complete(http_connection *conn, void *user_data) {
    YOU_LOG_DEBUG("");
    you_parser_struct *y = (you_parser_struct*)user_data;
    YOU_LOG_DEBUG("\n%s", y->script);
    free_http_connection(conn);
    run_python_script(y);
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

int start_you_parser_service(uv_loop_t *loop, const char url_buf[MAX_URL_LEN], int parser_port, you_parser_ready_cb complete) {
    you_parser_struct *y = (you_parser_struct*)malloc(sizeof(you_parser_struct));
    memset(y, 0, sizeof(you_parser_struct));
    http_parser_url_init(&y->url);
    
    http_parser_parse_url(url_buf, strlen(url_buf), 0, &y->url);
    if (!(y->url.field_set & (1<<UF_HOST))) {
        free(y);
        YOU_LOG_ERROR("");
        return -1;
    }
    
    if (y->url.port == 0) {
        y->url.port = 80;
    }
    
    y->parser_port = parser_port;
    y->complete = complete;

    char host[MAX_HOST_LEN] = {0};
    memcpy(host, url_buf + y->url.field_data[UF_HOST].off, y->url.field_data[UF_HOST].len);
    memcpy(y->url_buf, url_buf, strlen(url_buf));
    http_connection *conn = create_http_connection(loop, settings, y);
    http_connection_connect(conn, host, y->url.port);
    return 0;
}

const char *parser_service_host() {
    return "127.0.0.1";
    return "youplay.leanapp.cn";
}

unsigned short parser_service_port() {
    return 8888;
    return 80;
}
