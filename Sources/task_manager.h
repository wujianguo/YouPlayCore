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

typedef struct vod_task vod_task;

vod_task* create_vod_task(uv_loop_t *loop, const char url[MAX_URL_LEN], enum you_media_quality quality, int cur_index, void *user_data);

typedef void (*vod_data_read_cb)(vod_task *task, uint64_t pos, int len, char *buf, void *user_data);

int read_vod_data(vod_task *task, uint64_t pos, int len, vod_data_read_cb read_cb);

int stop_vod_task(vod_task *task);

#endif /* task_manager_h */
