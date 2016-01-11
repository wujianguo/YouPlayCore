//
//  gzip.h
//  YouPlayCore
//
//  Created by 吴建国 on 16/1/11.
//  Copyright © 2016年 wujianguo. All rights reserved.
//

#ifndef gzip_h
#define gzip_h

#include <zlib.h>
int httpgzdecompress(Byte *zdata, uLong nzdata, Byte *data, uLong *ndata);

#endif /* gzip_h */
