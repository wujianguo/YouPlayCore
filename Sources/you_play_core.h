//
//  you_play_core.h
//  YouPlayCore
//
//  Created by wujianguo on 16/1/1.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#ifndef you_play_core_h
#define you_play_core_h

#include "../http-server/http_server.h"

void start_you_play_service_in_new_thread(unsigned short service_port,
                                          const char parser_script_url[1024],
                                          unsigned short parser_port);

void start_you_play_service(unsigned short service_port,
                            const char parser_script_url[1024],
                            unsigned short parser_port,
                            uv_loop_t *loop);


#endif /* you_play_core_h */
