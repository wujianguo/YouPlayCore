//
//  task_manager.h
//  YouPlayCore
//
//  Created by 吴建国 on 16/1/19.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#ifndef task_manager_h
#define task_manager_h

#include "you_play_core.h"
#include "range.h"

typedef struct vod_task vod_task;
typedef void (*vod_data_read_cb)(vod_task *task, const char *buf, size_t len, void *user_data);
typedef void (*vod_data_header_cb)(vod_task *task, struct http_header *header, void *user_data);

vod_task* create_vod_task(uv_loop_t *loop, const char url[MAX_URL_LEN], enum you_media_quality quality, int cur_index, vod_data_read_cb read_cb, vod_data_header_cb header_cb, void *user_data);

int read_vod_data(vod_task *task, int index, uint64_t pos, int len);

int stop_vod_task(vod_task *task);



typedef struct media_task media_task;

typedef void (*media_task_meta_info_cb)(media_task *task, void *user);

typedef void (*media_task_read_cb)(media_task *task, range rg, const char *buf, void *user);

media_task* create_media_task(uv_loop_t *loop, const char url[MAX_URL_LEN], const char full_path[MAX_NAME_LEN], enum you_media_quality quality, media_task_meta_info_cb meta_cb, void *user);

int media_task_read_data(media_task *task, int index, range rg, media_task_read_cb read_cb, void *user);

int media_task_get_clips_num(media_task *task, int *num);

int media_task_get_meta_info(media_task *task, int index, uint64_t *filesize, double *duration);

int free_media_task(media_task *task, void *user);

#endif /* task_manager_h */
