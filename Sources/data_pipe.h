//
//  data_pipe.h
//  YouPlayCore
//
//  Created by wujianguo on 16/1/22.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#ifndef data_pipe_h
#define data_pipe_h

#include <stdio.h>

typedef struct data_pipe data_pipe;

typedef struct {
    
}ipipe_callback;

int ipipe_open(data_pipe *pipe);

int ipipe_close(data_pipe *pipe);

#endif /* data_pipe_h */
