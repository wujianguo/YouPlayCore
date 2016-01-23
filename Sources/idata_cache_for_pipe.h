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
#include "range.h"

typedef struct idata_cache idata_cache;

struct idata_cache_interface {
//    int (*set_clips_num)(idata_cache *cache, int num);
//    int (*undownload_range_queue)(idata_cache *cache, int index, range_queue *rgq);
    int (*set_filesize)(idata_cache *cache, int index, uint64_t filesize);
    int (*write_data)(idata_cache *cache, int index, range rg, char *buf);
};


#endif /* idata_cache_for_pipe_h */
