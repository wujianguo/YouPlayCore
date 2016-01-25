//
//  dispatch.c
//  YouPlayCore
//
//  Created by 吴建国 on 16/1/21.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#include <stdlib.h>
#include "dispatch.h"
#include "extractor.h"
#include "idata_pipe.h"

typedef struct {
    int index;
    char url[MAX_URL_LEN];
    idata_pipe *pipe;
    QUEUE node;
} dispatch_clip_node;

typedef struct dispatch {
    uv_loop_t *loop;
    extractor *ex;
    uv_timer_t timer;
    char url[MAX_URL_LEN];
    enum you_media_quality quality;
    
    int priority_index;
    uint64_t priority_pos;
    
    QUEUE clips;
} dispatch;

static void dispatch_immediately(dispatch *dis) {
    
}

static void on_extractor_complete(extractor_result *ret, void *user_data) {
    dispatch *dis = (dispatch*)user_data;
    dis->ex = NULL;
    
    QUEUE *q = NULL;
    QUEUE_FOREACH(q, &ret->qualities) {
        extract_quality_item *quality = QUEUE_DATA(q, extract_quality_item, node);
        QUEUE *q2 = NULL;
        QUEUE_FOREACH(q2, &quality->clips) {
            extract_clips *clip = QUEUE_DATA(q2, extract_clips, node);
            YOU_LOG_DEBUG("%d, %s", clip->index, clip->url);
            
            int find = 0;
            QUEUE *qu = NULL;
            dispatch_clip_node *n = NULL;
            QUEUE_FOREACH(qu, &dis->clips) {
                n = QUEUE_DATA(qu, dispatch_clip_node, node);
                if (n->index == clip->index) {
                    memset(n->url, 0, MAX_URL_LEN);
                    strncpy(n->url, clip->url, MAX_URL_LEN);
                    if (n->pipe) ipipe_update_url(n->pipe, clip->url);
                    find = 1;
                    break;
                }
            }
            if (find) continue;
            
            dispatch_clip_node *node = (dispatch_clip_node*)malloc(sizeof(dispatch_clip_node));
            memset(node, 0, sizeof(dispatch_clip_node));
            node->index = clip->index;
            QUEUE_INIT(&node->node);
            strncpy(node->url, clip->url, MAX_URL_LEN);
            QUEUE_INSERT_TAIL(&dis->clips, &node->node);
        }
    }
    dispatch_immediately(dis);
}

static void on_timer_expire(uv_timer_t *handle) {
    dispatch *dis = CONTAINER_OF(handle, dispatch, timer);
    dispatch_immediately(dis);
}

dispatch* create_dispatch(uv_loop_t *loop, const char url[MAX_URL_LEN], enum you_media_quality quality, data_cache *cache, struct idata_cache_interface interface) {
    dispatch *dis = (dispatch*)malloc(sizeof(dispatch));
    memset(dis, 0, sizeof(dispatch));
    dis->loop = loop;
    dis->quality = quality;
    strncpy(dis->url, url, MAX_URL_LEN);
    QUEUE_INIT(&dis->clips);
    uv_timer_init(loop, &dis->timer);
    uv_timer_start(&dis->timer, on_timer_expire, 2000, 1);

    return dis;
}

int dispatch_priority(dispatch *dis, int index, uint64_t pos) {
    dis->priority_index = index;
    dis->priority_pos = pos;

    if (QUEUE_EMPTY(&dis->clips)) {
        extractor_result *extract = find_extract_result(dis->url);
        if (extract)
            on_extractor_complete(extract, dis);
        else
            dis->ex = execute_extractor(dis->loop, dis->url, dis->quality, on_extractor_complete, dis);
    } else {
        dispatch_immediately(dis);
    }
    return 0;
}

int free_dispatch(dispatch* dis) {
    return 0;
}
