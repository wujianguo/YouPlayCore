//
//  idata_cache.h
//  YouPlayCore
//
//  Created by wujianguo on 16/1/22.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#ifndef idata_cache_h
#define idata_cache_h

#include "range.h"

#define MEM_CACHE_BUF_SIZE_PER_CLIP (2*1024*1024)

typedef struct idata_cache idata_cache;

typedef struct {
    int (*on_read)(idata_cache *cache, int index, range rg, char *buf, void *user_data);
    int (*on_error)(idata_cache *cache, int err_code, void *user_data);
    int (*on_complete)(idata_cache *cache, void *user_data);
}icache_callback;

int icache_set_user_data(idata_cache *cache, int index, void *user_data);

int icache_read_data(idata_cache *cache, int index, range rg, char *buf);

int icache_write_data(idata_cache *cache, int index, range rg, const char *buf);

int icache_set_clips_num(idata_cache *cache, int num);

int icache_set_filesize(idata_cache *cache, int index, uint64_t filesize);

int icache_get_undownload_range_queue(idata_cache *cache, int index, range_queue *rgq);

int icache_destroy(idata_cache *cache);

#endif /* idata_cache_h */
