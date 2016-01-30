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


typedef struct media_task media_task;

typedef void (*media_task_meta_info_cb)(media_task *task, void *user);

typedef void (*media_task_notify_can_read_cb)(media_task *task, void *user);

media_task* create_media_task(uv_loop_t *loop, const char *url, const char *full_path, enum you_media_quality quality);

int media_task_set_user_info(media_task *task, int index, media_task_meta_info_cb meta_cb, media_task_notify_can_read_cb read_cb, void *user_data);

int media_task_read_data(media_task *task, int index, range rg, char *buf);

int media_task_get_clips_num(media_task *task, int *num);

int media_task_get_meta_info(media_task *task, int index, uint64_t *filesize, double *duration);

int free_media_task(media_task *task, void *user_data);

#endif /* task_manager_h */
