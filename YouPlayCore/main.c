//
//  main.c
//  YouPlayCore
//
//  Created by wujianguo on 16/1/1.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#include "you_play_core.h"

int main(int argc, char **argv) {
    start_you_play_service(9812, "http://wujianguo.org/you_parser.py", 9822);
    return 0;
}
