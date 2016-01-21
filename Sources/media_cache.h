//
//  media_cache.h
//  YouPlayCore
//
//  Created by 吴建国 on 16/1/21.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#ifndef media_cache_h
#define media_cache_h

#include "../http-server/http_server.h"
#include "range.h"

typedef struct media_cache media_cache;

media_cache* open_media_cache(uv_loop_t *loop, const char full_path[MAX_NAME_LEN], const char url[MAX_URL_LEN], void *user_data);

typedef void (*media_cache_read_cb)(media_cache *media, int index, range rg, char *buf, void *reader, void *user_data);

int media_cache_read_data(media_cache *media, int index, range rg, char *buf, void *reader, media_cache_read_cb read_cb);

void media_cache_set_clips_num(media_cache *media, int num);

void media_cache_set_filesize(media_cache *media, int index, uint64_t filesize);

void media_cache_write_data(media_cache *media, int index, range rg, char *buf);

void media_cache_undownload_range_queue(media_cache *media, int index, range_queue *rgq);

typedef void (*media_cache_close_cb)(media_cache *media, void *user_data);

int close_media_cache(media_cache *media, media_cache_close_cb close_cb);

int cancle_close_media_cache(media_cache *media);

#endif /* media_cache_h */
