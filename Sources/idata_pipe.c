//
//  idata_pipe.c
//  YouPlayCore
//
//  Created by wujianguo on 16/1/22.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#include "idata_pipe.h"
#include "ipipe_struct.h"

int ipipe_destroy(idata_pipe *pipe) {
    return pipe->interface->destroy(pipe);
}