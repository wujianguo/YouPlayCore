//
//  mem_data_cache.c
//  YouPlayCore
//
//  Created by 吴建国 on 16/1/23.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#include "mem_data_cache.h"
#include "icache_struct.h"
#include "idata_cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../http-server/http_server.h"

#define MEM_CACHE_BUF_SIZE_PER_CLIP (2*1024*1024)

typedef struct {
    int index;
    char *buf;
    range down_rg;
    uint64_t filesize;
    QUEUE node;
    
    range want_rg;
    void *user_data;
}mem_cache_buf_node;

typedef struct {
    idata_cache icache;
    icache_callback callback;
    QUEUE clips;
    int clips_num;
    
    int callbacking;
    int destroy;
}imem_cache;

static imem_cache * cast_from_icache(idata_cache * p) {
    return (imem_cache *)p;
}

static idata_cache * cast_to_icache(imem_cache * p) {
    return (idata_cache *)p;
}

static imem_cache * cast_from_void(void * p) {
    return (imem_cache *)p;
}

static mem_cache_buf_node* find_or_new_clip(imem_cache *mem, int index) {
    int find = 0;
    QUEUE *q;
    mem_cache_buf_node *n = NULL;
    QUEUE_FOREACH(q, &mem->clips) {
        n = QUEUE_DATA(q, mem_cache_buf_node, node);
        if (n->index == index) {
            find = 1;
            break;
        }
    }
    if (!find) {
        n = (mem_cache_buf_node*)malloc(sizeof(mem_cache_buf_node));
        memset(n, 0, sizeof(mem_cache_buf_node));
        QUEUE_INIT(&n->node);
        n->index = index;
        n->buf = (char*)malloc(MEM_CACHE_BUF_SIZE_PER_CLIP);
        memset(n->buf, 0, MEM_CACHE_BUF_SIZE_PER_CLIP);
        n->down_rg.pos = ULLONG_MAX;
        n->down_rg.len = MEM_CACHE_BUF_SIZE_PER_CLIP;
        QUEUE_INSERT_TAIL(&mem->clips, &n->node);
    }
    return n;
}

static int set_user_data(idata_cache *cache, int index, void *user_data) {
    imem_cache *mem = cast_from_icache(cache);
    mem_cache_buf_node *n = find_or_new_clip(mem, index);
    n->user_data = user_data;
    return 0;
}

static int read_data(idata_cache *cache, int index, range rg, char *buf) {
    imem_cache *mem = cast_from_icache(cache);
    QUEUE *q;
    mem_cache_buf_node *n;
    QUEUE_FOREACH(q, &mem->clips) {
        n = QUEUE_DATA(q, mem_cache_buf_node, node);
        if (n->index == index) {
            if (is_range1_contains_range2(n->down_rg, rg)) {
                memcpy(buf, n->buf + rg.pos - n->down_rg.pos, rg.len);
                return 0;
            } else {
                range r = {n->down_rg.pos, MEM_CACHE_BUF_SIZE_PER_CLIP};
                ASSERT(is_range1_contains_range2(r, rg));
                n->want_rg = rg;
                return 1;
            }
        }
    }
    return 0;
}

static int write_data(idata_cache *cache, int index, range rg, const char *buf) {
    imem_cache *mem = cast_from_icache(cache);
    ASSERT(index < mem->clips_num);
    mem_cache_buf_node *n = find_or_new_clip(mem, index);
    ASSERT(n->filesize != 0);
    if (n->down_rg.pos == ULLONG_MAX)
        n->down_rg.pos = rg.pos;
    
    ASSERT(RANGE_END(n->down_rg) == rg.pos);
    ASSERT(n->down_rg.len + rg.len <= MEM_CACHE_BUF_SIZE_PER_CLIP);
    memcpy(n->buf + n->down_rg.pos, buf, rg.len);
    n->down_rg.len += rg.len;
    
    if (is_range1_contains_range2(n->down_rg, n->want_rg))
        mem->callback.on_read(cache, index, n->want_rg, n->buf, n->user_data);

    return 0;
}

static int set_clips_num(idata_cache *cache, int num) {
    imem_cache *mem = cast_from_icache(cache);
    mem->clips_num = num;
    return 0;
}

static int set_filesize(idata_cache *cache, int index, uint64_t filesize) {
    imem_cache *mem = cast_from_icache(cache);
    mem_cache_buf_node *n = find_or_new_clip(mem, index);
    n->filesize = filesize;
    return 0;
}

uint64_t get_filesize(idata_cache *cache, int index) {
    imem_cache *mem = cast_from_icache(cache);
    return find_or_new_clip(mem, index)->filesize;
}

int downloaded_range(idata_cache *cache, int index, range *rg) {
    imem_cache *mem = cast_from_icache(cache);
    range down = find_or_new_clip(mem, index)->down_rg;
    *rg = down;
    return 0;
}// todo: change range to range_queue

int can_download_more(idata_cache *cache, int index, int *size) {
    imem_cache *mem = cast_from_icache(cache);
    range rg = find_or_new_clip(mem, index)->down_rg;
    *size = MEM_CACHE_BUF_SIZE_PER_CLIP - (int)rg.len;
    return 0;
}

static int get_undownload_range_queue(idata_cache *cache, int index, range_queue *rgq) {
    ASSERT(0);
    return 0;
}

static int destroy(idata_cache *cache) {
    imem_cache *mem = cast_from_icache(cache);
    if (mem->callbacking) {
        mem->destroy = 1;
        return 0;
    }
    QUEUE *q;
    mem_cache_buf_node *n;
    while (!QUEUE_EMPTY(&mem->clips)) {
        q = QUEUE_HEAD(&mem->clips);
        n = QUEUE_DATA(q, mem_cache_buf_node, node);
        QUEUE_REMOVE(q);
        free(n->buf);
        free(n);
    }
    free(mem);
    return 0;
}

static icache_interface g_interface = {
    set_user_data,
    read_data,
    write_data,
    set_clips_num,
    set_filesize,
    get_filesize,
    downloaded_range,
    can_download_more,
    get_undownload_range_queue,
    destroy
};


idata_cache* open_mem_cache(icache_callback callback) {
    imem_cache *cache = (imem_cache*)malloc(sizeof(imem_cache));
    memset(cache, 0, sizeof(imem_cache));
    QUEUE_INIT(&cache->clips);
    cache->icache.interface = &g_interface;
    cache->callback = callback;
    return cast_to_icache(cache);
}