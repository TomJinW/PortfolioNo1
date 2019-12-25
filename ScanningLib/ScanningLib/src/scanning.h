//
// Created by zhang on 8/9/2018.
//
#import <iostream>
#import <UIKit/UIKit.h>




@interface Scanning : NSObject

+ (bool) scan: (NSString * _Nonnull) dataPath : (float * _Nonnull) intrinsics :
(int) width : (int) height : (float) scale : (int) imgCount : (bool) useMarker;

+ (float) getProcess;

@end



