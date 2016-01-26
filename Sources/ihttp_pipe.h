//
//  ihttp_pipe.h
//  YouPlayCore
//
//  Created by wujianguo on 16/1/21.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#ifndef ihttp_pipe_h
#define ihttp_pipe_h

#include "idata_pipe.h"
#include "idata_cache_for_pipe.h"
#include "range.h"

typedef struct uv_loop_t uv_loop_t;

idata_pipe* ihttp_pipe_create(uv_loop_t *loop,
                              ipipe_callback callback,
                              struct icache_interface_for_pipe cache_interface,
                              idata_cache *cache,
                              const char *url,
                              int index,
                              uint64_t start_pos,
                              void *user_data);

#endif /* ihttp_pipe_h */
