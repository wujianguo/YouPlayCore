//
//  ihttp_pipe.h
//  YouPlayCore
//
//  Created by wujianguo on 16/1/21.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#ifndef ihttp_pipe_h
#define ihttp_pipe_h

#include "idata_pipe.h"
#include "idata_cache_for_pipe.h"
#include "range.h"

idata_pipe* ihttp_pipe_create(ipipe_callback callback,
                              struct idata_cache_interface cache_interface,
                              idata_cache *cache,
                              const char *url,
                              range rg,
                              void *user_data);

#endif /* ihttp_pipe_h */
