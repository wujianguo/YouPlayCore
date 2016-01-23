//
//  mem_data_cache.h
//  YouPlayCore
//
//  Created by 吴建国 on 16/1/23.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#ifndef mem_data_cache_h
#define mem_data_cache_h

#include "idata_cache.h"

typedef struct uv_loop_t uv_loop_t;

idata_cache* open_mem_cache(uv_loop_t *loop, const char *full_path, const char *url, void *user_data);


#endif /* mem_data_cache_h */
