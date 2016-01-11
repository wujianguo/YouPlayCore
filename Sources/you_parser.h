//
//  you_parser.h
//  YouPlayCore
//
//  Created by wujianguo on 16/1/1.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#ifndef you_parser_h
#define you_parser_h

#include "../http-server/http_connection.h"

const char *parser_service_host();

unsigned short parser_service_port();

typedef void (*you_parser_ready_cb)(int port);

int start_you_parser_service(uv_loop_t *loop, const char url_buf[MAX_URL_LEN], int parser_port, you_parser_ready_cb complete);

#endif /* you_parser_h */
