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
#include "pipe.h"

typedef struct dispatch {
    uv_loop_t *loop;

    
} dispatch;

static void on_extractor_complete(extractor_result *ret, void *user_data) {

}

dispatch* create_dispatch(uv_loop_t *loop, const char url[MAX_URL_LEN], enum you_media_quality quality, media_cache *media, struct dispatch_interface interface) {
    dispatch *dis = (dispatch*)malloc(sizeof(dispatch));
    memset(dis, 0, sizeof(dispatch));
    dis->loop = loop;
    
    extractor_result *extract = find_extract_result(url);
    if (extract)
        on_extractor_complete(extract, dis);
    else
        execute_extractor(loop, url, quality, on_extractor_complete, dis);

    return dis;
}

int dispatch_priority(dispatch *dis, int index, uint64_t pos) {
    return 0;
}

int free_dispatch(dispatch* dis) {
    return 0;
}
