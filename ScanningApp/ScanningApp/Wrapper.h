//
//  Wrapper.h
//  ScanningApp
//
//  Created by Plex on 2018/9/11.
//  Copyright © 2018年 Dgene. All rights reserved.
//

#ifndef Wrapper_h
#define Wrapper_h
#import <Foundation/Foundation.h>


@interface Wrapper : NSObject

+ (bool) scan: (NSString * _Nonnull) dataPath : (float * _Nonnull) intrinsics :
(int) width : (int) height : (float) scale : (int) imgCount : (bool) useMarker;
+ (float) getProcess;
@end
#endif /* Wrapper_h */



