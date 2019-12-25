//
//  Common.swift
//  ScanningApp
//
//  Created by Plex on 2018/9/14.
//  Copyright © 2018年 Dgene. All rights reserved.
//

import Foundation

final class Common {
    static let shared = Common() //lazy init, and it only runs once
    
    var useMarker = false;
    var minDist:Float = 0.1
    var maxDist:Float = 0.4
    var maxImgCount:Int = 40
    var interval:Float = 0.2
}
