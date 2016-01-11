//
//  main.c
//  YouPlayCore
//
//  Created by wujianguo on 16/1/1.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#include "you_play_core.h"
#include <stdlib.h>



#include "cencode.h"
#include "cdecode.h"
static void test_base64() {
    char plain[] = "hello world";
    printf("%s\n", plain);
    base64_encodestate encode = {0};
    base64_init_encodestate(&encode);
    char out[1024] = {0};
    base64_encode_block(plain, strlen(plain), out, &encode);
    base64_encode_blockend(out+strlen(out), &encode);
    printf("%s\n",out);
    base64_decodestate decode = {0};
    base64_init_decodestate(&decode);
    char out2[1024] = {0};
    base64_decode_block(out, strlen(out), out2, &decode);
    printf("%s\n", out2);
}


#include "json.h"
static void test_json() {
    
}

static void on_connect(http_connection *conn, void *user_data) {
//    http://youplay.avosapps.com/api/v1/videos/aHR0cDovL3R2LnNvaHUuY29tLzIwMTYwMTA2L240MzM1NzYxMTEuc2h0bWw=?quality=highVid
    const char format[] = "GET /meta?url=aHR0cDovL3R2LnNvaHUuY29tLzIwMTUxMTAzL240MjUxNTgwMDQuc2h0bWw= HTTP/1.1\r\n"
    "Host: 127.0.0.1\r\n"
    "Connection: keep-alive\r\n"
    "Cache-Control: no-cache\r\n"
    "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
    "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_11_2) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/47.0.2526.106 Safari/537.36\r\n"
    "Accept-Encoding: gzip, deflate\r\n"
    "Accept-Language: zh-CN,zh;q=0.8,en-US;q=0.6,en;q=0.4\r\n"
    "\r\n";
    http_connection_send(conn, format, strlen(format));
}

static void on_send(http_connection *conn, void *user_data) {
    YOU_LOG_DEBUG("");
}

static void on_header_complete(http_connection *conn, struct http_header *header, void *user_data) {
    YOU_LOG_DEBUG("");
}

static void on_body(http_connection *conn, const char *at, size_t length, void *user_data) {
    char *s = (char*)malloc(length+1);
    memset(s, 0, length+1);
    memcpy(s, at, length);
    YOU_LOG_DEBUG("%p:%s", conn, s);
    free(s);
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

static void on_timer_expire(uv_timer_t *handle) {
    http_connection *conn = create_http_connection(handle->loop, settings, (void*)1);
    http_connection_connect(conn, "127.0.0.1", 9812);
}


int main(int argc, char **argv) {
    uv_timer_t timer_handle;
    uv_timer_init(uv_default_loop(), &timer_handle);
    uv_timer_start(&timer_handle, on_timer_expire, 1000, 0);

    start_you_play_service(9812, "http://wujianguo.org/you_parser.py", 9822, uv_default_loop());
    return 0;
}
