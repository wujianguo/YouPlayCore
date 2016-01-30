//
//  you_play_core.h
//  YouPlayCore
//
//  Created by wujianguo on 16/1/1.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#ifndef you_play_core_h
#define you_play_core_h


typedef struct uv_loop_t uv_loop_t;

void start_you_play_service_in_new_thread(unsigned short service_port,
                                          const char parser_script_url[1024],
                                          unsigned short parser_port);

void start_you_play_service(unsigned short service_port,
                            const char parser_script_url[1024],
                            unsigned short parser_port,
                            uv_loop_t *loop);



#define YOU_MEDIA_QUALITY_MAP(XX)                \
    XX(320p, "流畅", "320")                       \
    XX(480p, "标清", "480")                       \
    XX(720p, "高清", "720")                       \
    XX(1080p, "超高清", "1080")                   \

enum you_media_quality {
#define XX(code, _, __) you_media_quality_##code,
    YOU_MEDIA_QUALITY_MAP(XX)
#undef XX
    you_media_quality_default
};

const char* media_quality_str(enum you_media_quality quality);

int create_download_task(const char *url, enum you_media_quality quality, int *task_id);

int resume_download_task(int task_id);

int pause_download_task(int task_id);

int delete_download_task(int task_id);

#endif /* you_play_core_h */
