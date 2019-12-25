//
//  Wrapper.m
//  ScanningApp
//
//  Created by Plex on 2018/9/11.
//  Copyright © 2018年 Dgene. All rights reserved.
//

#import <vector>
#import "Wrapper.h"
#import <ScanningLib/scanning.h>


@implementation Wrapper

+ (bool) scan: (NSString * _Nonnull) dataPath : (float * _Nonnull) intrinsics :
(int) width : (int) height : (float) scale : (int) imgCount : (bool) useMarker{
    return [Scanning scan:dataPath :intrinsics :width :height :scale :imgCount : true];
}

+ (float) getProcess{
    return [Scanning getProcess];
}
@end
