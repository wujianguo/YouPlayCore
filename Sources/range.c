//
//  range.c
//  YouPlayCore
//
//  Created by 吴建国 on 16/1/15.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#include "range.h"

int is_range1_contains_range2(range rg1, range rg2) {
    return rg1.pos <= rg2.pos && RANGE_END(rg1) >= RANGE_END(rg2);
}