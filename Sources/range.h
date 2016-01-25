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

#define RANGE_END(rg) ((rg).pos + (rg).len)

typedef struct {
    range rg;
    QUEUE node;
} range_queue_node;

typedef QUEUE range_queue;

int is_range1_contains_range2(range rg1, range rg2);

#endif /* range_h */
