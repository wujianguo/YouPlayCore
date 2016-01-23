//
//  icache_struct.h
//  YouPlayCore
//
//  Created by 吴建国 on 16/1/23.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#ifndef icache_struct_h
#define icache_struct_h

#include "idata_cache.h"

typedef struct {
    int (*read_data)(idata_cache *cache, int index, range rg, void *reader);
    int (*write_data)(idata_cache *cache, int index, range rg, char *buf);
    int (*set_clips_num)(idata_cache *cache, int num);
    int (*set_filesize)(idata_cache *cache, int index, uint64_t filesize);
    int (*get_undownload_range_queue)(idata_cache *cache, int index, range_queue *rgq);
    int (*destroy)(idata_cache *cache);
}icache_interface;

#endif /* icache_struct_h */
