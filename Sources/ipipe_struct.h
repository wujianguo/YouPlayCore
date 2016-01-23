//
//  ipipe_struct.h
//  YouPlayCore
//
//  Created by wujianguo on 16/1/21.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#ifndef ipipe_struct_h
#define ipipe_struct_h

#include "idata_pipe.h"

typedef struct {
    int (*destroy)(idata_pipe *pipe);
}ipipe_interface;

#endif /* ipipe_struct_h */
