//
//  media_handler.h
//  YouPlayCore
//
//  Created by wujianguo on 16/1/1.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#ifndef media_handler_h
#define media_handler_h

#include "../http-server/http_server.h"

void media_handler_on_header_complete(http_request *req);

void media_handler_on_body(http_request *req, const char *at, size_t length);

void media_handler_on_message_complete(http_request *req);

void media_handler_on_send(http_request *req);

void media_handler_on_error(http_request *req, int err_code);

#endif /* media_handler_h */
