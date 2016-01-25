//
//  idata_pipe.h
//  YouPlayCore
//
//  Created by wujianguo on 16/1/22.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#ifndef idata_pipe_h
#define idata_pipe_h

typedef struct idata_pipe idata_pipe;

typedef struct {
    int (*on_error)(idata_pipe *pipe, int err_code, void *user_data);
    int (*on_complete)(idata_pipe *pipe, void *user_data);
}ipipe_callback;

int ipipe_update_url(idata_pipe *pipe, const char *url);

int ipipe_destroy(idata_pipe *pipe);

#endif /* idata_pipe_h */
