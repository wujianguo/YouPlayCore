//
//  task_manager.c
//  YouPlayCore
//
//  Created by 吴建国 on 16/1/19.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#include "task_manager.h"
#include "../http-server/http_server.h"
#include <stdlib.h>
#include "extractor.h"
#include "idata_cache.h"
#include "dispatch.h"
#include "idata_cache_for_dispatch.h"
#include "mem_data_cache.h"

static QUEUE g_media_task_list = {0};

static int g_media_task_init = 0;

typedef struct {
    void *user_data;
    media_task_meta_info_cb meta_cb;
    media_task_notify_can_read_cb read_cb;
    int index;
    QUEUE node;
} media_sub_task;

typedef struct media_task {
    int reference;
    char url[MAX_URL_LEN];
    enum you_media_quality quality;
    uv_loop_t *loop;

    int callbacking;
    int destroy;
    QUEUE node;
    
    QUEUE subs;
    
    dispatch *dis;
    idata_cache *cache;
} media_task;

int on_read(idata_cache *cache, int index, range rg, char *buf, void *user_data) {
    return 0;
}

int on_error(idata_cache *cache, int err_code, void *user_data) {
    return 0;
}

int on_complete(idata_cache *cache, void *user_data) {
    return 0;
}


static media_task* find_media_task(const char url[MAX_URL_LEN], enum you_media_quality quality) {
    QUEUE *q;
    media_task *task = NULL;
    QUEUE_FOREACH(q, &g_media_task_list) {
        task = QUEUE_DATA(q, media_task, node);
        if (task->quality==quality && strcmp(task->url, url)==0)
            return task;
    }
    return NULL;
}

media_task* create_media_task(uv_loop_t *loop, const char *url, const char *full_path, enum you_media_quality quality) {
    if (!g_media_task_init) {
        g_media_task_init = 1;
        QUEUE_INIT(&g_media_task_list);
    }
    media_task *task = find_media_task(url, quality);
    if (!task) {
        task = (media_task*)malloc(sizeof(media_task));
        memset(task, 0, sizeof(media_task));
        QUEUE_INIT(&task->subs);
        QUEUE_INIT(&task->node);
        task->loop = loop;
        strncpy(task->url, url, MAX_URL_LEN);
        task->quality = quality;
    
        icache_callback callback = {on_read, on_error, on_complete}; // todo: fill
        task->cache = open_mem_cache(callback);
        struct icache_interface_for_dispatch interface = {
            icache_set_clips_num,
            icache_set_filesize,
            icache_get_filesize,
            icache_write_data,
            icache_downloaded_range,
            icache_can_download_more
        };
        task->dis = create_dispatch(loop, url, quality, task->cache, interface);

        QUEUE_INSERT_TAIL(&g_media_task_list, &task->node);
    }
    task->reference++;
    return task;
}

int media_task_set_user_info(media_task *task, int index, media_task_meta_info_cb meta_cb, media_task_notify_can_read_cb read_cb, void *user_data) {
    ASSERT(g_media_task_init);
    QUEUE *q;
    media_sub_task *user = NULL;
    QUEUE_FOREACH(q, &task->subs) {
        user = QUEUE_DATA(q, media_sub_task, node);
        if (user->index == index) {
            user->meta_cb = meta_cb;
            user->read_cb = read_cb;
            user->user_data = user_data;
            return 0;
        }
    }
    user = (media_sub_task*)malloc(sizeof(media_sub_task));
    memset(user, 0, sizeof(media_sub_task));
    user->index = index;
    user->meta_cb = meta_cb;
    user->read_cb = read_cb;
    user->user_data = user_data;
    QUEUE_INIT(&user->node);
    QUEUE_INSERT_TAIL(&task->subs, &user->node);
    return 0;
}

int media_task_read_data(media_task *task, int index, range rg, char *buf) {
    ASSERT(g_media_task_init);
    if (icache_read_data(task->cache, index, rg, buf)==0) {
        dispatch_priority(task->dis, index, RANGE_END(rg));
    } else {
        dispatch_priority(task->dis, index, rg.pos);
    }
    return 0;
}

int media_task_get_clips_num(media_task *task, int *num) {
    ASSERT(g_media_task_init);
    
    return 0;
}

int media_task_get_meta_info(media_task *task, int index, uint64_t *filesize, double *duration) {
    ASSERT(g_media_task_init);
    *filesize = icache_get_filesize(task->cache, index);
    return 0;
}

int free_media_task(media_task *task, void *user_data) {
    ASSERT(g_media_task_init);
    QUEUE *q;
    media_sub_task *user = NULL;
    QUEUE_FOREACH(q, &task->subs) {
        user = QUEUE_DATA(q, media_sub_task, node);
        if (user->user_data == user_data) {
            
        }
    }
    return 0;
}
