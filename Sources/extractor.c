//
//  extractor.c
//  YouPlayCore
//
//  Created by wujianguo on 16/1/1.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#include "extractor.h"
#include "you_parser.h"
#include <stdlib.h>
#include "http_retrieve.h"
#include "json.h"

static QUEUE g_extractor_cache;
static int g_extractor_init = 0;

typedef struct extractor{
    http_retrieve *retrieve;
    extractor_result *result;
    extractor_complete_cb complete_cb;
    void *user_data;
}extractor;

/*
static void parse_response_to_result2(char *buf, size_t len, extractor_result *result) {
    json_value *json_ret = json_parse(buf, len);
    ASSERT(json_ret && json_ret->type == json_object);
    for (int i=0; i<json_ret->u.object.length; ++i) {
        if (strcmp(json_ret->u.object.values[i].name, "data")==0) {
            json_value *data = (json_value*)json_ret->u.object.values[i].value;
            ASSERT(data->type == json_object);
            for (int j=0; j<data->u.object.length; ++j) {
                if (strcmp(data->u.object.values[j].name, "urls")==0) {
                    json_value *urls = (json_value*)data->u.object.values[j].value;
                    ASSERT(urls->type == json_array);
                    for (int u=0; u<urls->u.array.length; ++u) {
                        json_value *url = (json_value*)urls->u.array.values[u];
                        ASSERT(url->type == json_string);
                        char x[MAX_URL_LEN] = {0};
                        memcpy(x, url->u.string.ptr, url->u.string.length);
                        YOU_LOG_DEBUG("%s", x);
                    }
                }
            }
            break;
        }
    }
}
*/
 
static void parse_response_to_result(char *buf, size_t len, extractor_result *result) {
    YOU_LOG_DEBUG("extractor_result:%p", result);
    json_value *json_ret = json_parse(buf, len);
    ASSERT(json_ret && json_ret->type == json_object);
    extract_quality_item *quality = (extract_quality_item*)malloc(sizeof(extract_quality_item));
    memset(quality, 0, sizeof(extract_quality_item));
    QUEUE_INIT(&quality->node);
    QUEUE_INIT(&quality->clips);
    QUEUE_INSERT_TAIL(&result->qualities, &quality->node);
    for (int i=0; i<json_ret->u.object.length; ++i) {
        if (strcmp(json_ret->u.object.values[i].name, "entries")==0) {
            json_value *entries = (json_value*)json_ret->u.object.values[i].value;
            ASSERT(entries->type == json_array);
            for (int j=0; j<entries->u.array.length; ++j) {
                json_value *entry = (json_value*)entries->u.array.values[j];
                ASSERT(entry->type == json_object);
                extract_clips *clip = (extract_clips*)malloc(sizeof(extract_clips));
                memset(clip, 0, sizeof(extract_clips));
                QUEUE_INIT(&clip->node);
                QUEUE_INSERT_TAIL(&quality->clips, &clip->node);
                for (int z=0; z<entry->u.object.length; ++z) {
                    if (strcmp(entry->u.object.values[z].name, "url")==0) {
                        json_value *url_value = (json_value*)entry->u.object.values[z].value;
                        ASSERT(url_value->type == json_string);
                        memcpy(clip->url, url_value->u.string.ptr, url_value->u.string.length);
                        YOU_LOG_DEBUG("%s", clip->url);
                    } else if (strcmp(entry->u.object.values[z].name, "playlist_index")==0) {
                        json_value *index_value = (json_value*)entry->u.object.values[z].value;
                        ASSERT(index_value->type == json_integer);
                        clip->index = (int)index_value->u.integer;
                        ASSERT(clip->index > 0);
                        clip->index -= 1;
                    }
                }
            }
            break;
        }
    }
}

static void on_timer_expire(uv_timer_t *handle) {
    extractor_result *result = (extractor_result*)handle->data;
    QUEUE_REMOVE(&result->node);
    
    QUEUE *q = NULL;
    extract_quality_item *quality = NULL;
    while (!QUEUE_EMPTY(&result->qualities)) {
        q = QUEUE_HEAD(&result->qualities);
        quality = QUEUE_DATA(q, extract_quality_item, node);
        QUEUE *q2 = NULL;
        extract_clips *clip = NULL;
        while (!QUEUE_EMPTY(&quality->clips)) {
            q2 = QUEUE_HEAD(&quality->clips);
            clip = QUEUE_DATA(q2, extract_clips, node);
            QUEUE_REMOVE(q2);
            free(clip);
        }
        QUEUE_REMOVE(q);
        free(quality);
    }
    
    free(result);
    // todo: free nodes
}

static void on_retrieve_error(http_retrieve *retrieve, int err_code, void *user_data) {
    ASSERT(0);
}

static void on_retrieve_complete(http_retrieve *retrieve, char *body, size_t body_len, void *user_data) {
    YOU_LOG_DEBUG("%s", body);
    extractor *ex = (extractor*)user_data;
    parse_response_to_result(body, body_len, ex->result);
    QUEUE_INSERT_TAIL(&g_extractor_cache, &ex->result->node);
    if (ex->complete_cb) {
        ex->complete_cb(ex->result, ex->user_data);
        ex->complete_cb = NULL;
    }
    ex->result->timer_handle.data = ex->result;
    uv_timer_start(&ex->result->timer_handle, on_timer_expire, 20*60*1000, 0);
    free_http_retrieve(retrieve);
}

static void init_extractor() {
    if (!g_extractor_init) {
        g_extractor_init = 1;
        QUEUE_INIT(&g_extractor_cache);
    }
}

extractor_result* find_extract_result(const char url[MAX_URL_LEN]) {
    init_extractor();
    QUEUE *q;
    extractor_result *ret = NULL;
    YOU_LOG_DEBUG("%s", url);
    QUEUE_FOREACH(q, &g_extractor_cache) {
        ret = QUEUE_DATA(q, extractor_result, node);
        YOU_LOG_DEBUG("%p, %s", ret, ret->url);
        if (strcmp(url, ret->url) == 0)
            return ret;
    }
    return NULL;
}

extractor* execute_extractor(uv_loop_t *loop, const char url[MAX_URL_LEN], enum you_media_quality quality, extractor_complete_cb complete_cb, void *user_data) {
    init_extractor();
    extractor *ex = (extractor*)malloc(sizeof(extractor));
    memset(ex, 0, sizeof(extractor));
    ex->result = (extractor_result*)malloc(sizeof(extractor_result));
    memset(ex->result, 0, sizeof(extractor_result));
    QUEUE_INIT(&ex->result->qualities);
    QUEUE_INIT(&ex->result->node);
    strncpy(ex->result->url, url, MAX_URL_LEN);
    ex->complete_cb = complete_cb;
    ex->user_data = user_data;
    uv_timer_init(loop, &ex->result->timer_handle);
    char u_format[] = "http://%s:%u/api/v1/videos2/%s";
    char u[MAX_URL_LEN] = {0};
    snprintf(u, MAX_URL_LEN, u_format, parser_service_host(), parser_service_port(), url);

    http_retrieve *retrieve = create_http_retrieve(loop, u, on_retrieve_complete, on_retrieve_error, ex);
    ex->retrieve = retrieve;
    return ex;
}

void cancel_extractor(extractor *ex) {
    ex->complete_cb = NULL;
}
