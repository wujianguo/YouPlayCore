//
//  idata_cache_for_pipe.h
//  YouPlayCore
//
//  Created by wujianguo on 16/1/22.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#ifndef idata_cache_for_pipe_h
#define idata_cache_for_pipe_h

#include <_types/_uint64_t.h>
typedef struct range range;
typedef struct data_cache data_cache;

struct idata_cache_interface {
    void (*set_clips_num)(data_cache *cache, int num);
    void (*set_filesize)(data_cache *cache, int index, uint64_t filesize);
    void (*write_data)(data_cache *cache, int index, range rg, char *buf);
    void (*undownload_range_queue)(data_cache *cache, int index, range_queue *rgq);
};


#endif /* idata_cache_for_pipe_h */
