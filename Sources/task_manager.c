//
//  task_manager.c
//  YouPlayCore
//
//  Created by 吴建国 on 16/1/19.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#include "task_manager.h"
#include <stdlib.h>

static QUEUE g_task_list = {0};
static int g_task_init = 0;

typedef struct vod_task {
    int reference;
}vod_task;

static vod_task* find_vod_task(const char url[MAX_URL_LEN]) {
    return NULL;
}

vod_task* create_vod_task(uv_loop_t *loop, const char url[MAX_URL_LEN], enum you_media_quality quality, int cur_index, void *user_data) {
    if (!g_task_init) {
        g_task_init = 1;
        QUEUE_INIT(&g_task_list);
    }
    vod_task *task = find_vod_task(url);
    if (!task) {
        task = (vod_task*)malloc(sizeof(vod_task));
        memset(task, 0, sizeof(vod_task));
    }
    task->reference++;
    return task;
}

int read_vod_data(vod_task *task, uint64_t pos, int len, vod_data_read_cb read_cb) {
    return 0;
}

int stop_vod_task(vod_task *task) {
    return 0;
}
