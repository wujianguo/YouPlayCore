//
//  media_mem_cache.h
//  YouPlayCore
//
//  Created by 吴建国 on 16/1/21.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#ifndef media_mem_cache_h
#define media_mem_cache_h

#include "../http-server/http_server.h"
#include "range.h"

typedef struct media_mem_cache media_mem_cache;

media_mem_cache* open_media_mem_cache(uv_loop_t *loop, const char full_path[MAX_NAME_LEN], const char url[MAX_URL_LEN], void *user_data);


#endif /* media_mem_cache_h */
