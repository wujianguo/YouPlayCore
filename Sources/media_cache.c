//
//  media_cache.c
//  YouPlayCore
//
//  Created by 吴建国 on 16/1/21.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#include "media_cache.h"

typedef struct media_cache {
    
} media_cache;


int media_cache_read_data(media_cache *media, int index, range rg, char *buf, void *reader, media_cache_read_cb read_cb) {
    return 0;
}

void media_cache_set_clips_num(media_cache *media, int num) {
    return;
}

void media_cache_set_filesize(media_cache *media, int index, uint64_t filesize) {
    
}

void media_cache_write_data(media_cache *media, int index, range rg, char *buf) {
    
}

void media_cache_undownload_range_queue(media_cache *media, int index, range_queue *rgq) {
    
}

int close_media_cache(media_cache *media, media_cache_close_cb close_cb) {
    return 0;
}

int cancle_close_media_cache(media_cache *media) {
    return 0;
}
