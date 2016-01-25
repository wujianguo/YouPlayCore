//
//  extractor.h
//  YouPlayCore
//
//  Created by wujianguo on 16/1/1.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#ifndef extractor_h
#define extractor_h

#include "you_play_core.h"

typedef struct {
    char url[MAX_URL_LEN];
    int index;
    double bytes;
    uint64_t duration;
    QUEUE node;
}extract_clips;

typedef struct {
    enum you_media_quality quality;
    uint64_t total_bytes;
    double total_duration;
    QUEUE clips;
    QUEUE node;
}extract_quality_item;

typedef struct {
    char url[MAX_URL_LEN];
    char name[MAX_NAME_LEN];
    QUEUE qualities;
    QUEUE node;
    uv_timer_t timer_handle;
}extractor_result;

typedef struct extractor extractor;

typedef void (*extractor_complete_cb)(extractor_result *ret, void *user_data);

extractor_result* find_extract_result(const char url[MAX_URL_LEN]);

extractor* execute_extractor(uv_loop_t *loop, const char url[MAX_URL_LEN], enum you_media_quality quality, extractor_complete_cb complete_cb, void *user_data);

void cancel_extractor(extractor *ex);

#endif /* extractor_h */
