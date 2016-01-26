//
//  idata_cache.c
//  YouPlayCore
//
//  Created by wujianguo on 16/1/22.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#include "idata_cache.h"
#include "icache_struct.h"

int icache_set_user_data(idata_cache *cache, int index, void *user_data) {
    return cache->interface->set_user_data(cache, index, user_data);
}

int icache_read_data(idata_cache *cache, int index, range rg, char *buf) {
    return cache->interface->read_data(cache, index, rg, buf);
}

int icache_write_data(idata_cache *cache, int index, range rg, const char *buf) {
    return cache->interface->write_data(cache, index, rg, buf);
}

int icache_set_clips_num(idata_cache *cache, int num) {
    return cache->interface->set_clips_num(cache, num);
}

int icache_set_filesize(idata_cache *cache, int index, uint64_t filesize) {
    return cache->interface->set_filesize(cache, index, filesize);
}

uint64_t icache_get_filesize(idata_cache *cache, int index) {
    return cache->interface->get_filesize(cache, index);
}

int icache_downloaded_range(idata_cache *cache, int index, range *rg) {
    // todo: change range to range_queue
    return cache->interface->downloaded_range(cache, index, rg);
}

int icache_can_download_more(idata_cache *cache, int index, int *size) {
    return cache->interface->can_download_more(cache, index, size);
}

int icache_get_undownload_range_queue(idata_cache *cache, int index, range_queue *rgq) {
    return cache->interface->get_undownload_range_queue(cache, index, rgq);
}

int icache_destroy(idata_cache *cache) {
    return cache->interface->destroy(cache);
}