//
//  dispatch.h
//  YouPlayCore
//
//  Created by 吴建国 on 16/1/21.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#ifndef dispatch_h
#define dispatch_h

#include "you_play_core.h"
#include "dispatch_interface.h"

typedef struct dispatch dispatch;

dispatch* create_dispatch(uv_loop_t *loop, const char url[MAX_URL_LEN], enum you_media_quality quality, media_cache *media, struct dispatch_interface interface);

int dispatch_priority(dispatch *dis, int index, uint64_t pos);

int free_dispatch(dispatch* dis);

#endif /* dispatch_h */
