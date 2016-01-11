//
//  meta_handler.h
//  YouPlayCore
//
//  Created by 吴建国 on 16/1/7.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#ifndef meta_handler_h
#define meta_handler_h


#include "../http-server/http_server.h"

void meta_handler_on_header_complete(http_request *req);

void meta_handler_on_body(http_request *req, const char *at, size_t length);

void meta_handler_on_message_complete(http_request *req);

void meta_handler_on_send(http_request *req);

#endif /* meta_handler_h */
