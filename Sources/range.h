//
//  range.h
//  YouPlayCore
//
//  Created by 吴建国 on 16/1/15.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#ifndef range_h
#define range_h

#include <_types/_uint64_t.h>
#include "../http-server/queue.h"

typedef struct {
    uint64_t pos;
    uint64_t len;
} range;

typedef struct {
    range rg;
    QUEUE node;
} range_queue;

#endif /* range_h */
