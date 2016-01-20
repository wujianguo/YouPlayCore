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
typedef void (*vod_data_read_cb)(vod_task *task, const char *buf, size_t len, void *user_data);
typedef void (*vod_data_header_cb)(vod_task *task, struct http_header *header, void *user_data);

vod_task* create_vod_task(uv_loop_t *loop, const char url[MAX_URL_LEN], enum you_media_quality quality, int cur_index, vod_data_read_cb read_cb, vod_data_header_cb header_cb, void *user_data);

int read_vod_data(vod_task *task, int index, uint64_t pos, int len);

int stop_vod_task(vod_task *task);

#endif /* task_manager_h */
