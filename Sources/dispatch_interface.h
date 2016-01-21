//
//  dispatch_interface.h
//  YouPlayCore
//
//  Created by 吴建国 on 16/1/21.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#ifndef dispatch_interface_h
#define dispatch_interface_h

#include "range.h"

typedef struct media_cache media_cache;

struct dispatch_interface {
    void (*set_clips_num)(media_cache *media, int num);
    void (*set_filesize)(media_cache *media, int index, uint64_t filesize);
    void (*write_data)(media_cache *media, int index, range rg, char *buf);
    void (*undownload_range_queue)(media_cache *media, int index, range_queue *rgq);
};


#endif /* dispatch_interface_h */
