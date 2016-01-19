//
//  http_retrieve.h
//  YouPlayCore
//
//  Created by 吴建国 on 16/1/18.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#ifndef http_retrieve_h
#define http_retrieve_h

#include "../http-server/http_server.h"

typedef struct http_retrieve http_retrieve;

typedef void (*http_retrieve_complete_cb)(http_retrieve *retrieve, char *body, size_t body_len, void *user_data);
typedef void (*http_retrieve_error_cb)(http_retrieve *retrieve, int err_code, void *user_data);

http_retrieve* create_http_retrieve(uv_loop_t *loop, const char url[MAX_URL_LEN], http_retrieve_complete_cb complete_cb, http_retrieve_error_cb error_cb, void *user_data);

void free_http_retrieve(http_retrieve *retrieve);

#endif /* http_retrieve_h */
